#include <iostream>

#include "cor/cor.hpp"
#include "cor/resources/agent.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/data.hpp"
#include "cor/resources/communicator.hpp"
#include "cor/message.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

COR_REGISTER_TYPE(cor::Data<std::vector<int>>);

static constexpr std::size_t ARRAY_SIZE = 5000;
static constexpr std::size_t NUM_AGENTS = 5;

template <typename T>
using Vector = cor::Data<std::vector<T>>;

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto agent_idp = domain->GetActiveResourceIdp();
    auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    cor::ResourcePtr<Vector<int>> data;

    if (domain->Idp() == cor::MasterDomain) {

        std::vector<int> array;
        for (int i = 1; i <= ARRAY_SIZE; ++i)
            array.push_back(i);

        data = domain->CreateLocal<Vector<int>>(domain->Idp(), "data", std::ref(array));

        auto comm_idp = domain->Spawn("test", NUM_AGENTS, "$HOME/placor/examples/libvecsum_spawn.dylib", {}, { "localhost" });

        auto comm = domain->CreateReference<cor::Communicator>(comm_idp, domain->Idp(), "comm");

        cor::Message smsg;
        smsg.SetType(0);
        smsg.Add<idp_t>(data->Idp());

        for (int i = 0; i < NUM_AGENTS; ++i)
            agent->Send(comm->GetIdp(i), smsg);

        std::size_t acc = 0;

        for (int i = 0; i < NUM_AGENTS; ++i) {
            auto rmsg = agent->Receive();
            auto res = rmsg.Get<std::size_t>();
            acc += res;
        }

        std::cout << "Global Sum -> " << acc << std::endl;

    } else {

        auto comm_idp = domain->GetPredecessorIdp(agent_idp);
        auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);
        auto rank = comm->GetIdm(agent_idp);

        auto rmsg = agent->Receive();
        auto data_idp = rmsg.Get<idp_t>();

        data = domain->CreateReference<Vector<int>>(data_idp, domain->Idp(), "data");

        std::size_t acc = 0;

        data->AcquireRead();
        auto array = data->Get();

        for (int i = rank * (ARRAY_SIZE/NUM_AGENTS); i < ((rank + 1) * (ARRAY_SIZE/NUM_AGENTS)); ++i)
            acc += array[i];

        data->ReleaseRead();

        cor::Message smsg;
        smsg.SetType(1);
        smsg.Add<std::size_t>(acc);
        agent->Send(comm->GetParent(), smsg);

    }
}
