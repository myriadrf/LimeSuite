/**
    @file ConnectionHandle.cpp
    @author Lime Microsystems
    @brief Represent a connection to a device
*/

#include "ConnectionHandle.h"
#include <iso646.h> // alternative operators for visual c++: not, and, or...
using namespace lime;

/*******************************************************************
 * Connection handle implementation
 ******************************************************************/
ConnectionHandle::ConnectionHandle(void):
    index(-1)
{
    return;
}

std::string ConnectionHandle::serialize(void) const
{
    std::string out;
    if (not name.empty()) out += name;
    if (not media.empty()) out += ", media="+media; 
    if (not module.empty()) out += ", module="+module;  
    if (not addr.empty()) out += ", addr="+addr;
    if (not serial.empty()) out += ", serial="+serial;
    if (index != -1) out += ", index="+std::to_string(index);

    return out;
}

std::string ConnectionHandle::ToString(void) const
{
    std::string out;

    if (not name.empty()) out = name;

    if (out.empty()) out = this->serialize();

    return out;
}

bool operator==(const ConnectionHandle &lhs, const ConnectionHandle &rhs)
{
    return lhs.serialize() == rhs.serialize();
}
