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
template <typename> friend class ResourcePtr;

public:
    explicit ConsistencyObject(ResourceManager *rsc_mgr, idp_t idp, bool is_owner, std::function<void(Resource*, Resource*)> update_func, std::string const& ctrl);

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
    // interface to ResourceManager
    void AcquireReplica(Resource *rsc);
    void ReleaseReplica(std::string const& requester);
    void CheckReplica(unsigned int size, std::string const& requester);

    void AcquireTokenUpdate(Resource *rsc, std::string const& replier);
    void CheckTokenUpdate(std::string const& requester);

    void Invalidate();

    void Update(Resource *rsc);
    void CheckUpdate(std::string const& requester);

    void TokenAck();

    // interface to ResourcePtr
    void IncrementLocalReferenceCounter();
    void DecrementLocalReferenceCounter();

    void SetResource(Resource *rsc);
    Resource *GetResource();

private:
    ResourceManager *_rsc_mgr;
    std::string _ctrl;
    std::function<void(Resource*, Resource*)> _update_func;

    // control resource accesses
    std::mutex _mtx;                // mutex

    idp_t _idp;                     // resource idp
    Resource *_rsc;                 // resource obj
    
    std::condition_variable _wfree; // wait free cv
    std::uint16_t _local_ref_cntr;  // local reference counter
    std::uint16_t _global_ref_cntr; // global reference counter

    bool _owner;                    // true -> created the resource; false -> created a reference for the resource
    bool _token;                    // true -> owns token; false -> do not owns token
    bool _validity;                 // true -> local obj is valid; false -> the opposite

    std::condition_variable _wwq;   // writers wait queue
    std::condition_variable _rwq;   // readers wait queue

    std::condition_variable _reqwq; // requests wait queue
    std::uint64_t _total_req;       // total requests
    std::uint64_t _next_req;        // next request to be processed

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
