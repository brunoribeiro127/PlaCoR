#ifdef COR_RPC_MANAGER_HPP

#include <iostream>

#include "cor/system/rpc_table.hpp"

namespace cor {

template <typename T, typename ... Args>
idp_t RpcManager::Create(idp_t ctx, std::string const& name, std::string const& ctrl, Args&& ... args)
{
    static_assert(ResourceTraits<T>::valid, "ERROR");

    if constexpr (ResourceTraits<T>::valid)
        return (*_con).template call<typename ResourceTraits<T>::func_type>(
            *(*_con).transport,
            ctrl,
            (uint32_t) ResourceTraits<T>::rpcid,
            ctx, name, std::forward<Args>(args)...
        ).ft().get().get();
}

}

#endif
