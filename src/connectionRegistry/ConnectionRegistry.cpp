/**
    @file   ConnectionRegistry.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  Implementation of connection registry
*/

#include "ConnectionRegistry.h"
#include "IConnection.h"
#include <mutex>
#include <map>
#include <stdexcept>
#include <memory>
#include <iostream>

/*******************************************************************
 * FIXME - temporary way to load connections until we make new loader
 ******************************************************************/
void __loadConnectionEVB7COMEntry(void);

static void __loadAllConnections(void)
{
    __loadConnectionEVB7COMEntry();
}

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

    if (not type.empty()) out += ", type="+type;
    if (not name.empty()) out += ", name="+name;
    if (not addr.empty()) out += ", addr="+addr;
    if (not serial.empty()) out += ", serial="+serial;
    if (index != -1) out += ", index="+std::to_string(index);

    if (not out.empty()) out = out.substr(2); //remove comma+space

    return out;
}

/*******************************************************************
 * Registry data structures
 ******************************************************************/
static std::mutex &registryMutex(void)
{
    static std::mutex mutex;
    return mutex;
}

static std::map<std::string, ConnectionRegistryEntry *> registryEntries;

struct SharedConnection
{
    SharedConnection(void):
        numInstances(0),
        connection(nullptr)
    {
        return;
    }
    size_t numInstances;
    IConnection *connection;
};

static std::map<std::string, std::shared_ptr<SharedConnection>> connectionCache;

/*******************************************************************
 * Registry implementation
 ******************************************************************/
std::vector<ConnectionHandle> ConnectionRegistry::findConnections(const ConnectionHandle &hint)
{
    __loadAllConnections();
    std::lock_guard<std::mutex> lock(registryMutex());

    std::vector<ConnectionHandle> result;
    for (const auto &entry : registryEntries)
    {
        const auto r = entry.second->enumerate(hint);
        result.insert(result.end(), r.begin(), r.end());
    }
    return result;
}

IConnection *ConnectionRegistry::makeConnection(const ConnectionHandle &handle)
{
    __loadAllConnections();
    std::lock_guard<std::mutex> lock(registryMutex());

    //use the identifier as a hint to perform a discovery
    //only identifiers from the discovery function itself is used in the factory
    for (const auto &entry : registryEntries)
    {
        const auto r = entry.second->enumerate(handle);
        if (r.empty()) continue;

        const auto realHandle = r.front(); //just pick the first

        //check the cache
        auto &sharedConnection = connectionCache[realHandle.serialize()];
        if (not sharedConnection)
        {
            //cache entry is empty, make a new connection
            sharedConnection.reset(new SharedConnection());
            try
            {
                sharedConnection->connection = entry.second->make(realHandle);
            }
            catch (...)
            {
                //factory failed, erase entry and re-throw
                connectionCache.erase(realHandle.serialize());
                throw;
            }
        }

        //return from cache, increment ref count
        sharedConnection->numInstances++;
        return sharedConnection->connection;
    }

    throw std::runtime_error("No connections found for " + handle.serialize());
}

void ConnectionRegistry::freeConnection(IConnection *conn)
{
    std::lock_guard<std::mutex> lock(registryMutex());

    for (auto &cacheEntry : connectionCache)
    {
        if (not cacheEntry.second) continue;
        if (cacheEntry.second->connection != conn) continue;
        cacheEntry.second->numInstances--;
        if (cacheEntry.second->numInstances != 0) continue;
        delete cacheEntry.second->connection;
        cacheEntry.second.reset();
    }

    //we never actually clear out the cache entries when they are empty
    //we can do it here, but they should never really grow indefinitely
}

/*******************************************************************
 * Entry implementation
 ******************************************************************/
ConnectionRegistryEntry::ConnectionRegistryEntry(const std::string &name)
{
    std::lock_guard<std::mutex> lock(registryMutex());
    registryEntries[_name] = this;
}

ConnectionRegistryEntry::~ConnectionRegistryEntry(void)
{
    std::lock_guard<std::mutex> lock(registryMutex());
    registryEntries.erase(_name);
}
