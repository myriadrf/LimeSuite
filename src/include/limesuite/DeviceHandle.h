#ifndef LIME_DEVICE_HANDLE_H
#define LIME_DEVICE_HANDLE_H

#include <string>

#include "limesuite/config.h"

namespace lime {
/*!
 * A SDRDevice handle identifies a particular device connection.
 * Handles are used by the device registry to return
 * enumeration results and to instantiate connections.
 *
 * When using DeviceHandle to specify an enumeration hint,
 * simply fill in only the fields that the user is interested in.
 * Each default field (empty string, -1 index) will be ignored
 * by the connection registry entry discovery implementation.
 */
class LIME_API DeviceHandle
{
public:

    DeviceHandle(void);

    //! Create from serialized string with key=value pairs
    DeviceHandle(const std::string &args);

    /*!
     * The name of the IConnection support module.
     * Example EVB7COM, EVB7STREAM, NOVENA7.
     */
    std::string module;

    /*!
     * The connection media type.
     * Example: SPI, COM, USB.
     */
    std::string media;

    /*!
     * The name of the device. Example: STREAM, NOVENA.
     */
    std::string name;

    /*!
     * An address such a URL, IP address, VID:PID or similar.
     */
    std::string addr;

    /*!
     * A serial number which is unique to a particular device.
     */
    std::string serial;

    /*!
     * Serialize this connection handle into a string format.
     * This string format can be used to represent the handle.
     */
    std::string serialize(void) const;

    /*!
     * Get a displayable string for this handle.
     * @return a string that may be printed
     */
    std::string ToString(void) const;
};

}
//! Check two connection handles for equality
bool operator==(const lime::DeviceHandle &lhs, const lime::DeviceHandle &rhs);

#endif