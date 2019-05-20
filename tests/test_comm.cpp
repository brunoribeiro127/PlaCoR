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
    auto comm_size = comm->GetTotalMembers();
    auto rank = comm->GetIdm(agent_idp);

    cor::Message msg;
    msg.Add<std::string>("Mensagem enviada por " + std::to_string(agent_idp)
        + " com rank " + std::to_string(rank) + " em " + std::to_string(comm_idp));

    if (rank == 0) {
        agent->Send(((rank + 1) % comm_size), comm_idp, msg);
        
        {
            auto msg = agent->Receive((comm_size - 1), comm_idp);
            auto content = msg.Get<std::string>();
            std::cout << content << std::endl;
        }
    } else {
        {
            auto msg = agent->Receive((rank - 1), comm_idp);
            auto content = msg.Get<std::string>();
            std::cout << content << std::endl;
        }

        agent->Send(((rank + 1) % comm_size), comm_idp, msg);
    }

}
