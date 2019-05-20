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

    void Send(idp_t dest, Message& msg) const;                          // Unicast
    void Send(std::vector<idp_t> const& dests, Message& msg) const;     // Multicast
    Message Receive() const;
    Message Receive(idp_t source) const;

    // Contextual Communication
    void Broadcast(idp_t comm, Message& msg) const;                     // Broadcast
    void Send(idm_t rank, idp_t comm, Message& msg) const;              // Contextual Unicast
    Message Receive(idm_t rank, idp_t comm) const;

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
