#ifndef COR_MUTEX_HPP
#define COR_MUTEX_HPP

#include "cor/resources/resource.hpp"
#include "cor/services/resource_factory.hpp"
#include "cor/elements/synchronizer.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

class Mutex: public Resource, public ResourceFactory<Mutex>, public Synchronizer
{

friend class ResourceFactory<Mutex>;
friend class cereal::access;

public:
    ~Mutex();

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    Mutex(Mutex&&) noexcept;
    Mutex& operator=(Mutex&&) noexcept;

protected:
    Mutex();
    explicit Mutex(idp_t idp);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<Resource>(this), cereal::base_class<Synchronizer>(this));
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

CEREAL_REGISTER_TYPE(cor::Mutex);

#endif
