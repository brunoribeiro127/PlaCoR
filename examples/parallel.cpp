#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

static constexpr idm_t MASTER = 0;

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto agent_idp = domain->GetActiveResourceIdp();
    auto clos_idp = domain->GetPredecessorIdp(agent_idp);
    auto clos = domain->GetLocalResource<cor::Closure>(clos_idp);
    auto clos_size = clos->GetTotalMembers();
    auto rank = clos->GetIdm(agent_idp);

    if (clos->GetParent() == 0) {
        auto new_clos_idp = domain->Spawn("ctx2", 2, "~/placor/examples/libex5.dylib", {}, { "localhost" });
    }

    if (rank == MASTER) {
        std::cout << "<" << clos_idp << "> <" << domain->GetLocalContext() << "> <" << domain->GetGlobalContext() << ">\n";
    }
}
