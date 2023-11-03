#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include "USBCommon.h"

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

class USBGeneric
{
  public:
    USBGeneric(void* usbContext);
    virtual ~USBGeneric();
    static const int32_t defaultTimeout = 1000;

    virtual bool Connect(uint16_t vid, uint16_t pid, const std::string& serial = "");
    virtual bool IsConnected();
    virtual void Disconnect();

    // return actual number of bytes transferred
    virtual int32_t BulkTransfer(uint8_t endPoint, uint8_t* data, int length, int32_t timeout_ms = defaultTimeout);

    // return actual number of bytes transferred
    virtual int32_t ControlTransfer(
        int requestType, int request, int value, int index, uint8_t* data, uint32_t length, int32_t timeout_ms = defaultTimeout);

    virtual int BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr);
    virtual bool WaitForXfer(int contextHandle, uint32_t timeout_ms);
    virtual int FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle);
    virtual void AbortEndpointXfers(uint8_t endPointAddr);
    virtual void WaitForXfers(uint8_t endPointAddr);

  protected:
    static const int USB_MAX_CONTEXTS{ 16 }; //maximum number of contexts for asynchronous transfers

    USBTransferContext* contexts;
    std::mutex contextsLock;

    bool isConnected;

    int GetUSBContextIndex();

#ifdef __unix__
    static int activeUSBconnections;
    static std::thread gUSBProcessingThread;

    libusb_device_handle* dev_handle; //a device handle
    libusb_context* ctx; //a libusb session

    virtual void HandleLibusbEvents();
#endif
};

} // namespace lime
