#ifndef COR_GROUP_HPP
#define COR_GROUP_HPP

#include <string>

#include "cor/resources/resource.hpp"
#include "cor/elements/dynamic_organizer.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

class Group: public Resource, public DynamicOrganizer
{

friend class ResourceManager;
friend class cereal::access;

public:
    ~Group();

    Group(const Group&) = delete;
    Group& operator=(const Group&) = delete;

    Group(Group&&) noexcept;
    Group& operator=(Group&&) noexcept;

protected:
    Group();
    explicit Group(idp_t idp, std::string const& module);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<Resource>(this), cereal::base_class<DynamicOrganizer>(this));
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

CEREAL_REGISTER_TYPE(cor::Group);

#endif
