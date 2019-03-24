#ifdef COR_CONTAINER_HPP

#include "cor/system/system.hpp"
#include "cor/utils/utils.hpp"

#include "cor/system/pod.hpp"
#include "cor/system/rpc_manager.hpp"

namespace cor {

template <typename T, typename ... Args>
ResourcePtr<T> Container::CreateLocal(idp_t ctx, std::string const& name, Args&& ... args)
{
    return global::pod->CreateLocal(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
idp_t Container::Create(idp_t ctx, std::string const& name, Args&& ... args)
{
    if (global::pod->ContainsResource(ctx))
        return global::pod->Create<T>(ctx, name, std::forward<Args>(args)...);
    else
        return CreateRemote<T>(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
idp_t Container::CreateRemote(idp_t ctx, std::string const& name, Args&& ... args)
{
    auto ctrl = global::pod->SearchResource(ctx);
    ctrl[1] = 'R';
    return global::rpc->Create<T>(ctx, name, ctrl, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
void Container::Run(idp_t idp, Args&&... args)
{
    auto ctrl = global::pod->SearchResource(idp);
    ctrl[1] = 'R';
    return global::rpc->Run<T>(idp, ctrl, std::forward<Args>(args)...);
}

}

#endif
