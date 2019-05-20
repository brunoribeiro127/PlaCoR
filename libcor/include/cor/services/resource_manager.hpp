#ifndef COR_RESOURCE_MANAGER_HPP
#define COR_RESOURCE_MANAGER_HPP

#include <map>
#include <mutex>
#include <tuple>
#include <future>
#include <typeindex>
#include <condition_variable>

#include "cor/resources/resource.hpp"
#include "cor/system/macros.hpp"
#include "cor/elements/mailbox.hpp"
#include "cor/services/mailer.hpp"

namespace cor {

class Controller;
class ConsistencyObject;
template <typename> class ResourcePtr;

class ResourceManager
{

friend class ConsistencyObject;
template <typename> friend class ResourcePtr;

public:
    explicit ResourceManager(Controller *ctrl, Mailer *mlr, bool first);
    ~ResourceManager();

    void CreateInitialContext(std::string const& ctrl);
    void CleanInitialContext();

    void CreateMetaDomain(std::string const& ctrl);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateLocal(idp_t ctx, std::string const& name, std::string const& ctrl, Args&& ... args);

    template <typename T, typename ... Args>
    idp_t Create(idp_t ctx, std::string const& name, std::string const& ctrl, Args&& ... args);

    template <typename T>
    ResourcePtr<T> CreateReference(idp_t idp, idp_t ctx, std::string const& name, std::string const& ctrl);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateCollective(idm_t rank, idp_t comm, idp_t ctx, std::string const& name, std::string const& ctrl, Args&& ... args);

    template <typename T>
    void AllocateResource(idp_t idp, idp_t ctx, std::string const& name, Resource *rsc, std::string const& ctrl);

    template <typename T>
    void CreateReplica(idp_t idp, std::string const& ctrl);

    void InsertResourceReplica(idp_t idp, Resource *rsc);

    ConsistencyObject *GetConsistencyObject(idp_t idp);

    bool ContainsResource(idp_t idp);

    Resource *GetResource(idp_t idp);

    template <typename T>
    ResourcePtr<T> GetLocalResource(idp_t idp);

    unsigned int GetTotalDomains();

    // not global function, only local domain
    idp_t GetDomainIdp(idp_t idp);
    
    // not global function, only local resources predecessor
    idp_t GetPredecessorIdp(idp_t idp);

    // interface to Controller
    void EraseResource(idp_t idp);

    void SendResourceAllocationInfo(idp_t idp);

    void FindGlobalResource(idp_t idp);
    void HandleFindGlobalResource(idp_t idp, std::string ctrl);
    void SendGlobalResourceFound(idp_t idp, std::string const& ctrl);
    void GlobalResourceFound(idp_t idp);

    void ReleaseReplica(idp_t idp, std::string requester);
    void CheckReplica(idp_t idp, unsigned int size, std::string requester);

    void CheckUpdate(idp_t idp, std::string requester);
    void Update(idp_t idp, Resource *rsc);

    void Invalidate(idp_t idp);

    void CheckTokenUpdate(idp_t idp, std::string requester);
    void AcquireTokenUpdate(idp_t idp, Resource *rsc, std::string replier);

    void TokenAck(idp_t idp);

    void CreateStaticGroup(idp_t comm, unsigned int total_members);
    void HandleCreateStaticGroup(idp_t comm);

    void SendStaticGroupCCIdp(idp_t comm, idp_t idp);
    void HandleStaticGroupCCIdp(idp_t comm, idp_t idp);
    idp_t GetStaticGroupCCIdp(idp_t comm);

    void SynchronizeStaticGroup(idp_t comm);
    void HandleSynchronizeStaticGroup(idp_t comm);

    std::string SearchResource(idp_t idp);
    void HandleSearchResource(idp_t idp, std::string ctrl);
    void HandleSearchResourceInfo(idp_t idp, std::string info);

    ResourceManager() = delete;
    ResourceManager(ResourceManager const&) = delete;
    ResourceManager& operator=(ResourceManager const&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

protected:
    // interface to ConsistencyObject
    void DeallocateResource(idp_t idp);

    void SendReplica(idp_t idp, Resource *rsc, std::string const& requester);

    void RequestReleaseReplica(idp_t idp);

    void RequestUpdate(idp_t idp);
    void ReplyUpdate(idp_t idp, Resource *rsc, std::string const& requester);

    void RequestInvalidate(idp_t idp);

    void RequestTokenUpdate(idp_t idp);
    void ReplyTokenUpdate(idp_t idp, Resource *rsc, std::string const& requester);

    void SendTokenAck(idp_t idp, std::string const& replier);

private:
    void JoinResourceGroup(idp_t idp);
    void LeaveResourceGroup(idp_t idp);

    idp_t GenerateIdp();

    // to improve
    //void DummyInsertWorldContext(idp_t idp, std::string const& name, Resource *rsc, std::string const& ctrl);

    Controller *_ctrl;
    Mailer *_mlr;
    bool _is_main_mgr;

    std::mutex _mtx;

    std::map<idp_t, ConsistencyObject*> _cst_objs;
    std::map<idp_t, idp_t> _predecessors;

    std::map<idp_t, std::condition_variable> _sync_gfind;
    std::map<idp_t, std::condition_variable> _sync_replicas;
    std::map<idp_t, std::condition_variable> _sync_free;

    // search resource
    std::map<idp_t, std::tuple<unsigned int, unsigned int, std::string>> _sr_vars;
    std::map<idp_t, std::condition_variable> _sr_cv;

    // static group
    std::map<idp_t, std::pair<unsigned int, unsigned int>> _sg_vars;
    std::map<idp_t, idp_t> _sg_cc;
    std::map<idp_t, std::condition_variable> _sg_cv;

};

}

#include "cor/services/resource_manager.tpp"

#endif
