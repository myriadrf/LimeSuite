/**
    @file LMS64CProtocol.cpp
    @author Lime Microsystems
    @brief Implementation of LMS64C protocol.
*/

#include "Logger.h"
#include "LMS64CProtocol.h"
#include <chrono>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <ciso646> // alternative operators for visual c++: not, and, or...
#include "ADCUnits.h"
#include <cstring>

//! CMD_LMS7002_RST options
const int LMS_RST_DEACTIVATE = 0;
const int LMS_RST_ACTIVATE = 1;
const int LMS_RST_PULSE = 2;

namespace lime {

LMS64CPacket::LMS64CPacket()
{
    std::memset(this, 0, sizeof(LMS64CPacket));
}

namespace LMS64CProtocol {

static const std::array<std::string, eCMD_STATUS::STATUS_COUNT> COMMAND_STATUS_TEXT = {
    "Undefined/Failure", "Completed", "Unknown command", "Busy", "Too many blocks", "Error", "Wrong order", "Resource denied"
};

static const std::string UNKNOWN{ "Unknown status" };

static inline const std::string& status2string(const int status)
{
    if (status >= 0 && status < eCMD_STATUS::STATUS_COUNT)
    {
        return COMMAND_STATUS_TEXT.at(status);
    }

    return UNKNOWN;
}

static const char ADC_UNITS_PREFIX[] = { ' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm' };

static int SPI16(ISerialPort& port,
    uint8_t chipSelect,
    eCMD_LMS writeCmd,
    const uint32_t* MOSI,
    eCMD_LMS readCmd,
    uint32_t* MISO,
    size_t count,
    uint32_t subDevice)
{
    LMS64CPacket pkt;

    size_t srcIndex = 0;
    size_t destIndex = 0;
    constexpr int maxBlocks = LMS64CPacket::payloadSize / (sizeof(uint32_t) / sizeof(uint8_t)); // = 14
    while (srcIndex < count)
    {
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = 0;
        pkt.periphID = chipSelect;
        pkt.subDevice = subDevice;

        // fill packet with same direction operations
        const bool willDoWrite = MOSI[srcIndex] & (1 << 31);
        for (int i = 0; i < maxBlocks && srcIndex < count; ++i)
        {
            const bool isWrite = MOSI[srcIndex] & (1 << 31);
            if (isWrite != willDoWrite)
                break; // change between write/read, flush packet

            if (isWrite)
            {
                pkt.cmd = writeCmd;
                int payloadOffset = pkt.blockCount * 4;
                pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 24;
                pkt.payload[payloadOffset + 1] = MOSI[srcIndex] >> 16;
                pkt.payload[payloadOffset + 2] = MOSI[srcIndex] >> 8;
                pkt.payload[payloadOffset + 3] = MOSI[srcIndex];
            }
            else
            {
                pkt.cmd = readCmd;
                int payloadOffset = pkt.blockCount * 2;
                pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 8;
                pkt.payload[payloadOffset + 1] = MOSI[srcIndex];
            }
            ++pkt.blockCount;
            ++srcIndex;
        }

        int sent = 0;
        int recv = 0;

        sent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            return -1;

        recv = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 1000);
        if (recv != sizeof(pkt) || pkt.status != STATUS_COMPLETED_CMD)
            return -1;

        for (int i = 0; MISO && i < pkt.blockCount && destIndex < count; ++i)
        {
            //MISO[destIndex] = 0;
            //MISO[destIndex] = pkt.payload[0] << 24;
            //MISO[destIndex] |= pkt.payload[1] << 16;
            MISO[destIndex] = (pkt.payload[i * 4 + 2] << 8) | pkt.payload[i * 4 + 3];
            ++destIndex;
        }
    }

    return 0;
}

int GetFirmwareInfo(ISerialPort& port, FirmwareInfo& info, uint32_t subDevice)
{
    info.deviceId = LMS_DEV_UNKNOWN;
    info.expansionBoardId = EXP_BOARD_UNKNOWN;
    info.firmware = 0;
    info.hardware = 0;
    info.protocol = 0;
    info.boardSerialNumber = 0;

    LMS64CPacket pkt;
    pkt.cmd = CMD_GET_INFO;
    pkt.subDevice = subDevice;
    int sent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (sent != sizeof(pkt))
    {
        return -1;
    }

    int recv = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 1000);
    if (recv != sizeof(pkt) || pkt.status != STATUS_COMPLETED_CMD)
    {
        return -1;
    }

    info.firmware = pkt.payload[0];
    info.deviceId = pkt.payload[1];
    info.protocol = pkt.payload[2];
    info.hardware = pkt.payload[3];
    info.expansionBoardId = pkt.payload[4];
    info.boardSerialNumber = 0;

    for (int i = 10; i < 18; i++)
    {
        info.boardSerialNumber <<= 8;
        info.boardSerialNumber |= pkt.payload[i];
    }

    return 0;
}

void FirmwareToDescriptor(const FirmwareInfo& fw, SDRDevice::Descriptor& descriptor)
{
    if (fw.deviceId >= eLMS_DEV::LMS_DEV_COUNT)
    {
        char strTemp[64];
        sprintf(strTemp, "Unknown (0x%X)", fw.deviceId);
        descriptor.name = std::string(strTemp);
    }
    else
        descriptor.name = GetDeviceName(static_cast<eLMS_DEV>(fw.deviceId));
    if (fw.expansionBoardId >= eEXP_BOARD::EXP_BOARD_COUNT)
    {
        char strTemp[64];
        sprintf(strTemp, "Unknown (0x%X)", fw.expansionBoardId);
        descriptor.expansionName = std::string(strTemp);
    }
    else
        descriptor.expansionName = GetExpansionBoardName(static_cast<eEXP_BOARD>(fw.expansionBoardId));
    descriptor.firmwareVersion = std::to_string(int(fw.firmware));
    descriptor.hardwareVersion = std::to_string(int(fw.hardware));
    descriptor.protocolVersion = std::to_string(int(fw.protocol));
    descriptor.serialNumber = fw.boardSerialNumber;
}

int LMS7002M_SPI(ISerialPort& port, uint8_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, size_t count, uint32_t subDevice)
{
    return SPI16(port, chipSelect, CMD_LMS7002_WR, MOSI, CMD_LMS7002_RD, MISO, count, subDevice);
}

int FPGA_SPI(ISerialPort& port, const uint32_t* MOSI, uint32_t* MISO, size_t count, uint32_t subDevice)
{
    return SPI16(port, 0, CMD_BRDSPI_WR, MOSI, CMD_BRDSPI_RD, MISO, count, subDevice);
}

int ADF4002_SPI(ISerialPort& port, const uint32_t* MOSI, size_t count, uint32_t subDevice)
{
    // only writes are supported
    LMS64CPacket pkt;

    size_t srcIndex = 0;
    constexpr int maxBlocks = LMS64CPacket::payloadSize / (sizeof(uint32_t) / sizeof(uint8_t)); // = 14
    const int blockSize = 3;

    while (srcIndex < count)
    {
        pkt.cmd = CMD_ADF4002_WR;
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = 0;
        pkt.periphID = 0;
        pkt.subDevice = subDevice;

        for (int i = 0; i < maxBlocks && srcIndex < count; ++i)
        {
            int payloadOffset = pkt.blockCount * blockSize;
            pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 16;
            pkt.payload[payloadOffset + 1] = MOSI[srcIndex] >> 8;
            pkt.payload[payloadOffset + 2] = MOSI[srcIndex];
            ++pkt.blockCount;
            ++srcIndex;
        }

        int sent = 0;
        int recv = 0;

        sent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            return -1;

        recv = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 1000);
        if (recv != sizeof(pkt) || pkt.status != STATUS_COMPLETED_CMD)
            return -1;
    }
    return 0;
}

int I2C_Write(ISerialPort& port, uint32_t address, const uint8_t* mosi, size_t count)
{
    return -1;
}
int I2C_Read(ISerialPort& port, uint32_t address, uint8_t* data, size_t count)
{
    return -1;
}

int CustomParameterWrite(ISerialPort& port, const std::vector<CustomParameterIO>& parameters, uint32_t subDevice)
{
    LMS64CPacket pkt;
    std::size_t index = 0;

    while (index < parameters.size())
    {
        pkt.cmd = CMD_ANALOG_VAL_WR;
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = 0;
        pkt.periphID = 0;
        pkt.subDevice = subDevice;
        int byteIndex = 0;
        constexpr int maxBlocks = LMS64CPacket::payloadSize / (sizeof(uint32_t) / sizeof(uint8_t)); // = 14

        while (pkt.blockCount < maxBlocks && index < parameters.size())
        {
            pkt.payload[byteIndex++] = parameters[index].id;
            int powerOf10 = 0;

            if (parameters[index].value > 65535.0 && (parameters[index].units != ""))
                powerOf10 = log10(parameters[index].value / 65.536) / 3;

            if (parameters[index].value < 65.536 && (parameters[index].units != ""))
                powerOf10 = log10(parameters[index].value / 65535.0) / 3;

            int unitsId = 0; // need to convert given units to their enum
            pkt.payload[byteIndex++] = unitsId << 4 | powerOf10;

            int value = parameters[index].value / pow(10, 3 * powerOf10);
            pkt.payload[byteIndex++] = (value >> 8);
            pkt.payload[byteIndex++] = (value & 0xFF);

            ++pkt.blockCount;
            ++index;
        }

        int sent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("CustomParameterWrite write failed");

        int recv = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
            throw std::runtime_error("CustomParameterWrite read failed");
    }

    return 0;
}

int CustomParameterRead(ISerialPort& port, std::vector<CustomParameterIO>& parameters, uint32_t subDevice)
{
    LMS64CPacket pkt;
    std::size_t index = 0;

    while (index < parameters.size())
    {
        pkt.cmd = CMD_ANALOG_VAL_RD;
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = 0;
        pkt.periphID = 0;
        pkt.subDevice = subDevice;
        int byteIndex = 0;
        constexpr int maxBlocks = LMS64CPacket::payloadSize / (sizeof(uint32_t) / sizeof(uint8_t)); // = 14

        while (pkt.blockCount < maxBlocks && index < parameters.size())
        {
            pkt.payload[byteIndex++] = parameters[index].id;
            ++pkt.blockCount;
            ++index;
        }

        int sent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("CustomParameterRead write failed");

        int recv = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
            throw std::runtime_error("CustomParameterRead read failed");

        for (std::size_t i = 0; i < pkt.blockCount; ++i)
        {
            int unitsIndex = pkt.payload[i * 4 + 1];
            std::size_t parameterIndex = index - pkt.blockCount + i;

            if (unitsIndex & 0x0F)
                parameters[parameterIndex].units = ADC_UNITS_PREFIX[unitsIndex & 0x0F];

            parameters[parameterIndex].units += adcUnits2string((unitsIndex & 0xF0) >> 4);

            if ((unitsIndex & 0xF0) >> 4 == RAW)
                parameters[parameterIndex].value = static_cast<uint16_t>(pkt.payload[i * 4 + 2] << 8 | pkt.payload[i * 4 + 3]);
            else
            {
                parameters[parameterIndex].value = static_cast<int16_t>(pkt.payload[i * 4 + 2] << 8 | pkt.payload[i * 4 + 3]);

                if ((unitsIndex & 0xF0) >> 4 == TEMPERATURE)
                    parameters[parameterIndex].value /= 10;
            }
        }
    }

    return 0;
}

int ProgramWrite(ISerialPort& port,
    const char* data,
    size_t length,
    int prog_mode,
    ProgramWriteTarget device,
    ProgressCallback callback,
    uint32_t subDevice)
{
#ifndef NDEBUG
    auto t1 = std::chrono::high_resolution_clock::now();
#endif
    //erasing FLASH can take up to 3 seconds before reply is received
    const int progTimeout_ms = 5000;
    char progressMsg[512];
    sprintf(progressMsg, "in progress...");
    bool abortProgramming = false;
    size_t bytesSent = 0;

    bool needsData = true;
    if (device == ProgramWriteTarget::FPGA && prog_mode == 2)
        needsData = false;
    if (device == ProgramWriteTarget::FX3 && (prog_mode == 0 || prog_mode == 1))
        needsData = false;

    eCMD_LMS cmd;
    if (device == ProgramWriteTarget::HPM || device == ProgramWriteTarget::FX3)
        cmd = CMD_MEMORY_WR;
    else if (device == FPGA)
        cmd = CMD_ALTERA_FPGA_GW_WR;
    else
    {
        sprintf(progressMsg, "Programming failed! Target device not supported");
        if (callback)
            callback(bytesSent, length, progressMsg);
        return ReportError(ENOTSUP, progressMsg);
    }

    LMS64CPacket packet;
    LMS64CPacket inPacket;
    packet.cmd = cmd;
    packet.blockCount = packet.payloadSize;
    packet.subDevice = subDevice;

    const size_t chunkSize = 32;
    static_assert(chunkSize < LMS64CPacket::payloadSize, "chunk must fit into packet payload");
    const uint32_t chunkCount = length / chunkSize + (length % chunkSize > 0) + 1; // +1 programming end packet

    for (uint32_t chunkIndex = 0; chunkIndex < chunkCount && !abortProgramming; ++chunkIndex)
    {
        memset(packet.payload, 0, packet.payloadSize);
        packet.payload[0] = prog_mode;
        packet.payload[1] = (chunkIndex >> 24) & 0xFF;
        packet.payload[2] = (chunkIndex >> 16) & 0xFF;
        packet.payload[3] = (chunkIndex >> 8) & 0xFF;
        packet.payload[4] = chunkIndex & 0xFF;
        packet.payload[5] = std::min(length - bytesSent, chunkSize);

        if (cmd == CMD_MEMORY_WR)
        {
            packet.payload[6] = 0;
            packet.payload[7] = 0;
            packet.payload[8] = 0;
            packet.payload[9] = 0;

            packet.payload[10] = (device >> 8) & 0xFF;
            packet.payload[11] = device & 0xFF;
        }

        if (needsData)
        {
            memcpy(&packet.payload[24], data, chunkSize);
            data += chunkSize;
        }

        if (port.Write(reinterpret_cast<uint8_t*>(&packet), sizeof(packet), progTimeout_ms) != sizeof(packet))
        {
            if (callback)
                callback(bytesSent, length, "Programming failed! Write operation failed");
            return false;
        }
        if (port.Read(reinterpret_cast<uint8_t*>(&inPacket), sizeof(inPacket), progTimeout_ms) != sizeof(inPacket))
        {
            if (callback)
                callback(bytesSent, length, "Programming failed! Read operation failed");
            return false;
        }

        if (inPacket.status != STATUS_COMPLETED_CMD)
        {
            sprintf(progressMsg, "Programming failed! %s", status2string(inPacket.status).c_str());
            if (callback)
                callback(bytesSent, length, progressMsg);
            return ReportError(EPROTO, progressMsg);
        }
        bytesSent += packet.payload[5];
        if (needsData == false) //only one packet is needed to initiate bitstream from flash
        {
            bytesSent = length;
            break;
        }
        if (callback)
        {
            bool completed = chunkIndex == chunkCount - 1;
            if (completed)
                sprintf(progressMsg, "Programming: completed");
            abortProgramming = callback(bytesSent, length, progressMsg);
            if (abortProgramming && !completed)
                return -1;
        }
    }
#ifndef NDEBUG
    auto t2 = std::chrono::high_resolution_clock::now();
    if ((device == 2 && prog_mode == 2) == false)
        lime::log(LogLevel::INFO,
            "Programming finished, %li bytes sent! %li ms\n",
            length,
            std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
    else
        lime::log(LogLevel::INFO, "FPGA configuring initiated\n");
#endif
    return 0;
}

int DeviceReset(ISerialPort& port, uint32_t socIndex, uint32_t subDevice)
{
    LMS64CPacket pkt;
    pkt.cmd = CMD_LMS7002_RST;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = 1;
    pkt.periphID = socIndex;
    pkt.subDevice = subDevice;

    pkt.payload[0] = LMS_RST_PULSE;

    int sent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (sent != sizeof(pkt))
        throw std::runtime_error("DeviceReset write failed");
    int recv = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
        throw std::runtime_error("DeviceReset read failed");
    return 0;
}

int GPIODirRead(ISerialPort& port, uint8_t* buffer, const size_t bufLength)
{
    if (bufLength > LMS64CPacket::payloadSize)
    {
        throw std::invalid_argument("Buffer is too big for one packet.");
    }

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_DIR_RD;
    pkt.blockCount = bufLength;

    int bytesSent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesSent != sizeof(pkt))
    {
        throw std::runtime_error("GPIODirRead write failed");
    }

    int bytesReceived = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesReceived < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
    {
        throw std::runtime_error("GPIODirRead read failed");
    }

    for (size_t i = 0; i < bufLength; ++i)
    {
        buffer[i] = pkt.payload[i];
    }

    return 0;
}

int GPIORead(ISerialPort& port, uint8_t* buffer, const size_t bufLength)
{
    if (bufLength > LMS64CPacket::payloadSize)
    {
        throw std::invalid_argument("Buffer is too big for one packet.");
    }

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;
    pkt.blockCount = bufLength;

    int bytesSent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesSent != sizeof(pkt))
    {
        throw std::runtime_error("GPIORead write failed");
    }

    int bytesReceived = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesReceived < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
    {
        throw std::runtime_error("GPIORead read failed");
    }

    for (size_t i = 0; i < bufLength; ++i)
    {
        buffer[i] = pkt.payload[i];
    }

    return 0;
}

int GPIODirWrite(ISerialPort& port, const uint8_t* buffer, const size_t bufLength)
{
    if (bufLength > LMS64CPacket::payloadSize)
    {
        throw std::invalid_argument("Buffer is too big for one packet.");
    }

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_DIR_WR;
    pkt.blockCount = bufLength;

    for (size_t i = 0; i < bufLength; ++i)
    {
        pkt.payload[i] = buffer[i];
    }

    int bytesSent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesSent != sizeof(pkt))
    {
        throw std::runtime_error("GPIODirWrite write failed");
    }

    int bytesReceived = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesReceived < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
    {
        throw std::runtime_error("GPIODirWrite read failed");
    }

    return 0;
}

int GPIOWrite(ISerialPort& port, const uint8_t* buffer, const size_t bufLength)
{
    if (bufLength > LMS64CPacket::payloadSize)
    {
        throw std::invalid_argument("Buffer is too big for one packet.");
    }

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_WR;
    pkt.blockCount = bufLength;

    for (size_t i = 0; i < bufLength; ++i)
    {
        pkt.payload[i] = buffer[i];
    }

    int bytesSent = port.Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesSent != sizeof(pkt))
    {
        throw std::runtime_error("GPIOWrite write failed");
    }

    int bytesReceived = port.Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (bytesReceived < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
    {
        throw std::runtime_error("GPIOWrite read failed");
    }

    return 0;
}

} // namespace LMS64CProtocol

} // namespace lime
