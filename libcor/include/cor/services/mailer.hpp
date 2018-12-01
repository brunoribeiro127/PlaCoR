#ifndef COR_MAILER_HPP
#define COR_MAILER_HPP

#include <thread>
#include <map>
#include <deque>
#include <mutex>
#include <condition_variable>

#include <ssrc/spread.h>
#include "cor/external/event/event.hpp"

#include "cor/system/macros.hpp"
#include "cor/message.hpp"

namespace cor {

class Mailer
{

friend class ResourceManager;

public:
    Mailer(std::string const& app_group);
    ~Mailer();

    void StartService();
    void StopService();

    void operator()();

    void SendMessage(idp_t idp, idp_t dest, Message& msg);
    void SendMessage(idp_t idp, std::vector<idp_t> const& dests, Message& msg);

    Message ReceiveMessage(idp_t idp);
    Message ReceiveMessage(idp_t idp, idp_t source);

    Mailer(Mailer const&) = delete;
    Mailer& operator=(Mailer const&) = delete;
    Mailer(Mailer&&) = delete;
    Mailer& operator=(Mailer&&) = delete;

protected:
    // methods to handle received messages
    void HandleMessage();
    void HandleRegularMessage();

    // handle resource message
    void HandleResourceMessage();

    // stop controller services
    void RequestServiceStop();

    // accessed by ResourceManager
    void CreateMailbox(idp_t idp);
    void DeleteMailbox(idp_t idp);

private:
    void JoinGroup(std::string const& group);
    void LeaveGroup(std::string const& group);

    void JoinMessageGroup(idp_t idp);
    void LeaveMessageGroup(idp_t idp);

    std::string GetMessageGroup(idp_t idp);
    idp_t GetIdpFromMessageGroup(std::string const& group);

    enum class MsgType: std::int16_t
    {
        Message,
        ServiceStop
    };

    constexpr typename std::underlying_type<MsgType>::type underlying_cast(MsgType t) const noexcept
    {
        return static_cast<typename std::underlying_type<MsgType>::type>(t);
    }

    std::string _app_group;

    // service thread
    std::thread _th_svc;

    // communication system
    ssrc::spread::Mailbox *_mbox;
    ev::EventBase *_base;
    ev::Event *_evread;

    // variables to handle received messages
    ssrc::spread::ScatterMessage _smsg;
    ssrc::spread::Message _msg;
    ssrc::spread::GroupList _groups;
    ssrc::spread::MembershipInfo _info;

    std::map<idp_t, std::deque<Message>> _mailboxes;
    std::map<idp_t, std::condition_variable> _rwq;
    std::mutex _mtx;

};

}

#endif
