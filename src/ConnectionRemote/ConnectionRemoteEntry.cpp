#include "ConnectionRemote.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace lime;

//! make a static-initialized entry in the registry
void __loadConnectionRemoteEntry(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static ConnectionRemoteEntry RemoteLinkEntry;
}

ConnectionRemoteEntry::ConnectionRemoteEntry(void):
    ConnectionRegistryEntry("Z_Remote") //Z just to appear last on the list
{
    return;
}

std::vector<ConnectionHandle> ConnectionRemoteEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> result;

    ConnectionHandle handle;
    handle.media = "TCP";
    handle.name = "Remote";
    handle.addr = hint.addr;
    result.push_back(handle);

    return result;
}

IConnection *ConnectionRemoteEntry::make(const ConnectionHandle &handle)
{
    // TODO parse port number
    return new ConnectionRemote(handle.addr.c_str(), 5000);
}
