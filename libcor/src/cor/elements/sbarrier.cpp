#include "cor/elements/sbarrier.hpp"

#include "cor/system/system.hpp"
#include "cor/system/pod.hpp"

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
    auto active_rsc_idp = global::pod->GetActiveResourceIdp();
    auto sorg = global::pod->GetLocalResource<cor::StaticOrganizer>(_comm);
    auto idm = sorg->GetIdm(active_rsc_idp);
    global::pod->SynchronizeStaticGroup(_comm);
}

}
