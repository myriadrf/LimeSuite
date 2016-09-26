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
#include "CyAPI.h"
#else
#include <libusb-1.0/libusb.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

namespace lime
{

#define USB_MAX_CONTEXTS 64 //maximum number of contexts for asynchronous transfers

/** @brief Wrapper class for holding USB asynchronous transfers contexts
*/
class USBTransferContext
{
public:
    USBTransferContext() : used(false)
    {
        id = idCounter++;
#ifndef __unix__
        inOvLap = new OVERLAPPED;
        memset(inOvLap, 0, sizeof(OVERLAPPED));
        inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
        context = NULL;
#else
        transfer = libusb_alloc_transfer(0);
        bytesXfered = 0;
        bytesExpected = 0;
        done = 0;
#endif
    }
    ~USBTransferContext()
    {
#ifndef __unix__
        CloseHandle(inOvLap->hEvent);
        delete inOvLap;
#else
        libusb_free_transfer(transfer);
#endif
    }
    bool reset()
    {
        if(used)
            return false;
#ifndef __unix__
        CloseHandle(inOvLap->hEvent);
        memset(inOvLap, 0, sizeof(OVERLAPPED));
        inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
#endif
        return true;
    }
    bool used;
    int id;
    static int idCounter;
#ifndef __unix__
    PUCHAR context;
    OVERLAPPED* inOvLap;
#else
    libusb_transfer* transfer;
    long bytesXfered;
    long bytesExpected;
    std::atomic<bool> done;
    std::mutex transferLock;
    std::condition_variable cv;
#endif
};

class ConnectionSTREAM : public LMS64CProtocol
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
        ConnectionSTREAM* port;
        bool mActive;
    private:
        StreamChannel() = default;
    };

    ConnectionSTREAM(void* ctx, const unsigned index, const int vid=-1, const int pid=-1);
    ~ConnectionSTREAM(void);

    int Open(const unsigned index, const int vid, const int pid);
    void Close();
    bool IsOpen();
    int GetOpenedIndex();

    virtual int Write(const unsigned char* buffer, int length, int timeout_ms = 0);
    virtual int Read(unsigned char* buffer, int length, int timeout_ms = 0);

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
    int UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format) override;

    //hooks to update FPGA plls when baseband interface data rate is changed
    int UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate);
    void EnterSelfCalibration(const size_t channel);
    void ExitSelfCalibration(const size_t channel);

protected:
    struct ThreadData
    {
        ConnectionSTREAM* dataPort; //! Connection interface
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

    eConnectionType GetType(void) {return USB_PORT;}

    USBTransferContext contexts[USB_MAX_CONTEXTS];
    USBTransferContext contextsToSend[USB_MAX_CONTEXTS];

    bool isConnected;

#ifndef __unix__
    CCyUSBDevice* USBDevicePrimary;
    //control endpoints
    CCyControlEndPoint* InCtrlEndPt3;
    CCyControlEndPoint* OutCtrlEndPt3;

    //end points for samples reading and writing
    CCyUSBEndPoint* InEndPt;
    CCyUSBEndPoint* OutEndPt;

#else
    libusb_device** devs; //pointer to pointer of device, used to retrieve a list of devices
    libusb_device_handle* dev_handle; //a device handle
    libusb_context* ctx; //a libusb session
#endif

    std::mutex mExtraUsbMutex;
};

class ConnectionSTREAMEntry : public ConnectionRegistryEntry
{
public:
    ConnectionSTREAMEntry(void);
    ConnectionSTREAMEntry(const std::string entryName);
    virtual ~ConnectionSTREAMEntry(void);
    virtual std::vector<ConnectionHandle> enumerate(const ConnectionHandle& hint);
    virtual IConnection* make(const ConnectionHandle& handle);
protected:
#ifndef __unix__
    std::string DeviceName(unsigned int index);
    CCyUSBDevice* USBDevicePrimary;
#else
    libusb_context* ctx; //a libusb session
    std::thread mUSBProcessingThread;
    void handle_libusb_events();
    std::atomic<bool> mProcessUSBEvents;
#endif
};

}
