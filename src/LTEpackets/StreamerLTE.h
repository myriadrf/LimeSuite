/**
@file   StreamerLTE.h
@author Lime Microsystems (limemicro.com)
@brief  Class for receiving and transmitting LTE packets
*/

#include <stdint.h>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <atomic>
#include "dataTypes.h"

class LMScomms;
class LMS_SamplesFIFO;

class StreamerLTE
{
public:
    enum StreamDataFormat
    {
        STREAM_12_BIT_IN_16,
        STREAM_12_BIT_COMPRESSED,
    };

    struct DataToGUI
    {
        std::vector<float> samplesI[2];
        std::vector<float> samplesQ[2];
        std::vector<float> fftBins_dbFS[2];
        float nyquist_MHz;
        float rxDataRate_Bps;
        float txDataRate_Bps;

        DataToGUI& operator=(const DataToGUI& src)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                this->samplesI[ch].clear();
                this->samplesI[ch].reserve(src.samplesI[ch].size());
                this->samplesI[ch] = src.samplesI[ch];
                this->samplesQ[ch].clear();
                this->samplesQ[ch].reserve(src.samplesQ[ch].size());
                this->samplesQ[ch] = src.samplesQ[ch];
                this->fftBins_dbFS[ch].clear();
                this->fftBins_dbFS[ch].reserve(src.fftBins_dbFS[ch].size());
                this->fftBins_dbFS[ch] = src.fftBins_dbFS[ch];
                this->nyquist_MHz = src.nyquist_MHz;
                this->rxDataRate_Bps = src.rxDataRate_Bps;
                this->txDataRate_Bps = src.txDataRate_Bps;
            }
            return *this;
        }
    };

    enum STATUS
    {
        SUCCESS,
        FAILURE,
    };

    struct Stats
    {
        uint32_t rxBufSize;
        uint32_t rxBufFilled;
        uint32_t rxAproxSampleRate;
        uint32_t txBufSize;
        uint32_t txBufFilled;
        uint32_t txAproxSampleRate;
    };

    StreamerLTE(LMScomms* dataPort);
    virtual ~StreamerLTE();

    virtual STATUS StartStreaming(const int fftSize, const int channelsCount, const StreamDataFormat format);
    virtual STATUS StopStreaming();

    DataToGUI GetIncomingData();
    Stats GetStats();    
protected:
    static STATUS SPI_write(LMScomms* dataPort, uint16_t address, uint16_t data);
    static uint16_t SPI_read(LMScomms* dataPort, uint16_t address);

    std::mutex mLockIncomingPacket;
    DataToGUI mIncomingPacket;

    LMS_SamplesFIFO *mRxFIFO;
    LMS_SamplesFIFO *mTxFIFO;

    static void ReceivePackets(LMScomms* dataPort, LMS_SamplesFIFO* rxFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps);
    static void ReceivePacketsUncompressed(LMScomms* dataPort, LMS_SamplesFIFO* rxFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps);
    static void ProcessPackets(StreamerLTE* pthis, const unsigned int fftSize, const int channelsCount, const StreamDataFormat format);
    static void TransmitPackets(LMScomms* dataPort, LMS_SamplesFIFO* txFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps);
    static void TransmitPacketsUncompressed(LMScomms* dataPort, LMS_SamplesFIFO* txFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps);

    std::atomic_bool mStreamRunning;
    std::atomic_bool stopRx;
    std::atomic_bool stopProcessing;
    std::atomic_bool stopTx;

    std::thread threadRx;
    std::thread threadProcessing;
    std::thread threadTx;
    LMScomms* mDataPort;

    std::atomic<int> mRxDataRate;
    std::atomic<int> mTxDataRate;
};
