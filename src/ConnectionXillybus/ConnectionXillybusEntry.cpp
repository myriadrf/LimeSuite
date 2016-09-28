/**
    @file ConnectionSTREAMEntry.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/
#ifdef __unix__
#include <unistd.h>
#endif
#include "ConnectionXillybus.h"
using namespace lime;
#include <fstream>


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
	std::string fileName = "\\\\.\\xillybus_write_8";
	std::ifstream fin(fileName.c_str());

	HANDLE fh = CreateFileA(fileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (fh != INVALID_HANDLE_VALUE || ::GetLastError() == ERROR_BUSY)
	{
		handle.media = "PCI-E";
		handle.name = "LimeSDR-PCIE";
		handle.index = 1;
		handles.push_back(handle);
		CloseHandle(fh);
	}

#else
    std::string fname = "/dev/xillybus_write_8";
    if( access( fname.c_str(), F_OK ) != -1 )
    {
        handle.media = "PCI-E";
        handle.name = "LimeSDR-PCIE";
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
