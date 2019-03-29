#ifdef COR_PROTO_AGENT_HPP

namespace cor {

template <typename R, typename ... P>
ProtoAgent<R(P...)>::ProtoAgent() = default;

template <typename R, typename ... P>
ProtoAgent<R(P...)>::ProtoAgent(idp_t idp, std::function<R(P...)> const& f) :
    Resource{idp},
    Executor<R(P...)>{idp, f}
{}

template <typename R, typename ... P>
ProtoAgent<R(P...)>::ProtoAgent(idp_t idp, std::string const& module, std::string const& function) :
    Resource{idp},
    Executor<R(P...)>{idp, module, function}
{}

template <typename R, typename ... P>
ProtoAgent<R(P...)>::~ProtoAgent() = default;

template <typename R, typename ... P>
ProtoAgent<R(P...)>::ProtoAgent(ProtoAgent<R(P...)>&&) noexcept = default;

template <typename R, typename ... P>
ProtoAgent<R(P...)>& ProtoAgent<R(P...)>::operator=(ProtoAgent<R(P...)>&&) noexcept = default;

}

#endif
