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

#define CREATE(...) REGISTERRPC(__VA_ARGS__)
#define RUN(...) REGISTERRPC(__VA_ARGS__)

#define RPCTABLE_CLASS cor::RPC
#define RPCTABLE_CONTENTS                                                                                           \
    CREATE(Group, group, Create<cor::Group, std::string const&>)                                                    \
    CREATE(Data<std::vector<int>>, data, Create<cor::Data<std::vector<int>>, std::vector<int>>)                     \
    CREATE(Agent<void(idp_t)>, agent1, Create<cor::Agent<void(idp_t)>, std::string const&, std::string const&>)     \
    RUN(Agent<void(idp_t)>, run1, Run<cor::Agent<void(idp_t)>, idp_t>)                                              \
    CREATE(Agent<void()>, agent, Create<cor::Agent<void()>, std::string const&, std::string const&>)                \
    RUN(Agent<void()>, run, Run<cor::Agent<void()>>)
#include "cor/external/crazygaze/rpc/RPCGenerate.h"

namespace cor {

    template <typename T, typename ENABLED = void>
    struct ResourceTraits {
        using rsc_type = T;
        static constexpr bool valid = false;
    };

    template <typename T, typename ENABLED = void>
    struct RunTraits {
        using rsc_type = T;
        static constexpr bool valid = false;
    }; 

}

#undef CREATE
#define CREATE(rsc, id, ...)                                                                                            \
    namespace cor {                                                                                                     \
        template <>                                                                                                     \
        struct ResourceTraits<rsc> {                                                                                    \
            using rsc_type = rsc;                                                                                       \
            using func_type = decltype(&RPCTABLE_CLASS::__VA_ARGS__);                                                   \
            static constexpr cz::rpc::Table<RPCTABLE_CLASS>::RPCId rpcid = cz::rpc::Table<RPCTABLE_CLASS>::RPCId::id;   \
            static constexpr bool valid = true;                                                                         \
        };                                                                                                              \
    }

#undef RUN
#define RUN(rsc, id, ...)                                                                                               \
    namespace cor {                                                                                                     \
        template <>                                                                                                     \
        struct RunTraits<rsc> {                                                                                         \
            using rsc_type = rsc;                                                                                       \
            using func_type = decltype(&RPCTABLE_CLASS::__VA_ARGS__);                                                   \
            static constexpr cz::rpc::Table<RPCTABLE_CLASS>::RPCId rpcid = cz::rpc::Table<RPCTABLE_CLASS>::RPCId::id;   \
            static constexpr bool valid = true;                                                                         \
        };                                                                                                              \
    }

RPCTABLE_CONTENTS;

#undef REGISTERRPC
#undef RPCTABLE_CLASS
#undef RPCTABLE_CONTENTS

#endif
