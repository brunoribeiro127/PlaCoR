#ifndef COR_HPP
#define COR_HPP

#include <string>

#include "cor/system/system.hpp"
#include "cor/system/macros.hpp"

namespace cor {

    void Initialize(std::string const& app_group, std::string const& context, unsigned int npods);

    void Finalize();

    Pod * const GetPod();

}

#define gPod (cor::GetPod())

#endif
