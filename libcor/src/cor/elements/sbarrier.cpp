#include "cor/elements/sbarrier.hpp"

#include "cor/system/system.hpp"
#include "cor/elements/pod.hpp"

namespace cor {

SBarrier::SBarrier() = default;

SBarrier::SBarrier(idp_t idp, idp_t comm) :
    _idp{idp},
    _comm{comm}
{}

SBarrier::~SBarrier() = default;

SBarrier::SBarrier(SBarrier&&) noexcept = default;

SBarrier& SBarrier::operator=(SBarrier&&) noexcept = default;

void SBarrier::Synchronize()
{
    global::pod->SynchronizeStaticGroup(_comm);
}

}
