#include "cor/resources/mutex.hpp"

namespace cor {

Mutex::Mutex() = default;

Mutex::Mutex(idp_t idp) : Resource{idp}, SMutex{idp} {}

Mutex::~Mutex() = default;

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

}
