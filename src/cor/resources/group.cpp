#include "cor/resources/group.hpp"

namespace cor {

Group::Group() = default;

Group::Group(idp_t idp, std::string const& module) :
    Resource{idp},
    DynamicOrganizer{idp, module}
{}

Group::~Group() = default;

Group::Group(Group&&) noexcept = default;

Group& Group::operator=(Group&&) noexcept = default;

}
