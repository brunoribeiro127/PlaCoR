#include "cor/elements/pod.hpp"

#include "cor/services/mailer.hpp"

using namespace dll;

namespace cor {

Pod::Pod(std::string const& app_group, std::string const& communicator, unsigned int npods) :
    _mlr{nullptr},
    _ctrl{nullptr},
    _modules{},
    _active_rscs{}
{
    // create a local instance of controller and mailer
    _mlr = new Mailer{app_group};
    _ctrl = new Controller{app_group, communicator, npods, _mlr};
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

idp_t Pod::Spawn(std::string const& comm, unsigned int npods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts)
{
    auto parent = GetActiveResourceIdp();
    return _ctrl->Spawn(comm, npods, parent, module, args, hosts);
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

void Pod::SynchronizeCollectiveGroup(std::string const& comm)
{
    _ctrl->SynchronizeCollectiveGroup(comm);
}

}
