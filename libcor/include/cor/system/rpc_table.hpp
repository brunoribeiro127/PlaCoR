#ifndef COR_RPC_TABLE_HPP
#define COR_RPC_TABLE_HPP

CZRPC_ALLOW_CONST_LVALUE_REFS;
CZRPC_ALLOW_NON_CONST_LVALUE_REFS;
CZRPC_ALLOW_RVALUE_REFS;

#include "cor/resources/agent.hpp"
#include "cor/resources/barrier.hpp"
#include "cor/resources/communicator.hpp"
#include "cor/resources/data.hpp"
//#include "cor/resources/domain.hpp"
#include "cor/resources/group.hpp"
#include "cor/resources/mutex.hpp"
#include "cor/resources/rwmutex.hpp"

#include "cor/system/rpc_class.hpp"

#define RPCTABLE_CLASS cor::RPC
#define RPCTABLE_CONTENTS                                                                                   \
    REGISTERRPC(Group, group, Create<cor::Group, std::string const&>)                                       \
    REGISTERRPC(Agent<void()>, agent, Create<cor::Agent<void()>, std::string const&, std::string const&>)   \
    REGISTERRPC(Data<std::vector<int>>, data, Create<cor::Data<std::vector<int>>, std::vector<int>>)
#include "cor/external/crazygaze/rpc/RPCGenerate.h"

namespace cor {

    template <typename T, typename ENABLED = void>
    struct ResourceTraits {
        using rsc_type = T;
        //using func_type = decltype(idp_t());
        //static constexpr cz::rpc::Table<RPC>::RPCId rpcid = cz::rpc::Table<RPC>::RPCId::genericRPC;
        static constexpr bool valid = false;
    };
/*
    template <>
    struct ResourceTraits<Domain, cz::rpc::Table<RPC>::RPCId::domain, &RPC::Create<cor::Domain, std::string>> {
        using rsc_type = Domain;
        using func_type = decltype(&RPC::Create<cor::Domain, std::string>);
        static constexpr cz::rpc::Table<RPC>::RPCId rpcid = cz::rpc::Table<RPC>::RPCId::domain;
        static constexpr bool valid = true;
    };
*/

}

#undef REGISTERRPC
#define REGISTERRPC(rsc, id, ...)                                                                                       \
    namespace cor {                                                                                                     \
        template <>                                                                                                     \
        struct ResourceTraits<rsc> {                                                                                    \
            using rsc_type = rsc;                                                                                       \
            using func_type = decltype(&RPCTABLE_CLASS::__VA_ARGS__);                                                   \
            static constexpr cz::rpc::Table<RPCTABLE_CLASS>::RPCId rpcid = cz::rpc::Table<RPCTABLE_CLASS>::RPCId::id;   \
            static constexpr bool valid = true;                                                                         \
        };                                                                                                              \
    }

// falta arranjar maneira de acrescentar o tipo completo em func_type

RPCTABLE_CONTENTS;

#undef REGISTERRPC
#undef RPCTABLE_CLASS
#undef RPCTABLE_CONTENTS

#endif
