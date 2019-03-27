#include "cor/elements/container.hpp"

namespace cor {

Container::Container() = default;

Container::Container(idp_t idp) : _idp{idp} {}

Container::~Container() = default;

Container::Container(Container&&) noexcept = default;

Container& Container::operator=(Container&&) noexcept = default;

std::string const& Container::GetGlobalContext()
{
    return global::pod->GetGlobalContext();
}

std::string const& Container::GetLocalContext()
{
    return global::pod->GetLocalContext();
}

unsigned int Container::GetTotalPods()
{    
    return global::pod->GetTotalPods();
}

unsigned int Container::GetTotalDomains()
{
    return global::pod->GetTotalDomains();
}

idp_t Container::GetActiveResourceIdp()
{
    return global::pod->GetActiveResourceIdp();
}

idp_t Container::GetDomainIdp()
{
    return global::pod->GetDomainIdp();
}

idp_t Container::GetDomainIdp(idp_t idp)
{
    return global::pod->GetDomainIdp(idp);
}

idp_t Container::GetPredecessorIdp(idp_t idp)
{
    return global::pod->GetPredecessorIdp(idp);
}

idp_t Container::Spawn(std::string const& context, unsigned int npods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts)
{
    return global::pod->Spawn(context, npods, module, args, hosts);   
}

}
