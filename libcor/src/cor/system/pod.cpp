#include "cor/system/pod.hpp"

#include "cor/services/mailer.hpp"

using namespace dll;

namespace cor {

Pod::Pod(std::string const& app_group, std::string const& context, unsigned int npods) :
    _mlr{nullptr},
    _ctrl{nullptr},
    _modules{},
    _active_rscs{}
{
    // create random id
    std::string id = random_string(9);

    // create a local instance of controller and mailer
    _mlr = new Mailer{id, app_group};
    _ctrl = new Controller{id, app_group, context, npods, _mlr};
}


Pod::~Pod()
{
    delete _ctrl;
    delete _mlr;
}

void Pod::Initialize()
{
    // start mailer and controller services
    _mlr->StartService();
    _ctrl->StartService();
}

void Pod::Finalize()
{
    // stop controller and mailer services
    _ctrl->StopService();
    _mlr->StopService();
}

std::string const& Pod::GetGlobalContext()
{
    return _ctrl->GetGlobalContext();
}

std::string const& Pod::GetLocalContext()
{
    return _ctrl->GetLocalContext();
}

unsigned int Pod::GetTotalPods()
{    
    return _ctrl->GetTotalPods();
}

unsigned int Pod::GetTotalDomains()
{
    return _ctrl->GetTotalDomains();
}

idp_t Pod::GetActiveResourceIdp()
{
    std::unique_lock<std::mutex> lk(_mtx); // shared_lock
    return _active_rscs.at(std::this_thread::get_id());
}

idp_t Pod::GetDomainIdp()
{
    auto idp = GetActiveResourceIdp();
    return _ctrl->GetDomainIdp(idp);
}

idp_t Pod::GetDomainIdp(idp_t idp)
{
    return _ctrl->GetDomainIdp(idp);
}

idp_t Pod::GetPredecessorIdp(idp_t idp)
{
    return _ctrl->GetPredecessorIdp(idp);
}

idp_t Pod::Spawn(std::string const& context, unsigned int npods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts)
{
    auto parent = GetActiveResourceIdp();
    return _ctrl->Spawn(context, npods, parent, module, args, hosts);
}

idp_t Pod::GenerateIdp()
{
    return _ctrl->GenerateIdp();
}

ConsistencyObject *Pod::GetConsistencyObject(idp_t idp)
{
    return _ctrl->GetConsistencyObject(idp);
}

void Pod::LoadModule(std::string const& module)
{
    std::unique_lock<std::mutex> lk(_mtx);
    if (_modules.find(module) == _modules.end()) {
        auto dylib = DynamicLoader::LoadDynamicLibrary(module);
        _modules.emplace(module, dylib);
    }
}

void Pod::InsertActiveResource(std::thread::id tid, idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);
    if (_active_rscs.find(tid) == _active_rscs.end())
        _active_rscs.emplace(tid, idp);
}

void Pod::RemoveActiveResource(std::thread::id tid)
{
    std::unique_lock<std::mutex> lk(_mtx);
    _active_rscs.erase(tid);
}

void Pod::ChangeActiveResource(std::thread::id tid, idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);
    _active_rscs.at(tid) = idp;
}

idp_t Pod::GetCurrentActiveResource(std::thread::id tid)
{
    std::unique_lock<std::mutex> lk(_mtx);
    return _active_rscs.at(tid);
}

void Pod::SendMessage(idp_t idp, idp_t dest, Message& msg)
{
    _mlr->SendMessage(idp, dest, msg);
}

void Pod::SendMessage(idp_t idp, std::vector<idp_t> const& dests, Message& msg)
{
    _mlr->SendMessage(idp, dests, msg);
}

Message Pod::ReceiveMessage(idp_t idp)
{
    return _mlr->ReceiveMessage(idp);
}

Message Pod::ReceiveMessage(idp_t idp, idp_t source)
{
    return _mlr->ReceiveMessage(idp, source);
}

void Pod::CreateStaticGroup(idp_t comm, unsigned int total_members)
{
    _ctrl->CreateStaticGroup(comm, total_members);
}

void Pod::SynchronizeStaticGroup(idp_t comm)
{
    _ctrl->SynchronizeStaticGroup(comm);
}

}
