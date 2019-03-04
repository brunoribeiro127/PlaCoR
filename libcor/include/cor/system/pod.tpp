#ifdef COR_POD_HPP

namespace cor {

template <typename T>
ResourcePtr<T> Pod::AllocateResource(idp_t idp, idp_t ctx, std::string const& name, Resource *rsc)
{
    return _ctrl->AllocateResource<T>(idp, ctx, name, rsc);
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

template <typename T, typename ... Args>
ResourcePtr<T> Pod::CreateLocal(idp_t ctx, std::string const& name, Args&& ... args)
{
    return _ctrl->CreateLocal<T>(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
idp_t Pod::CreateRemote(idp_t ctx, std::string const& name, Args&& ... args)
{
    return _ctrl->CreateRemote<T>(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
idp_t Pod::Create(idp_t ctx, std::string const& name, Args&& ... args)
{
    return _ctrl->Create<T>(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
ResourcePtr<T> Pod::CreateCollective(idp_t ctx, std::string const& name, unsigned int total_members, Args&& ... args)
{
    return _ctrl->CreateCollective<T>(ctx, name, total_members, std::forward<Args>(args)...);
}

template <typename T>
ResourcePtr<T> Pod::CreateReference(idp_t idp, idp_t ctx, std::string const& name)
{
    return _ctrl->CreateReference<T>(idp, ctx, name);
}

}

#endif
