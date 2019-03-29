#include <iostream>

#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
    void Test(idp_t idp);
}

static constexpr idm_t MASTER = 0;

void Test(idp_t idp)
{
    //std::cout << "Hello, World!" << std::endl;
    std::cout << "-->> " << idp << std::endl;
}

void Main(int argc, char *argv[])
{
    // get local domain
    auto domain = cor::GetDomain();

    // get local agent idp and resource
    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    // get agent rank
    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);
    auto comm_size = comm->GetTotalMembers();
    auto rank = comm->GetIdm(agent_idp);

    if (rank == MASTER) {

        auto group = domain->CreateLocal<cor::Group>(domain->Idp(), "group", "");

        cor::Message msg;
        msg.SetType(0);
        msg.Add<idp_t>(group->Idp());
/*
        for (int i = 1; i < comm_size; ++i)
            agent->Send(comm->GetIdp(i), msg);

        std::cin.ingore();
*/
        agent->Send(comm->GetMemberList(), msg);

        auto smsg  = agent->Receive();
        auto group_idp = smsg.Get<idp_t>();

        std::cout << group_idp << std::endl;

    } else {

        auto msg = agent->Receive();
        auto group_idp = msg.Get<idp_t>();

        std::cout << group_idp << std::endl;

        auto rsc_idp = domain->Create<cor::Agent<void(idp_t)>>(group_idp, "", domain->GetModuleName(), "Test");
        std::cout << rsc_idp << std::endl;
        domain->Run<void(idp_t)>(rsc_idp, 0);

    }
}
