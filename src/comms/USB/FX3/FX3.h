#pragma once
#include "USBGeneric.h"
#include "USBCommon.h"

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
    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    #include <libusb.h>
    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif
    #include <mutex>
    #include <condition_variable>
    #include <chrono>
#endif

namespace lime {

class USBTransferContext_FX3 : public USBTransferContext
{
  public:
    USBTransferContext_FX3()
        : USBTransferContext()
    {
#ifndef __unix__
        inOvLap = new OVERLAPPED;
        memset(inOvLap, 0, sizeof(OVERLAPPED));
        inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
        context = NULL;
        EndPt = nullptr;
#endif
    }

#ifndef __unix__
    ~USBTransferContext_FX3()
    {
        CloseHandle(inOvLap->hEvent);
        delete inOvLap;
    }
#endif

    bool Reset() override
    {
        if (used)
        {
            return false;
        }
#ifndef __unix__
        CloseHandle(inOvLap->hEvent);
        memset(inOvLap, 0, sizeof(OVERLAPPED));
        inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
#endif
        return true;
    }

#ifndef __unix__
    PUCHAR context;
    CCyUSBEndPoint* EndPt;
    OVERLAPPED* inOvLap;
#endif
};

class FX3 : public USBGeneric
{
  public:
    FX3(void* usbContext = nullptr);
    virtual ~FX3();

    virtual bool Connect(uint16_t vid, uint16_t pid, const std::string& serial = "") override;
    virtual void Disconnect() override;

#ifndef __unix__
    virtual int BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr) override;
    virtual bool WaitForXfer(int contextHandle, uint32_t timeout_ms) override;
    virtual int FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle) override;
    virtual void AbortEndpointXfers(uint8_t endPointAddr) override;
#endif
};

} // namespace lime
