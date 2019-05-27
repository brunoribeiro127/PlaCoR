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

    std::string const& GetGlobalContext();
    std::string const& GetLocalContext();

    unsigned int GetTotalPods();
    unsigned int GetTotalDomains();

    idp_t GetActiveResourceIdp();
    idp_t GetPredecessorIdp(idp_t idp);

    template <typename T>
    ResourcePtr<T> GetLocalResource(idp_t idp);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateLocal(idp_t ctx, std::string const& name, Args&& ... args);

    template <typename T, typename ... Args>
    idp_t CreateRemote(idp_t ctx, std::string const& name, Args&& ... args);

    template <typename T, typename ... Args>
    idp_t Create(idp_t ctx, std::string const& name, Args&& ... args);

    template <typename T>
    ResourcePtr<T> CreateReference(idp_t idp, idp_t ctx, std::string const& name);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateCollective(idp_t ctx, std::string const& name, unsigned int total_members, Args&& ... args);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateCollective(idp_t clos, idp_t ctx, std::string const& name, Args&& ... args);

    template <typename T, typename ... Args>
    void Run(idp_t idp, Args&&... args);

    template <typename T>
    void Wait(idp_t idp);

    template <typename T>
    auto Get(idp_t idp);

    idp_t Spawn(std::string const& context, unsigned int npods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts);

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
