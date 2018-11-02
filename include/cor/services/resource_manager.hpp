#ifndef COR_RESOURCE_MANAGER_HPP
#define COR_RESOURCE_MANAGER_HPP

#include <map>
#include <mutex>
#include <condition_variable>
#include <typeindex>

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

    void CreateMetaDomain(std::string const& ctrl);

    template <typename T>
    ResourcePtr<T> AllocateResource(idp_t idp, idp_t ctx, std::string const& name, bool global, Resource *rsc, std::string const& ctrl);

    template <typename T>
    ResourcePtr<T> CreateReference(idp_t idp, idp_t ref, idp_t ctx, std::string const& name, std::string const& ctrl);

    template <typename T>
    void CreateReplica(idp_t idp, std::string const& ctrl);

    void InsertResourceReplica(idp_t idp, Resource *rsc, std::string const& ctrl);

    ConsistencyObject *GetConsistencyObject(idp_t idp);

    Resource *GetResource(idp_t idp);

    template <typename T>
    ResourcePtr<T> GetLocalResource(idp_t idp);

    // not global function, only local domain
    idp_t GetDomainIdp(idp_t idp);
    
    // not global function, only local resources predecessor
    idp_t GetPredecessorIdp(idp_t idp);

    // interface to Controller
    void SendResourceAllocationInfo(idp_t idp);

    void FindGlobalResource(idp_t idp);
    void HandleFindGlobalResource(idp_t idp, std::string ctrl);
    void SendGlobalResourceFound(idp_t idp, std::string const& ctrl);
    void GlobalResourceFound(idp_t idp);

    void HandleJoinResourceGroup(idp_t idp, std::string requester);

    void CheckUpdate(idp_t idp, std::string requester);
    void Update(idp_t idp, Resource *rsc, std::string replier);

    void Invalidate(idp_t idp, std::string requester);

    void CheckTokenUpdate(idp_t idp, std::string requester);
    void AcquireTokenUpdate(idp_t idp, Resource *rsc, std::string replier);

    void TokenAck(idp_t idp, std::string const& requester);

    ResourceManager() = delete;
    ResourceManager(ResourceManager const&) = delete;
    ResourceManager& operator=(ResourceManager const&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

protected:
    // interface to ResourcePtr
    void IncrementResourceReferenceCounter(idp_t idp);
    void DecrementResourceReferenceCounter(idp_t idp);

    // interface to ConsistencyObject
    void SendReplica(idp_t idp, Resource *rsc, std::string const& requester);

    void RequestUpdate(idp_t idp);
    void ReplyUpdate(idp_t idp, Resource *rsc, std::string const& requester);

    void RequestInvalidate(idp_t idp);

    void RequestTokenUpdate(idp_t idp);
    void ReplyTokenUpdate(idp_t idp, Resource *rsc, std::string const& requester);

    void SendTokenAck(idp_t idp, std::string const& replier);

private:
    void JoinResourceGroup(idp_t idp);
    void InsertAlias(idp_t alias, idp_t idp);
    idp_t ResolveIdp(idp_t idp);

    Controller *_ctrl;
    Mailer *_mlr;
    bool _is_main_mgr;

    std::mutex _mtx;

    std::map<idp_t, std::uint16_t> _ref_cntrs;
    std::map<idp_t, ConsistencyObject*> _cst_objs;

    std::map<idp_t, idp_t> _predecessors;
    std::map<idp_t, idp_t> _alias;

    std::map<idp_t, std::condition_variable> _sync_gfind;
    std::map<idp_t, std::condition_variable> _sync_replicas;
};

}

#include "cor/services/resource_manager.tpp"

#endif
