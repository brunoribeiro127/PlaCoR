#ifdef COR_POD_HPP

namespace cor {

template <typename T>
ResourcePtr<T> Pod::AllocateResource(idp_t idp, idp_t ctx, std::string const& name, bool global, Resource *rsc)
{
    return _ctrl->AllocateResource<T>(idp, ctx, name, global, rsc);
}

template <typename T>
std::function<T> Pod::LoadFunction(std::string const& module, std::string const& function)
{
    std::unique_lock<std::mutex> lk(_mtx);
    auto dylib = _modules.at(module);
    return dylib->LoadFunction<T>(function);
}

template <typename T>
ResourcePtr<T> Pod::GetLocalResource(idp_t idp)
{
    return _ctrl->GetLocalResource<T>(idp);
}

template <typename T>
ResourcePtr<T> Pod::CreateReference(idp_t idp, idp_t ctx, std::string const& name)
{
    return _ctrl->CreateReference<T>(idp, ctx, name);
}

}

#endif
