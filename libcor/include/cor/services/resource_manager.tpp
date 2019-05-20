#ifdef COR_RESOURCE_MANAGER_HPP

#include "cor/elements/dynamic_organizer.hpp"
#include "cor/elements/static_organizer.hpp"
#include "cor/services/consistency_object.hpp"

namespace cor {

template <typename T, typename ... Args>
ResourcePtr<T> ResourceManager::CreateLocal(idp_t ctx, std::string const& name, std::string const& ctrl, Args&& ... args)
{
    auto idp = GenerateIdp();
    auto rsc = new T(idp, std::forward<Args>(args)...);
    AllocateResource<T>(idp, ctx, name, rsc, ctrl);
    return GetLocalResource<T>(idp);
}

template <typename T, typename ... Args>
idp_t ResourceManager::Create(idp_t ctx, std::string const& name, std::string const& ctrl, Args&& ... args)
{
    auto idp = GenerateIdp();
    auto rsc = new T(idp, std::forward<Args>(args)...);
    AllocateResource<T>(idp, ctx, name, rsc, ctrl);
    return idp;
}

template <typename T>
void ResourceManager::AllocateResource(idp_t idp, idp_t ctx, std::string const& name, Resource *rsc, std::string const& ctrl)
{
    // get ctx resource
    auto ctx_rsc = GetResource(ctx);

    // attach resource to the context
    auto dorg = dynamic_cast<DynamicOrganizer*>(ctx_rsc);
    if (dorg != nullptr) {
        dorg->Join(idp, name);
    } else {
        auto sorg = dynamic_cast<StaticOrganizer*>(ctx_rsc);
        if (sorg != nullptr)
            sorg->Join(idp, name);
        else
            throw std::runtime_error("Resource " + std::to_string(ctx) + " does not have an organizer!");
    }

    // join group to control consistency of the original resource
    JoinResourceGroup(idp);

    // create consistency object
    auto cobj = new ConsistencyObject{this, idp, true,
            [] (cor::Resource *rsc, cor::Resource *new_rsc) {
                rsc->~Resource();
                rsc = new (rsc) T(std::move(*static_cast<T*>(new_rsc)));
            }, ctrl};

    cobj->SetResource(rsc);

    // if the resource has a mailbox, create a mailbox in mailer for the resource
    if (dynamic_cast<Mailbox*>(rsc) != nullptr || dynamic_cast<StaticOrganizer*>(rsc))
        _mlr->CreateMailbox(idp);

    {
        // lock to access resource manager variables
        std::lock_guard<std::mutex> lk(_mtx);

        // insert local consistency object
        _cst_objs.emplace(idp, cobj);

        // insert relationship of ancestry
        _predecessors.emplace(idp, ctx);
    }

    //DummyInsertWorldContext(idp, name, rsc, ctrl);
}

template <typename T>
ResourcePtr<T> ResourceManager::CreateReference(idp_t idp, idp_t ctx, std::string const& name, std::string const& ctrl)
{
    // create replica
    CreateReplica<T>(idp, ctrl);

    // get ctx resource
    auto ctx_rsc = GetResource(ctx);

    // attach resource to the context
    auto org = dynamic_cast<DynamicOrganizer*>(ctx_rsc);
    if (org != nullptr)
        org->Join(idp, name);
    else
        throw std::runtime_error("Resource " + std::to_string(ctx) + " does not have an organizer!");

    {
        // lock to access resource manager variables
        std::lock_guard<std::mutex> lk(_mtx);

        // insert relationship of ancestry
        _predecessors.emplace(idp, ctx);
    }

    return GetLocalResource<T>(idp);
}

template <typename T, typename ... Args>
ResourcePtr<T> ResourceManager::CreateCollective(idm_t rank, idp_t comm, idp_t ctx, std::string const& name, std::string const& ctrl, Args&& ... args)
{
    ResourcePtr<T> rsc_ptr;

    if (rank == 0) {
        rsc_ptr = CreateLocal<T>(ctx, name, ctrl, std::forward<Args>(args)...);
        SendStaticGroupCCIdp(comm, rsc_ptr->Idp());
    } else {
        auto idp = GetStaticGroupCCIdp(comm);
        rsc_ptr = CreateReference<T>(idp, ctx, name, ctrl);
    }

    SynchronizeStaticGroup(comm);

    return rsc_ptr;
}

template <typename T>
void ResourceManager::CreateReplica(idp_t idp, std::string const& ctrl)
{
    std::unique_lock<std::mutex> lk(_mtx);

    if (_sync_gfind.find(idp) == _sync_gfind.end()) {
        // create global find sync entry for the resource
        _sync_gfind.emplace(std::piecewise_construct, std::forward_as_tuple(idp), std::forward_as_tuple());
    }

    // start a global find for the resource
    FindGlobalResource(idp);

    _sync_gfind[idp].wait(lk);

    // verifies if the replica of this resource has already been requested
    if (_sync_replicas.find(idp) == _sync_replicas.end()) {
        
        // create sync entry for the replica
        _sync_replicas.emplace(std::piecewise_construct, std::forward_as_tuple(idp), std::forward_as_tuple());

        // request the replica by joining the resource dsm group
        JoinResourceGroup(idp);
        
        // create consistency object for resource
        auto cobj = new ConsistencyObject{this, idp, false,
            [] (Resource *rsc, Resource *new_rsc) {
                rsc->~Resource();
                rsc = new (rsc) T(std::move(*static_cast<T*>(new_rsc)));
            }, ctrl};

        // insert local consistency object
        _cst_objs.emplace(idp, cobj);
    }

    // waiting for replica creation
    _sync_replicas[idp].wait(lk);
}

template <typename T>
ResourcePtr<T> ResourceManager::GetLocalResource(idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);

    auto cst_obj = _cst_objs.find(idp);
    if (cst_obj == _cst_objs.end()) {
        std::cout << "Resource " << idp << " does not exist locally!" << std::endl;
        throw std::runtime_error("Resource " + std::to_string(idp) + " does not exist locally!");
    }

    return ResourcePtr<T>{cst_obj->second};
}

}

#endif
