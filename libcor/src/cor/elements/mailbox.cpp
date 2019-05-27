#include "cor/elements/mailbox.hpp"

#include "cor/system/system.hpp"
#include "cor/system/pod.hpp"

namespace cor {

Mailbox::Mailbox() = default;

Mailbox::Mailbox(idp_t idp) : _idp{idp} {}

Mailbox::~Mailbox() = default;

Mailbox::Mailbox(Mailbox&&) noexcept = default;

Mailbox& Mailbox::operator=(Mailbox&&) noexcept = default;

void Mailbox::Send(idp_t dest, Message& msg) const
{
    global::pod->SendMessage(_idp, dest, msg);
}

void Mailbox::Send(std::vector<idp_t> const& dests, Message& msg) const
{
    global::pod->SendMessage(_idp, dests, msg);
}

Message Mailbox::Receive() const
{
    return global::pod->ReceiveMessage(_idp);
}

Message Mailbox::Receive(idp_t source) const
{
    return global::pod->ReceiveMessage(_idp, source);
}

void Mailbox::Broadcast(idp_t clos, Message& msg) const
{
    auto sorg = global::pod->GetLocalResource<cor::StaticOrganizer>(clos);
    auto dests = sorg->GetMemberList();
    global::pod->SendMessage(_idp, dests, msg);
}

void Mailbox::Send(idm_t rank, idp_t clos, Message& msg) const
{
    auto sorg = global::pod->GetLocalResource<cor::StaticOrganizer>(clos);
    auto dest = sorg->GetIdp(rank);
    global::pod->SendMessage(_idp, dest, msg);
}

Message Mailbox::Receive(idm_t rank, idp_t clos) const
{
    auto sorg = global::pod->GetLocalResource<cor::StaticOrganizer>(clos);
    auto source = sorg->GetIdp(rank);
    return global::pod->ReceiveMessage(_idp, source);
}

}
