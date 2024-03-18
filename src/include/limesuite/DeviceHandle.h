#ifndef LIME_DEVICE_HANDLE_H
#define LIME_DEVICE_HANDLE_H

#include <string>

#include "limesuite/config.h"

namespace lime {
/**
  @brief A DeviceHandle identifies a particular device.
  Handles are used by the device registry to return
  enumeration results and to instantiate connections.

  When using DeviceHandle to specify an enumeration hint,
  simply fill in only the fields that the user is interested in.
  Each default field (empty string) will be ignored
  by the connection registry entry discovery implementation.
 */
class LIME_API DeviceHandle
{
  public:
    DeviceHandle(void);

    /**
      @brief Create from serialized string with key=value pairs.
      @param args A serialized string with key=value pairs.
     */
    DeviceHandle(const std::string& args);

    /*!
      @brief The connection media type.
      Example: SPI, COM, USB, PCIe...
     */
    std::string media;

    /// The name of the device. Example: "LimeSDR-USB", "LimeSDR X3", "LimeSDR Mini".
    std::string name;

    /// An address such a URL, IP address, VID:PID or similar.
    std::string addr;

    /// A serial number which is unique to a particular device.
    std::string serial;

    /*!
     * Serialize this connection handle into a string format.
     * This string format can be used to represent the handle.
     * 
     * @return The handle in string format.
     */
    std::string Serialize(void) const;

    /*!
     * Get a displayable string for this handle.
     * @return A string that may be printed.
     */
    std::string ToString(void) const;

    /*!
     * Checks whether both handles are equal, but ignoring fields that are empty on the passed in one.
     * NOTE: foo.IsEqualIgnoringEmpty(bar) IS NOT EQUAL TO bar.IsEqualIgnoringEmpty(foo)
     * @param other The handle to compare to (empty values from this one are ignored)
     * @return Whether both DeviceHandles are equal (ignoring empty fields)
     */
    bool IsEqualIgnoringEmpty(const DeviceHandle& other) const;
};

} // namespace lime
//! Check two connection handles for equality
bool operator==(const lime::DeviceHandle& lhs, const lime::DeviceHandle& rhs);

#endif
