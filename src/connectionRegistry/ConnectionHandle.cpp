/**
    @file ConnectionHandle.cpp
    @author Lime Microsystems
    @brief Represent a connection to a device
*/

#include "ConnectionHandle.h"

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

    if (not module.empty()) out += ", module="+module;
    if (not media.empty()) out += ", media="+media;
    if (not name.empty()) out += ", name="+name;
    if (not addr.empty()) out += ", addr="+addr;
    if (not serial.empty()) out += ", serial="+serial;
    if (index != -1) out += ", index="+std::to_string(index);

    if (not out.empty()) out = out.substr(2); //remove comma+space

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
