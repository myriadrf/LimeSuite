/**
    @file IConnection.h
    @author Lime Microsystems
    @brief Interface class for connection types
*/

#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <string>
#include <vector>
#include <mutex>
#include <cstring> //memset
#include <functional>
#include <lms7002_defines.h>

struct LMSinfo
{
    eLMS_DEV device;
    eEXP_BOARD expansion;
    int firmware;
    int hardware;
    int protocol;
};

using namespace std;

class IConnection
{
public:

    /// Supported connection types.
    enum eConnectionType
    {
        CONNECTION_UNDEFINED = -1,
        COM_PORT = 0,
        USB_PORT = 1,
        SPI_PORT = 2,
        //insert new types here
        CONNECTION_TYPES_COUNT //used only for memory allocation
    };

    enum eLMS_PROTOCOL
    {
        LMS_PROTOCOL_UNDEFINED = 0,
        LMS_PROTOCOL_DIGIC,
        LMS_PROTOCOL_LMS64C,
        LMS_PROTOCOL_NOVENA,
    };

    enum DeviceStatus
    {
        SUCCESS,
        FAILURE,
        END_POINTS_NOT_FOUND,
        CANNOT_CLAIM_INTERFACE
    };

    enum TransferStatus
    {
        TRANSFER_SUCCESS,
        TRANSFER_FAILED,
        NOT_CONNECTED
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

    virtual TransferStatus TransferPacket(GenericPacket &pkt);

    LMSinfo GetInfo();

    int ReadStream(char *buffer, int length, unsigned int timeout_ms)
    {
        /*int handle = activeControlPort->BeginDataReading(buffer, length);
        activeControlPort->WaitForReading(handle, timeout_ms);
            long received = length;
            activeControlPort->FinishDataReading(buffer, received, handle);
            return received;
        */
        long len = length;
        int status = this->ReadDataBlocking(buffer, len, 0);
        return len;
    }

	IConnection(void);
	virtual ~IConnection(void);
	virtual int RefreshDeviceList() = 0;
	virtual DeviceStatus Open(unsigned i) = 0;
	virtual void Close() = 0;
	virtual bool IsOpen() = 0;
	virtual int GetOpenedIndex() = 0;

	virtual int Write(const unsigned char *buffer, int length, int timeout_ms = 0) = 0;
	virtual int Read(unsigned char *buffer, int length, int timeout_ms = 0) = 0;

	virtual std::vector<std::string> GetDeviceNames() = 0;

	virtual eConnectionType GetType() { return m_connectionType; };
	virtual bool SetParam(const char *name, const char* value) {return false;};

	virtual int BeginDataReading(char *buffer, long length){ return -1; };
	virtual int WaitForReading(int contextHandle, unsigned int timeout_ms){ return 0;};
	virtual int FinishDataReading(char *buffer, long &length, int contextHandle){ return 0;}
	virtual void AbortReading(){};
    virtual int ReadDataBlocking(char *buffer, long &length, int timeout_ms){ return 0; }

	virtual int BeginDataSending(const char *buffer, long length){ return -1; };
	virtual int WaitForSending(int contextHandle, unsigned int timeout_ms){ return 0;};
	virtual int FinishDataSending(const char *buffer, long &length, int contextHandle){ return 0;}
	virtual void AbortSending(){};

    /** @brief Sets callback function which gets called each time data is sent or received
    */
    void SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback);

protected:
    unsigned char* PreparePacket(const GenericPacket &pkt, int &length, const eLMS_PROTOCOL protocol);
    int ParsePacket(GenericPacket &pkt, const unsigned char* buffer, const int length, const eLMS_PROTOCOL protocol);
    eConnectionType m_connectionType;
    std::function<void(bool, const unsigned char*, const unsigned int)> callback_logData;
    bool mSystemBigEndian;
    std::mutex mControlPortLock;
};

#endif

