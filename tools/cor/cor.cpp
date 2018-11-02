#include <iostream>
#include <cstdlib>

#include "cor/cor.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/agent.hpp"

int main(int argc, char *argv[])
{
    std::string module, app_group, host_file;
    unsigned int number_pods;
    
    if (argc >= 4) {
        app_group.assign(argv[1]);
        number_pods = std::strtoul(argv[2], nullptr, 0);
        module.assign(argv[3]);
        for (int i = 0; i < 4; ++i, --argc, ++argv);
    } else {
        std::cerr << "Error: Wrong number of arguments\n";
        std::cout << "Usage: cor <app_group> <number_pods> <module> <args...>\n";
        return EXIT_FAILURE;
    }

    cor::Initialize(app_group, number_pods);

    //{
        auto domain = cor::Domain::Create(cor::MetaDomain, "", false, module);
        domain->LoadModule();

        auto agent = cor::Agent<void(int,char**)>::Create(domain.GetIdp(), "", false, "cor_main");
        agent->Run(argc, argv);
        agent->Wait();
        agent->Get();
    //}

    cor::Finalize();

    return EXIT_SUCCESS;
}
