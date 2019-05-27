#include <iostream>

#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);

    std::cout << "DOMAIN \t\t IDP <" << domain->Idp() << "> ALIAS <" << domain.Idp() << ">" << std::endl;
    std::cout << "COMMUNICATOR \t IDP <" << comm->Idp() << "> ALIAS <" << comm.Idp() << ">" << std::endl;
    std::cout << "AGENT \t\t IDP <" << agent->Idp() << "> ALIAS <" << agent.Idp() << ">" << std::endl;

    auto original_comm_predecessor = domain->GetPredecessorIdp(comm->Idp());
    std::cout << "ORIGINAL COMM PREDECESSOR -> " << original_comm_predecessor << std::endl;
}
