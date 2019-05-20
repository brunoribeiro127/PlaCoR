#include "cor/cor.hpp"

extern "C"
{
  void Main(int argc, char *argv[]);
  void TestServer(idp_t rsc_idp); 
}

enum MsgType: std::uint16_t
{
    Wait,
    Stop
};

void TestServer(idp_t rsc_idp)
{
    std::cout << "WORKER: Trabalhando" << std::endl;
    auto domain = cor::GetDomain();
      
    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(idp_t)>>(agent_idp);
    std::cout << "WORKER: Trabalhando" << std::endl;
    cor::Message msg;
    msg.Add<std::string>("Hello");
    agent->Send(rsc_idp, msg);
 
    auto data_idp = domain->GetIdp("_dataIdp");
std::cout << "WORKER: data_idp " << data_idp << std::endl;
    //auto data = domain->CreateReference<cor::Data<idp_t>>(data_idp, domain->Idp(), "_data_");
    auto data = domain->GetLocalResource<cor::Data<idp_t>>(data_idp);
std::cout << "WORKER: data_idp " << data_idp << std::endl;
    data->AcquireRead();
    auto domain_client = *data->Get();
    data->ReleaseRead();
std::cout << "--> WORKER: " << domain_client << std::endl;
    auto agentClient_idp = domain->Create<cor::Agent<void(idp_t)>>(domain_client, "rpcAgent", "/Users/amp/placor/examples/libclientRPC.dylib", "TestClient");
    std::cout << " Domain_client: 1 TestServer " << domain_client  << std::endl;
    domain->Run<cor::Agent<void(idp_t)>>(agentClient_idp, agent_idp);
    std::cout << " Domain_client: 2 TestServer " << domain_client  << std::endl;
    domain->Wait<cor::Agent<void(idp_t)>>(agentClient_idp);
    domain->Get<cor::Agent<void(idp_t)>, void>(agentClient_idp);
}

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();
    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);

    std::cout << "DOMAIN: " << domain->Idp() << "\n";
    std::cout << "COMM: " << comm_idp << "\n";
    std::cout << "SERVER: " << agent_idp << "\n";

    auto data = domain->CreateLocal<cor::Data<idp_t>>(domain->Idp(), "_dataIdp", 0 );
    cor::Message msg;
    msg.Add<idp_t>(domain->Idp());
    agent->Send(comm->GetParent(), msg);                                         //Send a message to the client Agent
    msg.Clear();
    msg.Add<idp_t>(agent_idp);
    agent->Send(comm->GetParent(), msg);
    
   std::cout << " RPC Server: To terminate send a Null message to:" << agent_idp << std::endl;
    
    msg.Clear();
    msg  = agent->Receive();
    if (msg.Type() ==1) {
      
      auto domain_client=msg.Get<idp_t>();
      std::cout << "Receveive type1 domain_client" << domain_client << std::endl;
      auto value=data->Get();
      *value=domain_client;
      std::cout << " Domain_client  2" << domain_client  << std::endl;
      auto rmsg = agent->Receive();
    }
}
