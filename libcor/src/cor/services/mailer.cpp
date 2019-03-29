#include "cor/services/mailer.hpp"

#include "cor/utils/utils.hpp"

#include <sstream>
#include <cassert>

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
    Connect(id);
}

Mailer::~Mailer()
{
    delete _evwrite;
    delete _evread;
    delete _base;
    delete _mbox;
}

void Mailer::Connect(std::string const& id)
{
    std::string name = "M" + id;

    // instanciate communication system
    _mbox = new ssrcspread::Mailbox("4803", name, true, ssrcspread::Mailbox::High);
    _base = new EventBase();
    _evread = new Event(*_base, _mbox->descriptor(), Events::Read | Events::Persist, [this](){ HandleRead(); });
    _evwrite = new Event(*_base, _mbox->descriptor(), Events::Write | Events::Persist, [this](){ HandleWrite(); });

    _in_smsg.add(_in_msg);
}

void Mailer::Close()
{
    _evread->end();
    _base->loop_break();
}

void Mailer::StartService()
{
    _th_svc = std::move(std::thread(&Mailer::operator(), this));
}

void Mailer::StopService()
{
    Close();
    _th_svc.join();
}

void Mailer::operator()()
{
    _evread->start();
    _base->loop();
}

void Mailer::SendMessage(idp_t idp, idp_t dest, Message& msg)
{
    // set message sender
    msg.SetSender(idp);

    // serialize message
    std::ostringstream oss(std::stringstream::binary);
    cereal::PortableBinaryOutputArchive oarchive(oss);
    oarchive(msg);
    auto data = oss.str();

    std::vector<std::string> groups;
    groups.push_back(std::move(GetMessageGroup(dest)));
    
    auto trigger = _out_queue([&](Out& out)
    {
        if (out.ongoingWrite)
        {
            out.q.push(std::make_pair(std::move(groups), std::move(data)));
            return false;
        }
        else
        {
            assert(out.q.size() == 0);
            out.ongoingWrite = true;
            _outgoing_groups = std::move(groups);
            _outgoing_msg = std::move(data);
            return true;
        }
    });

    if (trigger) {
        _evwrite->end();
        _evwrite->start();
    }
}

void Mailer::SendMessage(idp_t idp, std::vector<idp_t> const& dests, Message& msg)
{
    for (auto dest: dests)
        SendMessage(idp, dest, msg);
}

Message Mailer::ReceiveMessage(idp_t idp)
{
    std::unique_lock<std::mutex> lk(_mtx);

    // if the mailbox does not have messages, then wait for messages
    if (_mailboxes[idp].empty())
        _rwq[idp].wait(lk, [this, idp]{ return !_mailboxes[idp].empty(); });
    auto msg = std::move(_mailboxes[idp].front());
    _mailboxes[idp].pop_front();
    return msg;
}

Message Mailer::ReceiveMessage(idp_t idp, idp_t source)
{
    std::unique_lock<std::mutex> lk(_mtx);

    Message msg;
    bool found = false;

    while (!found) {

        for (auto it = _mailboxes[idp].begin(); !found && it != _mailboxes[idp].end(); ) {
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

void Mailer::HandleRead()
{
    try {
        _mbox->receive(_in_smsg, _in_groups);

        // verify if is a regular message
        if (_in_smsg.is_regular()) {

            Message msg;

            // deserialize message
            std::string sobj(_in_msg.begin(), _in_msg.size());
            std::istringstream iss(sobj, std::istringstream::binary);
            cereal::PortableBinaryInputArchive iarchive(iss);
            iarchive(msg);

            // get idp of receiver
            auto idp = GetIdpFromMessageGroup(_in_groups[0]);

            {
                // insert message in resource mailbox
                std::unique_lock<std::mutex> lk(_mtx);
                _mailboxes[idp].push_back(std::move(msg));
                _rwq[idp].notify_all();
            }
        }
    } catch (ssrcspread::Error const& e) {
        e.print();
        std::exit(0);
    }
}

void Mailer::HandleWrite()
{
    // build message
    _out_smsg.set_safe();
    _out_smsg.set_type(underlying_cast(MsgType::Message));
    _out_smsg.add(_outgoing_msg.c_str(), _outgoing_msg.size());

    for (auto& group: _outgoing_groups)
        _out_groups.add(group);

    // send message
    _mbox->send(_out_smsg, _out_groups);

    // clear message content
	_out_smsg.clear();
	_out_groups.clear();

    _out_queue([&](Out& out)
    {
        if (out.q.size())
        {
            auto pair = std::move(out.q.front());
            _outgoing_groups = std::move(pair.first);
            _outgoing_msg = std::move(pair.second);
            out.q.pop();
        }
        else
        {
            out.ongoingWrite = false;
            _outgoing_groups.clear();
            _outgoing_msg.clear();
        }
    });

    _evwrite->end();
    if (_outgoing_msg.size())
		_evwrite->start();
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
