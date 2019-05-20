#include "cor/cor.hpp"

extern "C"
{
  void Main(int argc, char *argv[]);
  void TestClient(idp_t rsc_idp);
}

enum MsgType: std::uint16_t
{
    Wait,
    Stop
};

void TestClient(idp_t rsc_idp)
{
    std::cout << "Client: Trabalhando\n";
    auto domain = cor::GetDomain();
    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(idp_t)>>(agent_idp);
    std::cout << "Client: Trabalhando\n";

    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);
    auto parentAgent_id=comm->GetParent();
    
    // cor::Message msg;
    // msg.Add<std::string>("Hello");
    // agent->Send(rsc_idp, msg);
}
void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto remote_comm_idp = domain->Spawn("clientRPC", 1, "~/placor/examples/libserverRPC.dylib", {}, { "localhost" });

    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    auto server_msg = agent->Receive();
    auto remote_domain_idp = server_msg.Get<idp_t>();
    auto server_msg2 = agent->Receive();
    auto remote_agent_idp = server_msg2.Get<idp_t>();

    std::cout << "Client: remote_domain_idp " << remote_domain_idp << std::endl;
    std::cout << "Client: remote_comm_idp " << remote_comm_idp << std::endl;
    std::cout << "Client: remote_agent_idp " << remote_agent_idp << std::endl;

    auto rsc_idp = domain->Create<cor::Agent<void(idp_t)>>(remote_domain_idp, "rpcAgent", "/Users/brunoribeiro/placor/examples/libserverRPC.dylib", "TestServer");
    std::cout << "Client: AgentRPC " << rsc_idp << std::endl;
    domain->Run<cor::Agent<void(idp_t)>>(rsc_idp, agent_idp);

    auto worker_msg = agent->Receive();
    auto work = worker_msg.Get<std::string>();
    std::cout << work << " World\n";

    domain->Wait<cor::Agent<void(idp_t)>>(rsc_idp);
    domain->Get<cor::Agent<void(idp_t)>, void>(rsc_idp);

    {
        cor::Message msg;

        msg.SetType(MsgType::Wait);
        msg.Add<idp_t>(domain->Idp());
        agent->Send(remote_agent_idp, msg);

        msg.Clear();
        msg.SetType(MsgType::Stop);
        agent->Send(remote_agent_idp, msg);
    }

}
