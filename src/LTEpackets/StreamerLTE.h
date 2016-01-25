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
#include <functional>
#include "dataTypes.h"

namespace lime{

class IConnection;
class LMS_SamplesFIFO;

static const int STATUS_FLAG_RX_END = (1 << 1); //!< An rx stream command completed
static const int STATUS_FLAG_RX_LATE = (1 << 2); //!< An rx stream command had a late time
static const int STATUS_FLAG_TX_LATE = (1 << 3); //!< An tx stream had a late time
static const int STATUS_FLAG_RX_DROP = (1 << 4); //!< An rx packet was dropped, no room

/*!
 * Callback to report the latest state from the RX receiver thread.
 * @param status status flags (see STATUS_FLAG_*)
 * @param counter the most recent timestamp
 */
typedef std::function<void(const int status, const uint64_t &counter)> RxReportFunction;

/*!
 * Callback to retrieve a RxCommand structure.
 * @param [out] command the rx command
 * @return true if the command is valid
 */
typedef std::function<bool(RxCommand &command)> RxPopCommandFunction;

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

    StreamerLTE(IConnection* dataPort);
    virtual ~StreamerLTE();

    virtual STATUS StartStreaming(const int fftSize, const int channelsCount, const StreamDataFormat format);
    virtual STATUS StopStreaming();

    DataToGUI GetIncomingData();
    Stats GetStats();
protected:
    static STATUS Reg_write(IConnection* dataPort, uint16_t address, uint16_t data);
    static uint16_t Reg_read(IConnection* dataPort, uint16_t address);

    std::mutex mLockIncomingPacket;
    DataToGUI mIncomingPacket;

    LMS_SamplesFIFO *mRxFIFO;
    LMS_SamplesFIFO *mTxFIFO;

    static void ReceivePackets(IConnection* dataPort, LMS_SamplesFIFO* rxFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps, const RxReportFunction &report, const RxPopCommandFunction &getCmd);
    static void ReceivePacketsUncompressed(IConnection* dataPort, LMS_SamplesFIFO* rxFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps, const RxReportFunction &report, const RxPopCommandFunction &getCmd);
    static void ProcessPackets(StreamerLTE* pthis, const unsigned int fftSize, const int channelsCount, const StreamDataFormat format);
    static void TransmitPackets(IConnection* dataPort, LMS_SamplesFIFO* txFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps);
    static void TransmitPacketsUncompressed(IConnection* dataPort, LMS_SamplesFIFO* txFIFO, std::atomic<bool>* terminate, std::atomic<uint32_t>* dataRate_Bps);

    std::atomic_bool mStreamRunning;
    std::atomic_bool stopRx;
    std::atomic_bool stopProcessing;
    std::atomic_bool stopTx;

    std::thread threadRx;
    std::thread threadProcessing;
    std::thread threadTx;
    IConnection* mDataPort;

    std::atomic<uint32_t> mRxDataRate;
    std::atomic<uint32_t> mTxDataRate;
};
}
