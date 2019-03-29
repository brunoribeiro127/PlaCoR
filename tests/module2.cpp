#include "cor/cor.hpp"

extern "C"
{
    void Dummy();
}

void Dummy()
{
    auto domain = cor::GetDomain();
    std::cout << domain->GetActiveResourceIdp() << std::endl;
}
