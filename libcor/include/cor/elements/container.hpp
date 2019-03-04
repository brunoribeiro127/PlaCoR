#ifndef COR_CONTAINER_HPP
#define COR_CONTAINER_HPP

#include "cereal/access.hpp"

#include "cor/system/macros.hpp"

namespace cor {

template <typename> class ResourcePtr;

class Container
{

friend class cereal::access;

public:
    virtual ~Container();

    Container(const Container&) = delete;
    Container& operator=(const Container&) = delete;

    Container(Container&&) noexcept;
    Container& operator=(Container&&) noexcept;

    template <typename T, typename ... Args>
    //ResourcePtr<T> Create(idp_t ctx, std::string const& name, Args&& ... args);
    void Create(idp_t ctx, std::string const& name, Args&& ... args);

protected:
    Container();
    explicit Container(idp_t idp);

private:
    template <class Archive>
    void serialize(Archive& ar) 
    {
        ar(_idp);
    }

    idp_t _idp;

};

}

#include "cor/elements/container.tpp"

#endif
