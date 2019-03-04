#include <iostream>
#include <cstdlib>

#include "cor/cor.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/agent.hpp"
#include "cor/resources/communicator.hpp"

int main(int argc, char *argv[])
{
    std::string app_group, context, module;
    unsigned int npods;
    unsigned int total_members;
    idp_t parent;
    
    if (argc >= 6) {
        app_group.assign(argv[1]);
        context.assign(argv[2]);
        total_members = npods = std::strtoul(argv[3], nullptr, 0);
        parent = std::strtoul(argv[4], nullptr, 0);
        module.assign(argv[5]);
        for (int i = 0; i < 6; ++i, --argc, ++argv);
    } else {
        std::cerr << "Error: Wrong number of arguments\n";
        std::cout << "Usage: " << argv[0] << " <app group> <context> <number pods> <parent> <module> <args...>\n";
        return EXIT_FAILURE;
    }

    cor::Initialize(app_group, context, npods);

    {
        auto domain = gPod->CreateLocal<cor::Domain>(cor::MetaDomain, "", module);

        auto comm = gPod->CreateCollective<cor::Communicator>(domain->Idp(), "", npods, total_members, parent);

        auto agent = gPod->CreateLocal<cor::Agent<void(int,char**)>>(comm->Idp(), "", module, "Main");
        agent->Run(argc, argv);
        agent->Wait();
    }

    cor::Finalize();

    return EXIT_SUCCESS;
}
