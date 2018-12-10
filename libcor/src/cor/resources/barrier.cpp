#include "cor/resources/barrier.hpp"

namespace cor {

Barrier::Barrier() = default;

Barrier::Barrier(idp_t idp, std::string const& comm) : Resource{idp}, SBarrier{idp, comm} {}

Barrier::~Barrier() = default;

Barrier::Barrier(Barrier&&) noexcept = default;

Barrier& Barrier::operator=(Barrier&&) noexcept = default;

}
