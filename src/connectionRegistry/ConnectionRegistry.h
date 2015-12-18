/**
    @file ConnectionRegistry.h
    @author Lime Microsystems
    @brief Registration and access for possible connection types.
*/

#ifndef CONNECTION_REGISTRY_H
#define CONNECTION_REGISTRY_H

class IConnection;

#include <string>
#include <vector>

/*!
 * A connection handle identifies a particular connection.
 * Handles are used by the connection registry to return
 * enumeration results and to instantiate connections.
 *
 * When using ConnectionHandle to specify an enumeration hint,
 * simply fill in only the fields that the user is interested in.
 * Each default field (empty string, -1 index) will be ignored
 * by the connection registry entry discovery implementation.
 */
class ConnectionHandle
{
public:

    ConnectionHandle(void);

    /*!
     * The connection type. Example: SPI, COM, USB.
     */
    std::string type;

    /*!
     * The name of the device. Example: STREAM, NOVENA.
     */
    std::string name;

    /*!
     * An address such a URL, IP address, or similar.
     */
    std::string addr;

    /*!
     * A serial number which is unique to a particular device.
     */
    std::string serial;

    /*!
     * An internal index which may enumerate device handles.
     */
    long long index;

    /*!
     * Serialize this connection handle into a string format.
     * This string format can be used to represent the handle.
     */
    std::string serialize(void) const;
};

/*!
 * The connection registry provides a way to register
 * discovery methods and factories for known connections,
 * and to query and instantiate available connections.
 */
class ConnectionRegistry
{
public:

    /*!
     * Discovery identifiers that can be used to create a connection.
     * The hint may contain a connection type, serial number, ip address, etc.
     * \param hint an optional connection handle with some fields filled-in
     * \return a list of handles which can be used to make a connection
     */
    static std::vector<ConnectionHandle> findConnections(const ConnectionHandle &hint = ConnectionHandle());

    /*!
     * Create a connection from an identifying handle.
     * \param handle a connection handle with fields filled-in
     * \return a pointer to a connection instance
     */
    static IConnection *makeConnection(const ConnectionHandle &handle);

    /*!
     * Free an connection created by makeConnection().
     */
    static void freeConnection(IConnection *conn);
};

/*******************************************************************
 * This section below is the registry API for connections.
 * The registry API is intended for device developers.
 ******************************************************************/

/*!
 * Create an overloaded instance of a ConnectionRegistryEntry
 * to register discovery and factory functions into the system.
 * The ConnectionRegistryEntry should be created prior to
 * discovering and instantiating connections.
 * One recommended use is at static initialization time.
 */
class ConnectionRegistryEntry
{
public:

    /*!
     * Instantiating the ConnectionRegistryEntry
     * registers it into the connection registry.
     * The name describes the type of connection,
     * that should be unique to the entry instance.
     */
    ConnectionRegistryEntry(const std::string &name);

    //! Unregister a connection type
    virtual ~ConnectionRegistryEntry(void);

    /*!
     * A discovery function takes a connection handle hint
     * and returns a list of identifiers that can be used
     * to create connection with makeConnection().
     * \param hint an optional connection handle with some fields filled-in
     * \return a list of handles which can be used to make a connection
     */
    virtual std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint) = 0;

    /*!
     * A factory function creates a connection from a connection handle.
     * \param handle a connection handle with fields filled-in
     * \return a pointer to a connection instance
     */
    virtual IConnection *make(const ConnectionHandle &handle) = 0;

private:
    std::string _name;
};

#endif

