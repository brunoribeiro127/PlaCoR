#include "cor/elements/synchronizer.hpp"

#include "cor/system/system.hpp"
#include "cor/elements/pod.hpp"

namespace cor {

Synchronizer::Synchronizer() = default;

Synchronizer::Synchronizer(idp_t idp) : _idp{idp} {}

Synchronizer::~Synchronizer() = default;

Synchronizer::Synchronizer(Synchronizer&&) noexcept = default;

Synchronizer& Synchronizer::operator=(Synchronizer&&) noexcept = default;

void Synchronizer::AcquireWrite() const
{
    // acquire write operation
    global::pod->GetConsistencyObject(_idp)->AcquireWrite();
}

void Synchronizer::ReleaseWrite() const
{
    // release write operation
    global::pod->GetConsistencyObject(_idp)->ReleaseWrite();
}

void Synchronizer::AcquireRead() const
{
    // acquire read operation
    global::pod->GetConsistencyObject(_idp)->AcquireRead();
}

void Synchronizer::ReleaseRead() const
{
    // release read operation
    global::pod->GetConsistencyObject(_idp)->ReleaseRead();
}

}
