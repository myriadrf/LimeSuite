#ifndef FT601_H
#define FT601_H

#include "USBCommon.h"
#include "USBGeneric.h"

#ifndef __unix__
#include "windows.h"
#include "FTD3XXLibrary/FTD3XX.h"
#else
#include <libusb.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#endif

namespace lime 
{

class USBTransferContext_FT601 : public USBTransferContext
{
public:
    USBTransferContext_FT601() : USBTransferContext()
    {
#ifndef __unix__
        context = NULL;
#else
        transfer = libusb_alloc_transfer(0);
        bytesXfered = 0;
        done = 0;
#endif
    }

    ~USBTransferContext_FT601()
    {
#ifdef __unix__
        if (transfer)
        {
            libusb_free_transfer(transfer);
        }
#endif
    }

    bool reset()
    {
        if (used)
        {
            return false;
        }

        return true;
    }
#ifndef __unix__
    PUCHAR context;
    OVERLAPPED inOvLap;
    uint8_t endPointAddr;
#else
    libusb_transfer* transfer;
    long bytesXfered;
    std::atomic<bool> done;
    std::mutex transferLock;
    std::condition_variable cv;
#endif
};

class FT601 : public USBGeneric
{
public:
    FT601(void* usbContext = nullptr);
    virtual ~FT601();

    virtual bool Connect(uint16_t vid, uint16_t pid, const std::string &serial = "") override;
    virtual bool IsConnected() override;
    virtual void Disconnect() override;

    virtual int32_t BulkTransfer(uint8_t endPoint, uint8_t *data, int length,
        int32_t timeout = USBGeneric::defaultTimeout) override;

    virtual int32_t ControlTransfer(int requestType, int request, int value, int index,
        uint8_t* data, uint32_t length,
        int32_t timeout = USBGeneric::defaultTimeout) override { return length; };

    virtual int BeginDataXfer(uint8_t *buffer, uint32_t length,
                              uint8_t endPointAddr) override;
    virtual bool WaitForXfer(int contextHandle, uint32_t timeout_ms) override;
    virtual int FinishDataXfer(uint8_t *buffer, uint32_t length, int contextHandle) override;
    virtual void AbortEndpointXfers(uint8_t endPointAddr) override;

  protected:
    static const int USB_MAX_CONTEXTS {16}; //maximum number of contexts for asynchronous transfers

    USBTransferContext_FT601* contexts;
    std::mutex contextsLock;

    bool isConnected;

#ifndef __unix__
    FT_HANDLE mFTHandle;
    int ReinitPipe(unsigned char ep);
#else
    int FT_SetStreamPipe(unsigned char ep, size_t size);
    int FT_FlushPipe(unsigned char ep);
    uint32_t mUsbCounter;
    libusb_device_handle* dev_handle; //a device handle
    libusb_context* ctx; //a libusb session
    void handle_libusb_events();
#endif
};

}

#endif // FT601_H