#include "cor/resources/communicator.hpp"

namespace cor {

Communicator::Communicator() = default;

Communicator::Communicator(idp_t idp, std::string const& comm, unsigned int total_members, idp_t parent) :
    Resource{idp},
    StaticOrganizer{idp, comm, total_members, parent}
{}

Communicator::~Communicator() = default;

Communicator::Communicator(Communicator&&) noexcept = default;

Communicator& Communicator::operator=(Communicator&&) noexcept = default;

}
