#include "cor/services/consistency_object.hpp"

#include "cor/services/resource_manager.hpp"
#include "cor/resources/resource.hpp"

namespace cor {

ConsistencyObject::ConsistencyObject(ResourceManager *rsc_mgr, idp_t idp, bool is_owner, std::function<void(Resource*, Resource*)> update_func, std::string const& ctrl) :
    _rsc_mgr{rsc_mgr},
    _ctrl{ctrl},
    _rsc{nullptr},
    _local_ref_cntr{0},
    _global_ref_cntr{1},
    _update_func{update_func},
    _reading{0},
    _writing{0},
    _wwriters{0},
    _total_req{0},
    _next_req{0},
    _idp{idp},
    _owner{is_owner},
    _token{is_owner ? true : false},
    _validity{is_owner ? true : false},
    _wreplica{false},
    _wtoken{false},
    _wtoken_ack{false},
    _wupdate{false}
{}

ConsistencyObject::~ConsistencyObject()
{
    delete _rsc;
}

void ConsistencyObject::AcquireRead()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    while (_writing > 0 || _wwriters > 0 || (!_validity && _reading == 1))
        _rwq.wait(lk);

    // start reading
    ++_reading;

    // check if the local object is valid
    if (!_validity) {
        // send update request
        _rsc_mgr->RequestUpdate(_idp);

        // wait for update
        _rwq.wait(lk, [this]{ return _validity; });

        // no longer waiting for update
        _wupdate = false;

        // notify all threads in the readers wait queue
        _rwq.notify_all();
    }

    // notify all waiting requests
    _reqwq.notify_all();
}

void ConsistencyObject::ReleaseRead()
{
    // lock to access member variables
    std::lock_guard<std::mutex> lk(_mtx);

    // stop reading
    --_reading;

    // if no one is reading and writers are waiting
    if (_reading == 0 && _wwriters > 0) {
        // notify a writer
        _wwq.notify_one();
    }

    // notify all waiting requests
    _reqwq.notify_all();
}

void ConsistencyObject::AcquireWrite()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    ++_wwriters;

    while (_reading > 0 || _writing > 0 || _wwriters > 1)
        _wwq.wait(lk);

    if (!_token) {
        // send token update request
        _rsc_mgr->RequestTokenUpdate(_idp);
        
        // wait for token update
        _wwq.wait(lk, [this]{ return _token && _validity; });

        // no longer waiting for token
        _wtoken = false;
    }

    // start writing
    --_wwriters;
    ++_writing;

    // notify all waiting requests
    _reqwq.notify_all();
}

void ConsistencyObject::ReleaseWrite()
{
    // lock to access member variables
    std::lock_guard<std::mutex> lk(_mtx);

    // send invalidate request
    _rsc_mgr->RequestInvalidate(_idp);

    // stop writing
    --_writing;

    if (_wwriters > 0)
        // notify a waiting writer
        _wwq.notify_one();
    else
        // notify all waiting readers
        _rwq.notify_all();

    // notify all waiting requests
    _reqwq.notify_all();
}

void ConsistencyObject::AcquireReplica(Resource *rsc)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    // assign resource pointer
    _rsc = rsc;

    // no longer waiting for replica
    _wreplica = false;
}

void ConsistencyObject::ReleaseReplica(std::string const& requester)
{
    if (_ctrl == requester) {

        std::unique_lock<std::mutex> lk(_mtx);

        std::cout << "ReleaseReplica 1 <" << _idp << ">" << std::endl;

        _rsc_mgr->LeaveResourceGroup(_idp);

    } else {

        std::unique_lock<std::mutex> lk(_mtx);

        std::cout << "ReleaseReplica 2 <" << _idp << ">" << std::endl;

        --_global_ref_cntr;

        if (!_local_ref_cntr) {
            if (!_token || (_token && _global_ref_cntr == 1)) {
                lk.unlock();
                _rsc_mgr->DeallocateResource(_idp);
                _rsc_mgr->RequestReleaseReplica(_idp);
            }
        }

    }
}

void ConsistencyObject::CheckReplica(unsigned int size, std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        // generate request number
        auto req_no = _total_req++;

        while (req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;

        // waiting for replica
        _wreplica = true;

        _global_ref_cntr = size;

        // notify waiting requests
        _reqwq.notify_all();

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        // generate request number
        auto req_no = _total_req++;

        while (_writing > 0 || _wtoken || _wtoken_ack || req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;

        if (_token && _validity)
            _rsc_mgr->SendReplica(_idp, _rsc, requester);

        _global_ref_cntr = size;

        // notify waiting requests
        _reqwq.notify_all();

    }
}

void ConsistencyObject::AcquireTokenUpdate(Resource *rsc, std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    // acquire token
    _token = true;

    // update resource and mark local replica as valid
    _update_func(_rsc, rsc);
    _validity = true;

    // send token ack
    _rsc_mgr->SendTokenAck(_idp, replier);

    // notify waiting writer
    _wwq.notify_all();
}

void ConsistencyObject::CheckTokenUpdate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        // generate request number
        auto req_no = _total_req++;

        while (req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;

        // waiting token status
        _wtoken = true;
        
        // notify waiting requests
        _reqwq.notify_all();

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        // generate request number
        auto req_no = _total_req++;

        while (_writing > 0 || _wtoken || _wtoken_ack || req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;

        // if it has the token
        if (_token) {
            // release token
            _token = false;

            // waiting token ack status
            _wtoken_ack = true;

            // reply token
            _rsc_mgr->ReplyTokenUpdate(_idp, _rsc, requester);

            // notify possible release replica
            _wfree.notify_one();
        }

        // notify waiting requests
        _reqwq.notify_all();
    }
}

void ConsistencyObject::Invalidate()
{

    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    // mark local replica as invalid
    _validity = false;
}

void ConsistencyObject::Update(Resource *rsc)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    // update resource and mark local replica as valid
    _update_func(_rsc, rsc);
    _validity = true;

    // notify waiting reader
    _rwq.notify_all();
}

void ConsistencyObject::CheckUpdate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        // generate request number
        auto req_no = _total_req++;

        while (req_no > _next_req)
            _reqwq.wait(lk);
        
        // increment to next request
        ++_next_req;

        // waiting update status
        _wupdate = true;

        // notify waiting requests
        _reqwq.notify_all();

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        // generate request number
        auto req_no = _total_req++;

        // wait for the end of all writing operations
        while (_writing > 0 || _wtoken || _wtoken_ack || req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;

        if (_token && _validity)
            _rsc_mgr->ReplyUpdate(_idp, _rsc, requester);

        // notify waiting requests
        _reqwq.notify_all();

    }
}

void ConsistencyObject::TokenAck()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    // no longer waiting for token ack
    _wtoken_ack = false;

    // notify waiting requests
    _reqwq.notify_all();
}

void ConsistencyObject::IncrementLocalReferenceCounter()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    ++_local_ref_cntr;
}

void ConsistencyObject::DecrementLocalReferenceCounter()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    --_local_ref_cntr;

    //std::cout << "<" << _idp << ">   " << _local_ref_cntr << "   " << _global_ref_cntr << std::endl;
    /*
    if (!_local_ref_cntr) {
        if (!_token || (_token && _global_ref_cntr == 1)) {
            lk.unlock();
            _rsc_mgr->DeallocateResource(_idp);
            _rsc_mgr->RequestReleaseReplica(_idp);
        }
    }
    */
}

void ConsistencyObject::SetResource(Resource *rsc)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    // assign resource pointer
    _rsc = rsc;
}

Resource *ConsistencyObject::GetResource()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    // return resource pointer
    return _rsc;
}

}
