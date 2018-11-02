#ifndef COR_MACROS_HPP
#define COR_MACROS_HPP

#include <cstdint>

using idp_t  = std::uint32_t;
using page_t = std::uint32_t;
using idm_t  = std::uint32_t;

namespace cor
{

    static constexpr idp_t MetaDomain = 0;
    static constexpr idp_t MasterDomain = 4294967040;
    static constexpr idp_t MasterAgent = 4294967039;

}

#endif
