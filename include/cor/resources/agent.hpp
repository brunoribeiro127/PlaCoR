#ifndef COR_AGENT_HPP
#define COR_AGENT_HPP

#include <functional>

#include "cor/resources/resource.hpp"
#include "cor/services/resource_factory.hpp"
#include "cor/elements/executor.hpp"
#include "cor/elements/mailbox.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

template <typename> class Agent;

template <typename R, typename ... P>
class Agent<R(P...)>: public Resource, public ResourceFactory<Agent<R(P...)>>, public Executor<R(P...)>, public Mailbox
{

friend class ResourceFactory<Agent<R(P...)>>;
friend class cereal::access;

public:    
    ~Agent();

    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;

    Agent(Agent&&) noexcept;
    Agent& operator=(Agent&&) noexcept;

protected:
    Agent();
    explicit Agent(idp_t idp, std::string const& function);
    explicit Agent(idp_t idp, std::function<R(P...)> const& f);

private:
	template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(
            cereal::base_class<Resource>(this),
            cereal::base_class<Executor<R(P...)>>(this),
            cereal::base_class<Mailbox>(this)
        );
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

#include "cor/resources/agent.tpp"

#endif
