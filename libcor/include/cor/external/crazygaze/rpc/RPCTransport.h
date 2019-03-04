#pragma once

namespace cz
{
namespace rpc
{

class Transport
{
  public:
	virtual ~Transport() {}

	// Send one single RPC
	virtual void send(std::string group, std::vector<char> data) = 0;

	// Receive one single RPC
	// data : Will contain the data for one single RPC, or empty if no RPC available
	// return: true if the transport is still alive, false if the transport closed
	virtual bool receive(std::string& group, std::vector<char>& data) = 0;

	// Close connection to the peer
	virtual void close() = 0;
};
}
}

