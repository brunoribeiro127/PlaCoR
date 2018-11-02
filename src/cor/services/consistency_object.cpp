#include "cor/services/consistency_object.hpp"

#include "cor/services/resource_manager.hpp"
#include "cor/resources/resource.hpp"

namespace cor {

ConsistencyObject::ConsistencyObject(ResourceManager *rsc_mgr, idp_t idp, bool is_owner, bool global, std::function<void(Resource*, Resource*)> update_func, std::string const& ctrl) :
    _rsc_mgr{rsc_mgr},
    _ctrl{ctrl},
    _rsc{nullptr},
    _global{global},
    _update_func{update_func},
    _reading{0},
    _writing{0},
    _wwriters{0},
    _total_req{0},
    _next_req{0},
    _idp{idp},
    _token{is_owner ? true : false},
    _validity{is_owner ? true : false},
    _wreplica{false},
    _wtoken{false},
    _wtoken_ack{false},
    _wupdate{false}
{}

ConsistencyObject::~ConsistencyObject() = default;

void ConsistencyObject::AcquireRead()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    //std::cout << "Acquire read for resource <" << _idp << ">\n";

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

        // notify all threads in the readers wait queue
        _rwq.notify_all();
    }

    // notify all waiting requests
    _reqwq.notify_all();

    //std::cout << "Acquire read for resource <" << _idp << "> succeded\n";
}

void ConsistencyObject::ReleaseRead()
{
    // lock to access member variables
    std::lock_guard<std::mutex> lk(_mtx);

    //std::cout << "ReleaseRead for resource <" << _idp << ">\n";

    // stop reading
    --_reading;

    // if no one is reading and writers are waiting
    if (_reading == 0 && _wwriters > 0) {
        // notify a writer
        _wwq.notify_one();
    }

    // notify all waiting requests
    _reqwq.notify_all();

    //std::cout << "ReleaseRead for resource <" << _idp << "> succeded\n";
}

void ConsistencyObject::AcquireWrite()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    //std::cout << "AcquireWrite for resource <" << _idp << ">\n";

    ++_wwriters;

    while (_reading > 0 || _writing > 0 || _wwriters > 1)
        _wwq.wait(lk);

    if (!_token) {
        // send token update request
        _rsc_mgr->RequestTokenUpdate(_idp);
        
        // wait for token update
        _wwq.wait(lk, [this]{ return _token && _validity; });
    }

    // start writing
    --_wwriters;
    ++_writing;

    // notify all waiting requests
    _reqwq.notify_all();

    //std::cout << "AcquireWrite for resource <" << _idp << "> succeded\n";
}

void ConsistencyObject::ReleaseWrite()
{
    // lock to access member variables
    std::lock_guard<std::mutex> lk(_mtx);

    //std::cout << "ReleaseWrite for resource <" << _idp << ">\n";

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

    //std::cout << "ReleaseWrite for resource <" << _idp << "> succeded\n";
}

void ConsistencyObject::AcquireReplica(Resource *rsc, std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    //std::cout << "AcquireReplica from " << replier << " for resource <" << _idp << ">\n";

    // assign resource pointer
    _rsc = rsc;

    // no longer waiting for replica
    _wreplica = false;

    //std::cout << "AcquireReplica from " << replier << " for resource <" << _idp << "> succeded\n";
}

void ConsistencyObject::CheckReplica(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "CheckReplica from " << requester << " for resource <" << _idp << ">\n";

        // generate request number
        auto req_no = _total_req++;

        while (req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;

        // waiting for replica
        _wreplica = true;

        // notify waiting requests
        _reqwq.notify_all();

        //std::cout << "CheckReplica from " << requester << " for resource <" << _idp << "> succeded\n";

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "CheckReplica from " << requester << " for resource <" << _idp << ">\n";

        // generate request number
        auto req_no = _total_req++;

        while (_writing > 0 || _wtoken || _wtoken_ack || req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;

        if (_token && _validity)
            _rsc_mgr->SendReplica(_idp, _rsc, requester);

        // notify waiting requests
        _reqwq.notify_all();

        //std::cout << "CheckReplica from " << requester << " for resource <" << _idp << "> succeded\n";
    
    }
}

void ConsistencyObject::AcquireTokenUpdate(Resource *rsc, std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    //std::cout << "AcquireTokenUpdate from " << replier << " for resource <" << _idp << ">\n";

    // acquire token
    _token = true;

    // update resource and mark local replica as valid
    _update_func(_rsc, rsc);
    _validity = true;

    // no longer waiting for token
    _wtoken = false;

    // send token ack
    _rsc_mgr->SendTokenAck(_idp, replier);

    // notify waiting writer
    _wwq.notify_all();

    //std::cout << "AcquireTokenUpdate from " << replier << " for resource <" << _idp << "> succeded\n";
}

void ConsistencyObject::CheckTokenUpdate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "ReleaseTokenUpdate from " << requester << " for resource <" << _idp << ">\n";

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

        //std::cout << "ReleaseTokenUpdate from " << requester << " for resource <" << _idp << "> succeded\n";

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "ReleaseTokenUpdate from " << requester << " for resource <" << _idp << ">\n";

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
        }

        // notify waiting requests
        _reqwq.notify_all();

        //std::cout << "ReleaseTokenUpdate from " << requester << " for resource <" << _idp << "> succeded\n";
    }
}

void ConsistencyObject::Invalidate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "Invalidate from " << requester << " for resource <" << _idp << ">\n";
/*
        // generate request number
        auto req_no = _total_req++;

        while (req_no > _next_req)
            _reqwq.wait(lk);
        
        // increment to next request
        ++_next_req;
*/
        // notify waiting requests
        //_reqwq.notify_all();

        //std::cout << "Invalidate from " << requester << " for resource <" << _idp << "> succeded\n";

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "Invalidate from " << requester << " for resource <" << _idp << ">\n";
/*
        // generate request number
        auto req_no = _total_req++;

        while (_writing > 0 || _reading > 0 || req_no > _next_req)
            _reqwq.wait(lk);

        // increment to next request
        ++_next_req;
*/
        // mark local replica as invalid
        _validity = false;

        // notify waiting requests
        //_reqwq.notify_all();

        //std::cout << "Invalidate from " << requester << " for resource <" << _idp << "> succeded\n";
    }
}

void ConsistencyObject::Update(Resource *rsc, std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    //std::cout << "Update from " << replier << " for resource <" << _idp << ">\n";

    // update resource and mark local replica as valid
    _update_func(_rsc, rsc);
    _validity = true;

    // no longer waiting for update
    _wupdate = false;

    // notify waiting reader
    _rwq.notify_all();

    //std::cout << "Update from " << replier << " for resource <" << _idp << "> succeded\n";
}

void ConsistencyObject::CheckUpdate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "CheckUpdate from " << requester << " for resource <" << _idp << ">\n";

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

        //std::cout << "CheckUpdate from " << requester << " for resource <" << _idp << "> succeded\n";

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);

        //std::cout << "CheckUpdate from " << requester << " for resource <" << _idp << ">\n";

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

        //std::cout << "CheckUpdate from " << requester << " for resource <" << _idp << "> succeded\n";

    }
}

void ConsistencyObject::TokenAck(std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);

    //std::cout << "TokenAck from " << replier << " for resource <" << _idp << ">\n";

    // no longer waiting for token ack
    _wtoken_ack = false;

    // notify waiting requests
    _reqwq.notify_all();

    //std::cout << "TokenAck from " << replier << " for resource <" << _idp << "> succeded\n";
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
