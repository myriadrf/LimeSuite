#pragma once

#include <mutex>
#include <string>
#include <thread>
#include "USBTransferContext.h"

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

/** @brief A generic class to communicate with a USB device. */
class USBGeneric
{
  public:
    USBGeneric() = delete;

    /**
      @brief Construct a new USBGeneric object.

      If this is the first object of this kind being constructed on a UNIX system
      it also creates the thread to handle USB events.
      @param usbContext The USB context to use.
     */
    USBGeneric(void* usbContext);
    virtual ~USBGeneric();
    static constexpr int32_t defaultTimeout = 1000; ///< The default timeout to use if none is specified.

    /**
      @brief Connects to a given USB device.
      @param vid The vendor ID of the device.
      @param pid The prduct ID of the device.
      @param serial The serial number of the device.
      @return The status of the operation (true on success).
     */
    virtual bool Connect(uint16_t vid, uint16_t pid, const std::string& serial = "");

    /**
      @brief Returns whether this instance is connected to a device.
      @return The state of the connection (true = connected).
     */
    virtual bool IsConnected();

    /** @brief Disconnects from the USB device. */
    virtual void Disconnect();

    /**
      @brief Transfers data using the bulk transfer USB protocol.
      @param endPoint The address to use for the transfer.
      @param data The pointer to the data buffer.
      @param length The length of data being transferred.
      @param timeout_ms The amount of time to wait (in ms) until the transfer is considered failed.
      @return Actual number of bytes transferred.
     */
    virtual int32_t BulkTransfer(uint8_t endPoint, uint8_t* data, int length, int32_t timeout_ms = defaultTimeout);

    /**
      @brief Transfers data using the control transfer USB protocol.
      @param requestType The request type to use in the setup packet.
      @param request The type of request being made.
      @param value The value of the request being made.
      @param index The index of the request being made.
      @param data The pointer to the data buffer.
      @param length The length of data being transferred.
      @param timeout_ms The amount of time to wait (in ms) until the transfer is considered failed.
      @return Actual number of bytes transferred.
     */
    virtual int32_t ControlTransfer(
        int requestType, int request, int value, int index, uint8_t* data, uint32_t length, int32_t timeout_ms = defaultTimeout);

    /**
      @brief Begins an asynchronous data transfer.
      @param buffer The pointer to the data buffer. 
      @param length The length of the data being transferred.
      @param endPointAddr The endpoint address to use for the transfer.
      @return The handle of the transfer context to pass to WaitForXfer and FinishDataXfer functions.
     */
    virtual int BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr);

    /**
      @brief Waits until an asynchronous data transfer finishes.
      @param contextHandle The context handle to wait for (received from BeginDataXfer).
      @param timeout_ms The timeout (in ms) to wait for the transfer.
      @return Indication whether the transfer is finished or not (true = finished).
     */
    virtual bool WaitForXfer(int contextHandle, int32_t timeout_ms = defaultTimeout);

    /**
      @brief Finishes an asynchronous data transfer.
      @param buffer The pointer to the data buffer. 
      @param length The length of the data being transferred.
      @param contextHandle The handle of the transfer (received from BeginDataXfer).
      @return The amount of bytes transferred in the transfer.
     */
    virtual int FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle);

    /**
      @brief Aborts all current asynchronous transfers at the given endpoint.
      @param endPointAddr The endpoint to abort all transfers from.
     */
    virtual void AbortEndpointXfers(uint8_t endPointAddr);

  protected:
    static const int USB_MAX_CONTEXTS{ 16 }; //maximum number of contexts for asynchronous transfers

    USBTransferContext* contexts;
    std::mutex contextsLock;

    bool isConnected;

    virtual int GetUSBContextIndex();
    virtual void WaitForXfers(uint8_t endPointAddr);

#ifdef __unix__
    static int activeUSBconnections;
    static std::thread gUSBProcessingThread;

    libusb_device_handle* dev_handle; //a device handle
    libusb_context* ctx; //a libusb session

    virtual void HandleLibusbEvents();
#endif
};

} // namespace lime
