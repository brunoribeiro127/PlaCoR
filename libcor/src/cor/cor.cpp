#include "cor/cor.hpp"

#include "cor/utils/utils.hpp"
#include "cor/system/pod.hpp"
#include "cor/system/rpc_manager.hpp"
#include "cor/external/event/event.hpp"

namespace cor {

void Initialize(std::string const& app_group, std::string const& context, unsigned int npods)
{
    // enable libevent multithreaded environment
    ev::thread::evthread_use_pthreads();

    // generate a random id for cor services
    auto id = random_string(9);

    // create and start rpc manager
    global::rpc = new RpcManager(id);
    global::rpc->StartService();

    // create and initialize pod
    global::pod = new Pod(id, app_group, context, npods);
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
