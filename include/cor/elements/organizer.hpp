#ifndef COR_ORGANIZER_HPP
#define COR_ORGANIZER_HPP

#include <string>
#include <map>
#include <mutex>

#include "cereal/types/string.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/utility.hpp"

#include "cor/system/macros.hpp"

namespace cor {

class Organizer
{

friend class ResourceManager; // needed by attach and detach operation
friend class cereal::access;

public:
    virtual ~Organizer();

    Organizer(const Organizer&) = delete;
    Organizer& operator=(const Organizer&) = delete;

    Organizer(Organizer&&) noexcept;
    Organizer& operator=(Organizer&&) noexcept;

    void LoadModule() const;
    std::string const& GetModuleName() const;

    idm_t GetIdm(idp_t idp) const;

    size_t GetTotalMembers() const;

protected:
    Organizer();
    explicit Organizer(idp_t idp, std::string const& module);
    
    void Attach(idp_t idp, std::string const& name);
    //void Detach(idp_t idp);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(_idp, _module, _members, _next_idm);
    }

    idp_t _idp;
    std::string _module;
    std::map<idp_t, std::pair<idm_t, std::string>> _members;
    idm_t _next_idm;

};

}

#endif
