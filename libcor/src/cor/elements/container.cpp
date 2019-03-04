#include "cor/elements/container.hpp"

namespace cor {

Container::Container() = default;

Container::Container(idp_t idp) : _idp{idp} {}

Container::~Container() = default;

Container::Container(Container&&) noexcept = default;

Container& Container::operator=(Container&&) noexcept = default;

}
