#include <thread>
#include <chrono>

#include "ConnectionRemoteServer.h"
#include "ConnectionRegistry.h"

using namespace lime;

int main(int argc, char** argv)
{
	ConnectionRemoteServer srv;

	ConnectionHandle handle;

	IConnection* device = ConnectionRegistry::makeConnection(handle);
	srv.SetDevice(device);
	while(1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}