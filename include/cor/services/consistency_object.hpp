#ifndef COR_CONSISTENCY_OBJECT_HPP
#define COR_CONSISTENCY_OBJECT_HPP

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "cor/system/macros.hpp"

namespace cor {

class Resource;
class ResourceManager;

class ConsistencyObject
{

friend class ResourceManager;

public:
    explicit ConsistencyObject(ResourceManager *rsc_mgr, idp_t idp, bool is_owner, bool global, std::function<void(Resource*, Resource*)> update_func, std::string const& ctrl);

    ~ConsistencyObject();

    ConsistencyObject() = delete;
    ConsistencyObject(ConsistencyObject const&) = delete;
    ConsistencyObject& operator=(ConsistencyObject const&) = delete;
    ConsistencyObject(ConsistencyObject&&) = delete;
    ConsistencyObject& operator=(ConsistencyObject&&) = delete;

    void AcquireRead();
    void ReleaseRead();

    void AcquireWrite();
    void ReleaseWrite();

protected:
    // interface to Resource Manager
    void AcquireReplica(Resource *rsc, std::string const& replier);
    void CheckReplica(std::string const& requester);

    void AcquireTokenUpdate(Resource *rsc, std::string const& replier);
    void CheckTokenUpdate(std::string const& requester);

    void Invalidate(std::string const& requester);

    void Update(Resource *rsc, std::string const& replier);
    void CheckUpdate(std::string const& requester);

    void TokenAck(std::string const& replier);

    void SetResource(Resource *rsc);
    Resource *GetResource();

private:
    ResourceManager *_rsc_mgr;
    std::string _ctrl;
    std::function<void(Resource*, Resource*)> _update_func;

    // control resource accesses
    std::mutex _mtx;                // mutex

    idp_t _idp;                     // resource idp
    Resource *_rsc;                 // resource
    bool _global;                   // 

    std::string _holder;            // token holder
    bool _token;                    // true -> owns token; false -> do not owns token
    bool _validity;                 // true -> local obj is valid; false -> the opposite

    std::condition_variable _wwq;   // writers wait queue
    std::condition_variable _rwq;   // readers wait queue

    std::condition_variable _reqwq; // requests wait queue
    std::uint64_t _total_req;
    std::uint64_t _next_req;
/*
    std::condition_variable _repwq; // replies wait queue
    std::uint64_t _total_rep;
    std::uint64_t _next_rep;
*/
    int _reading;                   // active readers
    int _writing;                   // active writers
    int _wwriters;                  // waiting writers

    bool _wreplica;                 // waiting for replica
    bool _wtoken;                   // waiting for token
    bool _wtoken_ack;               // waiting for token ack
    bool _wupdate;                  // waiting for update

};

}

#endif
