/**
    @file LMS64CProtocol.h
    @author Lime Microsystems
    @brief Implementation of LMS64C protocol.
*/

#pragma once
#include <IConnection.h>
#include <mutex>

/*!
 * Implement the LMS64CProtocol.
 * The LMS64CProtocol is an IConnection that implements
 * configuration and spi access over the LMS64C Protocol.
 * Connections using LMS64C may inherit from LMS64C.
 */
class LMS64CProtocol : public virtual IConnection
{
public:
    LMS64CProtocol(void);

    virtual ~LMS64CProtocol(void);

    //! DeviceReset implemented by LMS64C
    OperationStatus DeviceReset(void);

    //! TransactSPI implemented by LMS64C
    OperationStatus TransactSPI(const int index, const uint32_t *writeData, uint32_t *readData, const size_t size);

    /*!
     * Transfer a packet over the underlying transport layer.
     * TransferPacket performs a request/response
     * using the GenericPacket data structure.
     * Some implementations will cast to LMS64CProtocol
     * and directly use the TransferPacket() API call.
     */
    TransferStatus TransferPacket(GenericPacket &pkt);

    DeviceInfo GetDeviceInfo(void);

    LMSinfo GetInfo();

protected:

    //! implement in base class
    virtual eConnectionType GetType(void) = 0;

    //! virtual write function to be implemented by the base class
    virtual int Write(const unsigned char *buffer, int length, int timeout_ms = 0) = 0;

    //! virtual read function to be implemented by the base class
    virtual int Read(unsigned char *buffer, int length, int timeout_ms = 0) = 0;

private:

    unsigned char* PreparePacket(const GenericPacket &pkt, int &length, const eLMS_PROTOCOL protocol);
    int ParsePacket(GenericPacket &pkt, const unsigned char* buffer, const int length, const eLMS_PROTOCOL protocol);
    std::mutex mControlPortLock;
};
