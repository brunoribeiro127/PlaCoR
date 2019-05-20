#ifndef COR_RPC_TABLE_HPP
#define COR_RPC_TABLE_HPP

CZRPC_ALLOW_CONST_LVALUE_REFS;
CZRPC_ALLOW_NON_CONST_LVALUE_REFS;
CZRPC_ALLOW_RVALUE_REFS;

#include "cor/resources/agent.hpp"
#include "cor/resources/proto_agent.hpp"
#include "cor/resources/barrier.hpp"
#include "cor/resources/communicator.hpp"
#include "cor/resources/data.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/group.hpp"
#include "cor/resources/mutex.hpp"
#include "cor/resources/rwmutex.hpp"

#include "cor/system/rpc_class.hpp"

#define CREATE(...) REGISTERRPC(__VA_ARGS__)
#define RUN(...) REGISTERRPC(__VA_ARGS__)
#define WAIT(...) REGISTERRPC(__VA_ARGS__)
#define GET(...) REGISTERRPC(__VA_ARGS__)

#define RPCTABLE_CLASS cor::RPC
#define RPCTABLE_CONTENTS                                                                                           \
    CREATE(Group, group, Create<cor::Group, std::string const&>)                                                    \
    CREATE(Data<std::vector<int>>, data, Create<cor::Data<std::vector<int>>, std::vector<int>>)                     \
                                                                                                                    \
    CREATE(Agent<void()>, agent, Create<cor::Agent<void()>, std::string const&, std::string const&>)                \
    RUN(Agent<void()>, run, Run<cor::Agent<void()>>)                                                                \
    WAIT(Agent<void()>, wait, Wait<cor::Agent<void()>>)                                                             \
    GET(Agent<void()>, get, Get<cor::Agent<void()>, void>)                                                          \
                                                                                                                    \
    CREATE(Agent<idp_t()>, agent1, Create<cor::Agent<idp_t()>, std::string const&, std::string const&>)             \
    RUN(Agent<idp_t()>, run1, Run<cor::Agent<idp_t()>>)                                                             \
    WAIT(Agent<idp_t()>, wait1, Wait<cor::Agent<idp_t()>>)                                                          \
    GET(Agent<idp_t()>, get1, Get<cor::Agent<idp_t()>, idp_t>)                                                      \
                                                                                                                    \
    CREATE(Agent<void(idp_t)>, agent2, Create<cor::Agent<void(idp_t)>, std::string const&, std::string const&>)     \
    RUN(Agent<void(idp_t)>, run2, Run<cor::Agent<void(idp_t)>, idp_t>)                                              \
    WAIT(Agent<void(idp_t)>, wait2, Wait<cor::Agent<void(idp_t)>>)                                                  \
    GET(Agent<void(idp_t)>, get2, Get<cor::Agent<void(idp_t)>, void>)
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

    template <typename T, typename ENABLED = void>
    struct WaitTraits {
        using rsc_type = T;
        static constexpr bool valid = false;
    };

    template <typename T, typename ENABLED = void>
    struct GetTraits {
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

#undef WAIT
#define WAIT(rsc, id, ...)                                                                                              \
    namespace cor {                                                                                                     \
        template <>                                                                                                     \
        struct WaitTraits<rsc> {                                                                                        \
            using rsc_type = rsc;                                                                                       \
            using func_type = decltype(&RPCTABLE_CLASS::__VA_ARGS__);                                                   \
            static constexpr cz::rpc::Table<RPCTABLE_CLASS>::RPCId rpcid = cz::rpc::Table<RPCTABLE_CLASS>::RPCId::id;   \
            static constexpr bool valid = true;                                                                         \
        };                                                                                                              \
    }

#undef GET
#define GET(rsc, id, ...)                                                                                               \
    namespace cor {                                                                                                     \
        template <>                                                                                                     \
        struct GetTraits<rsc> {                                                                                         \
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
