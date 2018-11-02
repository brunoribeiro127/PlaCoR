#ifndef COR_MAILBOX_HPP
#define COR_MAILBOX_HPP

#include <vector>

#include "cereal/access.hpp"

#include "cor/system/macros.hpp"
#include "cor/message.hpp"

namespace cor {

class Mailbox
{

friend class cereal::access;

public:
    virtual ~Mailbox();

    Mailbox(const Mailbox&) = delete;
    Mailbox& operator=(const Mailbox&) = delete;

    Mailbox(Mailbox&&) noexcept;
    Mailbox& operator=(Mailbox&&) noexcept;

    void Send(idp_t dest, Message& msg) const;
    void Send(std::vector<idp_t> const& dests, Message& msg) const;

    Message Receive() const;
    Message Receive(idp_t source) const;

    // primitivas de comunicação no contexto de grupos
/*
    void Multicast(idp_t dest, Message& msg) const;
    void Broadcast(idp_t group, Message& msg) const;
    void Send(idp_t group, idm_t dest, Message& msg) const;
    Message Receive(idp_t group, idm_t source);
*/

protected:
    Mailbox();
    explicit Mailbox(idp_t idp);

private:
    template <class Archive>
    void serialize(Archive& ar) 
    {
        ar(_idp);
    }

    idp_t _idp;

};

}

#endif
