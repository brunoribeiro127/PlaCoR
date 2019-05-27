#ifndef COR_RESOURCE_PTR_HPP
#define COR_RESOURCE_PTR_HPP

namespace cor {

template <typename> class ResourcePtr;

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

    idp_t Idp() const;

protected:
    explicit ResourcePtr(idp_t idp, ConsistencyObject *cst_obj);

private:
    idp_t _idp;
    ConsistencyObject *_cst_obj;

};

}

#include "cor/services/resource_ptr.tpp"

#endif
