#ifndef COR_VALUE_HPP
#define COR_VALUE_HPP

#include "cor/system/macros.hpp"

namespace cor {

template <typename T>
class Value
{

friend class cereal::access;

public:
    ~Value();

    Value(const Value&) = delete;
    Value& operator=(const Value&) = delete;

    Value(Value&&) noexcept;
    Value& operator=(Value&&) noexcept;

    void AcquireWrite() const;
    void ReleaseWrite() const;

    void AcquireRead() const;
    void ReleaseRead() const;

    T* Get();

protected:
    Value();

    template <typename ... Args>
    explicit Value(idp_t idp, Args&&... args);

private:
    template <typename Archive>
    void serialize(Archive& ar) 
    {
        ar(_idp, _value);
    }

    idp_t _idp;
    std::unique_ptr<T> _value;

};

}

#include "cor/elements/value.tpp"

#endif
