/**
    @file LMS64CProtocol.cpp
    @author Lime Microsystems
    @brief Implementation of LMS64C protocol.
*/

#include "Logger.h"
#include "LMS64CProtocol.h"
#include <chrono>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <algorithm>
#include <iso646.h> // alternative operators for visual c++: not, and, or...
#include <ADCUnits.h>
#include <sstream>
#include <string.h>

//! CMD_LMS7002_RST options
const int LMS_RST_DEACTIVATE = 0;
const int LMS_RST_ACTIVATE = 1;
const int LMS_RST_PULSE = 2;

//! arbitrary spi constants used to dispatch calls
#define LMS7002M_SPI_INDEX 0x10
#define ADF4002_SPI_INDEX 0x30
namespace lime {

LMS64CPacket::LMS64CPacket()
{
     memset(this, 0, sizeof(LMS64CPacket));
}

static int convertStatus(const int &status, const LMS64CPacket &pkt)
{
    if (status != 0) return -1;
    switch (pkt.status)
    {
    case STATUS_COMPLETED_CMD: return 0;
    case STATUS_UNKNOWN_CMD:
        return ReportError(EPROTONOSUPPORT, "Command not supported");
    default: break;
    }
    return ReportError(EPROTO, status2string(pkt.status));
}

namespace LMS64CProtocol {

static int SPI16(ISerialPort& port, eCMD_LMS writeCmd, const uint32_t *MOSI, eCMD_LMS readCmd, uint32_t *MISO, size_t count)
{
    LMS64CPacket pkt;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = 0;
    pkt.periphID = 0;

    size_t srcIndex = 0;
    size_t destIndex = 0;
    const int maxBlocks = 14;
    while (srcIndex < count) {
        // fill packet with same direction operations
        const bool willDoWrite = MOSI[srcIndex] & (1 << 31);
        for (int i = 0; i < maxBlocks && srcIndex < count; ++i) {
            const bool isWrite = MOSI[srcIndex] & (1 << 31);
            if (isWrite != willDoWrite)
                break; // change between write/read, flush packet

            if (isWrite) {
                pkt.cmd = writeCmd;
                int payloadOffset = pkt.blockCount * 4;
                pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 24;
                pkt.payload[payloadOffset + 1] = MOSI[srcIndex] >> 16;
                pkt.payload[payloadOffset + 2] = MOSI[srcIndex] >> 8;
                pkt.payload[payloadOffset + 3] = MOSI[srcIndex];
            }
            else {
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

        sent = port.Write((uint8_t*)&pkt, sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            return -1;

        recv = port.Read((uint8_t*)&pkt, sizeof(pkt), 1000);
        if (recv != sizeof(pkt) || pkt.status != STATUS_COMPLETED_CMD)
            return -1;

        for (int i = 0; MISO && i < pkt.blockCount && destIndex < count; ++i) {
            //MISO[destIndex] = 0;
            //MISO[destIndex] = pkt.payload[0] << 24;
            //MISO[destIndex] |= pkt.payload[1] << 16;
            MISO[destIndex] = (pkt.payload[i * 4 + 2] << 8) | pkt.payload[i * 4 + 3];
            ++destIndex;
        }

        pkt.blockCount = 0;
        pkt.status = STATUS_UNDEFINED;
    }
    return 0;
}

int LMS7002M_SPI(ISerialPort& port, const uint32_t* MOSI, uint32_t* MISO, size_t count)
{
    return SPI16(port, CMD_LMS7002_WR, MOSI, CMD_LMS7002_RD, MISO, count);
}

int FPGA_SPI(ISerialPort& port, const uint32_t* MOSI, uint32_t* MISO, size_t count)
{
    return SPI16(port, CMD_BRDSPI_WR, MOSI, CMD_BRDSPI_RD, MISO, count);
}

int I2C_Write(ISerialPort& port, uint32_t address, const uint8_t* mosi, size_t count)
{
    return -1;
}
int I2C_Read(ISerialPort& port, uint32_t address, uint8_t* data, size_t count)
{
    return -1;
}

int CustomParameterWrite(ISerialPort& port, const int32_t *ids, const double *values, const size_t count, const std::string& units)
{
    LMS64CPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_WR;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = count;
    pkt.periphID = 0;
    int byteIndex = 0;
    for (size_t i = 0; i < count; ++i)
    {
        pkt.payload[byteIndex++] = ids[i];
        int powerOf10 = 0;
        if(values[i] > 65535.0 && (units != ""))
            powerOf10 = log10(values[i]/65.536)/3;
        if (values[i] < 65.536 && (units != ""))
            powerOf10 = log10(values[i]/65535.0) / 3;
        int unitsId = 0; // need to convert given units to their enum
        pkt.payload[byteIndex++] = unitsId << 4 | powerOf10;
        int value = values[i] / pow(10, 3*powerOf10);
        pkt.payload[byteIndex++] = (value >> 8);
        pkt.payload[byteIndex++] = (value & 0xFF);
    }
    int sent = port.Write((uint8_t*)&pkt, sizeof(pkt), 100);
    if (sent != sizeof(pkt))
        throw std::runtime_error("CustomParameterWrite write failed");
    int recv = port.Read((uint8_t*)&pkt, sizeof(pkt), 100);
    if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
        throw std::runtime_error("CustomParameterWrite read failed");
    return 0;
}

int CustomParameterRead(ISerialPort& port, const int32_t *ids, double *values, const size_t count, std::string* units)
{
    LMS64CPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_RD;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = count;
    pkt.periphID = 0;
    int byteIndex = 0;
    for (size_t i = 0; i < count; ++i)
        pkt.payload[byteIndex++] = ids[i];

    int sent = port.Write((uint8_t*)&pkt, sizeof(pkt), 100);
    if (sent != sizeof(pkt))
        throw std::runtime_error("CustomParameterRead write failed");
    int recv = port.Read((uint8_t*)&pkt, sizeof(pkt), 100);
    if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
        throw std::runtime_error("CustomParameterRead read failed");

    assert(pkt.blockCount == count);

    for (size_t i = 0; i < count; ++i)
    {
        int unitsIndex = pkt.payload[i * 4 + 1];
        if(units)
        {
            if (unitsIndex & 0x0F)
            {
                const char adc_units_prefix[] = {
                    ' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z',
                    'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm'};
                units[i] = adc_units_prefix[unitsIndex&0x0F]+adcUnits2string((unitsIndex & 0xF0)>>4);
            }
            else
                units[i] += adcUnits2string((unitsIndex & 0xF0)>>4);
        }
        if((unitsIndex & 0xF0)>>4 == RAW)
        {
            values[i] = (uint16_t)(pkt.payload[i * 4 + 2] << 8 | pkt.payload[i * 4 + 3]);
        }
        else
        {
            values[i] = (int16_t)(pkt.payload[i * 4 + 2] << 8 | pkt.payload[i * 4 + 3]);
            if((unitsIndex & 0xF0)>>4 == TEMPERATURE)
                values[i] /= 10;
        }
    }
    return 0;
}

int ProgramWrite(ISerialPort& port, const char* data, size_t length, int prog_mode, ProgramWriteTarget device, ProgressCallback callback)
{
#ifndef NDEBUG
    auto t1 = std::chrono::high_resolution_clock::now();
#endif
    //erasing FLASH can take up to 3 seconds before reply is received
    const int progTimeout_ms = 5000;
    char progressMsg[128];
    sprintf(progressMsg, "in progress...");
    bool abortProgramming = false;
    size_t bytesSent = 0;

    bool needsData = true;
    if(device == ProgramWriteTarget::FPGA && prog_mode == 2)
        needsData = false;
    if(device == ProgramWriteTarget::FX3 && (prog_mode == 0 || prog_mode == 1))
        needsData = false;

    eCMD_LMS cmd;
    if(device == ProgramWriteTarget::HPM || device == ProgramWriteTarget::FX3)
        cmd = CMD_MEMORY_WR;
    else if(device == FPGA)
        cmd = CMD_ALTERA_FPGA_GW_WR;
    else
    {
        sprintf(progressMsg, "Programming failed! Target device not supported");
        if(callback)
            callback(bytesSent, length, progressMsg);
        return ReportError(ENOTSUP, progressMsg);
    }

    LMS64CPacket packet;
    LMS64CPacket inPacket;
    packet.cmd = cmd;
    packet.blockCount = packet.payloadSize;

    const size_t chunkSize = 32;
    const uint32_t chunkCount = length/LMS64CPacket::payloadSize + (length%LMS64CPacket::payloadSize > 0) + 1; // +1 programming end packet

    for (uint32_t chunkIndex = 0; chunkIndex<chunkCount && !abortProgramming; ++chunkIndex)
    {
        memset(packet.payload, 0, packet.payloadSize);
        packet.payload[0] = prog_mode;
        packet.payload[1] = (chunkIndex >> 24) & 0xFF;
        packet.payload[2] = (chunkIndex >> 16) & 0xFF;
        packet.payload[3] = (chunkIndex >> 8) & 0xFF;
        packet.payload[4] = chunkIndex & 0xFF;
        packet.payload[5] = std::min(length - bytesSent, chunkSize);

        if(cmd == CMD_MEMORY_WR)
        {
            packet.payload[6] = 0;
            packet.payload[7] = 0;
            packet.payload[8] = 0;
            packet.payload[9] = 0;

            packet.payload[10] = (device >> 8) & 0xFF;
            packet.payload[11] = device & 0xFF;
        }

        if(needsData)
        {
            memcpy(&packet.payload[24], data, chunkSize);
            data += chunkSize;
        }

        if (port.Write((uint8_t*)&packet, sizeof(packet), progTimeout_ms) != sizeof(packet))
        {
            if(callback)
                callback(bytesSent, length, "Programming failed! Write operation failed");
            return false;
        }
        if (port.Read((uint8_t*)&inPacket, sizeof(inPacket), progTimeout_ms) != sizeof(inPacket))
        {
            if(callback)
                callback(bytesSent, length, "Programming failed! Read operation failed");
            return false;
        }

        if(inPacket.status != STATUS_COMPLETED_CMD)
        {
            sprintf(progressMsg, "Programming failed! %s", status2string(inPacket.status));
            if(callback)
                callback(bytesSent, length, progressMsg);
            return ReportError(EPROTO, progressMsg);
        }
        bytesSent += packet.payload[5];
        if(needsData == false) //only one packet is needed to initiate bitstream from flash
        {
            bytesSent = length;
            break;
        }
        if(callback)
        {
            abortProgramming = callback(bytesSent, length, progressMsg);
            if (abortProgramming)
            {
                callback(bytesSent, length, "Programming: aborted by user");
                return -1;
            }
        }
    }
    if(callback)
        callback(bytesSent, length, "Programming: completed");
#ifndef NDEBUG
    auto t2 = std::chrono::high_resolution_clock::now();
    if ((device == 2 && prog_mode == 2) == false)
        lime::log(LOG_LEVEL_INFO, "Programming finished, %li bytes sent! %li ms\n", length, std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
    else
        lime::log(LOG_LEVEL_INFO, "FPGA configuring initiated\n");
#endif
    return 0;
}

int DeviceReset(ISerialPort& port, uint32_t socIndex)
{
    LMS64CPacket pkt;
    pkt.cmd = CMD_LMS7002_RST;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = 1;
    pkt.periphID = socIndex;
    
    pkt.payload[0] = LMS_RST_PULSE;
        
    int sent = port.Write((uint8_t*)&pkt, sizeof(pkt), 100);
    if (sent != sizeof(pkt))
        throw std::runtime_error("DeviceReset write failed");
    int recv = port.Read((uint8_t*)&pkt, sizeof(pkt), 100);
    if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
        throw std::runtime_error("DeviceReset read failed");
    return 0;
}

}
/*
int LMS64CProtocol::DeviceReset(int ind)
{
    if (not this->IsOpen())
    {
        return ReportError(ENOTCONN, "connection is not open");
    }

    GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RST;
    pkt.periphID = ind;
    pkt.outBuffer.push_back (LMS_RST_PULSE);
    int status = this->TransferPacket(pkt);

    return convertStatus(status, pkt);
}

DeviceInfo LMS64CProtocol::GetDeviceInfo(void)
{
    LMSinfo lmsInfo = this->GetInfo();
    DeviceInfo devInfo;
    devInfo.deviceName = GetDeviceName(lmsInfo.device);
    devInfo.expansionName = GetExpansionBoardName(lmsInfo.expansion);
    devInfo.firmwareVersion = std::to_string(int(lmsInfo.firmware));
    devInfo.hardwareVersion = std::to_string(int(lmsInfo.hardware));
    devInfo.protocolVersion = std::to_string(int(lmsInfo.protocol));
    devInfo.boardSerialNumber = lmsInfo.boardSerialNumber;

    FPGAinfo gatewareInfo = this->GetFPGAInfo();
    devInfo.gatewareTargetBoard = GetDeviceName(eLMS_DEV(gatewareInfo.boardID));
    devInfo.gatewareVersion = std::to_string(int(gatewareInfo.gatewareVersion));
    devInfo.gatewareRevision = std::to_string(int(gatewareInfo.gatewareRevision));
    devInfo.hardwareVersion = std::to_string(int(gatewareInfo.hwVersion));

    return devInfo;
}

LMS64CProtocol::LMSinfo LMS64CProtocol::GetInfo()
{
    LMSinfo info;
    info.device = LMS_DEV_UNKNOWN;
    info.expansion = EXP_BOARD_UNKNOWN;
    info.firmware = 0;
    info.hardware = 0;
    info.protocol = 0;
    info.boardSerialNumber = 0;
    GenericPacket pkt;
    pkt.cmd = CMD_GET_INFO;
    int status = TransferPacket(pkt);
    if (status == 0 && pkt.inBuffer.size() >= 5)
    {
        info.firmware = pkt.inBuffer[0];
        info.device = pkt.inBuffer[1] < LMS_DEV_COUNT ? (eLMS_DEV)pkt.inBuffer[1] : LMS_DEV_UNKNOWN;
        info.protocol = pkt.inBuffer[2];
        info.hardware = pkt.inBuffer[3];
        info.expansion = pkt.inBuffer[4] < EXP_BOARD_COUNT ? (eEXP_BOARD)pkt.inBuffer[4] : EXP_BOARD_UNKNOWN;
        info.boardSerialNumber = 0;
        for (int i = 10; i < 18; i++)
        {
	        info.boardSerialNumber <<= 8;
	        info.boardSerialNumber |= pkt.inBuffer[i];
        }
    }
    return info;
}

LMS64CProtocol::FPGAinfo LMS64CProtocol::GetFPGAInfo()
{
    FPGAinfo info;
    info.boardID = LMS_DEV_UNKNOWN;
    info.gatewareVersion = 0;
    info.gatewareRevision = 0;
    GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_RD;
    const uint16_t addrs[] = {0x0000, 0x0001, 0x0002, 0x0003};
    for (size_t i = 0; i < 4; ++i)
    {
        pkt.outBuffer.push_back(addrs[i] >> 8);
        pkt.outBuffer.push_back(addrs[i] & 0xFF);
    }
    int status = this->TransferPacket(pkt);
    if (status == 0 && pkt.inBuffer.size() >= sizeof(addrs)*2)
    {
        info.boardID = eLMS_DEV((pkt.inBuffer[2] << 8) | pkt.inBuffer[3]);
        info.gatewareVersion = (pkt.inBuffer[6] << 8) | pkt.inBuffer[7];
        info.gatewareRevision = (pkt.inBuffer[10] << 8) | pkt.inBuffer[11];
        info.hwVersion = pkt.inBuffer[15]&0x7F;
    }
    return info;
}

int LMS64CProtocol::ProgramWrite(const char *data_src, const size_t length, const int prog_mode, const int device, ProgrammingCallback callback)
{
#ifndef NDEBUG
    auto t1 = std::chrono::high_resolution_clock::now();
#endif
    //erasing FLASH can take up to 3 seconds before reply is received
    const int progTimeout_ms = 5000;
    char progressMsg[128];
    sprintf(progressMsg, "in progress...");
    bool abortProgramming = false;
    int bytesSent = 0;

    bool needsData = true;
    if(device == FPGA && prog_mode == 2)
        needsData = false;
    if(device == FX3 && (prog_mode == 0 || prog_mode == 1))
        needsData = false;

    if(length == 0 && needsData)
    {
        return ReportError(EIO, "ProgramWrite length should be > 0");
    }

    if (not this->IsOpen())
    {
        return ReportError(ENOTCONN, "connection is not open");
    }

    const int pktSize = 32;
    int data_left = length;
    const int portionsCount = length/pktSize + (length%pktSize > 0) + 1; // +1 programming end packet
    int portionNumber;
    int status = 0;
    eCMD_LMS cmd;
    if(device == HPM || device == FX3)
        cmd = CMD_MEMORY_WR;
    else if(device == FPGA)
        cmd = CMD_ALTERA_FPGA_GW_WR;
    else
    {
        sprintf(progressMsg, "Programming failed! Target device not supported");
        if(callback)
            callback(bytesSent, length, progressMsg);
        return ReportError(ENOTSUP, progressMsg);
    }

    unsigned char ctrbuf[64];
    unsigned char inbuf[64];
    memset(ctrbuf, 0, 64);
    ctrbuf[0] = cmd;
    ctrbuf[1] = 0;
    ctrbuf[2] = 56;

    for (portionNumber = 0; portionNumber<portionsCount && !abortProgramming; ++portionNumber)
    {
        int offset = 8;
        memset(&ctrbuf[offset], 0, 56);
        ctrbuf[offset+0] = prog_mode;
        ctrbuf[offset+1] = (portionNumber >> 24) & 0xFF;
        ctrbuf[offset+2] = (portionNumber >> 16) & 0xFF;
        ctrbuf[offset+3] = (portionNumber >> 8) & 0xFF;
        ctrbuf[offset+4] = portionNumber & 0xFF;
        unsigned char data_cnt = data_left > pktSize ? pktSize : data_left;
        ctrbuf[offset+5] = data_cnt;
        if(cmd == CMD_MEMORY_WR)
        {
            ctrbuf[offset+6] = 0;
            ctrbuf[offset+7] = 0;
            ctrbuf[offset+8] = 0;
            ctrbuf[offset+9] = 0;

            ctrbuf[offset+10] = (device >> 8) & 0xFF;
            ctrbuf[offset+11] = device & 0xFF;
        }
        if(data_src != NULL)
        {
            memcpy(&ctrbuf[offset + 24], data_src, data_cnt);
            data_src += data_cnt;
        }

        if(Write(ctrbuf, sizeof(ctrbuf)) != sizeof(ctrbuf))
        {
            if(callback)
                callback(bytesSent, length, "Programming failed! Write operation failed");
            return ReportError(EIO, "Programming failed! Write operation failed");
        }
        if(Read(inbuf, sizeof(inbuf), progTimeout_ms) != sizeof(ctrbuf))
        {
            if(callback)
                callback(bytesSent, length, "Programming failed! Read operation failed");
            return ReportError(EIO, "Programming failed! Read operation failed");
        }
        data_left -= data_cnt;
        status = inbuf[1];
        bytesSent += data_cnt;

        if(status != STATUS_COMPLETED_CMD)
        {
            sprintf(progressMsg, "Programming failed! %s", status2string(status));
            if(callback)
                callback(bytesSent, length, progressMsg);
            return ReportError(EPROTO, progressMsg);
        }
        if(needsData == false) //only one packet is needed to initiate bitstream from flash
        {
            bytesSent = length;
            break;
        }
        if(callback)
            abortProgramming = callback(bytesSent, length, progressMsg);
    }
    if (abortProgramming == true)
    {
        sprintf(progressMsg, "programming: aborted by user");
        if(callback)
            callback(bytesSent, length, progressMsg);
        return ReportError(ECONNABORTED, "user aborted programming");
    }
    sprintf(progressMsg, "programming: completed");
    if(callback)
        callback(bytesSent, length, progressMsg);
#ifndef NDEBUG
    auto t2 = std::chrono::high_resolution_clock::now();
	if ((device == 2 && prog_mode == 2) == false)
	    lime::log(LOG_LEVEL_INFO, "Programming finished, %li bytes sent! %li ms\n", length, std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	else
        lime::log(LOG_LEVEL_INFO, "FPGA configuring initiated\n");
#endif
    return 0;
}

int LMS64CProtocol::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_WR;
    for (size_t i=0; i<bufLength; ++i)
        pkt.outBuffer.push_back(buffer[i]);
    return TransferPacket(pkt);
}

int LMS64CProtocol::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_RD;
    int status = TransferPacket(pkt);
    if(status != 0)
        return status;

    for (size_t i=0; i<bufLength; ++i)
        buffer[i] = pkt.inBuffer[i];
    return status;
}

int LMS64CProtocol::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_DIR_WR;
    for (size_t i=0; i<bufLength; ++i)
        pkt.outBuffer.push_back(buffer[i]);
    return TransferPacket(pkt);
}

int LMS64CProtocol::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_DIR_RD;
    int status = TransferPacket(pkt);
    if(status != 0)
        return convertStatus(status, pkt);

    for (size_t i=0; i<bufLength; ++i)
        buffer[i] = pkt.inBuffer[i];
    return status;
}

int LMS64CProtocol::ProgramMCU(const uint8_t *buffer, const size_t length, const MCU_PROG_MODE mode, ProgrammingCallback callback)
{
#ifndef NDEBUG
    auto timeStart = std::chrono::high_resolution_clock::now();
#endif
    const uint8_t fifoLen = 32;
    bool success = true;
    bool terminate = false;

    int packetNumber = 0;
    int status = STATUS_UNDEFINED;

    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_PROG_MCU;

    if (callback)
        terminate = callback(0, length,"");

    for(uint16_t CntEnd=0; CntEnd<length && !terminate; CntEnd+=32)
    {
        pkt.outBuffer.clear();
        pkt.outBuffer.reserve(fifoLen+2);
        pkt.outBuffer.push_back(mode);
        pkt.outBuffer.push_back(packetNumber++);
        for (uint8_t i=0; i<fifoLen; i++)
            pkt.outBuffer.push_back(buffer[CntEnd + i]);

        TransferPacket(pkt);
        status = pkt.status;
        if (callback)
            terminate = callback(CntEnd+fifoLen,length,"");
#ifndef NDEBUG
        lime::log(LOG_LEVEL_INFO, "MCU programming : %4i/%4li\r", CntEnd+fifoLen, long(length));
#endif
        if(status != STATUS_COMPLETED_CMD)
        {
            std::stringstream ss;
            ss << "Programming MCU: status : not completed, block " << packetNumber << std::endl;
            success = false;
            break;
        }

        if(mode == 3) // if boot mode , send only first packet
        {
            if (callback)
                callback(1, 1, "");
            break;
        }
	};
#ifndef NDEBUG
    auto timeEnd = std::chrono::high_resolution_clock::now();
    lime::log(LOG_LEVEL_INFO, "\nMCU Programming finished, %li ms\n",
            std::chrono::duration_cast<std::chrono::milliseconds>
            (timeEnd-timeStart).count());
#endif
    return success ? 0 : -1;
}
*/
}
