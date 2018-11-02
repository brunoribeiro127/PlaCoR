#include "cor/services/remote_session.hpp"
#include "cor/services/session_manager.hpp"

#include <iostream>

using namespace ssh;

namespace cor {

RemoteSession::RemoteSession(std::string const& host, std::string const& port, std::string const& cmd) :
    _host{host},
    _port{port},
    _cmd{cmd},
    _th_svc{},
    _event{},
    _sess{},
    _ch{_sess},
    _cin{_sess},
    _cout{_sess},
    _cerr{_sess},
    _mtx{},
    _cv{},
    _done{false}
{}

RemoteSession::~RemoteSession() = default;

void RemoteSession::Run()
{
    _th_svc = std::move(std::thread(&RemoteSession::operator(), this));
}

void RemoteSession::Wait()
{
    _th_svc.join();
}

void RemoteSession::Start()
{
    try {
        _sess.SetOption(SSH_OPTIONS_HOST, const_cast<char*>(_host.c_str()));
        _sess.SetOption(SSH_OPTIONS_PORT_STR, const_cast<char*>(_port.c_str()));
        _sess.Connect();
        _sess.PublicKeyAuth();
        _ch.OpenSession();
        
        // stdin
        _cin.SetOutChannel(_ch, SSH_CONNECTOR_STDOUT);
        _cin.SetInFD(0);

        // stdout
        _cout.SetOutFD(1);
        _cout.SetInChannel(_ch, SSH_CONNECTOR_STDOUT);

        // stderr
        _cerr.SetOutFD(2);
        _cerr.SetInChannel(_ch, SSH_CONNECTOR_STDERR);

        // set event loop
        _event.AddConnector(_cin);
        _event.AddConnector(_cout);
        _event.AddConnector(_cerr);

        // run command
        _ch.RequestExec(_cmd);

    } catch (SshException const& ex) {
        std::cerr << "Error: " << ex.GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }
}

void RemoteSession::Stop()
{
    try {
        _event.RemoveConnector(_cin);
        _event.RemoveConnector(_cout);
        _event.RemoveConnector(_cerr);
        
        _ch.Close();
        _ch.SendEof();

        auto exit_status = _ch.GetExitStatus();
        if (exit_status)
            std::cerr << "ERROR: job terminated with exit status " << exit_status << std::endl;

        _sess.Disconnect();
    } catch (SshException const& ex) {
        std::cerr << "Error: " << ex.GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }
}
/*
void RemoteSession::operator()()
{
    Start();
    _event.DoPoll();
    Stop();
}
*/

void RemoteSession::operator()()
{
    int rc;
    char buffer[256];
    int nbytes;

    auto session = ssh_new();
    if (session == nullptr) {
        std::cerr << "ERROR CREATING SSH SESSION" << std::endl;
        exit(-1);
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, "localhost");
    ssh_options_set(session, SSH_OPTIONS_PORT_STR, "22");

    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        std::cerr << "ERROR CONNECTING TO LOCALHOST: " << ssh_get_error(session) << std::endl;
        ssh_free(session);
        exit(-1);
    }

    rc = ssh_userauth_publickey_auto(session, nullptr, nullptr);
    if (rc != SSH_OK) {
        std::cerr << "ERROR AUTHENTICATING TO LOCALHOST: " << ssh_get_error(session) << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    auto channel = ssh_channel_new(session);
    if (channel == nullptr) {
        std::cerr << "ERROR CREATING SSH CHANNEL" << std::endl;
        exit(-1);
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        std::cerr << "ERROR OPENING SSH CHANNEL" << std::endl;
        ssh_channel_free(channel);
        exit(-1);
    }

    rc = ssh_channel_request_exec(channel, _cmd.c_str());
    if (rc != SSH_OK) {
        std::cerr << "ERROR REQUEST EXEC" << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        exit(-1);
    }

    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        if (write(1, buffer, nbytes) != (unsigned int) nbytes) {
            std::cerr << "-->> ERROR READING FROM CHANNEL" << std::endl;
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            exit(-1);
        }
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < -1) {
        std::cerr << "<<-- ERROR READING FROM CHANNEL" << std::endl;
        std::cerr << ssh_get_error(session) << std::endl;
        std::cout << "<<-- " << nbytes << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        exit(-1);
    }

    ssh_channel_send_eof(channel);

    ssh_channel_close(channel);
    ssh_channel_free(channel);

    ssh_disconnect(session);
    ssh_free(session);
}

}
