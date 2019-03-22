#include "cor/services/controller.hpp"

#include "cor/utils/utils.hpp"
#include "cor/services/page_manager.hpp"
#include "cor/services/session_manager.hpp"

#include <cereal/archives/portable_binary.hpp>

#include <thread>
#include <sstream>

#include <iostream> // to remove

using namespace ssrc::spread;
using namespace ev;

namespace cor {

Controller::Controller(std::string const& id, std::string const& app_group, std::string const& context, unsigned int npods, Mailer *mlr) :
    _app_group{app_group},
    _context{context},
    _npods{npods},
    _th_svc{},
    _is_main_ctrl{false},
    _psync{},
    _fsync{},
    _init_total_npods{0},
    _final_total_npods{0},
    _init_ctx_npods{0},
    _final_ctx_npods{0},
    _cv{},
    _mtx{},
    _mlr{mlr},
    _pg_mgr{nullptr},
    _rsc_mgr{nullptr},
    _sess_mgr{nullptr},
    _mbox{nullptr},
    _base{nullptr},
    _evread{nullptr}
{
    _psync = std::move(std::promise<bool>());
    _fsync = std::move(_psync.get_future());

    std::string name = "C" + id;

    // instanciate communication system
    _mbox = new ssrc::spread::Mailbox("4803", name, true, ssrc::spread::Mailbox::High);
    _base = new EventBase();
    _evread = new Event(*_base, _mbox->descriptor(), Events::Read | Events::Persist, [this](){ HandleMessage(); });

    _smsg.add(_msg);
}

Controller::~Controller()
{
    delete _evread;
    delete _base;
    delete _mbox;
}

void Controller::StartService()
{
    // create service thread
    _th_svc = std::move(std::thread(&Controller::operator(), this));

    // initialize global context
    Initialize();

    // create page manager initial context
    _pg_mgr->CreateInitialContext();

    // create resource manager initial context
    _rsc_mgr->CreateInitialContext(GetName());

    // intialize comm context
    InitializeContext();
}

void Controller::StopService()
{
    // finalize comm context
    FinalizeContext();

    // delete service resource manager
    _rsc_mgr->CleanInitialContext();

    // finalize synchronous
    Finalize();

    // join service thread
    _th_svc.join();
}

void Controller::operator()()
{
    _evread->start();
    _base->loop();
}

idp_t Controller::GenerateIdp()
{
    return _pg_mgr->GenerateIdp();
}

ConsistencyObject *Controller::GetConsistencyObject(idp_t idp)
{
    return _rsc_mgr->GetConsistencyObject(idp);
}

std::string const& Controller::GetGlobalContext()
{
    return _app_group;
}

std::string const& Controller::GetLocalContext()
{
    return _context;
}

unsigned int Controller::GetTotalPods()
{    
    std::unique_lock<std::mutex> lk(_mtx);
    return _init_total_npods;
}

unsigned int Controller::GetTotalDomains()
{
    return _rsc_mgr->GetTotalDomains();
}

idp_t Controller::GetDomainIdp(idp_t idp)
{
    return _rsc_mgr->GetDomainIdp(idp);
}

idp_t Controller::GetPredecessorIdp(idp_t idp)
{
    return _rsc_mgr->GetPredecessorIdp(idp);
}

idp_t Controller::Spawn(std::string const& context, unsigned int npods, idp_t parent, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts)
{
    // assemble command
    std::string cmd;
    cmd.append("corx");
    cmd.append(" ");
    cmd.append(_app_group);
    cmd.append(" ");
    cmd.append(context);
    cmd.append(" ");
    cmd.append(std::to_string(npods));
    cmd.append(" ");
    cmd.append(std::to_string(parent));
    cmd.append(" ");
    cmd.append(module);
    for (int i = 0; i < args.size(); ++i) {
        cmd.append(" ");
        cmd.append(args[i]);
    }

    //std::cout << cmd << std::endl;

    // spawn pods
    for (int i = 0; i < npods; ++i) {
        auto pos = i % hosts.size();
        auto host = hosts.at(pos);
        _sess_mgr->CreateRemoteSession(host, "22", cmd);
    }

    auto msg = _mlr->ReceiveMessage(parent);

    return msg.Sender();
}

void Controller::HandleMessage()
{
    try {
        _mbox->receive(_smsg, _groups);

        // verify if is a regular message
        if (_smsg.is_regular()) {
            HandleRegularMessage();
        }
        // verify if is a membership message
        else if (_smsg.is_membership()) {
            _smsg.get_membership_info(_info);

            if (_info.is_regular_membership()) {
                /*
                std::cout << "Received REGULAR membership for group " << _smsg.sender()
                        << " with " << _groups.size()
                        << " members , where I am member " << _smsg.type() << "\n";
                
                for (int i = 0; i < _groups.size(); ++i)
                    std::cout << "\t" << _groups.group(i) << "\n";
                */

                if (_info.caused_by_join()) {
                    // this message is received by the controllers of the group in which a member joined
                    //std::cout << "Due to the JOIN of " << _info.changed_member() << " in group " << _smsg.sender() << "\n";

                    auto ctrl = _info.changed_member();
                    auto group = _smsg.sender();

                    if (_app_group == group)
                        HandleInitialize();

                    if (IsContextGroup(group))
                        HandleInitializeContext();

                    if (IsResourceGroup(group)) {

                        auto idp = GetIdpFromResourceGroup(group);
                        auto size = _groups.size();

                        if (size > 1) {
                            std::thread t(&ResourceManager::CheckReplica, _rsc_mgr, idp, size, ctrl);
                            t.detach();
                        }
                    }

                } else if (_info.caused_by_leave()) {
                    // this message is received by the controllers of the group in which a member left
                    //std::cout << "Due to the LEAVE of " << _info.changed_member() << "\n";

                } else if (_info.caused_by_disconnect()) {
                    // this message is received by the controllers of the group in which a member disconnected
                    //std::cout << "Due to the DISCONNECT of " << _info.changed_member() << "\n";
                } else if (_info.caused_by_network()) {
                    // not important message to process for now
                    //std::cout << "Due to NETWORK change";
                }
            } else if (_info.is_transition()) {
                // not important message to process for now
                //std::cout << "received TRANSITIONAL membership for group " << _smsg.sender() << "\n";
            } else if (_info.is_self_leave()) {
                // this message is received by the controller that left the group
                //std::cout << "received membership message that left group " << _smsg.sender() << "\n";

                auto group = _smsg.sender();

                if (_app_group == group) {
                    _evread->end();
                    _base->loop_break();
                }

                if (IsResourceGroup(group)) {
                    auto idp = GetIdpFromResourceGroup(group);
                    std::cout << "EraseResource <" << idp << ">" << std::endl;
                    std::thread t(&ResourceManager::EraseResource, _rsc_mgr, idp);
                    t.detach();
                }

            }
        }
        // unkown type of message
        else {
            throw std::logic_error("unknown message type");
        }
    } catch (Error const& e) {
        e.print();
        std::exit(EXIT_FAILURE);
    }
}

void Controller::HandleRegularMessage()
{
    switch (MsgType(_smsg.type())) {
        case MsgType::PageRequest:
            if (_is_main_ctrl)
                HandlePageRequest();
            break;
        
        case MsgType::PageReply:
            HandlePageReply();
            break;

        case MsgType::ResourceAllocation:
            HandleResourceAllocationInfo();
            break;

        case MsgType::FindResourceRequest:
            HandleFindResourceRequest();
            break;

        case MsgType::FindResourceReply:
            HandleFindResourceReply();
            break;

        case MsgType::CreateReplica:
            HandleCreateReplica();
            break;

        case MsgType::ReleaseReplicaRequest:
            HandleReleaseReplicaRequest();
            break;

        case MsgType::UpdateRequest:
            HandleUpdateRequest();
            break;

        case MsgType::UpdateReply:
            HandleUpdateReply();
            break;

        case MsgType::InvalidateRequest:
            HandleInvalidateRequest();
            break;

        case MsgType::TokenUpdateRequest:
            HandleTokenUpdateRequest();
            break;

        case MsgType::TokenUpdateReply:
            HandleTokenUpdateReply();
            break;

        case MsgType::TokenAck:
            HandleTokenAck();
            break;
        
        case MsgType::CreateCollectiveInit:
            HandleCreateCollectiveInit();
            break;

        case MsgType::CreateCollectiveIdp:
            HandleCreateCollectiveIdp();
            break;

        case MsgType::StaticGroupCreate:
            HandleStaticGroupCreate();
            break;

        case MsgType::StaticGroupSynchronize:
            HandleStaticGroupSynchronize();
            break;

        case MsgType::SearchResourceRequest:
            HandleSearchResourceRequest();
            break;

        case MsgType::SearchResourceReply:
            HandleSearchResourceReply();
            break;

        case MsgType::FinalizeContext:
            HandleFinalizeContext();
            break;

        case MsgType::Finalize:
            HandleFinalize();
            break;
    }
}

void Controller::Initialize()
{
    // join app group and synchronize
    _mbox->join(_app_group);
    _is_main_ctrl = _fsync.get();

    // create page and resource managers
    _pg_mgr = new PageManager(this, _is_main_ctrl);
    _rsc_mgr = new ResourceManager(this, _mlr, _is_main_ctrl);
    _sess_mgr = new SessionManager();
}

void Controller::HandleInitialize()
{
    if (GetName() == _info.changed_member()) {
        if (_groups.size() == 1)
            _psync.set_value(true);
        else 
            _psync.set_value(false);
    }

    {
        std::unique_lock<std::mutex> lk(_mtx);
        _init_total_npods = _groups.size();
    }
}

void Controller::InitializeContext()
{
    // join communicator group
    JoinContextGroup(_context);

    {
        std::unique_lock<std::mutex> lk(_mtx);
        // wait for notification
        while (_init_ctx_npods < _npods)
            _cv.wait(lk);
    }
}

void Controller::HandleInitializeContext()
{
    std::unique_lock<std::mutex> lk(_mtx);
    _init_ctx_npods = _groups.size();
    if (_init_ctx_npods == _npods)
        _cv.notify_one();
}

void Controller::FinalizeContext()
{
    ScatterMessage msg;
    GroupList dest;

    // build message
    msg.set_safe();
    msg.set_type(underlying_cast(MsgType::FinalizeContext));
    dest.add(GetContextGroup(_context));

    // send finalize message
    _mbox->send(msg, dest);

    // wait for notification
    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (_final_ctx_npods < _init_ctx_npods)
            _cv.wait(lk);
    }

    LeaveContextGroup(_context);
}

void Controller::HandleFinalizeContext()
{
    std::unique_lock<std::mutex> lk(_mtx);
    ++_final_ctx_npods;
    if (_final_ctx_npods == _init_ctx_npods)
        _cv.notify_one();
}

void Controller::Finalize()
{
    ScatterMessage msg;
    GroupList dest;

    // build message
    msg.set_safe();
    msg.set_type(underlying_cast(MsgType::Finalize));
    dest.add(_app_group);

    // send finalize message
    _mbox->send(msg, dest);

    // wait for notification
    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (_final_total_npods < _init_total_npods)
            _cv.wait(lk);
    }

    // leave app group
    _mbox->leave(_app_group);

    _sess_mgr->StopService();
}

void Controller::HandleFinalize()
{
    std::unique_lock<std::mutex> lk(_mtx);
    ++_final_total_npods;
    if (_final_total_npods == _init_total_npods)
        _cv.notify_one();
}

void Controller::RequestPage()
{
    ScatterMessage rep;
    GroupList dest;

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::PageRequest));
    dest.add(_app_group);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandlePageRequest()
{
    ScatterMessage rep;
    GroupList dest;
    
    // generate new page
    auto page = _pg_mgr->GeneratePage();

    // serialize page
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(page);
    std::string const& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::PageReply));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(_smsg.sender());

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandlePageReply()
{
    page_t page;

    {
        // get page from message
        std::string tmp(_msg.begin(), _msg.size());
        std::istringstream iss(tmp, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(page);
    }

    // update local page of page manager
    _pg_mgr->UpdatePage(page);
}

void Controller::SendResourceAllocationInfo(idp_t idp)
{
    ScatterMessage rep;
    GroupList dest;
    
    // serialize idp
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp);
    std::string const& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::ResourceAllocation));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(_app_group);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleResourceAllocationInfo()
{
    idp_t idp;

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp);
    }

    std::thread t(&ResourceManager::GlobalResourceFound, _rsc_mgr, idp);
    t.detach();
}

void Controller::SendFindResourceRequest(idp_t idp)
{
    ScatterMessage req;
    GroupList dest;
    
    // serialize idp
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp);
    std::string const& tmp = oss.str();

    // build message
    req.set_safe();
    req.set_type(underlying_cast(MsgType::FindResourceRequest));
    req.set_self_discard();
    req.add(tmp.c_str(), tmp.size());
    dest.add(_app_group);

    // send message
    _mbox->send(req, dest);
}

void Controller::HandleFindResourceRequest()
{
    idp_t idp;
    auto ctrl = _smsg.sender();

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp);
    }

    // check if the resource exists
    std::thread t(&ResourceManager::HandleFindGlobalResource, _rsc_mgr, idp, ctrl);
    t.detach();
}

void Controller::SendFindResourceReply(idp_t idp, std::string const& ctrl)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize idp and resource
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::FindResourceReply));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(ctrl);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleFindResourceReply()
{
    idp_t idp;

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp);
    }

    std::thread t(&ResourceManager::GlobalResourceFound, _rsc_mgr, idp);
    t.detach();
}

void Controller::SendReplica(idp_t idp, Resource *rsc, std::string const& ctrl)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize idp and resource
    std::unique_ptr<Resource, std::function<void(Resource*)>> aux(rsc, [](Resource *rsc){});
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp, aux);
    std::string const& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::CreateReplica));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(ctrl);

    // send message
    _mbox->send(rep, dest);

    aux.reset();
}

void Controller::HandleCreateReplica()
{
    idp_t idp;
    std::unique_ptr<Resource> aux;

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp, aux);
    }

    auto rsc = aux.release();

    std::thread t(&ResourceManager::InsertResourceReplica, _rsc_mgr, idp, rsc);
    t.detach();
}

void Controller::SendReleaseReplicaRequest(idp_t idp)
{
    ScatterMessage req;
    GroupList dest;

    // dsm group of the resource
    std::string group = GetResourceGroup(idp);
    
    // build message
    req.set_safe();
    req.set_type(underlying_cast(MsgType::ReleaseReplicaRequest));
    dest.add(group);

    // send message
    _mbox->send(req, dest);
}

void Controller::HandleReleaseReplicaRequest()
{
    auto idp = GetIdpFromResourceGroup(_groups[0]);
    auto ctrl = _smsg.sender();

    std::thread t(&ResourceManager::ReleaseReplica, _rsc_mgr, idp, ctrl);
    t.detach();
}

void Controller::SendUpdateRequest(idp_t idp)
{
    ScatterMessage req;
    GroupList dest;

    // dsm group of the resource
    std::string group = GetResourceGroup(idp);
    
    // build message
    req.set_safe();
    req.set_type(underlying_cast(MsgType::UpdateRequest));
    dest.add(group);

    // send message
    _mbox->send(req, dest);
}

void Controller::HandleUpdateRequest()
{
    auto idp = GetIdpFromResourceGroup(_groups[0]);
    auto ctrl = _smsg.sender();

    // check if can provide an updated resource replica
    std::thread t(&ResourceManager::CheckUpdate, _rsc_mgr, idp, ctrl);
    t.detach();
}

void Controller::SendUpdateReply(idp_t idp, Resource *rsc, std::string const& ctrl)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize idp and resource
    std::unique_ptr<Resource, std::function<void(Resource*)>> aux(rsc, [](Resource *rsc){});
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp, aux);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::UpdateReply));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(ctrl);

    // send message
    _mbox->send(rep, dest);

    aux.reset();
}

void Controller::HandleUpdateReply()
{
    idp_t idp;
    std::unique_ptr<Resource> aux;

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp, aux);
    }

    auto rsc = aux.release();

    std::thread t(&ResourceManager::Update, _rsc_mgr, idp, rsc);
    t.detach();
}

void Controller::SendInvalidateRequest(idp_t idp)
{
    ScatterMessage req;
    GroupList dest;

    // dsm group of the resource
    std::string group = GetResourceGroup(idp);
    
    // build message
    req.set_safe();
    req.set_self_discard(); // SELF DISCARD INVALIDATE
    req.set_type(underlying_cast(MsgType::InvalidateRequest));
    dest.add(group);

    // send message
    _mbox->send(req, dest);
}

void Controller::HandleInvalidateRequest()
{
    auto idp = GetIdpFromResourceGroup(_groups[0]);

    // invalidate resource
    std::thread t(&ResourceManager::Invalidate, _rsc_mgr, idp);
    t.detach();
}

void Controller::SendTokenUpdateRequest(idp_t idp)
{
    ScatterMessage req;
    GroupList dest;

    // dsm group of the resource
    std::string group = GetResourceGroup(idp);
    
    // build message
    req.set_safe();
    req.set_type(underlying_cast(MsgType::TokenUpdateRequest));
    dest.add(group);

    // send message
    _mbox->send(req, dest);
}

void Controller::HandleTokenUpdateRequest()
{
    auto idp = GetIdpFromResourceGroup(_groups[0]);
    auto ctrl = _smsg.sender();

    // try to release the token and to provide an updated replica
    std::thread t(&ResourceManager::CheckTokenUpdate, _rsc_mgr, idp, ctrl);
    t.detach();
}

void Controller::SendTokenUpdateReply(idp_t idp, Resource *rsc, std::string const& ctrl)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize idp
    std::unique_ptr<Resource, std::function<void(Resource*)>> aux(rsc, [](Resource *rsc){});
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp, aux);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::TokenUpdateReply));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(ctrl);

    // send message
    _mbox->send(rep, dest);

    aux.reset();
}

void Controller::HandleTokenUpdateReply()
{
    idp_t idp;
    std::string new_holder;
    std::unique_ptr<Resource> aux;

    auto ctrl = _smsg.sender();

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp, aux);
    }

    auto rsc = aux.release();

    std::thread t(&ResourceManager::AcquireTokenUpdate, _rsc_mgr, idp, rsc, ctrl);
    t.detach();
}

void Controller::SendTokenAck(idp_t idp, std::string const& ctrl)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize idp
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::TokenAck));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(ctrl);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleTokenAck()
{
    idp_t idp;

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp);
    }

    std::thread t(&ResourceManager::TokenAck, _rsc_mgr, idp);
    t.detach();
}

void Controller::InitCreateCollective(std::string const& context, unsigned int total_members)
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    auto tid = ss.str();

    {
        std::unique_lock<std::mutex> lk(_mtx);
        _cc_first.emplace(std::piecewise_construct, std::forward_as_tuple(context, tid), std::forward_as_tuple());
        _cc_vars.emplace(context, std::make_pair(0, total_members));
        _cc_cv[context].notify_all();
    }

    SendCreateCollectiveInit(context, tid);

    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (_cc_vars.at(context).first != _cc_vars.at(context).second)
            _cc_cv[context].wait(lk);
    }
}

void Controller::SendCreateCollectiveInit(std::string const& context, std::string const& tid)
{
    ScatterMessage rep;
    GroupList dest;

    auto group = GetContextGroup(context);

    // serialize tid
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(tid);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::CreateCollectiveInit));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(group);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleCreateCollectiveInit()
{
    std::string tid;
    auto context = GetContextFromContextGroup(_groups[0]);

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(tid);
    }

    auto local = (_smsg.sender() == GetName());

    std::thread t(&Controller::HandleInitCreateCollective, this, context, tid, local);
    t.detach();
}

void Controller::HandleInitCreateCollective(std::string const& context, std::string const& tid, bool local)
{
    std::unique_lock<std::mutex> lk(_mtx);

    if (_cc_vars.find(context) == _cc_vars.end())
        _cc_cv[context].wait(lk);

    _cc_vars.at(context).first += 1;

    if (local) {
        if (_cc_vars.at(context).first == 1)
            _cc_first.at({context, tid}).set_value(true);
        else
            _cc_first.at({context, tid}).set_value(false);
    }

    if (_cc_vars.at(context).first == _cc_vars.at(context).second)
        _cc_cv[context].notify_all();
}

void Controller::SendCreateCollectiveIdp(std::string const& context, idp_t idp)
{
    ScatterMessage rep;
    GroupList dest;

    auto group = GetContextGroup(context);

    // serialize idp
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::CreateCollectiveIdp));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(group);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleCreateCollectiveIdp()
{
    idp_t idp;
    auto context = GetContextFromContextGroup(_groups[0]);

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp);
    }

    {
        std::unique_lock<std::mutex> lk(_mtx);
        _cc_idp[context] = idp;
        _cc_cv[context].notify_all();
    }
}

bool Controller::GetCreateCollectiveFirst(std::string const& context)
{
    std::future<bool> future;

    std::stringstream ss;
    ss << std::this_thread::get_id();
    auto tid = ss.str();

    {
        std::unique_lock<std::mutex> lk(_mtx);
        future = _cc_first.at({context, tid}).get_future();
    }

    return future.get();
}

idp_t Controller::GetCreateCollectiveIdp(std::string const& context)
{
    std::unique_lock<std::mutex> lk(_mtx);

    if (_cc_idp.find(context) == _cc_idp.end())
        _cc_cv[context].wait(lk);

    return _cc_idp.at(context);
}

void Controller::CreateStaticGroup(idp_t comm, unsigned int total_members)
{
    _rsc_mgr->CreateStaticGroup(comm, total_members);
}

void Controller::SendStaticGroupCreate(idp_t comm)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize comm
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(comm);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::StaticGroupCreate));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(_app_group);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleStaticGroupCreate()
{
    idp_t comm;

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(comm);
    }

    std::thread t(&ResourceManager::HandleCreateStaticGroup, _rsc_mgr, comm);
    t.detach();
}

void Controller::SynchronizeStaticGroup(idp_t comm)
{
    _rsc_mgr->SynchronizeStaticGroup(comm);
}

void Controller::SendStaticGroupSynchronize(idp_t comm)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize comm
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(comm);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::StaticGroupSynchronize));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(_app_group);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleStaticGroupSynchronize()
{
    idp_t comm;

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(comm);
    }

    std::thread t(&ResourceManager::HandleSynchronizeStaticGroup, _rsc_mgr, comm);
    t.detach();
}

void Controller::SendSearchResourceRequest(idp_t idp)
{
    ScatterMessage req;
    GroupList dest;

    // serialize idp
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp);
    const std::string& tmp = oss.str();

    // build message
    req.set_safe();
    req.set_type(underlying_cast(MsgType::SearchResourceRequest));
    req.add(tmp.c_str(), tmp.size());
    dest.add(_app_group);

    // send message
    _mbox->send(req, dest);
}

void Controller::HandleSearchResourceRequest()
{
    idp_t idp;
    auto ctrl = _smsg.sender();

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp);
    }

    std::thread t(&ResourceManager::HandleSearchResource, _rsc_mgr, idp, ctrl);
    t.detach();
}

void Controller::SendSearchResourceReply(idp_t idp, std::string const& info, std::string const& ctrl)
{
    ScatterMessage rep;
    GroupList dest;

    // serialize idp and info
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(idp, info);
    const std::string& tmp = oss.str();

    // build message
    rep.set_safe();
    rep.set_type(underlying_cast(MsgType::SearchResourceReply));
    rep.add(tmp.c_str(), tmp.size());
    dest.add(ctrl);

    // send message
    _mbox->send(rep, dest);
}

void Controller::HandleSearchResourceReply()
{
    idp_t idp;
    std::string info;
    auto ctrl = _smsg.sender();

    {
        std::string sobj(_msg.begin(), _msg.size());
        std::istringstream iss(sobj, std::istringstream::binary);
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(idp, info);
    }

    std::thread t(&ResourceManager::HandleSearchResourceInfo, _rsc_mgr, idp, info);
    t.detach();
}

std::string const& Controller::GetName() const
{
    return _mbox->private_group();
}

void Controller::JoinContextGroup(std::string const& context)
{
    std::string group = GetContextGroup(context);
    _mbox->join(group);
}

void Controller::LeaveContextGroup(std::string const& context)
{
    std::string group = GetContextGroup(context);
    _mbox->leave(group);
}

bool Controller::IsContextGroup(std::string const& group)
{
    return (group.find("$") != std::string::npos);
}

std::string Controller::GetContextGroup(std::string const& context)
{
    return _app_group + "$" + context;
}

std::string Controller::GetContextFromContextGroup(std::string const& group)
{
    return group.substr(group.find("$") + 1);
}

void Controller::JoinResourceGroup(idp_t idp)
{
    std::string group = GetResourceGroup(idp);
    _mbox->join(group);
}

void Controller::LeaveResourceGroup(idp_t idp)
{
    std::string group = GetResourceGroup(idp);
    _mbox->leave(group);
}

bool Controller::IsResourceGroup(std::string const& group)
{
    return (group.find("@R") != std::string::npos);
}

std::string Controller::GetResourceGroup(idp_t idp)
{
    return _app_group + "@R" + std::to_string(idp);
}

idp_t Controller::GetIdpFromResourceGroup(std::string const& group)
{
    return std::stoul(group.substr(group.find("@") + 2));
}

std::string Controller::SearchResource(idp_t idp)
{
    return _rsc_mgr->SearchResource(idp);
}

bool Controller::ContainsResource(idp_t idp)
{
    return _rsc_mgr->ContainsResource(idp);
}

// to remove
void Controller::Debug()
{
    std::cout << _mbox->private_group() << std::endl;
}

}
