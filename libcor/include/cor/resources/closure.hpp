#ifndef COR_CLOSURE_HPP
#define COR_CLOSURE_HPP

#include "cor/resources/resource.hpp"
#include "cor/elements/static_organizer.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

class Closure: public Resource, public StaticOrganizer
{

friend class ResourceManager;
friend class cereal::access;

public:
    ~Closure();

    Closure(const Closure&) = delete;
    Closure& operator=(const Closure&) = delete;

    Closure(Closure&&) noexcept;
    Closure& operator=(Closure&&) noexcept;

protected:
    Closure();
    explicit Closure(idp_t idp, unsigned int total_members, idp_t parent);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<Resource>(this), cereal::base_class<StaticOrganizer>(this));
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

CEREAL_REGISTER_TYPE(cor::Closure);

#endif
