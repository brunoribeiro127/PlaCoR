#ifdef COR_RPC_MANAGER_HPP

#include "cor/system/rpc_table.hpp"

namespace cor {

template <typename T, typename ... Args>
idp_t RpcManager::Create(idp_t ctx, std::string const& name, std::string const& ctrl, Args&&... args)
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

template <typename T, typename ... Args>
void RpcManager::Run(idp_t idp, std::string const& ctrl, Args&&... args)
{
    static_assert(RunTraits<T>::valid, "ERROR");

    if constexpr (RunTraits<T>::valid)
        (*_con).template call<typename RunTraits<T>::func_type>(
            *(*_con).transport,
            ctrl,
            (uint32_t) RunTraits<T>::rpcid,
            idp, std::forward<Args>(args)...
        ).ft().get().get();
}

template <typename T>
void RpcManager::Wait(idp_t idp, std::string const& ctrl)
{
    static_assert(WaitTraits<T>::valid, "ERROR");

    if constexpr (WaitTraits<T>::valid)
        (*_con).template call<typename WaitTraits<T>::func_type>(
            *(*_con).transport,
            ctrl,
            (uint32_t) WaitTraits<T>::rpcid,
            idp
        ).ft().get().get();
}

template <typename T, typename R>
R RpcManager::Get(idp_t idp, std::string const& ctrl)
{
    static_assert(GetTraits<T>::valid, "ERROR");

    if constexpr (GetTraits<T>::valid)
        return (*_con).template call<typename GetTraits<T>::func_type>(
            *(*_con).transport,
            ctrl,
            (uint32_t) GetTraits<T>::rpcid,
            idp
        ).ft().get().get();
}

}

#endif
