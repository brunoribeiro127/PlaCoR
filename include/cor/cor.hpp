#ifndef COR_HPP
#define COR_HPP

#include <string>

#include "cor/elements/pod.hpp"

namespace cor {

    void Initialize(std::string const& app_group, unsigned int number_pods);

    void Finalize();

    Pod * const GetPod();

    void Spawn(int number_pods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts);

}

#define gPod (cor::GetPod())

#endif
