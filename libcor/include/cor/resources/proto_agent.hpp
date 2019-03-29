#ifndef COR_PROTO_AGENT_HPP
#define COR_PROTO_AGENT_HPP

#include <functional>

#include "cor/resources/resource.hpp"
#include "cor/elements/executor.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

template <typename> class ProtoAgent;

template <typename R, typename ... P>
class ProtoAgent<R(P...)>: public Resource, public Executor<R(P...)>
{

friend class ResourceManager;
friend class cereal::access;

public:    
    ~ProtoAgent();

    ProtoAgent(const ProtoAgent&) = delete;
    ProtoAgent& operator=(const ProtoAgent&) = delete;

    ProtoAgent(ProtoAgent&&) noexcept;
    ProtoAgent& operator=(ProtoAgent&&) noexcept;

protected:
    ProtoAgent();

    ProtoAgent(idp_t idp, std::function<R(P...)> const& f);
    ProtoAgent(idp_t idp, std::string const& module, std::string const& function);

private:
	template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(
            cereal::base_class<Resource>(this),
            cereal::base_class<Executor<R(P...)>>(this)
        );
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

#include "cor/resources/proto_agent.tpp"

#endif
