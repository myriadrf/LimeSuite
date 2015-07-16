/**
@file   LMScomms.h
@author Lime Microsystems (www.limemicro.com)
@brief  Class for handling data transmission to LMS boards
*/

#ifndef LMS_COMMS_H
#define LMS_COMMS_H

#include "lms7002_defines.h"
#include "connectionManager/ConnectionManager.h"
#include <string.h>
#include <mutex>

struct LMSinfo
{
    eLMS_DEV device;
    eEXP_BOARD expansion;
    int firmware;
    int hardware;
    int protocol;
};

/*  @brief Class for abstracting transfering data to and from chip
*/
class LMScomms : public ConnectionManager
{	
public:
	enum TransferStatus
	{
		TRANSFER_SUCCESS,
		TRANSFER_FAILED,
		NOT_CONNECTED
	};

    enum eLMS_PROTOCOL
    {
        LMS_PROTOCOL_UNDEFINED = 0,
        LMS_PROTOCOL_DIGIC,
        LMS_PROTOCOL_LMS64C,
        LMS_PROTOCOL_NOVENA,
    };
    struct ProtocolDIGIC
    {
        static const int pktLength = 64;
        static const int maxDataLength = 60;
        ProtocolDIGIC() : cmd(0), i2cAddr(0), blockCount(0) {};
        unsigned char cmd;
        unsigned char i2cAddr;
        unsigned char blockCount;
        unsigned char reserved;
        unsigned char data[maxDataLength];
    };

    struct ProtocolLMS64C
    {
        static const int pktLength = 64;
        static const int maxDataLength = 56;
        ProtocolLMS64C() :cmd(0),status(STATUS_UNDEFINED),blockCount(0)
        {
            memset(reserved, 0, 5);
        };
        unsigned char cmd;
        unsigned char status;
        unsigned char blockCount;
        unsigned char reserved[5];
        unsigned char data[maxDataLength];
    };

    struct ProtocolNovena
    {
        static const int pktLength = 128;
        static const int maxDataLength = 128;
        ProtocolNovena() :cmd(0),status(0) {};
        unsigned char cmd;
        unsigned char status;
        unsigned char blockCount;
        unsigned char data[maxDataLength];
    };

    struct GenericPacket
    {   
        GenericPacket()
        {
            cmd = CMD_GET_INFO;
            status = STATUS_UNDEFINED;
        }

        eCMD_LMS cmd;
        eCMD_STATUS status;
        vector<unsigned char> outBuffer;
        vector<unsigned char> inBuffer;
    };

    LMScomms();
    ~LMScomms();

    virtual TransferStatus TransferPacket(GenericPacket &pkt);
    LMSinfo GetInfo();
    void SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback);
protected:
    std::function<void(bool, const unsigned char*, const unsigned int)> callback_logData;
    std::mutex mControlPortLock;
    unsigned char* PreparePacket(const GenericPacket &pkt, int &length, const eLMS_PROTOCOL protocol);
    int ParsePacket(GenericPacket &pkt, const unsigned char* buffer, const int length, const eLMS_PROTOCOL protocol);
protected:
    bool mSystemBigEndian;
};

#endif // LMS_COMMS_H

