/**
@file   ConnectionNovenaRF7.h
@author Lime Microsystems (www.limemicro.com)
@brief  Class for NovenaRF7 communications through SPI port
*/

#ifndef CONNECTION_NOVENA_RF7_H
#define CONNECTION_NOVENA_RF7_H

#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <LMS64CProtocol.h>
#include <fstream>
#include <string>
#include <vector>

namespace lime {

class ConnectionNovenaRF7 : public LMS64CProtocol
{
public:
    static const int cSPI_BUF_SIZE;
    static const int cSPI_SPEED_HZ;

    ConnectionNovenaRF7();
    ~ConnectionNovenaRF7();

    DeviceStatus Open(const char *spiDevPath);
    void Close();
    bool IsOpen();

    int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
    int Read(unsigned char *buffer, int length, int timeout_ms = 0);

    eConnectionType GetType(void)
    {
        return SPI_PORT;
    }

    void UpdateExternalBandSelect(const size_t channel, const int trfBand, const int rfePath);

protected:
    std::vector<unsigned char> rxbuf;
    int fd;
    std::fstream m_SEN;
};

class ConnectionNovenaRF7Entry : public ConnectionRegistryEntry
{
public:
    ConnectionNovenaRF7Entry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);
};

}

#endif //CONNECTION_NOVENA_RF7_H
