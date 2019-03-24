#ifndef COR_RPC_CLASS_HPP
#define COR_RPC_CLASS_HPP

#include <string>

#include "cor/system/macros.hpp"

namespace cor {

class RPC
{

public:
    explicit RPC();
    ~RPC();

    template <typename T, typename ... Args>
    idp_t Create(idp_t ctx, std::string const& name, Args&& ... args);

    template <typename T, typename ... Args>
    void Run(idp_t idp, Args&&... args);

};

}

#include "cor/system/rpc_class.tpp"

#endif
