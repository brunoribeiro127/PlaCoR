#ifdef COR_RESOURCE_FACTORY_HPP

#include "cor/system/system.hpp"
#include "cor/elements/pod.hpp"

namespace cor {

template <typename T>
template <typename ... Args>
ResourcePtr<T> ResourceFactory<T>::Create(idp_t ctx, std::string const& name, bool global, Args&& ... args)
{
    // se for dominio tem de se rever a politica de criação!!!
    auto idp = global::pod->GenerateIdp();
    auto rsc = new T(idp, std::forward<Args>(args)...);
    return global::pod->AllocateResource<T>(idp, ctx, name, global, rsc);
}

}

#endif
