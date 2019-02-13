#include "cor/resources/communicator.hpp"

namespace cor {

Communicator::Communicator() = default;

Communicator::Communicator(idp_t idp, unsigned int total_members, idp_t parent) :
    Resource{idp},
    StaticOrganizer{idp, total_members, parent}
{}

Communicator::~Communicator() = default;

Communicator::Communicator(Communicator&&) noexcept = default;

Communicator& Communicator::operator=(Communicator&&) noexcept = default;

}
