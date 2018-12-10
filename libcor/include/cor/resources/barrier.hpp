#ifndef COR_BARRIER_HPP
#define COR_BARRIER_HPP

#include <string>

#include "cor/resources/resource.hpp"
#include "cor/elements/sbarrier.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

class Barrier: public Resource, public SBarrier
{

friend class ResourceManager;
friend class cereal::access;

public:
    ~Barrier();

    Barrier(const Barrier&) = delete;
    Barrier& operator=(const Barrier&) = delete;

    Barrier(Barrier&&) noexcept;
    Barrier& operator=(Barrier&&) noexcept;

protected:
    Barrier();
    explicit Barrier(idp_t idp, std::string const& comm);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<Resource>(this), cereal::base_class<SBarrier>(this));
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

CEREAL_REGISTER_TYPE(cor::Barrier);

#endif
