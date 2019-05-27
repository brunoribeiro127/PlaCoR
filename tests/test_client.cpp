#include <iostream>

#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

static constexpr std::size_t NUM_AGENTS = 5;

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    auto remote_comm_idp = domain->Spawn("server", 1, "~/placor/tests/libtest_server.so", {}, { "localhost" });
    auto remote_comm = domain->CreateReference<cor::Communicator>(remote_comm_idp, domain->Idp(), "REMOTE COMMUNICATOR");
    auto remote_domain_idp = domain->GetPredecessorIdp(remote_comm_idp);

    {
        std::vector<idp_t> _remote_agents;

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            auto rsc_idp = domain->Create<cor::Agent<idp_t(idp_t)>>(remote_domain_idp, "", "libtest_server.so", "Test");
            _remote_agents.push_back(rsc_idp);
        }

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            domain->Run<idp_t(idp_t)>(_remote_agents[i], agent_idp);
        }

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            domain->Wait<idp_t(idp_t)>(_remote_agents[i]);
        }

        for (auto i = 0; i < NUM_AGENTS; ++i) {
            auto res = domain->Get<idp_t(idp_t)>(_remote_agents[i]);
            std::cout << "WORKER: " << res << std::endl;
        }
    }

    {
        cor::Message msg;
        agent->Send(0, remote_comm_idp, msg);
    }
}
