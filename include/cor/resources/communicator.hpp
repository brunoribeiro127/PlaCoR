#ifndef COR_COMMUNICATOR_HPP
#define COR_COMMUNICATOR_HPP

#include <string>

#include "cor/resources/resource.hpp"
#include "cor/elements/static_organizer.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

class Communicator: public Resource, public StaticOrganizer
{

friend class ResourceManager;
friend class cereal::access;

public:
    ~Communicator();

    Communicator(const Communicator&) = delete;
    Communicator& operator=(const Communicator&) = delete;

    Communicator(Communicator&&) noexcept;
    Communicator& operator=(Communicator&&) noexcept;

protected:
    Communicator();
    explicit Communicator(idp_t idp, std::string const& comm, unsigned int total_members, idp_t parent);

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

CEREAL_REGISTER_TYPE(cor::Communicator);

#endif
