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

friend class Container;
friend class DynamicOrganizer;
template <typename> friend class Value;
template <typename> friend class Executor;
friend class Mailbox;
friend class StaticOrganizer;
friend class SBarrier;
friend class SMutex;
friend class SRWMutex;

public:
    explicit Pod(std::string const& id, std::string const& app_group, std::string const& context, unsigned int npods);

    ~Pod();

    void Initialize();
    void Finalize();

    std::string const& GetGlobalContext();
    std::string const& GetLocalContext();

    unsigned int GetTotalPods();
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
    ResourcePtr<T> CreateLocal(idp_t ctx, std::string const& name, Args&& ... args);

    template <typename T, typename ... Args>
    idp_t Create(idp_t ctx, std::string const& name, Args&& ... args);

    template <typename T>
    ResourcePtr<T> CreateReference(idp_t idp, idp_t ctx, std::string const& name);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateCollective(idp_t ctx, std::string const& name, unsigned int total_members, Args&& ... args);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateCollective(idp_t comm, idp_t ctx, std::string const& name, Args&& ... args);

    idp_t Spawn(std::string const& context, unsigned int npods, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts);

    // to remove
    void Debug();

    Pod() = delete;
    Pod(const Pod&) = delete;
    Pod& operator=(const Pod&) = delete;
    Pod(Pod&&) = delete;
    Pod& operator=(Pod&&) = delete;

protected:
    // accessed by Container
    std::string SearchResource(idp_t idp);
    bool ContainsResource(idp_t idp);

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
    void CreateStaticGroup(idp_t comm, unsigned int total_members);

    // accessed by SBarrier
    void SynchronizeStaticGroup(idp_t comm);

private:
    Mailer *_mlr;
    Controller *_ctrl;

    std::map<std::string, dll::DynamicLibrary*> _modules;
    std::map<std::thread::id, idp_t> _active_rscs;
    std::mutex _mtx;

};

}

#include "cor/system/pod.tpp"

#endif
