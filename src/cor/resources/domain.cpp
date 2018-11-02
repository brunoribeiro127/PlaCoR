#include "cor/resources/domain.hpp"

namespace cor {

Domain::Domain() = default;

Domain::Domain(idp_t idp, std::string const& module) :
    Resource{idp},
    Organizer{idp, module}
{}

Domain::~Domain() = default;

Domain::Domain(Domain&&) noexcept = default;

Domain& Domain::operator=(Domain&&) noexcept = default;

}
