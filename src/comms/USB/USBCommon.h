#ifndef USBCOMMON_H
#define USBCOMMON_H

#include "limesuite/IComms.h"
#include "LMS64CProtocol.h"
#include "FX3.h"

using namespace lime;

class USB_CSR_Pipe : public ISerialPort
{
public:
    explicit USB_CSR_Pipe(FX3& port) : port(port) {};

    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override = 0;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override = 0;
protected:
    FX3& port;
};

class LMS64C_LMS7002M_Over_USB : public IComms
{
public:
    LMS64C_LMS7002M_Over_USB(USB_CSR_Pipe& dataPort);

    virtual void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;
    virtual void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;

    virtual int ResetDevice(int chipSelect) override;
private:
    USB_CSR_Pipe &pipe;
};

class LMS64C_FPGA_Over_USB : public IComms
{
public:
    LMS64C_FPGA_Over_USB(USB_CSR_Pipe &dataPort);

    void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;
    void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;

    virtual int GPIODirRead(uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIOWrite(const uint8_t *buffer, const size_t bufLength) override;
    
    virtual int CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units) override;
    virtual int CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units) override;

    virtual int ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;
private:
    USB_CSR_Pipe &pipe;
};

#endif // USBCOMMON_H
