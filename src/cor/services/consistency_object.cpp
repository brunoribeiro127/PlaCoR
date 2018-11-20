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
    //std::cout << "BEGIN AcquireRead -> " << _idp << std::endl;

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

    //std::cout << "END AcquireRead -> " << _idp << std::endl;
}

void ConsistencyObject::ReleaseRead()
{
    // lock to access member variables
    std::lock_guard<std::mutex> lk(_mtx);
    //std::cout << "BEGIN ReleaseRead -> " << _idp << std::endl;

    // stop reading
    --_reading;

    // if no one is reading and writers are waiting
    if (_reading == 0 && _wwriters > 0) {
        // notify a writer
        _wwq.notify_one();
    }

    // notify all waiting requests
    _reqwq.notify_all();

    //std::cout << "END ReleaseRead -> " << _idp << std::endl;
}

void ConsistencyObject::AcquireWrite()
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);
    //std::cout << "BEGIN AcquireWrite -> " << _idp << std::endl;

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

    //std::cout << "END AcquireWrite -> " << _idp << std::endl;
}

void ConsistencyObject::ReleaseWrite()
{
    // lock to access member variables
    std::lock_guard<std::mutex> lk(_mtx);
    //std::cout << "BEGIN ReleaseWrite -> " << _idp << std::endl;

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

    //std::cout << "END ReleaseWrite -> " << _idp << std::endl;
}

void ConsistencyObject::AcquireReplica(Resource *rsc, std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);
    //std::cout << "BEGIN AcquireReplica -> " << _idp << std::endl;

    // assign resource pointer
    _rsc = rsc;

    // no longer waiting for replica
    _wreplica = false;

    //std::cout << "END AcquireReplica -> " << _idp << std::endl;
}

void ConsistencyObject::CheckReplica(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN SELF CheckReplica -> " << _idp << std::endl;

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

        //std::cout << "END SELF CheckReplica -> " << _idp << std::endl;

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN CheckReplica -> " << _idp << std::endl;

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

        //std::cout << "END CheckReplica -> " << _idp << std::endl;

    }
}

void ConsistencyObject::AcquireTokenUpdate(Resource *rsc, std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);
    //std::cout << "BEGIN AcquireTokenUpdate -> " << _idp << std::endl;

    // acquire token
    _token = true;

    // update resource and mark local replica as valid
    _update_func(_rsc, rsc);
    _validity = true;

    // send token ack
    _rsc_mgr->SendTokenAck(_idp, replier);

    // notify waiting writer
    _wwq.notify_all();

    //std::cout << "END AcquireTokenUpdate -> " << _idp << std::endl;
}

void ConsistencyObject::CheckTokenUpdate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN SELF CheckTokenUpdate -> " << _idp << std::endl;

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

        //std::cout << "END SELF CheckTokenUpdate -> " << _idp << std::endl;

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN CheckTokenUpdate -> " << _idp << std::endl;

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

        //std::cout << "END CheckTokenUpdate -> " << _idp << std::endl;
    }
}

void ConsistencyObject::Invalidate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN SELF Invalidate -> " << _idp << std::endl;

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

        //std::cout << "END SELF Invalidate -> " << _idp << std::endl;

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN Invalidate -> " << _idp << std::endl;

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

        //std::cout << "END Invalidate -> " << _idp << std::endl;
    }
}

void ConsistencyObject::Update(Resource *rsc, std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);
    //std::cout << "BEGIN Update -> " << _idp << std::endl;

    // update resource and mark local replica as valid
    _update_func(_rsc, rsc);
    _validity = true;

    // notify waiting reader
    _rwq.notify_all();

    //std::cout << "END Update -> " << _idp << std::endl;
}

void ConsistencyObject::CheckUpdate(std::string const& requester)
{
    if (_ctrl == requester) {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN SELF CheckUpdate -> " << _idp << std::endl;

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

        //std::cout << "END SELF CheckUpdate -> " << _idp << std::endl;

    } else {

        // lock to access member variables
        std::unique_lock<std::mutex> lk(_mtx);
        //std::cout << "BEGIN CheckUpdate -> " << _idp << std::endl;

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

        //std::cout << "END CheckUpdate -> " << _idp << std::endl;

    }
}

void ConsistencyObject::TokenAck(std::string const& replier)
{
    // lock to access member variables
    std::unique_lock<std::mutex> lk(_mtx);
    //std::cout << "BEGIN TokenAck -> " << _idp << std::endl;

    // no longer waiting for token ack
    _wtoken_ack = false;

    // notify waiting requests
    _reqwq.notify_all();

    //std::cout << "END TokenAck -> " << _idp << std::endl;
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
