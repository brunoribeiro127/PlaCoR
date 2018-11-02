#include "cor/elements/organizer.hpp"

#include "cor/elements/pod.hpp"
#include "cor/services/consistency_object.hpp"
#include "cor/system/system.hpp"

namespace cor {

Organizer::Organizer() = default;

Organizer::Organizer(idp_t idp, std::string const& module) :
    _idp{idp},
    _module{module},
    _next_idm{0}
{}

Organizer::~Organizer() = default;

Organizer::Organizer(Organizer&&) noexcept = default;

Organizer& Organizer::operator=(Organizer&&) noexcept = default;

void Organizer::LoadModule() const
{
    // if has a module, then load it
    if (!_module.empty())
        global::pod->LoadModule(_module);
}

std::string const& Organizer::GetModuleName() const
{
    return _module;
}

size_t Organizer::GetTotalMembers() const
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

idm_t Organizer::GetIdm(idp_t idp) const
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

void Organizer::Attach(idp_t idp, std::string const& name)
{   
    // get consistency object
    auto cobj = global::pod->GetConsistencyObject(_idp);

    // acquire write
    cobj->AcquireWrite();
/*
    std::cout << "----------\n";
    std::cout << "Organizer: " << _idp << "\n";
    for (const auto& p: _members)
        std::cout << p.first << "\n";
    std::cout << "----------\n";
*/
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

}
