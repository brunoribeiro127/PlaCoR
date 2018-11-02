#include "session_manager.hpp"

#include "remote_session.hpp"

#include <thread>
#include <algorithm>

using namespace std::chrono_literals;

SessionManager::SessionManager() :
    _sessions{},
    _mtx{}
{}

SessionManager::~SessionManager()
{
    std::for_each(_sessions.begin(), _sessions.end(), [](RemoteSession *rs) -> void { delete rs; });
}

void SessionManager::StartService()
{
}

void SessionManager::StopService()
{
    std::unique_lock<std::mutex> lk(_mtx);

    for (auto rs: _sessions)
        rs->Wait();
}

void SessionManager::CreateRemoteSession(std::string const& host, std::string const& port, std::string const& cmd)
{
    auto rs = new RemoteSession(host, port, cmd);
    rs->Run();

    {
        std::unique_lock<std::mutex> lk(_mtx);
        _sessions.emplace_back(rs);
    }
}
