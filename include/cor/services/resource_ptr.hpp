#ifndef COR_RESOURCE_PTR_HPP
#define COR_RESOURCE_PTR_HPP

#include "cor/system/macros.hpp"

#include "cor/services/resource_manager.hpp"

namespace cor {

template <typename T>
class ResourcePtr
{

friend class ResourceManager;

public:
    ResourcePtr();
    ~ResourcePtr();

    ResourcePtr(ResourcePtr const&) noexcept;
    ResourcePtr& operator=(ResourcePtr const&) noexcept;

    ResourcePtr(ResourcePtr&&) noexcept;
    ResourcePtr& operator=(ResourcePtr&&) noexcept;

    T* operator->() const;
    T& operator*() const;

    idp_t GetIdp() const;

    // Cast operator to implicitely convert the resource pointer to its raw pointer type.
    //operator T*();

protected:
    explicit ResourcePtr(ResourceManager *rsc_mgr, idp_t idp);

private:
    ResourceManager *_rsc_mgr;
    idp_t _idp;

};

}

#include "cor/services/resource_ptr.tpp"

#endif
