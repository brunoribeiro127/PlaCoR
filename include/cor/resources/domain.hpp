#ifndef COR_DOMAIN_HPP
#define COR_DOMAIN_HPP

#include <string>

#include "cor/resources/resource.hpp"
#include "cor/services/resource_factory.hpp"
#include "cor/elements/organizer.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

class Domain: public Resource, public ResourceFactory<Domain>, public Organizer
{

friend class ResourceFactory<Domain>;
friend class cereal::access;

friend class ResourceManager; // to create meta-domain

public:
    ~Domain();

    Domain(const Domain&) = delete;
    Domain& operator=(const Domain&) = delete;

    Domain(Domain&&) noexcept;
    Domain& operator=(Domain&&) noexcept;

protected:
    Domain();
    explicit Domain(idp_t idp, std::string const& module);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<Resource>(this), cereal::base_class<Organizer>(this));
    }

};

}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>

CEREAL_REGISTER_TYPE(cor::Domain);

#endif
