#include "cor/services/mailer.hpp"

#include "cor/utils/utils.hpp"

#include <sstream>

#include <cereal/archives/portable_binary.hpp>

namespace ssrcspread = ssrc::spread;
using namespace ev;

namespace cor {

Mailer::Mailer(std::string const& id, std::string const& app_group) : 
    _app_group{app_group},
    _th_svc{},
    _mbox{nullptr},
    _base{nullptr},
    _evread{nullptr},
    _mailboxes{},
    _rwq{},
    _mtx{}
{
    std::string name = "M" + id;

    // instanciate communication system
    _mbox = new ssrcspread::Mailbox("4803", name, true, ssrcspread::Mailbox::High);
    _base = new EventBase();
    _evread = new Event(*_base, _mbox->descriptor(), Events::Read | Events::Persist, [this](){ HandleMessage(); });

    _smsg.add(_msg);
}

Mailer::~Mailer()
{

    delete _evread;
    delete _base;
    delete _mbox;
}

void Mailer::StartService()
{
    _th_svc = std::move(std::thread(&Mailer::operator(), this));
}

void Mailer::StopService()
{
    RequestServiceStop();
    _th_svc.join();
}

void Mailer::operator()()
{
    _evread->start();
    _base->loop();
}

void Mailer::SendMessage(idp_t idp, idp_t dest, Message& msg)
{
    ssrcspread::ScatterMessage smsg;
    ssrcspread::GroupList groups;

    // set message sender
    msg.SetSender(idp);

    // serialize message
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(msg);
    std::string const& tmp = oss.str();

    // build message
    smsg.set_safe();
    smsg.set_type(underlying_cast(MsgType::Message));
    smsg.add(tmp.c_str(), tmp.size());

    std::string group = GetMessageGroup(dest);
    groups.add(group);
    
    // send message
    _mbox->send(smsg, groups);
}

void Mailer::SendMessage(idp_t idp, std::vector<idp_t> const& dests, Message& msg)
{
    ssrcspread::ScatterMessage smsg;
    ssrcspread::GroupList groups;

    // set message sender
    msg.SetSender(idp);

    // serialize message
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(msg);
    std::string const& tmp = oss.str();

    // build message
    smsg.set_safe();
    smsg.set_type(underlying_cast(MsgType::Message));
    smsg.add(tmp.c_str(), tmp.size());

    for (auto const& dest: dests) {
        std::string group = GetMessageGroup(dest);
        groups.add(group);
    }

    // send message
    _mbox->send(smsg, groups);
}

Message Mailer::ReceiveMessage(idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);

    // if the mailbox does not have messages, then wait for messages
    if (_mailboxes[idp].empty())
        _rwq[idp].wait(lk, [this, idp]{ return !_mailboxes[idp].empty(); });

    auto val(std::move(_mailboxes[idp].front()));
    _mailboxes[idp].pop_front();
    return val;
}

Message Mailer::ReceiveMessage(idp_t idp, idp_t source)
{
    std::unique_lock<std::mutex> lk(_mtx);

    Message msg;
    bool found = false;

    while (!found) {

        for (auto it = _mailboxes[idp].begin(); it != _mailboxes[idp].end(); ) {
            if (source == it->Sender()) {
                msg = std::move(*it);
                _mailboxes[idp].erase(it);
                found = true;
            }
        }

        // if the message was not found, then wait for new messages
        if (!found)
            _rwq[idp].wait(lk, [this, idp]{ return !_mailboxes[idp].empty(); });
    }

    return msg;
}


void Mailer::HandleMessage()
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
                    // this message is received by the mailers of the group in which a member joined
                    //std::cout << "Due to the JOIN of " << _info.changed_member() << " in group " << _smsg.sender() << "\n";
                } else if (_info.caused_by_leave()) {
                    // this message is received by the mailers of the group in which a member left
                    //std::cout << "Due to the LEAVE of " << _info.changed_member() << "\n";
                } else if (_info.caused_by_disconnect()) {
                    // this message is received by the mailers of the group in which a member disconnected
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
            }
        }
        // unkown type of message
        else {
            throw std::logic_error("unknown message type");
        }
    } catch (ssrcspread::Error const& e) {
        e.print();
        std::exit(0);
    }
}

void Mailer::HandleRegularMessage()
{
    switch (MsgType(_smsg.type())) {

        case MsgType::Message:
            HandleResourceMessage();
            break;

        case MsgType::ServiceStop:
            // needs to delete every resource linked to this pod
            _evread->end();
            _base->loop_break();
            break;
    }
}

void Mailer::HandleResourceMessage()
{
    Message msg;

    // deserialize message
    std::string sobj(_msg.begin(), _msg.size());
    std::istringstream iss(sobj, std::istringstream::binary);
    cereal::PortableBinaryInputArchive iarchive(iss);
    iarchive(msg);

    // get idp of receiver
    auto idp = GetIdpFromMessageGroup(_groups[0]);

    {
        // insert message in resource mailbox
        std::unique_lock<std::mutex> lk(_mtx);
        _mailboxes[idp].push_back(std::move(msg));
        _rwq[idp].notify_all();
    }
}

void Mailer::RequestServiceStop()
{
    ssrcspread::ScatterMessage req;
    ssrcspread::GroupList dest;

    // build message
    req.set_safe();
    req.set_type(underlying_cast(MsgType::ServiceStop));
    dest.add(_mbox->private_group());

    // send message
    _mbox->send(req, dest);
}

void Mailer::CreateMailbox(idp_t idp)
{
    // join msg group of resource
    JoinMessageGroup(idp);

    {
        // create mailbox for resource
        std::unique_lock<std::mutex> lk(_mtx);
        _mailboxes.emplace(std::piecewise_construct, std::make_tuple(idp), std::make_tuple());
        _rwq.emplace(std::piecewise_construct, std::make_tuple(idp), std::make_tuple());
    }
}

void Mailer::DeleteMailbox(idp_t idp)
{
    // leave msg group of resource
    LeaveMessageGroup(idp);

    {
        // delete mailbox for resource
        std::unique_lock<std::mutex> lk(_mtx);
        _mailboxes.erase(idp);
        _rwq.erase(idp);
    }
}

void Mailer::JoinMessageGroup(idp_t idp)
{
    std::string group = GetMessageGroup(idp);
    _mbox->join(group);
}

void Mailer::LeaveMessageGroup(idp_t idp)
{
    std::string group = GetMessageGroup(idp);
    _mbox->leave(group);
}

std::string Mailer::GetMessageGroup(idp_t idp)
{
    return _app_group + "@M" + std::to_string(idp);
}

idp_t Mailer::GetIdpFromMessageGroup(std::string const& group)
{
    return std::stoul(group.substr(group.find("@") + 2));
}

}
