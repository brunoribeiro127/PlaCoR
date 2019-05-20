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
    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);
    auto comm_size = comm->GetTotalMembers();
    auto rank = comm->GetIdm(agent_idp);

    if (comm->GetParent() == 0) {
        auto new_comm_idp = domain->Spawn("ctx2", 2, "~/placor/examples/libex5.dylib", {}, { "localhost" });
    }

    if (rank == MASTER) {
        std::cout << "<" << comm_idp << "> <" << domain->GetLocalContext() << "> <" << domain->GetGlobalContext() << ">\n";
    }
}
