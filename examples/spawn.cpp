#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto agent_idp = domain->GetActiveResourceIdp();
    auto clos_idp = domain->GetPredecessorIdp(agent_idp);
    auto clos = domain->GetLocalResource<cor::Closure>(clos_idp);
    auto clos_size = clos->GetTotalMembers();
    auto rank = clos->GetIdm(agent_idp);

    std::cout << "Parent " << clos->GetParent() << "\n";
    std::cout << "Agent " << agent_idp << " with rank " << rank << " in Closure " << clos_idp << " with a total of " << clos_size  << " members\n";
}
