/**
@file   StreamerLTE.h
@author Lime Microsystems (limemicro.com)
@brief  Class for receiving and transmitting LTE packets
*/

#include <stdint.h>
#include <thread>
#include <atomic>
#include <vector>
#include "StreamerFIFO.h"
class LMScomms;

struct PacketLTE
{
    uint8_t reserved[8];
    uint64_t counter;
    int16_t samples[2040];
};

class StreamerLTE
{
public:
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

    enum STATUS
    {
        SUCCESS,
        FAILURE,
    };

    StreamerLTE(LMScomms* dataPort);
    virtual ~StreamerLTE();

    virtual STATUS StartStreaming(unsigned int fftSize);
    virtual STATUS StopStreaming();

    DataToGUI GetIncomingData();
    ProgressStats GetStats();
    unsigned long mTransmitDelay;

    STATUS SPI_write(uint16_t address, uint16_t data);
    uint16_t SPI_read(uint16_t address);
protected:
    std::mutex mLockIncomingPacket;
    DataToGUI mIncomingPacket;

    StreamerFIFO<PacketLTE> *mRxFIFO;
    StreamerFIFO<PacketLTE> *mTxFIFO;

    static void ReceivePackets(StreamerLTE* pthis);
    static void ProcessPackets(StreamerLTE* pthis, unsigned int fftSize);
    static void TransmitPackets(StreamerLTE* pthis);

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
    std::atomic<int> mRxFIFOfilled;
    std::atomic<int> mTxFIFOfilled;
};
