/**
    @file   IConnection.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  Implementation of IConnection interface
*/

#include "IConnection.h"
#include "ErrorReporting.h"
#include <cstring> //memcpy
#include <chrono>
#include <thread>
#include <iso646.h>

using namespace lime;

DeviceInfo::DeviceInfo(void):
    addrSi5351(-1),
    addrADF4002(-1)
{
    return;
}

StreamMetadata::StreamMetadata(void):
    timestamp(0),
    hasTimestamp(false),
    endOfBurst(false),
    lateTimestamp(false),
    packetDropped(false)
{
    return;
}

StreamConfig::StreamConfig(void):
    isTx(false),
    performanceLatency(0.5),
    bufferLength(0),
    format(STREAM_12_BIT_IN_16),
    linkFormat(STREAM_12_BIT_IN_16)
{
    return;
}

IConnection::IConnection(void)
{
    callback_logData = nullptr;
    unsigned short test = 0x1234;
    unsigned char* bytes = (unsigned char*)&test;
    if(bytes[0] == 0x12 && bytes[1] == 0x34)
        mSystemBigEndian = true;
    else
        mSystemBigEndian = false;
}

IConnection::~IConnection(void)
{
    return;
}

const ConnectionHandle &IConnection::GetHandle(void) const
{
    return _handle;
}

bool IConnection::IsOpen(void)
{
    return false;
}

DeviceInfo IConnection::GetDeviceInfo(void)
{
    DeviceInfo info;
    info.addrsLMS7002M.push_back(0);
    return info;
}

/***********************************************************************
 * Serial API
 **********************************************************************/

int IConnection::TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    ReportError(ENOTSUP);
    return -1;
}

int IConnection::WriteI2C(const int addr, const std::string &data)
{
    ReportError(ENOTSUP);
    return -1;
}

int IConnection::ReadI2C(const int addr, const size_t numBytes, std::string &data)
{
    ReportError(ENOTSUP);
    return -1;
}

/***********************************************************************
 * LMS7002M Driver callbacks
 **********************************************************************/

int IConnection::DeviceReset(void)
{
    ReportError(ENOTSUP);
    return -1;
}

void IConnection::UpdateExternalBandSelect(const size_t channel, const int trfBand, const int rfePath)
{
    return;
}

int IConnection::UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate)
{
    return 0;
}

int IConnection::UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate, const double txPhase, const double rxPhase)
{
    return 0;
}

void IConnection::EnterSelfCalibration(const size_t channel)
{
    return;
}

void IConnection::ExitSelfCalibration(const size_t channel)
{
    return;
}

/***********************************************************************
 * Reference clocks API
 **********************************************************************/

double IConnection::GetReferenceClockRate(void)
{
    //this is the populated TCXO on many boards
    return 61.44e6/2;
}

int IConnection::SetReferenceClockRate(const double rate)
{
    return ReportError(EPERM, "SetReferenceClockRate not implemented");
}

double IConnection::GetTxReferenceClockRate(void)
{
    return this->GetReferenceClockRate();
}

int IConnection::SetTxReferenceClockRate(const double rate)
{
    return this->SetReferenceClockRate(rate);
}

/***********************************************************************
 * Timestamp API
 **********************************************************************/

uint64_t IConnection::GetHardwareTimestamp(void)
{
    return 0;
}

void IConnection::SetHardwareTimestamp(const uint64_t now)
{
    return;
}

double IConnection::GetHardwareTimestampRate(void)
{
    return 1.0;
}

/***********************************************************************
 * Stream API
 **********************************************************************/

int IConnection::SetupStream(size_t &streamID, const StreamConfig &config)
{
    streamID = ~0;
    return ReportError(EPERM, "SetupStream not implemented");
}

int IConnection::CloseStream(const size_t streamID)
{
    return ReportError(EPERM, "CloseStream not implemented");
}

size_t IConnection::GetStreamSize(const size_t streamID)
{
    //this should be overloaded, but if not,
    //pick a number that will probably work (power of 2)
    return 16*1024;
}

int IConnection::ControlStream(const size_t streamID, const bool enable)
{
    return ReportError(EPERM, "ControlStream not implemented");
}

int IConnection::ReadStream(const size_t streamID, void* buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata)
{
    return ReportError(EPERM, "ReadStream not implemented");
}

int IConnection::WriteStream(const size_t streamID, const void* buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata)
{
    return ReportError(EPERM, "WriteStream not implemented");
}

int IConnection::ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata &metadata)
{
    return ReportError(EPERM, "ReadStreamStatus not implemented");
}

int IConnection::UploadWFM(const void * const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex)
{
    return ReportError(EPERM, "UploadTxWFM not implemented");
}

/** @brief Sets callback function which gets called each time data is sent or received
*/
void IConnection::SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback)
{
    callback_logData = callback;
}

int IConnection::ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms)
{
    return ReportError(ENOTSUP, "Not implemented");
}
/***********************************************************************
 * Programming API
 **********************************************************************/

int IConnection::ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int index, ProgrammingCallback callback)
{
    ReportError(ENOTSUP, "ProgramWrite not supported");
    return -1;
}

int IConnection::ProgramRead(char *buffer, const size_t length, const int index, ProgrammingCallback callback)
{
    ReportError(ENOTSUP, "ProgramRead not supported");
    return -1;
}

int IConnection::ProgramUpdate(const bool download, ProgrammingCallback callback)
{
    return 0;
}

/***********************************************************************
 * GPIO API
 **********************************************************************/

int IConnection::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIOWrite not supported");
    return -1;
}

int IConnection::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIORead not supported");
    return -1;
}

int IConnection::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIOWrite not supported");
    return -1;
}

int IConnection::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIORead not supported");
    return -1;
}

/***********************************************************************
 * Register API
 **********************************************************************/

int IConnection::WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size)
{
    ReportError(ENOTSUP, "WriteRegisters not supported");
    return -1;
}

int IConnection::ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size)
{
    ReportError(ENOTSUP, "ReadRegisters not supported");
    return -1;
}

int IConnection::WriteRegister(const uint32_t addr, const uint32_t data)
{
    return this->WriteRegisters(&addr, &data, 1);
}

/***********************************************************************
 * Aribtrary settings API
 **********************************************************************/

int IConnection::CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string* units)
{
    ReportError(ENOTSUP, "CustomParameterWrite not supported");
    return -1;
}

int IConnection::CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units)
{
    ReportError(ENOTSUP, "CustomParameterRead not supported");
    return -1;
}
