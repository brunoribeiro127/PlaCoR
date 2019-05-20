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

    idp_t GetIdp() const;

protected:
    explicit ResourcePtr(ConsistencyObject *cst_obj);

private:
    ConsistencyObject *_cst_obj;

};

}

#include "cor/services/resource_ptr.tpp"

#endif
