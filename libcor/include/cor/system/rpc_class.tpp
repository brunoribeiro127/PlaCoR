#ifdef COR_RPC_CLASS_HPP

#include <iostream>
#include "cor/system/system.hpp"
#include "cor/system/pod.hpp"

namespace cor {

template <typename T, typename ... Args>
idp_t RPC::Create(idp_t ctx, std::string const& name, Args&& ... args)
{
    return global::pod->Create<T>(ctx, name, std::forward<Args>(args)...);
}

template <typename T, typename ... Args>
void RPC::Run(idp_t idp, Args&&... args)
{
    auto rsc = global::pod->GetLocalResource<T>(idp);
    rsc->Run(std::forward<Args>(args)...);
}

}

#endif
