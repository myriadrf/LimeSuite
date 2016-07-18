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
#include <fifo.h>

namespace lime {

class ConnectionNovenaRF7 : public LMS64CProtocol
{
public:
    class StreamChannel : public IStreamChannel
    {
    public:
        StreamChannel(IConnection* port);
        ~StreamChannel();

        int Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms = 100);
        int Write(const void* samples, const uint32_t count, const Metadata* meta, const int32_t timeout_ms = 100);
        IStreamChannel::Info GetInfo();

        bool IsActive() const;
        int Start();
        int Stop();
        StreamConfig config;
    protected:
        RingFIFO* fifo;
        ConnectionNovenaRF7* port;
        bool mActive;
    private:
        StreamChannel() = default;
    };

    static const int cSPI_BUF_SIZE;
    static const int cSPI_SPEED_HZ;

    ConnectionNovenaRF7();
    ~ConnectionNovenaRF7();

    int Open(const char *spiDevPath);
    void Close();
    bool IsOpen();

    int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
    int Read(unsigned char *buffer, int length, int timeout_ms = 0);

    eConnectionType GetType(void)
    {
        return SPI_PORT;
    }

    void UpdateExternalBandSelect(const size_t channel, const int trfBand, const int rfePath);

    //IConnection stream API implementation
    int SetupStream(size_t& streamID, const StreamConfig& config);
    int CloseStream(const size_t streamID);
    size_t GetStreamSize(const size_t streamID);
    int ControlStream(const size_t streamID, const bool enable);
    int ReadStream(const size_t streamID, void* buffs, const size_t length, const long timeout_ms, StreamMetadata& metadata);
    int WriteStream(const size_t streamID, const void* buffs, const size_t length, const long timeout_ms, const StreamMetadata& metadata);
    int ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata& metadata);

protected:
    struct ThreadData
    {
        ConnectionNovenaRF7* dataPort; //! Connection interface
        std::atomic<bool>* terminate; //! true exit loop
        std::atomic<uint32_t>* dataRate_Bps; //! report rate
        std::vector<StreamChannel*> channels; //! channels FIFOs
        std::atomic<bool>* generateData; //! generate data
        std::condition_variable* safeToConfigInterface;
        std::atomic<uint64_t>* lastTimestamp; //! report latest timestamp
    };
    static void ReceivePacketsLoop(const ThreadData args);
    static void TransmitPacketsLoop(const ThreadData args);
    int UpdateThreads();

    std::thread rxThread;
    std::atomic<bool> rxRunning;
    std::atomic<bool> terminateRx;
    std::atomic<uint32_t> rxDataRate_Bps;
    std::vector<StreamChannel*> mRxStreams;

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
