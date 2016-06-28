#ifndef STREAM_BOARD_API_H
#define STREAM_BOARD_API_H

#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include "LMS_StreamBoard_FIFO.h"

namespace lime{
class IConnection;

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

    struct DataToGUI
    {
        std::vector<float> samplesI;
        std::vector<float> samplesQ;
        std::vector<float> fftBins_dbFS;
        float nyquist_Hz;
    };

    struct ProgressStats
    {
        float RxRate_Bps;
        float TxRate_Bps;
        float RxFIFOfilled;
        float TxFIFOfilled;
    };

    LMS_StreamBoard(IConnection* dataPort);
    virtual ~LMS_StreamBoard();

    void SetRxFrameStart(const bool startValue);
    virtual Status StartReceiving(unsigned int fftSize);
    virtual Status StopReceiving();

	virtual Status StartCyclicTransmitting(const int16_t* isamples, const int16_t* qsamples, uint32_t framesCount);
    virtual Status StopCyclicTransmitting();

    DataToGUI GetIncomingData();
    ProgressStats GetStats();

    Status Reg_write(uint16_t address, uint16_t data);
    uint16_t Reg_read(uint16_t address);

    void SetCaptureToFile(bool enable, const char* filename, uint32_t samplesCount);
    void SetWidowFunction(int func);
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
    IConnection* mDataPort;

    std::atomic<unsigned long> mRxDataRate;
    std::atomic<unsigned long> mTxDataRate;
    std::atomic<int> mRxFIFOfilled;
    std::atomic<int> mTxFIFOfilled;

    std::vector<int16_t> mCyclicTransmittingSourceData;
    std::atomic_bool mTxCyclicRunning;
    std::thread threadTxCyclic;
    std::atomic_bool stopTxCyclic;

    bool captureToFile;
    uint32_t samplesToCapture;
    std::string captureFilename;
    int windowFunction;
};
}
#endif

