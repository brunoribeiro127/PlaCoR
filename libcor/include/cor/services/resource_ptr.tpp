#ifdef COR_RESOURCE_PTR_HPP

namespace cor {

template <typename T>
ResourcePtr<T>::ResourcePtr() :
    _cst_obj{nullptr}
{}

template <typename T>
ResourcePtr<T>::ResourcePtr(ConsistencyObject *cst_obj) :
    _cst_obj{cst_obj}
{
    if (_cst_obj != nullptr)
        _cst_obj->IncrementLocalReferenceCounter();
}

template <typename T>
ResourcePtr<T>::~ResourcePtr()
{
    if (_cst_obj != nullptr)
        _cst_obj->DecrementLocalReferenceCounter();
}

template <typename T>
ResourcePtr<T>::ResourcePtr(ResourcePtr<T> const& other) noexcept :
    _cst_obj{other._cst_obj}
{
    if (_cst_obj != nullptr)
        _cst_obj->IncrementLocalReferenceCounter();
}

template <typename T>
ResourcePtr<T>& ResourcePtr<T>::operator=(ResourcePtr<T> const& other) noexcept
{
    _cst_obj = other._cst_obj;
    other._cst_obj = nullptr;
    return *this;
}

template <typename T>
ResourcePtr<T>::ResourcePtr(ResourcePtr<T>&& other) noexcept :
    _cst_obj{other._cst_obj}
{
    if (_cst_obj != nullptr)
        _cst_obj->IncrementLocalReferenceCounter();
}

template <typename T>
ResourcePtr<T>& ResourcePtr<T>::operator=(ResourcePtr&& other) noexcept
{
    _cst_obj = other._cst_obj;
    other._cst_obj = nullptr;
    return *this;
}

template <typename T>
T* ResourcePtr<T>::operator->() const
{
    if (_cst_obj == nullptr)
        throw std::runtime_error("Access to an invalid resource!");

    return dynamic_cast<T*>(_cst_obj->GetResource());
}

template <typename T>
T& ResourcePtr<T>::operator*() const
{
    if (_cst_obj == nullptr)
        throw std::runtime_error("Access to an invalid resource!");

    return *(dynamic_cast<T*>(_cst_obj->GetResource()));
}

}

#endif
