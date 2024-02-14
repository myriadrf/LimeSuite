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

    virtual bool IsConnected() override;

#ifndef __unix__
    virtual int BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr) override;
    virtual bool WaitForXfer(int contextHandle, uint32_t timeout_ms) override;
    virtual int FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle) override;
    virtual void AbortEndpointXfers(uint8_t endPointAddr) override;

    virtual int32_t BulkTransfer(uint8_t endPoint, uint8_t* data, int length, int32_t timeout_ms = defaultTimeout) override;

    virtual int32_t ControlTransfer(
        int requestType, int request, int value, int index, uint8_t* data, uint32_t length, int32_t timeout_ms = defaultTimeout)
        override;
#endif

    static constexpr int CTR_W_REQCODE = 0xC1;
    static constexpr int CTR_W_VALUE = 0x0000;
    static constexpr int CTR_W_INDEX = 0x0000;

    static constexpr int CTR_R_REQCODE = 0xC0;
    static constexpr int CTR_R_VALUE = 0x0000;
    static constexpr int CTR_R_INDEX = 0x0000;

    static constexpr uint8_t CONTROL_BULK_OUT_ADDRESS = 0x0F;
    static constexpr uint8_t CONTROL_BULK_IN_ADDRESS = 0x8F;

    static constexpr uint8_t STREAM_BULK_OUT_ADDRESS = 0x01;
    static constexpr uint8_t STREAM_BULK_IN_ADDRESS = 0x81;

  protected:
    virtual int GetUSBContextIndex() override;

#ifndef __unix__
    virtual void WaitForXfers(uint8_t endPointAddr) override;

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

    std::mutex FX3mutex;
#endif
};

} // namespace lime
