#include <iostream>
#include <cstdlib>

#include "cor/cor.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/agent.hpp"
#include "cor/resources/communicator.hpp"

int main(int argc, char *argv[])
{
    std::string app_group, communicator, module;
    unsigned int npods;
    idp_t parent;
    
    if (argc >= 6) {
        app_group.assign(argv[1]);
        communicator.assign(argv[2]);
        npods = std::strtoul(argv[3], nullptr, 0);
        parent = std::strtoul(argv[4], nullptr, 0);
        module.assign(argv[5]);
        for (int i = 0; i < 6; ++i, --argc, ++argv);
    } else {
        std::cerr << "Error: Wrong number of arguments\n";
        std::cout << "Usage: " << argv[0] << " <app group> <communicator> <number pods> <parent> <module> <args...>\n";
        return EXIT_FAILURE;
    }

    cor::Initialize(app_group, communicator, npods);

    //{
        auto domain = gPod->Create<cor::Domain>(cor::MetaDomain, "", false, module);

        auto comm = gPod->CreateCollective<cor::Communicator>(domain.GetIdp(), communicator, npods, false, communicator, npods, parent);

        auto agent = gPod->Create<cor::Agent<void(int,char**)>>(comm.GetIdp(), "", false, module, "Main");
        agent->Run(argc, argv);
        agent->Wait();
    //}

    cor::Finalize();

    return EXIT_SUCCESS;
}
