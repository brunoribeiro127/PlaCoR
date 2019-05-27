#include "cor/resources/barrier.hpp"

namespace cor {

Barrier::Barrier() = default;

Barrier::Barrier(idp_t idp, idp_t clos) : Resource{idp}, SBarrier{idp, clos} {}

Barrier::~Barrier() = default;

Barrier::Barrier(Barrier&&) noexcept = default;

Barrier& Barrier::operator=(Barrier&&) noexcept = default;

}
