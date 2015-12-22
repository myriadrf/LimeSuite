/**
    @file LMS64CProtocol.cpp
    @author Lime Microsystems
    @brief Implementation of LMS64C protocol.
*/

#include "LMS64CProtocol.h"

LMS64CProtocol::LMS64CProtocol(void)
{
    return;
}

LMS64CProtocol::~LMS64CProtocol(void)
{
    return;
}

OperationStatus LMS64CProtocol::DeviceReset(void)
{
    if (not this->IsOpen()) return OperationStatus::DISCONNECTED;

    IConnection::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RST;
    pkt.outBuffer.push_back(LMS_RST_PULSE);
    IConnection::TransferStatus status = this->TransferPacket(pkt);
    if (status != IConnection::TRANSFER_SUCCESS) return OperationStatus::FAILED;
    switch (pkt.status)
    {
    case STATUS_COMPLETED_CMD: return OperationStatus::SUCCESS;
    case STATUS_UNKNOWN_CMD: return OperationStatus::UNSUPPORTED;
    }
    return OperationStatus::FAILED;
}

OperationStatus LMS64CProtocol::TransactSPI(const int index, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    //TODO for multiple LMS7002M, the index would need to be encoded into the packet

    if (not this->IsOpen()) return OperationStatus::DISCONNECTED;

    IConnection::TransferStatus status;
    IConnection::GenericPacket pkt;

    if (readData != nullptr)
    {
        pkt.cmd = CMD_LMS7002_RD;
        for (size_t i = 0; i < size; ++i)
        {
            uint16_t addr = (writeData[i] >> 16) & 0x7fff;
            pkt.outBuffer.push_back(addr >> 8);
            pkt.outBuffer.push_back(addr & 0xFF);
        }

        status = this->TransferPacket(pkt);

        for (size_t i = 0; i < size; ++i)
        {
            readData[i] = (pkt.inBuffer[2*sizeof(uint16_t)*i + 2] << 8) | pkt.inBuffer[2*sizeof(uint16_t)*i + 3];
        }
    }
    else
    {
        pkt.cmd = CMD_LMS7002_WR;
        for (size_t i = 0; i < size; ++i)
        {
            uint16_t addr = (writeData[i] >> 16) & 0x7fff;
            uint16_t data = writeData[i] & 0xffff;
            pkt.outBuffer.push_back(addr >> 8);
            pkt.outBuffer.push_back(addr & 0xFF);
            pkt.outBuffer.push_back(data >> 8);
            pkt.outBuffer.push_back(data & 0xFF);
        }

        status = this->TransferPacket(pkt);
    }

    if (status != IConnection::TRANSFER_SUCCESS) return OperationStatus::FAILED;
    switch (pkt.status)
    {
    case STATUS_COMPLETED_CMD: return OperationStatus::SUCCESS;
    case STATUS_UNKNOWN_CMD: return OperationStatus::UNSUPPORTED;
    }
    return OperationStatus::FAILED;
}

OperationStatus LMS64CProtocol::WriteSi5351C(const uint16_t *writeData, const size_t size)
{
    GenericPacket pkt;
    pkt.cmd = CMD_SI5351_WR;

    for (size_t i = 0; i < size; i++)
    {
        pkt.outBuffer.push_back(writeData[i] >> 8);
        pkt.outBuffer.push_back(writeData[i] & 0xff);
    }

    IConnection::TransferStatus status;
    status = this->TransferPacket(pkt);
    if (status != IConnection::TRANSFER_SUCCESS) return OperationStatus::FAILED;
    switch (pkt.status)
    {
    case STATUS_COMPLETED_CMD: return OperationStatus::SUCCESS;
    case STATUS_UNKNOWN_CMD: return OperationStatus::UNSUPPORTED;
    }
    return OperationStatus::FAILED;
}

/** @brief Transfers data between packet and connected device
    @param pkt packet containing output data and to receive incomming data
    @return 0: success, other: failure
*/
IConnection::TransferStatus LMS64CProtocol::TransferPacket(GenericPacket& pkt)
{
    std::lock_guard<std::mutex> lock(mControlPortLock);
    TransferStatus status = TRANSFER_SUCCESS;
    if(IsOpen() == false)
        return NOT_CONNECTED;

    int packetLen;
    eLMS_PROTOCOL protocol = LMS_PROTOCOL_UNDEFINED;
    if(this->GetType() == IConnection::SPI_PORT)
        protocol = LMS_PROTOCOL_NOVENA;
    else
        protocol = LMS_PROTOCOL_LMS64C;
    switch(protocol)
    {
    case LMS_PROTOCOL_UNDEFINED:
        return TRANSFER_FAILED;
    case LMS_PROTOCOL_LMS64C:
        packetLen = ProtocolLMS64C::pktLength;
        break;
    case LMS_PROTOCOL_NOVENA:
        packetLen = pkt.outBuffer.size() > ProtocolNovena::pktLength ? ProtocolNovena::pktLength : pkt.outBuffer.size();
        break;
    default:
        packetLen = 0;
        return TRANSFER_FAILED;
    }
    int outLen = 0;
    unsigned char* outBuffer = NULL;
    outBuffer = PreparePacket(pkt, outLen, protocol);
    unsigned char* inBuffer = new unsigned char[outLen];
    memset(inBuffer, 0, outLen);

    int outBufPos = 0;
    int inDataPos = 0;
    if(outLen == 0)
    {
        //printf("packet outlen = 0\n");
        outLen = 1;
    }

    if(protocol == LMS_PROTOCOL_NOVENA)
    {
        bool transferData = true; //some commands are fake, so don't need transferring
        if(pkt.cmd == CMD_GET_INFO)
        {
            //spi does not have GET INFO, fake it to inform what device it is
            pkt.status = STATUS_COMPLETED_CMD;
            pkt.inBuffer.clear();
            pkt.inBuffer.resize(64, 0);
            pkt.inBuffer[0] = 0; //firmware
            pkt.inBuffer[1] = LMS_DEV_NOVENA; //device
            pkt.inBuffer[2] = 0; //protocol
            pkt.inBuffer[3] = 0; //hardware
            pkt.inBuffer[4] = EXP_BOARD_UNSUPPORTED; //expansion
            transferData = false;
        }

        if(transferData)
        {
            if (callback_logData)
                callback_logData(true, outBuffer, outLen);
            int bytesWritten = Write(outBuffer, outLen);
            if( bytesWritten == outLen)
            {
                if(pkt.cmd == CMD_LMS7002_RD)
                {
                    inDataPos = Read(&inBuffer[inDataPos], outLen);
                    if(inDataPos != outLen)
                        status = TRANSFER_FAILED;
                    else
                    {
                        if (callback_logData)
                            callback_logData(false, inBuffer, inDataPos);
                    }
                }
                ParsePacket(pkt, inBuffer, inDataPos, protocol);
            }
            else
                status = TRANSFER_FAILED;
        }
    }
    else
    {
        for(int i=0; i<outLen; i+=packetLen)
        {
            int bytesToSend = packetLen;
            if (callback_logData)
                callback_logData(true, &outBuffer[outBufPos], bytesToSend);
            if( Write(&outBuffer[outBufPos], bytesToSend) )
            {
                outBufPos += packetLen;
                long readLen = packetLen;
                int bread = Read(&inBuffer[inDataPos], readLen);
                if(bread != readLen && protocol != LMS_PROTOCOL_NOVENA)
                {
                    status = TRANSFER_FAILED;
                    break;
                }
                if (callback_logData)
                    callback_logData(false, &inBuffer[inDataPos], bread);
                inDataPos += bread;
            }
            else
            {
                status = TRANSFER_FAILED;
                break;
            }
        }
        ParsePacket(pkt, inBuffer, inDataPos, protocol);
    }
    delete outBuffer;
    delete inBuffer;
    return status;
}


/** @brief Takes generic packet and converts to specific protocol buffer
    @param pkt generic data packet to convert
    @param length returns length of returned buffer
    @param protocol which protocol to use for data
    @return pointer to data buffer, must be manually deleted after use
*/
unsigned char* LMS64CProtocol::PreparePacket(const GenericPacket& pkt, int& length, const eLMS_PROTOCOL protocol)
{
    unsigned char* buffer = NULL;
    if(protocol == LMS_PROTOCOL_UNDEFINED)
        return NULL;

    if(protocol == LMS_PROTOCOL_LMS64C)
    {
        ProtocolLMS64C packet;
        int maxDataLength = packet.maxDataLength;
        packet.cmd = pkt.cmd;
        packet.status = pkt.status;
        int byteBlockRatio = 1; //block ratio - how many bytes in one block
        switch( packet.cmd )
        {
        case CMD_PROG_MCU:
        case CMD_GET_INFO:
        case CMD_SI5351_RD:
        case CMD_SI5356_RD:
            byteBlockRatio = 1;
            break;
        case CMD_SI5351_WR:
        case CMD_SI5356_WR:
            byteBlockRatio = 2;
            break;
        case CMD_LMS7002_RD:
        case CMD_BRDSPI_RD:
        case CMD_BRDSPI8_RD:
            byteBlockRatio = 2;
            break;
        case CMD_ADF4002_WR:
            byteBlockRatio = 3;
            break;
        case CMD_LMS7002_WR:
        case CMD_BRDSPI_WR:
        case CMD_ANALOG_VAL_WR:
            byteBlockRatio = 4;
            break;
        default:
            byteBlockRatio = 1;
        }
        if (packet.cmd == CMD_LMS7002_RD || packet.cmd == CMD_BRDSPI_RD)
            maxDataLength = maxDataLength/2;
        if (packet.cmd == CMD_ANALOG_VAL_RD)
            maxDataLength = maxDataLength / 4;
        int blockCount = pkt.outBuffer.size()/byteBlockRatio;
        int bufLen = blockCount/(maxDataLength/byteBlockRatio)
                    +(blockCount%(maxDataLength/byteBlockRatio)!=0);
        bufLen *= packet.pktLength;
        if(bufLen == 0)
            bufLen = packet.pktLength;
        buffer = new unsigned char[bufLen];
        memset(buffer, 0, bufLen);
        int srcPos = 0;
        for(int j=0; j*packet.pktLength<bufLen; ++j)
        {
            int pktPos = j*packet.pktLength;
            buffer[pktPos] = packet.cmd;
            buffer[pktPos+1] = packet.status;
            if(blockCount > (maxDataLength/byteBlockRatio))
            {
                buffer[pktPos+2] = maxDataLength/byteBlockRatio;
                blockCount -= buffer[pktPos+2];
            }
            else
                buffer[pktPos+2] = blockCount;
            memcpy(&buffer[pktPos+3], packet.reserved, sizeof(packet.reserved));
            int bytesToPack = (maxDataLength/byteBlockRatio)*byteBlockRatio;
            for (int k = 0; k<bytesToPack && srcPos < pkt.outBuffer.size(); ++srcPos, ++k)
                buffer[pktPos + 8 + k] = pkt.outBuffer[srcPos];
        }
        length = bufLen;
    }
    else if(protocol == LMS_PROTOCOL_NOVENA)
    {
        if(pkt.cmd == CMD_LMS7002_RST)
        {
            buffer = new unsigned char[8];
            buffer[0] = 0x88;
            buffer[1] = 0x06;
            buffer[2] = 0x00;
            buffer[3] = 0x18;
            buffer[4] = 0x88;
            buffer[5] = 0x06;
            buffer[6] = 0x00;
            buffer[7] = 0x38;
            length = 8;
        }
        else
        {
            buffer = new unsigned char[pkt.outBuffer.size()];
            memcpy(buffer, &pkt.outBuffer[0], pkt.outBuffer.size());
            if (pkt.cmd == CMD_LMS7002_WR)
            {
                for(int i=0; i<pkt.outBuffer.size(); i+=4)
                    buffer[i] |= 0x80;
            }
            length = pkt.outBuffer.size();
        }
    }
    return buffer;
}

/** @brief Parses given data buffer into generic packet
    @param pkt destination packet
    @param buffer received data from board
    @param length received buffer length
    @param protocol which protocol to use for data parsing
    @return 1:success, 0:failure
*/
int LMS64CProtocol::ParsePacket(GenericPacket& pkt, const unsigned char* buffer, const int length, const eLMS_PROTOCOL protocol)
{
    if(protocol == LMS_PROTOCOL_UNDEFINED)
        return -1;

    if(protocol == LMS_PROTOCOL_LMS64C)
    {
        ProtocolLMS64C packet;
        int inBufPos = 0;
        pkt.inBuffer.resize(packet.maxDataLength*(length / packet.pktLength + (length % packet.pktLength)), 0);
        for(int i=0; i<length; i+=packet.pktLength)
        {
            pkt.cmd = (eCMD_LMS)buffer[i];
            pkt.status = (eCMD_STATUS)buffer[i+1];
            memcpy(&pkt.inBuffer[inBufPos], &buffer[i+8], packet.maxDataLength);
            inBufPos += packet.maxDataLength;
        }
    }
    else if(protocol == LMS_PROTOCOL_NOVENA)
    {
        pkt.cmd = CMD_LMS7002_RD;
        pkt.status = STATUS_COMPLETED_CMD;
        pkt.inBuffer.clear();
        for(int i=0; i<length; i+=2)
        {
            //reading from spi returns only registers values
            //fill addresses as zeros to match generic format of address, value pairs
            pkt.inBuffer.push_back(0); //should be address msb
            pkt.inBuffer.push_back(0); //should be address lsb
            pkt.inBuffer.push_back(buffer[i]);
            pkt.inBuffer.push_back(buffer[i+1]);
        }
    }
    return 1;
}
