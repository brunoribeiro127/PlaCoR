#ifdef COR_VALUE_HPP

#include "cor/system/system.hpp"
#include "cor/system/pod.hpp"

namespace cor {

template <typename T>
Value<T>::Value() = default;

template <typename T>
template <typename ... Args>
Value<T>::Value(idp_t idp, Args&&... args) : _idp{idp}, _value{std::forward<Args>(args)...} {}

template <typename T>
Value<T>::~Value() = default;

template <typename T>
Value<T>::Value(Value<T>&&) noexcept = default;

template <typename T>
Value<T>& Value<T>::operator=(Value<T>&&) noexcept = default;

template <typename T>
void Value<T>::AcquireWrite() const
{
    // acquire write operation
    global::pod->GetConsistencyObject(_idp)->AcquireWrite();
}

template <typename T>
void Value<T>::ReleaseWrite() const
{
    // release write operation
    global::pod->GetConsistencyObject(_idp)->ReleaseWrite();
}

template <typename T>
void Value<T>::AcquireRead() const
{
    // acquire read operation
    global::pod->GetConsistencyObject(_idp)->AcquireRead();
}

template <typename T>
void Value<T>::ReleaseRead() const
{
    // release read operation
    global::pod->GetConsistencyObject(_idp)->ReleaseRead();
}

template <typename T>
T& Value<T>::Get()
{
    return _value;
}

}

#endif
