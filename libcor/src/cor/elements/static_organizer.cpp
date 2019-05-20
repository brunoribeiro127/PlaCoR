#include "cor/elements/static_organizer.hpp"

#include "cor/system/system.hpp"
#include "cor/system/pod.hpp"
#include "cor/services/consistency_object.hpp"
#include "cor/message.hpp"

#include <algorithm>

namespace cor {

StaticOrganizer::StaticOrganizer() = default;

StaticOrganizer::StaticOrganizer(idp_t idp, unsigned int total_members, idp_t parent) :
    _idp{idp},
    _total_members{total_members},
    _parent{parent},
    _members{},
    _next_idm{0}
{}

StaticOrganizer::~StaticOrganizer() = default;

StaticOrganizer::StaticOrganizer(StaticOrganizer&&) noexcept = default;

StaticOrganizer& StaticOrganizer::operator=(StaticOrganizer&&) noexcept = default;

void StaticOrganizer::Join(idp_t idp, std::string const& name)
{
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);

    // acquire write
    cobj->AcquireWrite();

    // verify if the resource has already been attached
    if (_members.find(idp) != _members.end())
        throw std::logic_error("Resource " + std::to_string(idp) + " already joined!");

    // verify if the name of the resource is unique in the context of the ancestor
    for (const auto& p: _members) {
        if (p.second.second == name)
            throw std::logic_error("Resource name <" + name + "> is not unique in the context of the ancestor!");
    }

    // generate new idm
    auto idm = _next_idm++;

    // insert new resource
    if (name.empty())
        _members.emplace(idp, std::make_pair(idm, std::to_string(idp)));
    else
        _members.emplace(idp, std::make_pair(idm, name));

    // release write
    cobj->ReleaseWrite();

    // Synchronize join
    global::pod->CreateStaticGroup(_idp, _total_members);

    if (idm == 0 && _parent != 0) {
        Message msg;
        global::pod->SendMessage(_idp, _parent, msg);
    }
}

void StaticOrganizer::Leave(idp_t idp)
{
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);

    // acquire write
    cobj->AcquireWrite();

    // erase resource idp
    _members.erase(idp);

    // release write
    cobj->ReleaseWrite();
}

idp_t StaticOrganizer::GetParent() const
{
    return _parent;
}

size_t StaticOrganizer::GetTotalMembers() const
{
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);

    // acquire read
    cobj->AcquireRead();

    auto size = _members.size();

    // release read
    cobj->ReleaseRead();

    return size;
}

std::vector<idp_t> StaticOrganizer::GetMemberList() const
{
    std::vector<idp_t> list;

    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);
    
    // acquire read
    cobj->AcquireRead();

    for (auto const& member: _members)
        list.push_back(member.first);

    // release read
    cobj->ReleaseRead();

    return list;
}

idp_t StaticOrganizer::GetIdp(idm_t idm) const
{
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);
    
    // acquire read
    cobj->AcquireRead();

    auto it = std::find_if(_members.begin(), _members.end(),
        [idm](auto&& member) -> bool {
            return member.second.first == idm;
        });

    if (it != _members.end()) {

        auto idp = it->first;
    
        // release read
        cobj->ReleaseRead();

        return idp;
    } else {
        throw std::runtime_error("Resource with idm <" + std::to_string(idm) + "> does not exist!");
    }
}

idp_t StaticOrganizer::GetIdp(std::string const& name) const
{
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);
    
    // acquire read
    cobj->AcquireRead();

    auto it = std::find_if(_members.begin(), _members.end(),
        [name](auto&& member) -> bool {
            return member.second.second == name;
        });

    if (it != _members.end()) {

        auto idp = it->first;
    
        // release read
        cobj->ReleaseRead();

        return idp;
    } else {
        throw std::runtime_error("Resource with name <" + name + "> does not exist!");
    }
}

idm_t StaticOrganizer::GetIdm(idp_t idp) const
{
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);
    
    // acquire read
    cobj->AcquireRead();

    auto idm = _members.at(idp).first;

    // release read
    cobj->ReleaseRead();

    return idm;
}

idm_t StaticOrganizer::GetIdm(std::string const& name) const
{
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);
    
    // acquire read
    cobj->AcquireRead();

    auto it = std::find_if(_members.begin(), _members.end(),
        [name](auto&& member) -> bool {
            return member.second.second == name;
        });

    if (it != _members.end()) {

        auto idm = it->second.first;
    
        // release read
        cobj->ReleaseRead();

        return idm;
    } else {
        throw std::runtime_error("Resource with name <" + name + "> does not exist!");
    }
}

}
