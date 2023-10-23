#pragma once
#include "USBGeneric.h"

#include <vector>
#include <set>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include <ciso646>

#ifndef __unix__
#include "windows.h"
#include "CyAPI.h"
#else
#include <libusb.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

namespace lime
{

/** @brief Wrapper class for holding USB asynchronous transfers contexts
*/
class USBTransferContext
{
public:
    USBTransferContext() : used(false)
    {
#ifndef __unix__
        inOvLap = new OVERLAPPED;
        memset(inOvLap, 0, sizeof(OVERLAPPED));
        inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
        context = NULL;
        EndPt = nullptr;
#else
        transfer = libusb_alloc_transfer(0);
        bytesXfered = 0;
        done = 0;
#endif
    }
    ~USBTransferContext()
    {
#ifndef __unix__
        CloseHandle(inOvLap->hEvent);
        delete inOvLap;
#else
        if (transfer)
            libusb_free_transfer(transfer);
#endif
    }
    bool reset()
    {
        if(used)
            return false;
#ifndef __unix__
        CloseHandle(inOvLap->hEvent);
        memset(inOvLap, 0, sizeof(OVERLAPPED));
        inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
#endif
        return true;
    }
    bool used;
#ifndef __unix__
    PUCHAR context;
    CCyUSBEndPoint* EndPt;
    OVERLAPPED* inOvLap;
#else
    libusb_transfer* transfer;
    long bytesXfered;
    std::atomic<bool> done;
    std::mutex transferLock;
    std::condition_variable cv;
#endif
};

class FX3 : public USBGeneric
{
public:
    FX3(void* usbContext = nullptr);
    virtual ~FX3();

    virtual bool Connect(uint16_t vid, uint16_t pid, const std::string &serial = "") override;
    virtual bool IsConnected() override;
    virtual void Disconnect() override;

    virtual int32_t BulkTransfer(uint8_t endPoint, uint8_t *data, int length,
        int32_t timeout = USBGeneric::defaultTimeout) override;

    virtual int32_t ControlTransfer(int requestType, int request, int value, int index,
        uint8_t* data, uint32_t length,
        int32_t timeout = USBGeneric::defaultTimeout) override;

    virtual int BeginDataXfer(uint8_t *buffer, uint32_t length,
                              uint8_t endPointAddr) override;
    virtual bool WaitForXfer(int contextHandle, uint32_t timeout_ms) override;
    virtual int FinishDataXfer(uint8_t *buffer, uint32_t length, int contextHandle) override;
    virtual void AbortEndpointXfers(uint8_t endPointAddr) override;

  protected:
    static const int USB_MAX_CONTEXTS = 16; //maximum number of contexts for asynchronous transfers

    USBTransferContext* contexts;
    std::mutex contextsLock;

    bool isConnected;

#ifdef __unix__
    libusb_device_handle* dev_handle; //a device handle
    libusb_context* ctx; //a libusb session
    void handle_libusb_events();
#endif
};

}
