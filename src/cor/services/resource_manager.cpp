#include "cor/services/resource_manager.hpp"

#include "cor/resources/domain.hpp"
#include "cor/system/macros.hpp"

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
    _ref_cntrs{},
    _cst_objs{},
    _sync_replicas{},
    _predecessors{},
    _alias{},
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

        // initialize resource reference counter
        _ref_cntrs.emplace(cor::MetaDomain, 0);
    }

    // self join of meta-domain resource
    rsc->Attach(cor::MetaDomain, "MetaDomain");

    SendResourceAllocationInfo(cor::MetaDomain);
}

void ResourceManager::InsertResourceReplica(idp_t idp, Resource *rsc, std::string const& ctrl)
{
    // if the resource has a mailbox, create a mailbox in mailer for the resource
    if (dynamic_cast<Mailbox*>(rsc) != nullptr)
        _mlr->CreateMailbox(idp);

    {
        // lock to access resource manager variables
        std::lock_guard<std::mutex> lk(_mtx);

        // update replica validity
        _cst_objs.at(idp)->AcquireReplica(rsc, ctrl);

        // initialize resource reference counter
        _ref_cntrs.emplace(idp, 0);

        // notify waiting threads that the replica has been created
        _sync_replicas[idp].notify_all();
    }
}

Resource *ResourceManager::GetResource(idp_t idp)
{
    auto orig_id = ResolveIdp(idp);
    auto cobj = GetConsistencyObject(orig_id);
    return cobj->GetResource();
}

ConsistencyObject *ResourceManager::GetConsistencyObject(idp_t idp)
{
    // lock to access resource manager variables
    std::lock_guard<std::mutex> lk(_mtx);

    // return consistency object of resource idp
    return _cst_objs.at(idp);
}

idp_t ResourceManager::GetDomainIdp(idp_t idp)
{
    // lock to access resource manager variables
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
    // lock to access resource manager variables
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

void ResourceManager::HandleJoinResourceGroup(idp_t idp, std::string requester)
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

void ResourceManager::Update(idp_t idp, Resource *rsc, std::string replier)
{
    GetConsistencyObject(idp)->Update(rsc, replier);
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

void ResourceManager::TokenAck(idp_t idp, std::string const& requester)
{
    GetConsistencyObject(idp)->TokenAck(requester);
}

void ResourceManager::IncrementResourceReferenceCounter(idp_t idp)
{
    // lock to access resource manager variables
    std::lock_guard<std::mutex> lk(_mtx);

    // increment resource reference counter
    ++_ref_cntrs[idp];
    //std::cout << "REF COUNT " << idp << " -> " << _ref_cntrs[idp] << "\n";
}

void ResourceManager::DecrementResourceReferenceCounter(idp_t idp)
{
    // lock to access resource manager variables
    std::lock_guard<std::mutex> lk(_mtx);

    // decrement resource reference counter
    --_ref_cntrs[idp];
    //std::cout << "REF COUNT " << idp << " -> " << _ref_cntrs[idp] << "\n";
}

void ResourceManager::JoinResourceGroup(idp_t idp)
{
    _ctrl->JoinResourceGroup(idp);
}

void ResourceManager::InsertAlias(idp_t alias, idp_t idp)
{
    // lock to access resource manager variables
    std::unique_lock<std::mutex> lk(_mtx); //shared_lock
    _alias.emplace(alias, idp);
}

idp_t ResourceManager::ResolveIdp(idp_t idp)
{
    // lock to access resource manager variables
    std::unique_lock<std::mutex> lk(_mtx); //shared_lock

    auto it = _alias.find(idp);
    if (it == _alias.end())
        return idp;
    else
        return it->second;
}

}
