#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <vector>
#include <mutex>

#include "libssh/libssh.hpp"

class RemoteSession;

class SessionManager
{

public:
    SessionManager();
    ~SessionManager();

    SessionManager(SessionManager const&) = delete;
    SessionManager& operator=(SessionManager const&) = delete;

    SessionManager(SessionManager&&) noexcept = delete;
    SessionManager& operator=(SessionManager&&) noexcept = delete;
    
    void StartService();
    void StopService();

    void CreateRemoteSession(std::string const& host, std::string const& port, std::string const& cmd);

private:
    std::vector<RemoteSession*> _sessions;
    std::mutex _mtx;

};

#endif
