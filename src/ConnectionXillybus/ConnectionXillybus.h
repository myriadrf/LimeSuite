/**
    @file ConnectionSTREAM.h
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <LMS64CProtocol.h>
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include "fifo.h"

#ifndef __unix__
#include "windows.h"
#else
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

struct USBStreamService;

namespace lime{

class ConnectionXillybus : public LMS64CProtocol
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
        ConnectionXillybus* port;
        bool mActive;
    private:
        StreamChannel() = default;
    };

    ConnectionXillybus(const unsigned index);
    ~ConnectionXillybus(void);

	int Open(const unsigned index);
	void Close();
	bool IsOpen();
	int GetOpenedIndex();

	int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
	int Read(unsigned char *buffer, int length, int timeout_ms = 0);

	uint64_t GetHardwareTimestamp(void);
	void SetHardwareTimestamp(const uint64_t now);
	double GetHardwareTimestampRate(void);

	//IConnection stream API implementation
    int SetupStream(size_t& streamID, const StreamConfig& config);
    int CloseStream(const size_t streamID);
    size_t GetStreamSize(const size_t streamID);
    int ControlStream(const size_t streamID, const bool enable);
    int ReadStream(const size_t streamID, void* buffs, const size_t length, const long timeout_ms, StreamMetadata& metadata);
    int WriteStream(const size_t streamID, const void* buffs, const size_t length, const long timeout_ms, const StreamMetadata& metadata);
    int ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata& metadata);

	//hooks to update FPGA plls when baseband interface data rate is changed
	int UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate);
	void EnterSelfCalibration(const size_t channel);
	void ExitSelfCalibration(const size_t channel);
protected:
    struct ThreadData
    {
        ConnectionXillybus* dataPort; //! Connection interface
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

    virtual int BeginDataReading(char* buffer, uint32_t length);
    virtual int WaitForReading(int contextHandle, unsigned int timeout_ms);
    virtual int FinishDataReading(char* buffer, uint32_t length, int contextHandle);
    virtual void AbortReading();

    virtual int BeginDataSending(const char* buffer, uint32_t length);
    virtual int WaitForSending(int contextHandle, uint32_t timeout_ms);
    virtual int FinishDataSending(const char* buffer, uint32_t length, int contextHandle);
    virtual void AbortSending();

	std::thread rxThread;
    std::thread txThread;
    std::atomic<bool> rxRunning;
    std::atomic<bool> txRunning;
    std::atomic<bool> terminateRx;
    std::atomic<bool> terminateTx;
    std::mutex streamStateLock;
    std::condition_variable safeToConfigInterface;
    std::atomic<bool> generateData;
    float mExpectedSampleRate; //rate used for generating data
    std::atomic<uint32_t> rxDataRate_Bps;
    std::atomic<uint32_t> txDataRate_Bps;
    std::vector<StreamChannel*> mRxStreams;
    std::vector<StreamChannel*> mTxStreams;
    std::atomic<uint64_t> rxLastTimestamp;
    uint64_t mTimestampOffset;

    int ConfigureFPGA_PLL(unsigned int pllIndex, const double interfaceClk_Hz, const double phaseShift_deg);
private:

    eConnectionType GetType(void)
    {
        return USB_PORT;
    }

    std::string m_hardwareName;
    int m_hardwareVer;

    bool isConnected;

    //! Stream service used by the stream and time API
    std::shared_ptr<USBStreamService> mStreamService;

#ifndef __unix__
    HANDLE hWrite;
    HANDLE hRead;
    HANDLE hWriteStream;
    HANDLE hReadStream;
    std::string writeStreamPort;
    std::string readStreamPort;
#else
    int hWrite;
    int hRead;
    int hWriteStream;
    int hReadStream;
    std::string writeStreamPort;
    std::string readStreamPort;
#endif
};



class ConnectionXillybusEntry : public ConnectionRegistryEntry
{
public:
    ConnectionXillybusEntry(void);

    ~ConnectionXillybusEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
    #ifndef __unix__
    std::string DeviceName(unsigned int index);
    #else
    #endif
};

}
