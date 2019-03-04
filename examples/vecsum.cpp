#include <iostream>

#include "cor/cor.hpp"
#include "cor/resources/domain.hpp"
#include "cor/resources/communicator.hpp"
#include "cor/resources/agent.hpp"
#include "cor/resources/data.hpp"
#include "cor/resources/group.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

COR_REGISTER_TYPE(cor::Data<std::vector<int>>);

static constexpr std::size_t ARRAY_SIZE = 5000;
static constexpr idm_t MASTER = 0;

template <typename T>
using Vector = cor::Data<std::vector<T>>;

void Main(int argc, char *argv[])
{
    // get local agent idp and resource
    auto agent_idp = gPod->GetActiveResourceIdp();
    auto agent = gPod->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);

    // get agent rank
    auto comm_idp = gPod->GetPredecessorIdp(agent_idp);
    auto comm = gPod->GetLocalResource<cor::Communicator>(comm_idp);
    auto comm_size = comm->GetTotalMembers();
    auto rank = comm->GetIdm(agent_idp);

    cor::ResourcePtr<Vector<int>> data;

    if (rank == MASTER) {
        std::vector<int> array;

        for (int i = 1; i <= ARRAY_SIZE; ++i)
            array.push_back(i);

        data = gPod->CreateLocal<Vector<int>>(gPod->GetDomainIdp(), "data", std::ref(array));

        cor::Message msg;
        msg.SetType(0);
        msg.Add<idp_t>(data->Idp());

        for (int i = 1; i < comm_size; ++i)
            agent->Send(comm->GetIdp(i), msg);

    } else {

        auto msg = agent->Receive();
        auto data_idp = msg.Get<idp_t>();

        data = gPod->CreateReference<Vector<int>>(data_idp, gPod->GetDomainIdp(), "data");
    }

    std::size_t acc = 0;

    data->AcquireRead();
    auto array = data->Get();

    for (int i = rank * (ARRAY_SIZE/comm_size); i < ((rank + 1) * (ARRAY_SIZE/comm_size)); ++i)
        acc += array[i];

    data->ReleaseRead();

    if (rank == MASTER) {
        for (int i = 1; i < comm_size; ++i) {
            auto msg = agent->Receive();
            auto res = msg.Get<std::size_t>();
            acc += res;
        }
        std::cout << "Global Sum -> " << acc << std::endl;
    } else {
        cor::Message msg;
        msg.SetType(1);
        msg.Add<std::size_t>(acc);
        agent->Send(comm->GetIdp(0), msg);
    }
}
