#include "cor/cor.hpp"

#include "cor/system/system.hpp"
#include "cor/elements/pod.hpp"

namespace cor {

void Initialize(std::string const& app_group, std::string const& context, unsigned int npods)
{
    global::pod = new Pod{app_group, context, npods};
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

void Spawn(std::string const& comm, unsigned int npods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts)
{
    global::pod->Spawn(comm, npods, module, args, hosts);
}

}
