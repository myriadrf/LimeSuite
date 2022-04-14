#ifndef CONNECTION_REMOTE_SERVER_H
#define CONNECTION_REMOTE_SERVER_H

#include <thread>
#include "IConnection.h"

namespace lime
{

class ConnectionRemoteServer
{
public:
	ConnectionRemoteServer();
	~ConnectionRemoteServer();

	void SetDevice(IConnection* dev);
	int Start(uint16_t listenPort);
	void Shutdown();
protected:
	IConnection* device;
	bool m_Active;
	void ProcessConnections();
	int socketFd;
	std::thread remoteThread;
};

}
#endif