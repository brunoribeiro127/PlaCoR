#include <iostream>

#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

static constexpr std::size_t NUM_AGENTS = 10;

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    auto remote_comm_idp = domain->Spawn("server", 1, "~/placor/tests/libtest_server.dylib", {}, { "localhost" });

    auto server_msg = agent->Receive();
    auto remote_domain_idp = server_msg.Get<idp_t>(0);

    {
        std::vector<idp_t> _remote_agents;

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            auto rsc_idp = domain->Create<cor::Agent<idp_t()>>(remote_domain_idp, "", "/Users/brunoribeiro/placor/tests/libtest_server.dylib", "TestIdp");
            _remote_agents.push_back(rsc_idp);
        }

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            domain->Run<cor::Agent<idp_t()>>(_remote_agents[i]);
        }

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            domain->Wait<cor::Agent<idp_t()>>(_remote_agents[i]);
        }

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            auto res = domain->Get<cor::Agent<idp_t()>, idp_t>(_remote_agents[i]);
            std::cout << "WORKER: " << res << std::endl;
        }
    }

    {
        cor::Message msg;
        agent->Send(server_msg.Sender(), msg);
    }
}
