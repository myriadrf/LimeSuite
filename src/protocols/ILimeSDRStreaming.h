#ifndef ILIMESDRSTREAMING_H
#define ILIMESDRSTREAMING_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

#include "dataTypes.h"
#include "fifo.h"
#include "LMS64CProtocol.h"

namespace lime
{

class ILimeSDRStreaming : public LMS64CProtocol
{
public:

    class Streamer;

    class StreamChannel : public IStreamChannel
    {
    public:
        struct Frame
        {
            uint64_t timestamp;
            static const uint16_t samplesCount = 1360;
            complex16_t samples[samplesCount];
        };
        StreamChannel(Streamer* streamer, StreamConfig config);
        ~StreamChannel();

        int Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms = 100);
        int Write(const void* samples, const uint32_t count, const Metadata* meta, const int32_t timeout_ms = 100);
        StreamChannel::Info GetInfo();

        bool IsActive() const;
        int Start();
        int Stop();
        StreamConfig config;
        Streamer* mStreamer;
        unsigned overflow;
        unsigned underflow;
        unsigned pktLost;
    protected:
        RingFIFO* fifo;
        bool mActive;
        std::atomic<uint64_t> sampleCnt;
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    private:
        StreamChannel() = default;
    };

    class Streamer
    {
    public:
        Streamer(ILimeSDRStreaming* port);
        ~Streamer();

        int SetupStream(size_t& streamID, const StreamConfig& config);
        int CloseStream(const size_t streamID);
        size_t GetStreamSize();

        void EnterSelfCalibration();
        void ExitSelfCalibration();
        uint64_t GetHardwareTimestamp(void);
        void SetHardwareTimestamp(const uint64_t now);
        int UpdateThreads(bool stopAll = false);

        std::atomic<uint32_t> rxDataRate_Bps;
        std::atomic<uint32_t> txDataRate_Bps;
        ILimeSDRStreaming* dataPort;
        std::thread rxThread;
        std::thread txThread;
        std::atomic<bool> rxRunning;
        std::atomic<bool> txRunning;
        std::atomic<bool> terminateRx;
        std::atomic<bool> terminateTx;

        std::vector<StreamChannel*> mRxStreams;
        std::vector<StreamChannel*> mTxStreams;
        std::atomic<uint64_t> rxLastTimestamp;
        std::atomic<uint64_t> txLastLateTime;
        uint64_t mTimestampOffset;
        int mChipID;
        unsigned txBatchSize;
        unsigned rxBatchSize;
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

    int UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex) override;

protected:
    virtual int ReceiveData(char* buffer, int length, int epIndex, int timeout = 100);
    virtual int SendData(const char* buffer, int length, int epIndex, int timeout = 100);
    virtual void ReceivePacketsLoop(Streamer* args) = 0;
    virtual void TransmitPacketsLoop(Streamer* args) = 0;
    std::vector<Streamer*> mStreamers;
    std::condition_variable safeToConfigInterface;
    double mExpectedSampleRate; //rate used for generating data

    std::function<void(Streamer* args)> RxLoopFunction;
    std::function<void(Streamer* args)> TxLoopFunction;

    virtual int ResetStreamBuffers(){return 0;};
};

} //lime
#endif
