#include "cor/resources/closure.hpp"

namespace cor {

Closure::Closure() = default;

Closure::Closure(idp_t idp, unsigned int total_members, idp_t parent) :
    Resource{idp},
    StaticOrganizer{idp, total_members, parent}
{}

Closure::~Closure() = default;

Closure::Closure(Closure&&) noexcept = default;

Closure& Closure::operator=(Closure&&) noexcept = default;

}
