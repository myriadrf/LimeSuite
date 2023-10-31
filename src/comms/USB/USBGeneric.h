#pragma once

#include <chrono>
#include <string>

namespace lime {

class USBGeneric
{
  public:
    virtual ~USBGeneric(){};
    static const int32_t defaultTimeout = 1000;

    virtual bool Connect(uint16_t vid, uint16_t pid, const std::string& serial = "") = 0;
    virtual bool IsConnected() = 0;
    virtual void Disconnect() = 0;

    // return actual number of bytes transferred
    virtual int32_t BulkTransfer(uint8_t endPoint, uint8_t* data, int length, int32_t timeout_ms = defaultTimeout) = 0;

    // return actual number of bytes transferred
    virtual int32_t ControlTransfer(int requestType,
        int request,
        int value,
        int index,
        uint8_t* data,
        uint32_t length,
        int32_t timeout_ms = defaultTimeout) = 0;

    virtual int BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr) = 0;
    virtual bool WaitForXfer(int contextHandle, uint32_t timeout_ms) = 0;
    virtual int FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle) = 0;
    virtual void AbortEndpointXfers(uint8_t endPointAddr) = 0;
};

} // namespace lime
