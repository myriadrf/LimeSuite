/**
    @file   IConnection.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  Implementation of IConnection interface
*/

#include "IConnection.h"
#include <cstring> //memcpy

RFICInfo::RFICInfo(void):
    spiIndexRFIC(-1),
    spiIndexSi5351(-1),
    rxChannel(-1),
    txChannel(-1)
{
    return;
}

StreamMetadata::StreamMetadata(void):
    timestamp(-1),
    endOfBurst(false)
{
    return;
}

IConnection::IConnection(void)//:
    //m_connectionType(CONNECTION_UNDEFINED)
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

bool IConnection::IsOpen(void)
{
    return false;
}

std::vector<RFICInfo> IConnection::ListRFICs(void)
{
    return std::vector<RFICInfo>(1);
}

OperationStatus IConnection::DeviceReset(void)
{
    return UNSUPPORTED;
}

OperationStatus IConnection::TransactSPI(const int index, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    return UNSUPPORTED;
}

OperationStatus IConnection::WriteSi5351C(const uint16_t *writeData, const size_t size)
{
    return UNSUPPORTED;
}

bool IConnection::RxStreamControl(const int streamID, const size_t burstSize, const StreamMetadata &metadata)
{
    return false;
}

int IConnection::ReadStream(const int streamID, void * const *buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata)
{
    return -1;
}

int IConnection::WriteStream(const int streamID, const void * const *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata)
{
    return -1;
}

/** @brief Transfers data between packet and connected device
    @param pkt packet containing output data and to receive incomming data
    @return 0: success, other: failure
*/
IConnection::TransferStatus IConnection::TransferPacket(GenericPacket& pkt)
{
    throw std::runtime_error("TransferPacket -> This is the old call, removing soon....");
}

/** @brief Returns connected device information
*/
LMSinfo IConnection::GetInfo()
{
    LMSinfo info;
    info.device = LMS_DEV_UNKNOWN;
    info.expansion = EXP_BOARD_UNKNOWN;
    info.firmware = 0;
    info.hardware = 0;
    info.protocol = 0;
    GenericPacket pkt;
    pkt.cmd = CMD_GET_INFO;
    IConnection::TransferStatus status = TransferPacket(pkt);
    if (status == IConnection::TRANSFER_SUCCESS && pkt.inBuffer.size() >= 5)
    {
        info.firmware = pkt.inBuffer[0];
        info.device = pkt.inBuffer[1] < LMS_DEV_COUNT ? (eLMS_DEV)pkt.inBuffer[1] : LMS_DEV_UNKNOWN;
        info.protocol = pkt.inBuffer[2];
        info.hardware = pkt.inBuffer[3];
        info.expansion = pkt.inBuffer[4] < EXP_BOARD_COUNT ? (eEXP_BOARD)pkt.inBuffer[4] : EXP_BOARD_UNKNOWN;
    }
    return info;
}

/** @brief Sets callback function which gets called each time data is sent or received
*/
void IConnection::SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback)
{
    callback_logData = callback;
}
