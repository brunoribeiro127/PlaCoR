#ifdef COR_CONTROLLER_HPP

#include "cor/utils/utils.hpp"

namespace cor {

template <typename T>
ResourcePtr<T> Controller::AllocateResource(idp_t idp, idp_t ctx, std::string const& name, Resource *rsc)
{
    return _rsc_mgr->AllocateResource<T>(idp, ctx, name, rsc, GetName());
}

template <typename T>
ResourcePtr<T> Controller::GetLocalResource(idp_t idp)
{
    return _rsc_mgr->GetLocalResource<T>(idp);
}

template <typename T>
ResourcePtr<T> Controller::CreateReference(idp_t idp, idp_t ctx, std::string const& name)
{
    return _rsc_mgr->CreateReference<T>(idp, ctx, name, GetName());
}

template <typename T, typename ... Args>
ResourcePtr<T> Controller::CreateLocal(idp_t ctx, std::string const& name, Args&& ... args)
{
    return _rsc_mgr->CreateLocal<T>(ctx, name, GetName(), std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
idp_t Controller::CreateRemote(idp_t ctx, std::string const& name, Args&& ... args)
{
    return _rsc_mgr->CreateRemote<T>(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
idp_t Controller::Create(idp_t ctx, std::string const& name, Args&& ... args)
{
    return _rsc_mgr->Create<T>(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
ResourcePtr<T> Controller::CreateCollective(idp_t ctx, std::string const& name, unsigned int total_members, Args&& ... args)
{
    ResourcePtr<T> rsc_ptr;

    InitCreateCollective(_context, total_members);
    auto first = GetCreateCollectiveFirst(_context);

    if (first) {
        rsc_ptr = CreateLocal<T>(ctx, name, std::forward<Args>(args)...);
        SendCreateCollectiveIdp(_context, rsc_ptr->Idp());
    } else {
        auto idp = GetCreateCollectiveIdp(_context);
        if (_rsc_mgr->ContainsResource(idp))
            rsc_ptr = GetLocalResource<T>(idp);
        else
            rsc_ptr = CreateReference<T>(idp, ctx, name);
    }

    //FinalizeCreateCollective(_context);

    return rsc_ptr;
}

}

#endif
