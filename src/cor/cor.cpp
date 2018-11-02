#include "cor/cor.hpp"

#include "cor/system/system.hpp"
#include "cor/elements/pod.hpp"

namespace cor {

void Initialize(std::string const& app_group, unsigned int number_pods)
{
    global::pod = new Pod(app_group, number_pods);
    global::pod->Initialize();
}

void Finalize()
{
    global::pod->Finalize();
}

Pod * const GetPod()
{
    return global::pod;
}

void Spawn(int number_pods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts)
{
    global::pod->Spawn(number_pods, module, args, hosts);
}

}
