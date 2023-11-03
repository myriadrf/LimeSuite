#ifndef USBCOMMON_H
#define USBCOMMON_H

#include "limesuite/IComms.h"
#include "LMS64CProtocol.h"
#include "limesuite/DeviceHandle.h"
#include "limesuite/DeviceRegistry.h"

#include <atomic>
#include <condition_variable>
#include <set>

#ifdef __unix__
    #include <libusb.h>
#endif

namespace lime {

class USBTransferContext
{
  public:
    explicit USBTransferContext();
    virtual ~USBTransferContext();
    virtual bool Reset();

    bool used;

#ifdef __unix__
    libusb_transfer* transfer;
    long bytesXfered;
    std::atomic<bool> done;
    std::mutex transferLock;
    std::condition_variable cv;
#endif
};

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

class USBEntry : public DeviceRegistryEntry
{
  public:
    USBEntry(const std::string& name, std::set<VidPid> deviceIds);
    virtual ~USBEntry();

    virtual std::vector<DeviceHandle> enumerate(const DeviceHandle& hint);

  protected:
#ifdef __unix__
    static libusb_context* ctx;
#endif
  private:
    std::set<VidPid> mDeviceIds;
#ifdef __unix__
    std::string GetUSBDeviceSpeedString(libusb_device* device);
    DeviceHandle GetDeviceHandle(libusb_device_handle* tempHandle, libusb_device* device, libusb_device_descriptor desc);
#endif
};

class USB_CSR_Pipe : public ISerialPort
{
  public:
    explicit USB_CSR_Pipe(){};

    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override = 0;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override = 0;
};

class LMS64C_LMS7002M_Over_USB : public IComms
{
  public:
    LMS64C_LMS7002M_Over_USB(USB_CSR_Pipe& dataPort);

    virtual void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int ResetDevice(int chipSelect) override;

  private:
    USB_CSR_Pipe& pipe;
};

class LMS64C_FPGA_Over_USB : public IComms
{
  public:
    LMS64C_FPGA_Over_USB(USB_CSR_Pipe& dataPort);

    void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) override;

    virtual int CustomParameterWrite(
        const int32_t* ids, const double* values, const size_t count, const std::string& units) override;
    virtual int CustomParameterRead(const int32_t* ids, double* values, const size_t count, std::string* units) override;

    virtual int ProgramWrite(
        const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;

  private:
    USB_CSR_Pipe& pipe;
};

} // namespace lime

#endif // USBCOMMON_H
