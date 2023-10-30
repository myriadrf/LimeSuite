#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"
#include <mutex>
#include <map>
#include <memory>
#include <iostream>
#include <iso646.h> // alternative operators for visual c++: not, and, or...
using namespace lime;

#if 0
    #define verbose_printf(...) printf(__VA_ARGS__)
#else
#define verbose_printf(...)
#endif

static std::mutex gRegistryMutex;
static std::map<std::string, DeviceRegistryEntry *> registryEntries;

void __loadLimeSDR();
void __loadLimeSDR_X3();
void __loadLimeSDR_XTRX();
void __loadLimeSDR_MMX8();
void __loadLimeSDR_Mini();

void __loadDevicesSupport()
{
    __loadLimeSDR();
    __loadLimeSDR_X3();
    __loadLimeSDR_XTRX();
    __loadLimeSDR_MMX8();
    __loadLimeSDR_Mini();
}

/*******************************************************************
 * Registry implementation
 ******************************************************************/
std::vector<DeviceHandle> DeviceRegistry::enumerate(const DeviceHandle &hint)
{
    __loadDevicesSupport();
    std::lock_guard<std::mutex> lock(gRegistryMutex);

    std::vector<DeviceHandle> results;
    for (const auto &entry : registryEntries)
    {
        // filter by media type if specified
        if (not hint.media.empty() and hint.media != entry.first) continue;

        for (auto handle : entry.second->enumerate(hint))
            results.push_back(handle);
    }
    return results;
}

SDRDevice *DeviceRegistry::makeDevice(const DeviceHandle &handle)
{
    __loadDevicesSupport();
    std::lock_guard<std::mutex> lock(gRegistryMutex);

    //use the identifier as a hint to perform a discovery
    //only identifiers from the discovery function itself is used in the factory
    for (const auto &entry : registryEntries)
    {
        const auto r = entry.second->enumerate(handle);
        if (r.empty())
            continue;

        auto realHandle = r.front(); //just pick the first
        return entry.second->make(realHandle);
    }
    char reason[128];
    std::sprintf(reason, "No devices found with given handle(%s)", handle.Serialize().c_str());
    throw (std::runtime_error(reason));
    return nullptr;
}

void DeviceRegistry::freeDevice(SDRDevice *device)
{
    //some client code may end up freeing a null connection
    if (device == nullptr) return;

    std::lock_guard<std::mutex> lock(gRegistryMutex);

    delete device;
}

std::vector<std::string> DeviceRegistry::moduleNames(void)
{
    __loadDevicesSupport();
    std::vector<std::string> names;
    std::lock_guard<std::mutex> lock(gRegistryMutex);
    for (const auto &entry : registryEntries)
        names.push_back(entry.first);
    return names;
}

/*******************************************************************
 * Entry implementation
 ******************************************************************/
DeviceRegistryEntry::DeviceRegistryEntry(const std::string &name):
    _name(name)
{
    std::lock_guard<std::mutex> lock(gRegistryMutex);
    registryEntries[_name] = this;
    verbose_printf("DeviceRegistry Added: %s\n", _name.c_str());
}

DeviceRegistryEntry::~DeviceRegistryEntry(void)
{
    std::lock_guard<std::mutex> lock(gRegistryMutex);
    registryEntries.erase(_name);
    verbose_printf("DeviceRegistry Removed: %s\n", _name.c_str());
}
