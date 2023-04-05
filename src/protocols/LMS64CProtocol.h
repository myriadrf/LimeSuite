/**
    @file LMS64CProtocol.h
    @author Lime Microsystems
    @brief Implementation of LMS64C protocol.
*/

#pragma once
#include <mutex>
#include <LMS64CCommands.h>
#include <LMSBoards.h>
#include <thread>

namespace lime{

struct LMS64CPacket
{
    static constexpr int size = 64;
    static constexpr int payloadSize = 56;
    static constexpr int headerSize = size - payloadSize;
    LMS64CPacket();
    uint8_t cmd;
    uint8_t status;
    uint8_t blockCount;
    uint8_t periphID;
    uint8_t reserved[4];
    uint8_t payload[payloadSize];
};

class ISerialPort
{
public:
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) = 0;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) = 0;
};

namespace LMS64CProtocol
{

enum ProgramWriteTarget
{
    HPM,
    FX3, //
    FPGA, // prog_modes: 0-bitstream to FPGA, 1-to FLASH, 2-bitstream from FLASH

    PROGRAM_WRITE_TARGET_COUNT
};

int LMS7002M_SPI(ISerialPort& port, const uint32_t* mosi, uint32_t *miso, size_t count);
int FPGA_SPI(ISerialPort& port, const uint32_t* mosi, uint32_t *miso, size_t count);

int I2C_Write(ISerialPort& port, uint32_t address, const uint8_t* data, size_t count);
int I2C_Read(ISerialPort& port, uint32_t address, uint8_t* data, size_t count);

int CustomParameterWrite(ISerialPort& port, const int32_t *ids, const double *values, const size_t count, const std::string& units);
int CustomParameterRead(ISerialPort& port, const int32_t *ids, double *values, const size_t count, std::string* units);

typedef bool(*ProgressCallback)(size_t bytesSent, size_t bytesTotal, const char* progressMsg); // return true to stop progress
int ProgramWrite(ISerialPort& port, const char* data, size_t length, int prog_mode, ProgramWriteTarget device, ProgressCallback callback = nullptr);

int DeviceReset(ISerialPort& port, uint32_t socIndex);

}

}
