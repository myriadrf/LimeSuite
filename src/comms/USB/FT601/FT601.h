#pragma once

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
#endif
    }

#ifndef __unix__
    PUCHAR context;
    OVERLAPPED inOvLap;
    uint8_t endPointAddr;
#endif
};

class FT601 : public USBGeneric
{
public:
    FT601(void* usbContext = nullptr);
    virtual ~FT601();

    virtual bool Connect(uint16_t vid, uint16_t pid, const std::string &serial = "") override;
    virtual void Disconnect() override;

#ifndef __unix__
    virtual int32_t BulkTransfer(uint8_t endPoint, uint8_t *data, int length,
        int32_t timeout = USBGeneric::defaultTimeout) override;
#endif

    virtual int32_t ControlTransfer(int requestType, int request, int value, int index,
        uint8_t* data, uint32_t length,
        int32_t timeout = USBGeneric::defaultTimeout) override;

#ifndef __unix__
    virtual int BeginDataXfer(uint8_t *buffer, uint32_t length,
                              uint8_t endPointAddr) override;
    virtual bool WaitForXfer(int contextHandle, uint32_t timeout_ms) override;
    virtual int FinishDataXfer(uint8_t *buffer, uint32_t length, int contextHandle) override;
    virtual void AbortEndpointXfers(uint8_t endPointAddr) override;
#endif

    int ResetStreamBuffers();
  protected:
#ifndef __unix__
    FT_HANDLE mFTHandle;
    int ReinitPipe(unsigned char ep);
#else
    int FT_SetStreamPipe(unsigned char ep, size_t size);
    int FT_FlushPipe(unsigned char ep);
    uint32_t mUsbCounter;
#endif
};

}
