#ifdef COR_CONTAINER_HPP

#include "cor/system/system.hpp"

namespace cor {

template <typename T, typename ... Args>
void Container::Create(idp_t ctx, std::string const& name, Args&& ... args)
{
    //return global::pod->Create<T>(ctx, name, std::forward<Args>(args)...);
}

}

#endif
