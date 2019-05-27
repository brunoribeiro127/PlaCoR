#include "cor/elements/sbarrier.hpp"

#include "cor/system/system.hpp"
#include "cor/system/pod.hpp"

namespace cor {

SBarrier::SBarrier() = default;

SBarrier::SBarrier(idp_t idp, idp_t clos) :
    _idp{idp},
    _clos{clos}
{}

SBarrier::~SBarrier() = default;

SBarrier::SBarrier(SBarrier&&) noexcept = default;

SBarrier& SBarrier::operator=(SBarrier&&) noexcept = default;

void SBarrier::Synchronize()
{
    auto active_rsc_idp = global::pod->GetActiveResourceIdp();
    auto sorg = global::pod->GetLocalResource<cor::StaticOrganizer>(_clos);
    auto idm = sorg->GetIdm(active_rsc_idp);
    global::pod->SynchronizeStaticGroup(_clos);
}

}
