#ifdef COR_RESOURCE_PTR_HPP

namespace cor {

template <typename T>
ResourcePtr<T>::ResourcePtr() :
    _rsc_mgr{nullptr},
    _idp{0}
{}

template <typename T>
ResourcePtr<T>::ResourcePtr(ResourceManager *rsc_mgr, idp_t idp) :
    _rsc_mgr(rsc_mgr),
    _idp(idp)
{
    // increment resource reference counter
    if (_rsc_mgr != nullptr)
        _rsc_mgr->IncrementResourceReferenceCounter(_idp);
}

template <typename T>
ResourcePtr<T>::~ResourcePtr()
{
    // decrement resource reference counter
    if (_rsc_mgr != nullptr)
        _rsc_mgr->DecrementResourceReferenceCounter(_idp);
}

template <typename T>
ResourcePtr<T>::ResourcePtr(ResourcePtr<T> const& other) noexcept :
    _rsc_mgr(other._rsc_mgr),
    _idp(other._idp)
{
    if (_rsc_mgr != nullptr)
        _rsc_mgr->IncrementResourceReferenceCounter(_idp);
}

template <typename T>
ResourcePtr<T>& ResourcePtr<T>::operator=(ResourcePtr<T> const& other) noexcept = default;

template <typename T>
ResourcePtr<T>::ResourcePtr(ResourcePtr<T>&& other) noexcept :
    _rsc_mgr(other._rsc_mgr),
    _idp(other._idp)
{
    if (_rsc_mgr != nullptr)    
        _rsc_mgr->IncrementResourceReferenceCounter(_idp);
}

template <typename T>
ResourcePtr<T>& ResourcePtr<T>::operator=(ResourcePtr&& other) noexcept = default;

template <typename T>
T* ResourcePtr<T>::operator->() const
{
    // protect against nullptr
    return dynamic_cast<T*>(_rsc_mgr->GetResource(_idp));
}

template <typename T>
T& ResourcePtr<T>::operator*() const
{
    // protect against nullptr
    return *(dynamic_cast<T*>(_rsc_mgr->GetResource(_idp)));
}

template <typename T>
idp_t ResourcePtr<T>::GetIdp() const
{
    return _idp;
}

}

#endif
