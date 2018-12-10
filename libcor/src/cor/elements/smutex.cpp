#include "cor/elements/smutex.hpp"

#include "cor/system/system.hpp"
#include "cor/elements/pod.hpp"

namespace cor {

SMutex::SMutex() = default;

SMutex::SMutex(idp_t idp) : _idp{idp} {}

SMutex::~SMutex() = default;

SMutex::SMutex(SMutex&&) noexcept = default;

SMutex& SMutex::operator=(SMutex&&) noexcept = default;

void SMutex::Acquire()
{
    global::pod->GetConsistencyObject(_idp)->AcquireWrite();
}

void SMutex::Release()
{
    global::pod->GetConsistencyObject(_idp)->ReleaseWrite();
}

}
