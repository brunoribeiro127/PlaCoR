#ifndef COR_HPP
#define COR_HPP

#include <string>

#include "cor/system/system.hpp"
#include "cor/system/macros.hpp"
#include "cor/resources/domain.hpp"

namespace cor {

    ResourcePtr<Domain> Initialize(std::string const& app_group, std::string const& context, unsigned int npods, std::string const& module);

    void Finalize();

    ResourcePtr<Domain> GetDomain();

}

#endif
