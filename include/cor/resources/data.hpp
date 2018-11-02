#ifndef COR_DATA_HPP
#define COR_DATA_HPP

#include "cor/resources/resource.hpp"
#include "cor/services/resource_factory.hpp"
#include "cor/elements/value.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

template <typename T>
class Data: public Resource, public ResourceFactory<Data<T>>, public Value<T>
{

friend class ResourceFactory<Data<T>>;
friend class cereal::access;

public:    
    ~Data();

    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;

    Data(Data&&) noexcept;
    Data& operator=(Data&&) noexcept;

protected:
    Data();

    template <typename ... Args>
    explicit Data(idp_t idp, Args&&... args);

private:
	template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<Resource>(this), cereal::base_class<Value<T>>(this));
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

#include "cor/resources/data.tpp"

#endif
