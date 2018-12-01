#ifndef COR_RESOURCE_HPP
#define COR_RESOURCE_HPP

#include "cor/system/macros.hpp"

#include "cereal/types/polymorphic.hpp"

namespace cor {

class Resource
{

friend class cereal::access;

public:
    virtual ~Resource();

    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    Resource(Resource&&) noexcept;
    Resource& operator=(Resource&&) noexcept;

protected:
    Resource(); // needed by cereal
    explicit Resource(idp_t idp);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(_idp);
    }

protected:
    idp_t _idp;

};

}

#endif
