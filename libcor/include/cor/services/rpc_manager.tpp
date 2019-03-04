#ifdef COR_RPC_MANAGER_HPP

namespace cor {

template <typename T, typename ... Args>
idp_t RpcManager::CallCreate(std::string const& group, idp_t ctx, std::string const& name, Args&& ... args)
{
    if constexpr (ResourceTraits<T>::valid)
        std::cout << "blele" << std::endl;
}

}

#endif
