#include <iostream>

#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
    void TestVoid();
    idp_t TestIdp();
}

static constexpr idm_t MASTER = 0;

void TestVoid()
{
    auto domain = cor::GetDomain();
    std::cout << "WORKER: " << domain->GetActiveResourceIdp() << std::endl;
}

idp_t TestIdp()
{
    auto domain = cor::GetDomain();
    return domain->GetActiveResourceIdp();
}

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();
    
    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);

    {
        cor::Message msg;
        msg.Add<idp_t>(domain->Idp());
        agent->Send(comm->GetParent(), msg);
    }

    {
        auto msg = agent->Receive(comm->GetParent());
    }
}
