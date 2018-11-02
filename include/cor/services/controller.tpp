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
    auto ref = GenerateIdp();
    return _rsc_mgr->CreateReference<T>(idp, ref, ctx, name, GetName());
}

}

#endif
