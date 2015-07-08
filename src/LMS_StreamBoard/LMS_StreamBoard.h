#ifndef STREAM_BOARD_API_H
#define STREAM_BOARD_API_H

#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include "LMS_StreamBoard_FIFO.h"

class LMScomms;

class LMS_StreamBoard
{
public:
    struct SamplesPacket
    {
    public:
        SamplesPacket() : channel(0)
        {}        
        ~SamplesPacket()
        {}

        SamplesPacket& operator=(const SamplesPacket& obj)
        {   
            memcpy(this->iqdata, obj.iqdata, sizeof(float)*samplesCount);
            this->channel = obj.channel;
            return *this;
        }
        const static int samplesCount = 32768;
        float iqdata[samplesCount];
        int channel;
    };


    enum Status
    {
        SUCCESS,
        FAILURE,
    };

    static Status CaptureIQSamples(LMScomms* serPort, int16_t *isamples, int16_t *qsamples, uint32_t framesCount, bool frameStart);
    static Status ConfigurePLL(LMScomms *serPort, const float fOutTx_MHz, const float fOutRx_MHz, const float phaseShift_deg);

    struct DataToGUI
    {
        std::vector<double> samplesI;
        std::vector<double> samplesQ;
        std::vector<double> fftBins_dbFS;
        float nyquist_MHz;
    };

    struct ProgressStats
    {
        float RxRate_Bps;
        float TxRate_Bps;
        float RxFIFOfilled;
        float TxFIFOfilled;
    };

    LMS_StreamBoard(LMScomms* dataPort);
    virtual ~LMS_StreamBoard();

    void SetRxFrameStart(const bool startValue);
    virtual Status StartReceiving(unsigned int fftSize);
    virtual Status StopReceiving();

    DataToGUI GetIncomingData();
    ProgressStats GetStats();    

    Status SPI_write(uint16_t address, uint16_t data);
    uint16_t SPI_read(uint16_t address);
protected:
    static int FindFrameStart(const char* buffer, const int bufLen, const bool frameStart);
    std::mutex mLockIncomingPacket;
    DataToGUI mIncomingPacket;

    LMS_StreamBoard_FIFO<SamplesPacket> *mRxFIFO;
    LMS_StreamBoard_FIFO<SamplesPacket> *mTxFIFO;

    static void ReceivePackets(LMS_StreamBoard* pthis);
    static void ProcessPackets(LMS_StreamBoard* pthis, unsigned int fftSize);
    static void TransmitPackets(LMS_StreamBoard* pthis);

    std::atomic_bool mRxFrameStart;
    std::atomic_bool mStreamRunning;
    std::atomic_bool stopRx;
    std::atomic_bool stopProcessing;
    std::atomic_bool stopTx;

    std::thread threadRx;
    std::thread threadProcessing;
    std::thread threadTx;
    LMScomms* mDataPort;

    std::atomic<float> mRxDataRate;
    std::atomic<float> mTxDataRate;
    std::atomic<float> mRxFIFOfilled;
    std::atomic<float> mTxFIFOfilled;
};

#endif