#ifdef COR_DATA_HPP

namespace cor {

template <typename T>
Data<T>::Data() = default;

template <typename T>
template <typename ... Args>
Data<T>::Data(idp_t idp, Args&&... args) : Resource{idp}, Value<T>{idp, std::forward<Args>(args)...} {}

template <typename T>
Data<T>::~Data() = default;

template <typename T>
Data<T>::Data(Data<T>&&) noexcept = default;

template <typename T>
Data<T>& Data<T>::operator=(Data<T>&&) noexcept = default;

}

#endif
