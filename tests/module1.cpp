#include "cor/cor.hpp"

extern "C"
{
    void Main(int argc, char *argv[]);
}

void Main(int argc, char *argv[])
{
    auto domain = cor::GetDomain();

    auto group = domain->CreateLocal<cor::Group>(domain->Idp(), "group", "~/placor/tests/libmodule2.dylib");

    auto agent = domain->CreateLocal<cor::Agent<void()>>(group->Idp(), "agent", "~/placor/tests/libmodule2.dylib", "Dummy");
    agent->Run();
    agent->Wait();
    agent->Get();
}
