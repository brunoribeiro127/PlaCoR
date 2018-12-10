#include "cor/resources/resource.hpp"

namespace cor {

Resource::Resource() = default;

Resource::Resource(idp_t idp) : _idp{idp} {}

Resource::~Resource() = default;

Resource::Resource(Resource&&) noexcept = default;

Resource& Resource::operator=(Resource&&) noexcept = default;

idp_t Resource::Idp() const
{
    return _idp;
}

}
