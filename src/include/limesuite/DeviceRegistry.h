#ifndef LIME_DEVICES_REGISTRY_H
#define LIME_DEVICES_REGISTRY_H

#include <string>
#include <vector>

#include "limesuite/config.h"
#include "limesuite/DeviceHandle.h"

namespace lime {

class SDRDevice;

/*!
 * The connection registry provides a way to register
 * discovery methods and factories for known device boards,
 * and to query and instantiate available connections to them.
 */
class LIME_API DeviceRegistry
{
  public:
    /*!
     * Discovery identifiers that can be used to create a connection.
     * The hint may contain a connection type, serial number, IP address, etc.
     * \param hint An optional connection handle with some fields filled-in
     * \return A list of handles which can be used to make a connection
     */
    static std::vector<DeviceHandle> enumerate(const DeviceHandle& hint = DeviceHandle());

    /*!
     * Create a connection from an identifying handle.
     * Returns a null pointer when no factories are available.
     * \param handle A connection handle with fields filled-in
     * \return A pointer to a connection instance (or null)
     */
    static SDRDevice* makeDevice(const DeviceHandle& handle);

    /*!
     * Free an connection created by makeConnection().
     * \param conn The connection to free.
     */
    static void freeDevice(SDRDevice* conn);

    /*!
     * Get a list of available registry entry modules by name.
     * \return std::vector<std::string> The list of available registry modules.
     */
    static std::vector<std::string> moduleNames(void);
};

/*******************************************************************
 * This section below is the registry API for devices.
 * The registry API is intended for device developers.
 ******************************************************************/

/*!
 * Create an overloaded instance of a DeviceRegistryEntry
 * to register discovery and factory functions into the system.
 * The DeviceRegistryEntry should be created prior to
 * discovering and instantiating connections.
 * One recommended use is at static initialization time.
 */
class LIME_API DeviceRegistryEntry
{
  public:
    /*!
     * Instantiating the DeviceRegistryEntry
     * registers it into the connection registry.
     * The name describes the device board type,
     * that should be unique to the entry instance.
     * 
     * \param name The name of the device entry.
     */
    DeviceRegistryEntry(const std::string& name);

    //! Unregister a connection type
    virtual ~DeviceRegistryEntry(void);

    /*!
     * A discovery function takes a connection handle hint
     * and returns a list of identifiers that can be used
     * to create connection with makeConnection().
     * \param hint An optional connection handle with some fields filled-in.
     * \return A list of handles which can be used to make a connection.
     */
    virtual std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) = 0;

    /*!
     * A factory function creates a SDRDevice from a device handle.
     * \param handle A device handle with fields filled-in.
     * \return A pointer to a SDRDevice instance.
     */
    virtual SDRDevice* make(const DeviceHandle& handle) = 0;

  private:
    std::string _name;
};

} // namespace lime
#endif
