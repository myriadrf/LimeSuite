/**
    @file ConnectionSTREAMEntry.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/
#include <unistd.h>
#include "ConnectionXillybus.h"
using namespace lime;


//! make a static-initialized entry in the registry
void __loadConnectionXillybusEntry(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static ConnectionXillybusEntry XillybusEntry;
}


ConnectionXillybusEntry::ConnectionXillybusEntry(void):
    ConnectionRegistryEntry("PCIEXillybus")
{
}

ConnectionXillybusEntry::~ConnectionXillybusEntry(void)
{
}

std::vector<ConnectionHandle> ConnectionXillybusEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> handles;
    ConnectionHandle handle;
#ifndef __unix__
        
        handle.media = "PCI-E";
        handle.name = "LmsSDR-PCIE";
        handle.index = 1;
        handles.push_back(handle);
#else
    std::string fname = "/dev/xillybus_write_8";
    if( access( fname.c_str(), F_OK ) != -1 )
    {
        handle.media = "PCI-E";
        handle.name = "LmsSDR-PCIE";
        handle.index = 1;
        handles.push_back(handle);
    } 
#endif
    return handles;
}

IConnection *ConnectionXillybusEntry::make(const ConnectionHandle &handle)
{
    return new ConnectionXillybus(handle.index);
}
