#ifndef COR_RPC_MANAGER_HPP
#define COR_RPC_MANAGER_HPP

#include <string>

#include "cor/external/crazygaze/rpc/RPC.h"
#include "cor/system/rpc_table.hpp"

namespace cor {

class RpcManager
{

public:
    explicit RpcManager(std::string const& id);
    ~RpcManager();

    RpcManager() = delete;
    RpcManager(RpcManager const&) = delete;
    RpcManager& operator=(RpcManager const&) = delete;
    RpcManager(RpcManager&&) noexcept = delete;
    RpcManager& operator=(RpcManager&&) noexcept = delete;

    template <typename T, typename ... Args>
    idp_t CallCreate(std::string const& group, idp_t ctx, std::string const& name, Args&& ... args);

    //void StopService();

private:
    cz::rpc::Connection<RPC, RPC> *_con;

};

}

#include "cor/services/rpc_manager.tpp"

#endif
