#ifndef ILIMESDRSTREAMING_H
#define ILIMESDRSTREAMING_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "dataTypes.h"
#include "fifo.h"
#include "LMS64CProtocol.h"

namespace lime
{

class ILimeSDRStreaming : public LMS64CProtocol
{
public:
    class StreamChannel : public IStreamChannel
    {
    public:
        struct Frame
        {
            uint64_t timestamp;
            static const uint16_t samplesCount = 1360;
            complex16_t samples[samplesCount];
        };
        StreamChannel(IConnection* port, StreamConfig config);
        ~StreamChannel();

        int Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms = 100);
        int Write(const void* samples, const uint32_t count, const Metadata* meta, const int32_t timeout_ms = 100);
        StreamChannel::Info GetInfo();

        bool IsActive() const;
        int Start();
        int Stop();
        StreamConfig config;
        std::atomic<uint64_t> txLastLateTime;
    protected:
        RingFIFO* fifo;
        ILimeSDRStreaming* port;
        bool mActive;
    private:
        StreamChannel() = default;
    };

    ILimeSDRStreaming();
    virtual ~ILimeSDRStreaming();

    virtual int SetupStream(size_t& streamID, const StreamConfig& config);
    virtual int CloseStream(const size_t streamID);
    virtual size_t GetStreamSize(const size_t streamID);
    virtual int ControlStream(const size_t streamID, const bool enable);
    virtual int ReadStream(const size_t streamID, void* buffs, const size_t length, const long timeout_ms, StreamMetadata& metadata);
    virtual int WriteStream(const size_t streamID, const void* buffs, const size_t length, const long timeout_ms, const StreamMetadata& metadata);
    virtual int ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata& metadata);

    virtual int UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz) = 0;
    virtual void EnterSelfCalibration(const size_t channel);
    virtual void ExitSelfCalibration(const size_t channel);
    virtual uint64_t GetHardwareTimestamp(void);
    virtual void SetHardwareTimestamp(const uint64_t now);
    virtual double GetHardwareTimestampRate(void);

    struct ThreadData
    {
        ILimeSDRStreaming* dataPort; //! Connection interface
        std::atomic<bool>* terminate; //! true exit loop
        std::atomic<uint32_t>* dataRate_Bps; //! report rate
        std::vector<StreamChannel*> channels; //! channels FIFOs
        std::atomic<bool>* generateData; //! generate data
        std::condition_variable* safeToConfigInterface;
        std::atomic<uint64_t>* lastTimestamp; //! report latest timestamp
        std::function<void(const uint64_t)> reportLateTx; //! report late tx packet
    };
    virtual void ReceivePacketsLoop(const ThreadData args) = 0;
    virtual void TransmitPacketsLoop(const ThreadData args) = 0;
    virtual int UpdateThreads(bool stopAll = false);

    StreamConfig config;
    std::thread rxThread;
    std::thread txThread;
    std::atomic<bool> rxRunning;
    std::atomic<bool> txRunning;
    std::atomic<bool> terminateRx;
    std::atomic<bool> terminateTx;
    std::mutex streamStateLock;
    std::condition_variable safeToConfigInterface;
    std::atomic<bool> generateData;
    double mExpectedSampleRate; //rate used for generating data
    std::atomic<uint32_t> rxDataRate_Bps;
    std::atomic<uint32_t> txDataRate_Bps;
    std::vector<StreamChannel*> mRxStreams;
    std::vector<StreamChannel*> mTxStreams;
    std::atomic<uint64_t> rxLastTimestamp;
    uint64_t mTimestampOffset;

    std::function<void(const ThreadData args)> RxLoopFunction;
    std::function<void(const ThreadData args)> TxLoopFunction;

    std::atomic<uint64_t> txLastLateTime;
    void reportLateTxTimestamp(const uint64_t timestamp)
    {
        for (auto stream : mTxStreams)
        {
            stream->txLastLateTime = timestamp;
        }
    }
};

} //lime
#endif
