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
#include "Streamer.h"

namespace lime
{

class ILimeSDRStreaming : public LMS64CProtocol
{
public:

    ILimeSDRStreaming();
    virtual ~ILimeSDRStreaming();

    StreamChannel* SetupStream(const StreamConfig& config) override;

    virtual int UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz) = 0;
    virtual uint64_t GetHardwareTimestamp(void);
    virtual void SetHardwareTimestamp(const uint64_t now);

    int UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex) override;
    virtual int ResetStreamBuffers(){return 0;};
    std::vector<Streamer*> mStreamers;

    virtual int GetBuffersCount() const = 0;
    virtual int CheckStreamSize(int size) const = 0;
    virtual int ReceiveData(char* buffer, int length, int epIndex, int timeout = 100);
    virtual int SendData(const char* buffer, int length, int epIndex, int timeout = 100);
   
    virtual int BeginDataSending(const char* buffer, uint32_t length, int ep)=0;
    virtual int WaitForSending(int contextHandle, uint32_t timeout_ms) =0;
    virtual int FinishDataSending(const char* buffer, uint32_t length, int contextHandle) =0;
    virtual void AbortSending(int ep)=0;
    
    virtual int BeginDataReading(char* buffer, uint32_t length, int ep)=0;
    virtual int WaitForReading(int contextHandle, unsigned int timeout_ms)=0;
    virtual int FinishDataReading(char* buffer, uint32_t length, int contextHandle)=0;
    virtual void AbortReading(int ep)=0;;
};

} //lime
#endif
