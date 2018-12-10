#ifndef COR_CONTROLLER_HPP
#define COR_CONTROLLER_HPP

#include <string>
#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>

#include <ssrc/spread.h>
#include "cor/external/event/event.hpp"

#include "cor/system/macros.hpp"
#include "cor/services/page_manager.hpp"
#include "cor/services/resource_manager.hpp"

namespace cor {

class Resource;
class SessionManager;
class ResourceManager;
class ConsistencyObject;
template <typename> class ResourcePtr;

class Controller
{

friend class PageManager;
friend class ResourceManager;

public:
    explicit Controller(std::string const& app_group, std::string const& communicator, unsigned int npods, Mailer *mlr);

    ~Controller();

    void StartService();
    void StopService();

    void operator()();

    idp_t GenerateIdp();

    template <typename T>
    ResourcePtr<T> AllocateResource(idp_t idp, idp_t ctx, std::string const& name, bool global, Resource *rsc);

    ConsistencyObject *GetConsistencyObject(idp_t idp);

    unsigned int GetTotalDomains();

    // not global function, only local domain
    idp_t GetDomainIdp(idp_t idp);

    // not global function, only local resources predecessor
    idp_t GetPredecessorIdp(idp_t idp);

    template <typename T>
    ResourcePtr<T> GetLocalResource(idp_t idp);

    template <typename T, typename ... Args>
    ResourcePtr<T> Create(idp_t ctx, std::string const& name, bool global, Args&& ... args);

    template <typename T, typename ... Args>
    ResourcePtr<T> CreateCollective(idp_t ctx, std::string const& name, unsigned int total_nembers, bool global, Args&& ... args);

    template <typename T>
    ResourcePtr<T> CreateReference(idp_t idp, idp_t ctx, std::string const& name);

    idp_t Spawn(std::string const& comm, unsigned int npods, idp_t parent, std::string const& module, std::vector<std::string> const& args, std::vector<std::string> const& hosts);

    // accessed by StaticOrganizer
    void SynchronizeCollectiveGroup(std::string const& comm);

    Controller() = delete;
    Controller(Controller const&) = delete;
    Controller& operator=(Controller const&) = delete;
    Controller(Controller&&) = delete;
    Controller& operator=(Controller&&) = delete;

protected:
    // methods to handle received messages
    void HandleMessage();
    void HandleRegularMessage();

    void Initialize();
    void HandleInitialize();

    void InitializeContext();
    void HandleInitializeContext();

    void FinalizeContext();
    void HandleFinalizeContext();

    void Finalize();
    void HandleFinalize();

    // interface to PageManager
    // request new page
    void RequestPage();
    void HandlePageRequest();
    void HandlePageReply();

    // interface to ResourceManager
    void SendResourceAllocationInfo(idp_t idp);
    void HandleResourceAllocationInfo();

    void SendFindResourceRequest(idp_t idp);
    void HandleFindResourceRequest();
    void SendFindResourceReply(idp_t idp, std::string const& ctrl);
    void HandleFindResourceReply();

    void SendReplica(idp_t idp, Resource *rsc, std::string const& ctrl);
    void HandleCreateReplica();

    void SendUpdateRequest(idp_t idp);
    void HandleUpdateRequest();
    void SendUpdateReply(idp_t idp, Resource *rsc, std::string const& ctrl);
    void HandleUpdateReply();

    void SendInvalidateRequest(idp_t idp);
    void HandleInvalidateRequest();

    void SendTokenUpdateRequest(idp_t idp);
    void HandleTokenUpdateRequest();
    void SendTokenUpdateReply(idp_t idp, Resource *rsc, std::string const& ctrl);
    void HandleTokenUpdateReply();

    void SendTokenAck(idp_t idp, std::string const& ctrl);
    void HandleTokenAck();

    void SendCollectiveGroupCreate(std::string const& comm);
    void HandleCollectiveGroupCreate();

    void SendCollectiveGroupSync(std::string const& comm);
    void HandleCollectiveGroupSync();

    void SendCollectiveGroupIdp(std::string const& comm, idp_t idp);
    void HandleCollectiveGroupIdp();

private:
    std::string const& GetName() const;

    void JoinResourceGroup(idp_t idp);
    void LeaveResourceGroup(idp_t idp);
    bool IsResourceGroup(std::string const& group);
    std::string GetResourceGroup(idp_t idp);
    idp_t GetIdpFromResourceGroup(std::string const& group);

    void JoinCommunicatorGroup(std::string const& comm);
    void LeaveCommunicatorGroup(std::string const& comm);
    bool IsCommunicatorGroup(std::string const& group);
    std::string GetCommunicatorGroup(std::string const& comm);
    std::string GetCommFromCommunicatorGroup(std::string const& group);

    enum class MsgType: std::int16_t
    {
        PageRequest,
        PageReply,

        ResourceAllocation,

        FindResourceRequest,
        FindResourceReply,

        CreateReplica,

        UpdateRequest,
        UpdateReply,
        
        InvalidateRequest,

        TokenUpdateRequest,
        TokenUpdateReply,

        TokenAck,

        CollectiveGroupCreate,
        CollectiveGroupSync,
        CollectiveGroupIdp,

        FinalizeContext,
        Finalize
    };

    constexpr typename std::underlying_type<MsgType>::type underlying_cast(MsgType t) const noexcept
    {
        return static_cast<typename std::underlying_type<MsgType>::type>(t);
    }

    std::string _app_group;
    std::string _communicator;
    unsigned int _npods;

    // service thread
    std::thread _th_svc;

    // auxiliary variables to synchronize the initial context
    bool _is_main_ctrl;
    std::promise<bool> _psync;
    std::future<bool> _fsync;
    
    unsigned int _init_total_npods;
    unsigned int _final_total_npods;
    unsigned int _init_ctx_npods;
    unsigned int _final_ctx_npods;

    std::condition_variable _cv;
    std::mutex _mtx;

    // services
    Mailer *_mlr;
    PageManager *_pg_mgr;
    ResourceManager *_rsc_mgr;
    SessionManager *_sess_mgr;

    // communication system
    ssrc::spread::Mailbox *_mbox;
    ev::EventBase *_base;
    ev::Event *_evread;

    // variables to handle received messages
    ssrc::spread::ScatterMessage _smsg;
    ssrc::spread::Message _msg;
    ssrc::spread::GroupList _groups;
    ssrc::spread::MembershipInfo _info;

};

}

#include "cor/services/controller.tpp"

#endif
