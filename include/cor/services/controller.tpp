#ifdef COR_CONTROLLER_HPP

namespace cor {

template <typename T>
ResourcePtr<T> Controller::AllocateResource(idp_t idp, idp_t ctx, std::string const& name, bool global, Resource *rsc)
{
    return _rsc_mgr->AllocateResource<T>(idp, ctx, name, global, rsc, GetName());
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
ResourcePtr<T> Controller::Create(idp_t ctx, std::string const& name, bool global, Args&& ... args)
{
    return _rsc_mgr->Create<T>(ctx, name, global, GetName(), std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
ResourcePtr<T> Controller::CreateCollective(idp_t ctx, std::string const& name, unsigned int total_nembers, bool global, Args&& ... args)
{
    return _rsc_mgr->CreateCollective<T>(ctx, name, total_nembers, global, GetName(), std::forward<Args>(args)...);
}

}

#endif
