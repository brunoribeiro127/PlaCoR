#include "cor/services/resource_manager.hpp"

#include "cor/services/controller.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/group.hpp"
#include "cor/system/macros.hpp"
#include "cor/services/resource_ptr.hpp"

#include <typeinfo>
#include <cassert>
#include <stdexcept>

// to remove
#include <iostream>
#include <cassert>

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
}

void ResourceManager::CleanInitialContext()
{
    //std::cout << "<" << _ctrl->GetName() << "> CLEAN INITIAL CONTEXT" << std::endl;

    //_ctrl->LeaveResourceGroup(cor::MetaDomain);
/*
    {
        std::unique_lock<std::mutex> lk(_mtx);

        std::cout << "--->>> " << std::to_string(_cst_objs.size()) << "\t" << std::to_string(_predecessors.size()) << std::endl;
    }
*/
}

void ResourceManager::CreateMetaDomain(std::string const& ctrl)
{
    // create meta-domain resource
    auto rsc = new Domain(cor::MetaDomain, "");

    // join group to control consistency of the resource
    JoinResourceGroup(cor::MetaDomain);

    // create consistency object
    auto cobj = new ConsistencyObject(this, cor::MetaDomain, true, true,
        [] (Resource *rsc, Resource *new_rsc) {
            rsc->~Resource();
            rsc = new (rsc) Domain(std::move(*static_cast<Domain*>(new_rsc)));
        }, ctrl);

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
/*
    std::unique_lock<std::mutex> lk(_mtx);

    auto ctx = _predecessors.at(idp);

    auto ctx_cst_obj = _cst_objs.find(ctx);
    if (ctx != cor::MetaDomain && ctx_cst_obj != _cst_objs.end()) {

        auto ctx_rsc = ctx_cst_obj->second->GetResource();

        lk.unlock();

        auto dorg = dynamic_cast<DynamicOrganizer*>(ctx_rsc);
        if (dorg != nullptr)
            dorg->Leave(idp);

        lk.lock();
    }

    auto rsc_cst_obj = _cst_objs.at(idp);

    _cst_objs.erase(idp);
    _predecessors.erase(idp);

    delete rsc_cst_obj;
*/
    //std::cout << "<" << _ctrl->GetName() << "> DEALLOCATE RESOURCE " << std::to_string(idp) << std::endl;
}

void ResourceManager::ReleaseReplica(idp_t idp, bool self)
{
    GetConsistencyObject(idp)->ReleaseReplica(self);
}

void ResourceManager::CheckReplica(idp_t idp, std::string requester)
{
    GetConsistencyObject(idp)->CheckReplica(requester);
}

void ResourceManager::SendReplica(idp_t idp, Resource *rsc, std::string const& requester)
{
    _ctrl->SendReplica(idp, rsc, requester);
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

void ResourceManager::Invalidate(idp_t idp, std::string requester)
{
    GetConsistencyObject(idp)->Invalidate(requester);
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

void ResourceManager::CreateCollectiveGroup(std::string const& comm, unsigned int total_members)
{
    {
        std::unique_lock<std::mutex> lk(_mtx);
        _cg_sync.emplace(std::piecewise_construct, std::forward_as_tuple(comm), std::forward_as_tuple());
        _cg_vars.emplace(comm, std::make_pair(0, total_members));
        
        if (_cg_cv.find(comm) == _cg_cv.end())
            _cg_cv.emplace(std::piecewise_construct, std::forward_as_tuple(comm), std::forward_as_tuple());
        else
            _cg_cv.at(comm).notify_all();
    }

    _ctrl->SendCollectiveGroupCreate(comm);

    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (std::get<0>(_cg_vars.at(comm)) != std::get<1>(_cg_vars.at(comm)))
            _cg_cv.at(comm).wait(lk);
        std::get<0>(_cg_vars.at(comm)) = 0;
    }
}

void ResourceManager::HandleCreateCollectiveGroup(std::string const& comm, bool self)
{
    std::unique_lock<std::mutex> lk(_mtx);
    if (_cg_vars.find(comm) == _cg_vars.end()) {

        if (_cg_cv.find(comm) == _cg_cv.end())
            _cg_cv.emplace(std::piecewise_construct, std::forward_as_tuple(comm), std::forward_as_tuple());

        _cg_cv.at(comm).wait(lk);
    }

    std::get<0>(_cg_vars.at(comm)) += 1;

    if (std::get<0>(_cg_vars.at(comm)) == 1)
        std::get<0>(_cg_sync.at(comm)).set_value(self);

    if (std::get<0>(_cg_vars.at(comm)) == std::get<1>(_cg_vars.at(comm)))
        _cg_cv.at(comm).notify_one();
}

void ResourceManager::SynchronizeCollectiveGroup(std::string const& comm)
{
    _ctrl->SendCollectiveGroupSync(comm);
    
    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (std::get<0>(_cg_vars.at(comm)) != std::get<1>(_cg_vars.at(comm)))
            _cg_cv.at(comm).wait(lk);
        std::get<0>(_cg_vars.at(comm)) = 0;
    }
}

void ResourceManager::HandleSynchronizeCollectiveGroup(std::string const& comm)
{
    std::unique_lock<std::mutex> lk(_mtx);

    std::get<0>(_cg_vars.at(comm)) += 1;

    if (std::get<0>(_cg_vars.at(comm)) == std::get<1>(_cg_vars.at(comm)))
        _cg_cv.at(comm).notify_one();
}

void ResourceManager::SendCollectiveGroupIdp(std::string const& comm, idp_t idp)
{
    _ctrl->SendCollectiveGroupIdp(comm, idp);
}

bool ResourceManager::GetCollectiveGroupFirst(std::string const& comm)
{
    std::future<bool> future;

    {
        std::unique_lock<std::mutex> lk(_mtx);
        future = std::get<0>(_cg_sync.at(comm)).get_future();
    }

    return future.get();
}

idp_t ResourceManager::GetCollectiveGroupIdp(std::string const& comm)
{
    std::future<idp_t> future;

    {
        std::unique_lock<std::mutex> lk(_mtx);
        future = std::get<1>(_cg_sync.at(comm)).get_future();
    }

    return future.get();
}

void ResourceManager::SetCollectiveGroupIdp(std::string const& comm, idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);
    std::get<1>(_cg_sync.at(comm)).set_value(idp);
}

idp_t ResourceManager::GenerateIdp()
{
    return _ctrl->GenerateIdp();
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