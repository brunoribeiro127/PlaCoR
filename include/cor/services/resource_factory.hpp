#ifndef COR_RESOURCE_FACTORY_HPP
#define COR_RESOURCE_FACTORY_HPP

#include "cor/services/resource_ptr.hpp"

namespace cor {

template <typename T>
class ResourceFactory
{

public:
    template <typename ... Args>
    static ResourcePtr<T> Create(idp_t ctx, std::string const& name, bool global, Args&&... args);

};

}

#include "cor/services/resource_factory.tpp"

#endif
