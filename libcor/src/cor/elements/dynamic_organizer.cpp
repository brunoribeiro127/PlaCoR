#include "cor/elements/dynamic_organizer.hpp"

#include "cor/system/system.hpp"
#include "cor/services/consistency_object.hpp"

#include <algorithm>

namespace cor {

DynamicOrganizer::DynamicOrganizer() = default;

DynamicOrganizer::DynamicOrganizer(idp_t idp, std::string const& module) :
    _idp{idp},
    _module{module},
    _members{},
    _next_idm{0}
{
    LoadModule();
}

DynamicOrganizer::~DynamicOrganizer() = default;

DynamicOrganizer::DynamicOrganizer(DynamicOrganizer&&) noexcept = default;

DynamicOrganizer& DynamicOrganizer::operator=(DynamicOrganizer&&) noexcept = default;

void DynamicOrganizer::Join(idp_t idp, std::string const& name)
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
}

void DynamicOrganizer::Leave(idp_t idp)
{
    
}

void DynamicOrganizer::LoadModule() const
{
    // if has a module, then load it
    if (!_module.empty())
        global::pod->LoadModule(_module);
}

std::string const& DynamicOrganizer::GetModuleName() const
{
    return _module;
}

size_t DynamicOrganizer::GetTotalMembers() const
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

idp_t DynamicOrganizer::GetIdp(idm_t idm) const
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

idp_t DynamicOrganizer::GetIdp(std::string const& name) const
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

idm_t DynamicOrganizer::GetIdm(idp_t idp) const
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

idm_t DynamicOrganizer::GetIdm(std::string const& name) const
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
