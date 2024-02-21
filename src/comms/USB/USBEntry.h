#ifndef LIME_USBENTRY_H
#define LIME_USBENTRY_H

#include "limesuite/DeviceRegistry.h"
#include "USBEntry.h"
#include <cstdint>
#include <set>

#ifdef __unix__
    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    #include <libusb.h>
    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif
#endif

namespace lime {

struct VidPid {
    uint16_t vid;
    uint16_t pid;

    bool operator<(const VidPid& other) const
    {
        if (vid == other.vid)
        {
            return pid < other.pid;
        }

        return vid < other.vid;
    }
};

/** @brief An abstract class for a USB device entry. */
class USBEntry : public DeviceRegistryEntry
{
  public:
    USBEntry(const std::string& name, const std::set<VidPid>& deviceIds);
    virtual ~USBEntry();

    virtual std::vector<DeviceHandle> enumerate(const DeviceHandle& hint);

  protected:
#ifdef __unix__
    static libusb_context* ctx;
    static uint ctxRefCount;
#endif
  private:
    std::set<VidPid> mDeviceIds;
#ifdef __unix__
    std::string GetUSBDeviceSpeedString(libusb_device* device);
    DeviceHandle GetDeviceHandle(libusb_device_handle* tempHandle, libusb_device* device, const libusb_device_descriptor& desc);
#endif
};

} // namespace lime

#endif // LIME_USBENTRY_H