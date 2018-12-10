#ifndef COR_POD_HPP
#define COR_POD_HPP

#include <string>
#include <thread>
#include <mutex>
#include <vector>

#include "cor/services/controller.hpp"
#include "cor/message.hpp"
#include "cor/system/macros.hpp"
#include "cor/external/dll/dll.hpp"

#include "cor/services/resource_ptr.hpp"

namespace cor {

class Mailer;
class Resource;
class ConsistencyObject;

class Pod
{

template <typename> friend class ResourceFactory;

friend class DynamicOrganizer;
template <typename> friend class Value;
template <typename> friend class Executor;
friend class Mailbox;
friend class StaticOrganizer;
friend class SBarrier;
friend class SMutex;
friend class SRWMutex;

public:
    explicit Pod(std::string const& app_group, std::string const& communicator, unsigned int npods);

    ~Pod();

    void Initialize();
    void Finalize();

    unsigned int GetTotalDomains();

    idp_t GetActiveResourceIdp();
    idp_t GetDomainIdp();

    // not global function, only local domain
    idp_t GetDomainIdp(idp_t idp);

    // not global function, only local resources predecessor
    idp_t GetPredecessorIdp(idp_t idp);

    template <typename T>
    ResourcePtr<T> GetLocalResource(idp_t idp);

    template <typename T, typename ... Args>
    ResourcePtr<T> Create(idp_t ctx, std::string const& name, bool global, Args&& ... args);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateCollective(idp_t ctx, std::string const& name, unsigned int total_members, bool global, Args&& ... args);

    template <typename T>
    ResourcePtr<T> CreateReference(idp_t idp, idp_t ctx, std::string const& name);

    idp_t Spawn(std::string const& comm, unsigned int npods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts);

    Pod() = delete;
    Pod(const Pod&) = delete;
    Pod& operator=(const Pod&) = delete;
    Pod(Pod&&) = delete;
    Pod& operator=(Pod&&) = delete;

protected:
    // accessed by ResourceFactory
    idp_t GenerateIdp();

    template <typename T>
    ResourcePtr<T> AllocateResource(idp_t idp, idp_t ctx, std::string const& name, bool global, Resource *rsc);

    // accessed by Organizer and Value
    ConsistencyObject *GetConsistencyObject(idp_t idp);

    // accessed by Organizer
    void LoadModule(std::string const& module);

    // accessed by Executor
    template <typename T>
    std::function<T> LoadFunction(std::string const& module, std::string const& function);

    void InsertActiveResource(std::thread::id tid, idp_t idp);
    void RemoveActiveResource(std::thread::id tid);
    void ChangeActiveResource(std::thread::id tid, idp_t idp);
    idp_t GetCurrentActiveResource(std::thread::id tid);

    // accessed by Mailbox
    void SendMessage(idp_t idp, idp_t dest, Message& msg);
    void SendMessage(idp_t idp, std::vector<idp_t> const& dests, Message& msg);
    Message ReceiveMessage(idp_t idp);
    Message ReceiveMessage(idp_t idp, idp_t source);

    // accessed by StaticOrganizer
    void SynchronizeCollectiveGroup(std::string const& comm);

private:
    Mailer *_mlr;
    Controller *_ctrl;

    std::map<std::string, dll::DynamicLibrary*> _modules;
    std::map<std::thread::id, idp_t> _active_rscs;
    std::mutex _mtx;

};

}

#include "cor/elements/pod.tpp"

#endif
