#include "cor/services/resource_manager.hpp"

#include "cor/services/controller.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/group.hpp"
#include "cor/system/macros.hpp"
#include "cor/services/resource_ptr.hpp"

#include <typeinfo>
#include <cassert>
#include <stdexcept>
#include <algorithm>

// to remove
#include <iostream>
#include <cassert>
#include <thread>

namespace cor {

ResourceManager::ResourceManager(Controller *ctrl, Mailer *mlr, bool first) :
    _ctrl{ctrl},
    _mlr{mlr},
    _is_main_mgr{first},
    _cst_objs{},
    _sync_replicas{},
    _predecessors{},
    _mtx{}
{}

ResourceManager::~ResourceManager() = default;

void ResourceManager::CreateInitialContext(std::string const& ctrl)
{
    if (_is_main_mgr)
        CreateMetaDomain(ctrl);
    else
        CreateReplica<Domain>(cor::MetaDomain, ctrl);

    GetConsistencyObject(cor::MetaDomain)->IncrementLocalReferenceCounter();
}

void ResourceManager::CleanInitialContext()
{
    //std::cout << "BEGIN CleanInitialContext" << std::endl;
    GetConsistencyObject(cor::MetaDomain)->DecrementLocalReferenceCounter();
    //std::cout << "END CleanInitialContext" << std::endl;
}

void ResourceManager::CreateMetaDomain(std::string const& ctrl)
{
    // create meta-domain resource
    auto rsc = new Domain(cor::MetaDomain, "");

    // join group to control consistency of the resource
    JoinResourceGroup(cor::MetaDomain);

    // create consistency object
    auto cobj = new ConsistencyObject{this, cor::MetaDomain, true,
        [] (Resource *rsc, Resource *new_rsc) {
            rsc->~Resource();
            rsc = new (rsc) Domain(std::move(*static_cast<Domain*>(new_rsc)));
        }, ctrl};

    // assign resource to consistency object
    cobj->SetResource(rsc);

    {
        // lock to access resource manager variables
        std::lock_guard<std::mutex> lk(_mtx);

        // insert local consistency object
        _cst_objs.emplace(cor::MetaDomain, cobj);

        // insert relationship of ancestry
        _predecessors.emplace(cor::MetaDomain, cor::MetaDomain);
    }

    // self join of meta-domain resource
    rsc->Join(cor::MetaDomain, "MetaDomain");

    SendResourceAllocationInfo(cor::MetaDomain);
}

void ResourceManager::InsertResourceReplica(idp_t idp, Resource *rsc)
{
    // if the resource has a mailbox, create a mailbox in mailer for the resource
    if (dynamic_cast<Mailbox*>(rsc) != nullptr || dynamic_cast<StaticOrganizer*>(rsc))
        _mlr->CreateMailbox(idp);

    {
        // lock to access resource manager variables
        std::lock_guard<std::mutex> lk(_mtx);

        // update replica validity
        _cst_objs.at(idp)->AcquireReplica(rsc);

        // notify waiting threads that the replica has been created
        _sync_replicas[idp].notify_all();
    }
}

bool ResourceManager::ContainsResource(idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);
    return (_cst_objs.find(idp) != _cst_objs.end());
}

Resource *ResourceManager::GetResource(idp_t idp)
{
    auto cobj = GetConsistencyObject(idp);
    return cobj->GetResource();
}

ConsistencyObject *ResourceManager::GetConsistencyObject(idp_t idp)
{
    std::lock_guard<std::mutex> lk(_mtx);

    // return consistency object of resource idp
    return _cst_objs.at(idp);
}

unsigned int ResourceManager::GetTotalDomains()
{
    auto rsc = GetResource(cor::MetaDomain);
    auto meta_domain = dynamic_cast<Domain*>(rsc);
    auto total_members = meta_domain->GetTotalMembers();
    return total_members - 1;
}

idp_t ResourceManager::GetDomainIdp(idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx); // shared_lock

    auto ret = idp;
    auto ctx = _predecessors.at(ret);

    while (ctx != cor::MetaDomain) {
        ret = ctx;
        ctx = _predecessors.at(ret);
    }

    return ret;
}

idp_t ResourceManager::GetPredecessorIdp(idp_t idp)
{
    // lock to access resource manager variables
    std::unique_lock<std::mutex> lk(_mtx); // shared_lock
    return _predecessors.at(idp);
}

void ResourceManager::EraseResource(idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);

    auto rsc_cst_obj = _cst_objs.at(idp);
    _cst_objs.erase(idp);
    delete rsc_cst_obj;

    _sync_free[idp].notify_all();
}

void ResourceManager::SendResourceAllocationInfo(idp_t idp)
{
    _ctrl->SendResourceAllocationInfo(idp);
}

void ResourceManager::FindGlobalResource(idp_t idp)
{
    _ctrl->SendFindResourceRequest(idp);
}

void ResourceManager::HandleFindGlobalResource(idp_t idp, std::string ctrl)
{
    std::unique_lock<std::mutex> lk(_mtx); // shared_lock

    // if resource exists, then send find resource reply
    if (_predecessors.find(idp) != _predecessors.end())
        SendGlobalResourceFound(idp, ctrl);
}

void ResourceManager::SendGlobalResourceFound(idp_t idp, std::string const& ctrl)
{
    _ctrl->SendFindResourceReply(idp, ctrl);
}

void ResourceManager::GlobalResourceFound(idp_t idp)
{
    // lock to access resource manager variables
    std::unique_lock<std::mutex> lk(_mtx);

    if (_sync_gfind.find(idp) != _sync_gfind.end())
        _sync_gfind[idp].notify_all();
}

void ResourceManager::DeallocateResource(idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);

    std::cout << "BEGIN <" << _ctrl->GetName() << "> DEALLOCATE RESOURCE " << std::to_string(idp) << std::endl;

    auto rsc_cst_obj = _cst_objs.at(idp);
    auto rsc = rsc_cst_obj->GetResource();

    if (idp == cor::MetaDomain)
        _predecessors.erase(idp);

    if (dynamic_cast<DynamicOrganizer*>(rsc) != nullptr /* || dynamic_cast<StaticOrganizer*>(rsc) != nullptr */) {

        // get all keys with same value from predecessors
        std::vector<idp_t> rscs;
        std::for_each(_predecessors.begin(), _predecessors.end(),
            [&] (const auto& pair) -> void {
                if (pair.second == idp)
                    rscs.push_back(pair.first);
            });

        for (const auto& rsc_idp: rscs) {

            std::cout << "--->>> " << std::to_string(rsc_idp) << std::endl;

            if (_cst_objs.find(rsc_idp) != _cst_objs.end()) {

                std::cout << "WAIT FOR DEALLOCATE OF RESOURCE <" << rsc_idp << ">" << std::endl;
                _sync_free[rsc_idp].wait(lk);
                std::cout << "WAIT SUCCEDED FOR DEALLOCATE OF RESOURCE <" << rsc_idp << ">" << std::endl;

                _predecessors.erase(rsc_idp);
                _sync_free.erase(rsc_idp);
                
            } else {
                _predecessors.erase(rsc_idp);
            }
        }
    }

    if (idp != cor::MetaDomain) {
        auto ctx_idp = _predecessors.at(idp);
        auto ctx = _cst_objs.at(ctx_idp)->GetResource();

        lk.unlock();

        auto dorg = dynamic_cast<DynamicOrganizer*>(ctx);
        if (dorg != nullptr) {
            dorg->Leave(idp);
        }
    /*
        else {

            auto sorg = dynamic_cast<StaticOrganizer*>(ctx);
            if (sorg != nullptr)
                sorg->Leave(idp);
        }
    */

        lk.lock();
    }

    std::cout << "END <" << _ctrl->GetName() << "> DEALLOCATE RESOURCE " << std::to_string(idp) << std::endl;
}

void ResourceManager::CheckReplica(idp_t idp, unsigned int size, std::string requester)
{
    GetConsistencyObject(idp)->CheckReplica(size, requester);
}

void ResourceManager::SendReplica(idp_t idp, Resource *rsc, std::string const& requester)
{
    _ctrl->SendReplica(idp, rsc, requester);
}

void ResourceManager::RequestReleaseReplica(idp_t idp)
{
    _ctrl->SendReleaseReplicaRequest(idp);
}

void ResourceManager::ReleaseReplica(idp_t idp, std::string requester)
{
    GetConsistencyObject(idp)->ReleaseReplica(requester);
}

void ResourceManager::RequestUpdate(idp_t idp)
{
    _ctrl->SendUpdateRequest(idp);
}

void ResourceManager::CheckUpdate(idp_t idp, std::string requester)
{
    GetConsistencyObject(idp)->CheckUpdate(requester);
}

void ResourceManager::ReplyUpdate(idp_t idp, Resource *rsc, std::string const& requester)
{
    _ctrl->SendUpdateReply(idp, rsc, requester);
}

void ResourceManager::Update(idp_t idp, Resource *rsc)
{
    GetConsistencyObject(idp)->Update(rsc);
}

void ResourceManager::RequestInvalidate(idp_t idp)
{
    _ctrl->SendInvalidateRequest(idp);
}

void ResourceManager::Invalidate(idp_t idp)
{
    GetConsistencyObject(idp)->Invalidate();
}

void ResourceManager::RequestTokenUpdate(idp_t idp)
{
    _ctrl->SendTokenUpdateRequest(idp);
}

void ResourceManager::CheckTokenUpdate(idp_t idp, std::string requester)
{
    GetConsistencyObject(idp)->CheckTokenUpdate(requester);
}

void ResourceManager::ReplyTokenUpdate(idp_t idp, Resource *rsc, std::string const& requester)
{
    _ctrl->SendTokenUpdateReply(idp, rsc, requester);
}

void ResourceManager::AcquireTokenUpdate(idp_t idp, Resource *rsc, std::string replier)
{
    GetConsistencyObject(idp)->AcquireTokenUpdate(rsc, replier);
}

void ResourceManager::SendTokenAck(idp_t idp, std::string const& replier)
{
    _ctrl->SendTokenAck(idp, replier);
}

void ResourceManager::TokenAck(idp_t idp)
{
    GetConsistencyObject(idp)->TokenAck();
}

void ResourceManager::JoinResourceGroup(idp_t idp)
{
    _ctrl->JoinResourceGroup(idp);
}

void ResourceManager::LeaveResourceGroup(idp_t idp)
{
    _ctrl->LeaveResourceGroup(idp);
}

idp_t ResourceManager::GenerateIdp()
{
    return _ctrl->GenerateIdp();
}

void ResourceManager::CreateStaticGroup(idp_t comm, unsigned int total_members)
{
    {
        std::unique_lock<std::mutex> lk(_mtx);
        _sg_vars.emplace(comm, std::make_pair(0, total_members));
        _sg_cv[comm].notify_all();
    }

    _ctrl->SendStaticGroupCreate(comm);

    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (_sg_vars[comm].first != _sg_vars[comm].second)
            _sg_cv[comm].wait(lk);
        _sg_vars[comm].first = 0;
    }
}

void ResourceManager::HandleCreateStaticGroup(idp_t comm)
{
    std::unique_lock<std::mutex> lk(_mtx);

    if (_sg_vars.find(comm) == _sg_vars.end())
        _sg_cv[comm].wait(lk);

    _sg_vars[comm].first += 1;

    if (_sg_vars[comm].first == _sg_vars[comm].second)
        _sg_cv[comm].notify_all();
}

void ResourceManager::SynchronizeStaticGroup(idp_t comm)
{
    _ctrl->SendStaticGroupSynchronize(comm);

    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (_sg_vars[comm].first != _sg_vars[comm].second)
            _sg_cv[comm].wait(lk);
        _sg_vars[comm].first = 0;
    }
}

void ResourceManager::HandleSynchronizeStaticGroup(idp_t comm)
{
    std::unique_lock<std::mutex> lk(_mtx);

    _sg_vars[comm].first += 1;

    if (_sg_vars[comm].first == _sg_vars[comm].second)
        _sg_cv[comm].notify_all();
}

/*
void ResourceManager::DummyInsertWorldContext(idp_t idp, std::string const& name, Resource *rsc, std::string const& ctrl)
{
    if (dynamic_cast<Domain*>(rsc) != nullptr) {
        if (idp == cor::MasterDomain) {
            std::cout << "ANTES" << std::endl;
            Create<cor::Group>(cor::ResourceWorld, idp, "ResourceWorld", true, ctrl, "");
            std::cout << "DEPOIS" << std::endl;
        }
        else
            CreateReference<cor::Group>(cor::ResourceWorld, idp, "ResourceWorld", ctrl);
    }

    auto rsc_world = GetLocalResource<cor::Group>(cor::ResourceWorld);
    rsc_world->Join(idp, name);
}
*/

}