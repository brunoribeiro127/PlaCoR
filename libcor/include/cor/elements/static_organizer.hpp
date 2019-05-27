#ifndef COR_STATIC_ORGANIZER_HPP
#define COR_STATIC_ORGANIZER_HPP

#include <string>
#include <map>
#include <vector>

#include "cereal/types/string.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/utility.hpp"

#include "cor/system/macros.hpp"

namespace cor {

class StaticOrganizer
{

friend class cereal::access;

public:
    virtual ~StaticOrganizer();

    StaticOrganizer(const StaticOrganizer&) = delete;
    StaticOrganizer& operator=(const StaticOrganizer&) = delete;

    StaticOrganizer(StaticOrganizer&&) noexcept;
    StaticOrganizer& operator=(StaticOrganizer&&) noexcept;

    void Join(idp_t idp, std::string const& name);
    void Leave(idp_t idp);

    idp_t GetParent() const;
    std::size_t GetTotalMembers() const;
    std::vector<idp_t> GetMemberList() const;

    idp_t GetIdp(idm_t idm) const;
    idp_t GetIdp(std::string const& name) const;

    idm_t GetIdm(idp_t idp) const;
    idm_t GetIdm(std::string const& name) const;

protected:
    StaticOrganizer();
    explicit StaticOrganizer(idp_t idp, unsigned int total_members, idp_t parent);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(_idp, _total_members, _parent, _members, _next_idm);
    }

    idp_t _idp;
    unsigned int _total_members;
    idp_t _parent;
    std::map<idp_t, std::pair<idm_t, std::string>> _members;
    idm_t _next_idm;

};

}

#endif
