#include <iostream>

#include "cor/cor.hpp"
#include "cor/resources/group.hpp"
#include "cor/resources/agent.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/communicator.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

static constexpr idm_t MASTER = 0;

void Main(int argc, char *argv[])
{
    // get local domain
    auto domain_idp = gPod->GetDomainIdp();
    auto domain = gPod->GetLocalResource<cor::Domain>(domain_idp);

    // get local agent idp and resource
    auto agent_idp = gPod->GetActiveResourceIdp();
    auto agent = gPod->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    // get agent rank
    auto comm_idp = gPod->GetPredecessorIdp(agent_idp);
    auto comm = gPod->GetLocalResource<cor::Communicator>(comm_idp);
    auto comm_size = comm->GetTotalMembers();
    auto rank = comm->GetIdm(agent_idp);

    if (rank == MASTER) {

        auto group = gPod->CreateLocal<cor::Group>(gPod->GetDomainIdp(), "group", "");

        cor::Message msg;
        msg.SetType(0);
        msg.Add<idp_t>(group->Idp());

        for (int i = 1; i < comm_size; ++i)
            agent->Send(comm->GetIdp(i), msg);

        std::cin.ignore();

    } else {

        auto msg = agent->Receive();
        auto group_idp = msg.Get<idp_t>();

        auto rsc_idp = domain->Create<cor::Group>(group_idp, "test", "");
        std::cout << rsc_idp << std::endl;
    }

}
