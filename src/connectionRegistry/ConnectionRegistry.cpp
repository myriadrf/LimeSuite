/**
    @file   ConnectionRegistry.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  Implementation of connection registry
*/

#include "ConnectionRegistry.h"
#include "IConnection.h"
#include <mutex>
#include <map>
#include <memory>
#include <iostream>
#include <iso646.h> // alternative operators for visual c++: not, and, or...

/*******************************************************************
 * FIXME - temporary way to load connections until we make new loader
 ******************************************************************/
void __loadConnectionEVB7COMEntry(void);
void __loadConnectionSTREAMEntry(void);

static void __loadAllConnections(void)
{
    __loadConnectionEVB7COMEntry();
    __loadConnectionSTREAMEntry();
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

    std::vector<ConnectionHandle> results;
    for (const auto &entry : registryEntries)
    {
        for (auto handle : entry.second->enumerate(hint))
        {
            //insert the module name, which can be filtered on in makeConnection()
            handle.module = entry.first;
            results.push_back(handle);
        }
    }
    return results;
}

IConnection *ConnectionRegistry::makeConnection(const ConnectionHandle &handle)
{
    __loadAllConnections();
    std::lock_guard<std::mutex> lock(registryMutex());

    //use the identifier as a hint to perform a discovery
    //only identifiers from the discovery function itself is used in the factory
    for (const auto &entry : registryEntries)
    {
        //filter by module name when specified
        if (not handle.module.empty() and handle.module != entry.first) continue;

        const auto r = entry.second->enumerate(handle);
        if (r.empty()) continue;

        auto realHandle = r.front(); //just pick the first
        realHandle.module = entry.first;

        //check the cache
        auto &sharedConnection = connectionCache[realHandle.serialize()];
        if (not sharedConnection)
        {
            //cache entry is empty, make a new connection
            sharedConnection.reset(new SharedConnection());
            try
            {
                sharedConnection->connection = entry.second->make(realHandle);
                if (sharedConnection->connection != nullptr)
                {
                    sharedConnection->connection->_handle = realHandle;
                }
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

    return nullptr;
}

void ConnectionRegistry::freeConnection(IConnection *conn)
{
    //some client code may end up freeing a null connection
    if (conn == nullptr) return;

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
ConnectionRegistryEntry::ConnectionRegistryEntry(const std::string &name):
    _name(name)
{
    std::lock_guard<std::mutex> lock(registryMutex());
    registryEntries[_name] = this;
}

ConnectionRegistryEntry::~ConnectionRegistryEntry(void)
{
    std::lock_guard<std::mutex> lock(registryMutex());
    registryEntries.erase(_name);
}
