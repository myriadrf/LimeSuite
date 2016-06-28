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
class LMS64CProtocol;

static const int STATUS_FLAG_TIME_UP = (1 << 0); //!< Update the internal timestamp
static const int STATUS_FLAG_RX_END = (1 << 1); //!< An rx stream command completed
static const int STATUS_FLAG_RX_LATE = (1 << 2); //!< An rx stream command had a late time
static const int STATUS_FLAG_TX_LATE = (1 << 3); //!< An tx stream had a late time
static const int STATUS_FLAG_RX_DROP = (1 << 4); //!< An rx packet was dropped, no room
static const int STATUS_FLAG_TX_TIME = (1 << 5); //!< The tx packet has a timestamp
static const int STATUS_FLAG_TX_END = (1 << 6); //!< The tx packet ends a burst (flush)

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

/*!
 * Receiver and transmitter thread arguments.
 */
struct StreamerLTE_ThreadData
{
    IConnection* dataPort; //!< Connection interface
    LMS_SamplesFIFO* FIFO; //!< FIFO (tx pops, rx pushes)
    std::atomic<bool>* terminate; //!< true exit loop
    std::atomic<uint32_t>* dataRate_Bps; //!< report rate
    RxReportFunction report; //!< status report out
    RxPopCommandFunction getCmd; //!< external commands in
};

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
        float nyquist_Hz;
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
                this->nyquist_Hz = src.nyquist_Hz;
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
    void SetCaptureToFile(bool enable, const char* filename, uint32_t samplesCount);
    void SetWidowFunction(int func);
protected:
    static STATUS Reg_write(IConnection* dataPort, uint16_t address, uint16_t data);
    static uint16_t Reg_read(IConnection* dataPort, uint16_t address);

    std::mutex mLockIncomingPacket;
    DataToGUI mIncomingPacket;

    LMS_SamplesFIFO *mRxFIFO;
    LMS_SamplesFIFO *mTxFIFO;

    static void ResetUSBFIFO(LMS64CProtocol* port);
    static void ReceivePackets(const StreamerLTE_ThreadData &args);
    static void ReceivePacketsUncompressed(const StreamerLTE_ThreadData &args);
    static void ProcessPackets(StreamerLTE* pthis, const unsigned int fftSize, const int channelsCount, const StreamDataFormat format);
    static void TransmitPackets(const StreamerLTE_ThreadData &args);
    static void TransmitPacketsUncompressed(const StreamerLTE_ThreadData &args);

    std::atomic<bool> mStreamRunning;
    std::atomic<bool> stopRx;
    std::atomic<bool> stopProcessing;
    std::atomic<bool> stopTx;

    std::thread threadRx;
    std::thread threadProcessing;
    std::thread threadTx;
    IConnection* mDataPort;

    std::atomic<uint32_t> mRxDataRate;
    std::atomic<uint32_t> mTxDataRate;

    bool captureToFile;
    uint32_t samplesToCapture;
    std::string captureFilename;
    int windowFunction;
};
}
