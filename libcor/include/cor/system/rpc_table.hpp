#ifndef COR_RPC_TABLE_HPP
#define COR_RPC_TABLE_HPP

#include "cor/system/macros.hpp"
#include "cor/system/system.hpp"

namespace cor {

class RPC
{

public:
    template <typename T, typename ... Args>
    idp_t Create(idp_t ctx, std::string const& name, Args&& ... args)
    {
        return global::pod->Create<T>(ctx, name, std::forward<Args>(args)...);
    }

};

}

CZRPC_ALLOW_CONST_LVALUE_REFS;
CZRPC_ALLOW_NON_CONST_LVALUE_REFS;
CZRPC_ALLOW_RVALUE_REFS;

#include "cor/resources/domain.hpp"
#include "cor/resources/group.hpp"
#include "cor/resources/agent.hpp"
#include "cor/resources/data.hpp"

#define RPCTABLE_CLASS cor::RPC
#define RPCTABLE_CONTENTS \
    REGISTERRPC(Domain, domain, Create<cor::Domain, std::string>)                                       \
    REGISTERRPC(Group, group, Create<cor::Group, std::string>)                                          \
    REGISTERRPC(Agent<void()>, agent, Create<cor::Agent<void()>, std::string, std::string>)             \
    REGISTERRPC(Data<std::vector<int>>, data, Create<cor::Data<std::vector<int>>, std::vector<int>>)
#include "cor/external/crazygaze/rpc/RPCGenerate.h"

namespace cor {

    template <typename T, typename ENABLED = void>
    struct ResourceTraits {
        using type = T;
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
#define REGISTERRPC(rsc, id, ...) \
    namespace cor { \
        template <> \
        struct ResourceTraits<rsc> { \
            using rsc_type = rsc; \
            using func_type = decltype(&RPCTABLE_CLASS::__VA_ARGS__); \
            static constexpr cz::rpc::Table<RPCTABLE_CLASS>::RPCId rpcid = cz::rpc::Table<RPCTABLE_CLASS>::RPCId::id; \
            static constexpr bool valid = true; \
        }; \
    }

RPCTABLE_CONTENTS;

#undef REGISTERRPC
#undef RPCTABLE_CLASS
#undef RPCTABLE_CONTENTS

#endif
