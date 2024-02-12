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
#endif // !__unix__

namespace lime {

/** @brief A class for communicating with devices using the Cypress USB 3.0 CYUSB3014-BZXC USB controller. */
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

    virtual int GetUSBContextIndex() override;

#ifndef __unix__
    static const int MAX_EP_CNT = 16;
    CCyFX3Device* USBDevicePrimary;
    //control endpoints
    CCyControlEndPoint* InCtrlEndPt3;
    CCyControlEndPoint* OutCtrlEndPt3;

    //end points for samples reading and writing
    CCyUSBEndPoint* InEndPt[MAX_EP_CNT];
    CCyUSBEndPoint* OutEndPt[MAX_EP_CNT];

    CCyUSBEndPoint* InCtrlBulkEndPt;
    CCyUSBEndPoint* OutCtrlBulkEndPt;
#endif
};

} // namespace lime
