#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

void Main(int argc, char *argv[])
{
    // obter o domínio local
    auto domain = cor::GetDomain();

    // obter o idp do agent que está a executar a função de entrada do módulo do utilizador
    auto agent_idp = domain->GetActiveResourceIdp();

    // obter o identificador do comunicador
    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    // obter uma referência para o comunicador
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);
    // obter o número total de agente que fazem parte do comunicador
    auto comm_size = comm->GetTotalMembers();
    // obter o rank do agente no comunicador
    auto rank = comm->GetIdm(agent_idp);

    std::cout << "Parent " << comm->GetParent() << "\n";

    std::cout << "Agent " << agent_idp << " with rank " << rank << " in Communicator " << comm_idp << " with a total of " << comm_size  << " members\n";
}
