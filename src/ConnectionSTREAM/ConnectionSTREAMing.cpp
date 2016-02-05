/**
    @file ConnectionSTREAMing.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection (streaming API)
*/

#include "ConnectionSTREAM.h"
#include "LMS_StreamBoard.h"
#include "StreamerLTE.h"
#include "fifo.h" //from StreamerLTE
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>

using namespace lime;

#define LTE_CHAN_COUNT 2
#define STREAM_MTU (PacketFrame::maxSamplesInPacket/(LTE_CHAN_COUNT))

/***********************************************************************
 * Custom StreamerLTE for streaming API hooks
 **********************************************************************/

struct USBStreamService : StreamerLTE
{
    USBStreamService(
        LMS64CProtocol *dataPort,
        const size_t channelsCount = LTE_CHAN_COUNT,
        const StreamDataFormat format = STREAM_12_BIT_COMPRESSED
    ):
        StreamerLTE(dataPort),
        format(format),
        rxStreamUseCount(1), //always need rx for status reporting
        txStreamUseCount(0),
        mTxThread(nullptr),
        mRxThread(nullptr),
        rxStreamingContinuous(false),
        txTimeEnabled(false),
        mLastRxTimestamp(0),
        mTimestampOffset(0),
        mHwCounterRate(0.0)
    {
        mRxFIFO->Reset(2*4096, channelsCount);
        mTxFIFO->Reset(2*4096, channelsCount);

        //switch off Rx
        uint16_t regVal = Reg_read(dataPort, 0x0005);
        Reg_write(dataPort, 0x0005, regVal & ~0x6);

        //enable MIMO mode, 12 bit compressed values
        if (channelsCount == 2)
        {
            Reg_write(dataPort, 0x0001, 0x0003);
            Reg_write(dataPort, 0x0007, 0x000A);
        }
        else
        {
            Reg_write(dataPort, 0x0001, 0x0001);
            Reg_write(dataPort, 0x0007, 0x0008);
        }

        //USB FIFO reset
        //TODO direction specific reset using isTx
        LMS64CProtocol::GenericPacket ctrPkt;
        ctrPkt.cmd = CMD_USB_FIFO_RST;
        ctrPkt.outBuffer.push_back(0x01);
        dataPort->TransferPacket(ctrPkt);
        ctrPkt.outBuffer[0] = 0x00;
        dataPort->TransferPacket(ctrPkt);
    }

    ~USBStreamService(void)
    {
        this->updateThreadState(true);
    }

    void updateThreadState(const bool forceStop = false)
    {
        auto reporter = std::bind(&USBStreamService::handleRxStatus, this, std::placeholders::_1, std::placeholders::_2);
        auto getRxCmd = std::bind(&ConcurrentQueue<RxCommand>::try_pop, &mRxCmdQueue, std::placeholders::_1);

        stopRx = false;
        stopTx = false;

        StreamerLTE_ThreadData threadRxArgs;
        threadRxArgs.dataPort = mDataPort;
        threadRxArgs.FIFO = mRxFIFO;
        threadRxArgs.terminate = &stopRx;
        threadRxArgs.dataRate_Bps = &mRxDataRate;
        threadRxArgs.report = reporter;
        threadRxArgs.getCmd = getRxCmd;

        StreamerLTE_ThreadData threadTxArgs;
        threadTxArgs.dataPort = mDataPort;
        threadTxArgs.FIFO = mTxFIFO;
        threadTxArgs.terminate = &stopTx;
        threadTxArgs.dataRate_Bps = &mTxDataRate;

        if (mRxThread == nullptr and rxStreamUseCount != 0 and not forceStop)
        {
            //restore stream state continuous
            if (rxStreamingContinuous)
            {
                RxCommand rxCmd;
                rxCmd.waitForTimestamp = false;
                rxCmd.timestamp = 0;
                rxCmd.finiteRead = false;
                rxCmd.numSamps = 0;
                mRxCmdQueue.push(rxCmd);
            }

            if (format == STREAM_12_BIT_COMPRESSED)
            {
                mRxThread = new std::thread(ReceivePackets, threadRxArgs);
            }
            else
            {
                mRxThread = new std::thread(ReceivePacketsUncompressed, threadRxArgs);
            }
        }

        if (mTxThread == nullptr and txStreamUseCount != 0 and not forceStop)
        {
            if (format == STREAM_12_BIT_COMPRESSED)
            {
                mTxThread = new std::thread(TransmitPackets, threadTxArgs);
            }
            else
            {
                mTxThread = new std::thread(TransmitPacketsUncompressed, threadTxArgs);
            }
        }

        if ((forceStop or txStreamUseCount == 0) and mTxThread != nullptr)
        {
            stopTx = true;
            mTxThread->join();
            delete mTxThread;
            mTxThread = nullptr;
        }

        if ((forceStop or rxStreamUseCount == 0) and mRxThread != nullptr)
        {
            stopRx = true;
            mRxThread->join();
            delete mRxThread;
            mRxThread = nullptr;
        }
    }

    void start(void)
    {
        if (mHwCounterRate == 0.0) return; //not configured

        //clear any residual data from FIFO
        ResetUSBFIFO(dynamic_cast<LMS64CProtocol *>(mDataPort));

        //switch on Rx
        auto regVal = Reg_read(mDataPort, 0x0005);
        int syncDis = 1 << 5; //disabled
        int rxTxEnb = 1 << 2; //streaming on
        int txFromRam = 0 << 1; //off
        int txNormal = 1 << 0; //normal op
        Reg_write(mDataPort, 0x0005, (regVal & ~0x27) | syncDis | rxTxEnb | txFromRam | txNormal);

        this->updateThreadState();
    }

    void stop(void)
    {
        //stop threads first, then disable FPGA
        this->updateThreadState(true);
        //stop Tx Rx if they were active
        uint32_t regVal = Reg_read(mDataPort, 0x0005);
        Reg_write(mDataPort, 0x0005, regVal & ~(1 << 2));
    }

    void handleRxStatus(const int status, const uint64_t &counter)
    {
        if (status == STATUS_FLAG_TIME_UP)
        {
            if (counter < mLastRxTimestamp) std::cerr << "C";
            mLastRxTimestamp = counter;
            return;
        }

        StreamMetadata metadata;
        metadata.hasTimestamp = true;
        metadata.timestamp = counter + mTimestampOffset;
        const bool isTx = ((status & STATUS_FLAG_TX_LATE) != 0);
        metadata.endOfBurst = (status & STATUS_FLAG_RX_END) != 0;
        metadata.lateTimestamp = (status & STATUS_FLAG_RX_LATE) != 0;
        metadata.lateTimestamp = (status & STATUS_FLAG_TX_LATE) != 0;
        metadata.packetDropped = (status & STATUS_FLAG_RX_DROP) != 0;

        if (isTx) mTxStatQueue.push(metadata);
        else mRxStatQueue.push(metadata);
    }

    LMS_SamplesFIFO *GetRxFIFO(void) const
    {
        return mRxFIFO;
    }

    LMS_SamplesFIFO *GetTxFIFO(void) const
    {
        return mTxFIFO;
    }

    const StreamDataFormat format;
    std::atomic<int> rxStreamUseCount;
    std::atomic<int> txStreamUseCount;
    std::thread *mTxThread;
    std::thread *mRxThread;
    //! was the last rx command for continuous streaming?
    //! this lets us restore streaming after calibration
    std::atomic<bool> rxStreamingContinuous;
    std::atomic<bool> txTimeEnabled;
    std::atomic<uint64_t> mLastRxTimestamp;
    std::atomic<int64_t> mTimestampOffset;
    std::atomic<double> mHwCounterRate;
    ConcurrentQueue<RxCommand> mRxCmdQueue;
    ConcurrentQueue<StreamMetadata> mRxStatQueue;
    ConcurrentQueue<StreamMetadata> mTxStatQueue;
};

struct USBStreamServiceChannel
{
    USBStreamServiceChannel(bool isTx, const size_t channelsCount, const bool convertFloat):
        isTx(isTx),
        channelsCount(channelsCount),
        convertFloat(convertFloat)
    {
        for (size_t i = 0; i < channelsCount; i++)
        {
            FIFOBuffers.push_back(new complex16_t[STREAM_MTU]);
        }
    }

    ~USBStreamServiceChannel(void)
    {
        for (size_t i = 0; i < channelsCount; i++)
        {
            delete [] FIFOBuffers[i];
        }
        FIFOBuffers.clear();
    }

    const bool isTx;
    const size_t channelsCount;
    const bool convertFloat;
    std::vector<complex16_t *> FIFOBuffers;
};

/***********************************************************************
 * Streaming API implementation
 **********************************************************************/

std::string ConnectionSTREAM::SetupStream(size_t &streamID, const StreamConfig &config)
{
    streamID = ~0;

    //API format check
    bool convertFloat = false;
    if (config.format == StreamConfig::STREAM_COMPLEX_FLOAT32) convertFloat = true;
    else if (config.format == StreamConfig::STREAM_12_BIT_IN_16) convertFloat = false;
    else return "ConnectionSTREAM::setupStream() only complex floats or int16";

    //check channel config
    //provide a default channel 0 if none specified
    std::vector<size_t> channels(config.channels);
    if (channels.empty()) channels.push_back(0);
    if (channels.size() > 2) return "SoapyIConnection::setupStream() 2 channels max";
    if (channels.front() > 1) return "SoapyIConnection::setupStream() channels must be [0, 1]";
    if (channels.back() > 1) return "SoapyIConnection::setupStream() channels must be [0, 1]";
    bool pos0isA = channels.front() == 0;
    bool pos1isA = channels.back() == 0;

    //determine sample positions based on channels
    auto s3 = pos0isA?LMS7002M::AQ:LMS7002M::BQ;
    auto s2 = pos0isA?LMS7002M::AI:LMS7002M::BI;
    auto s1 = pos1isA?LMS7002M::AQ:LMS7002M::BQ;
    auto s0 = pos1isA?LMS7002M::AI:LMS7002M::BI;

    //Note: only when FPGA is also in 1-ch mode
    //if (channels.size() == 1) s0 = s3;

    //configure LML based on channel config
    LMS7002M rfic;
    rfic.SetConnection(this);
    if (config.isTx) rfic.ConfigureLML_BB2RF(s0, s1, s2, s3);
    else             rfic.ConfigureLML_RF2BB(s0, s1, s2, s3);

    if (config.isTx) mStreamService->txStreamUseCount++;
    if (!config.isTx) mStreamService->rxStreamUseCount++;
    mStreamService->updateThreadState();

    streamID = size_t(new USBStreamServiceChannel(config.isTx, channels.size(), convertFloat));
    return ""; //success
}

void ConnectionSTREAM::CloseStream(const size_t streamID)
{
    auto *stream = (USBStreamServiceChannel *)streamID;
    if (stream->isTx) mStreamService->txStreamUseCount--;
    if (!stream->isTx) mStreamService->rxStreamUseCount--;
    delete stream;
}

size_t ConnectionSTREAM::GetStreamSize(const size_t streamID)
{
    return STREAM_MTU;
}

bool ConnectionSTREAM::ControlStream(const size_t streamID, const bool enable, const size_t burstSize, const StreamMetadata &metadata)
{
    auto *stream = (USBStreamServiceChannel *)streamID;

    if (!stream->isTx)
    {
        RxCommand rxCmd;
        rxCmd.waitForTimestamp = metadata.hasTimestamp;
        rxCmd.timestamp = metadata.timestamp-mStreamService->mTimestampOffset;
        rxCmd.finiteRead = metadata.endOfBurst;
        rxCmd.numSamps = burstSize;
        mStreamService->mRxCmdQueue.push(rxCmd);
        mStreamService->rxStreamingContinuous = not metadata.endOfBurst;
    }

    return true;
}

int ConnectionSTREAM::ReadStream(const size_t streamID, void * const *buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata)
{
    auto *stream = (USBStreamServiceChannel *)streamID;

    size_t samplesCount = std::min<size_t>(length, STREAM_MTU);

    complex16_t **popBuffer(nullptr);
    if (stream->convertFloat) popBuffer = (complex16_t **)stream->FIFOBuffers.data();
    else popBuffer = (complex16_t **)buffs;

    uint32_t fifoFlags = 0;
    size_t sampsPopped = mStreamService->GetRxFIFO()->pop_samples(
        popBuffer,
        samplesCount,
        stream->channelsCount,
        &metadata.timestamp,
        timeout_ms,
        &fifoFlags);
    metadata.hasTimestamp = metadata.timestamp != 0;
    metadata.timestamp += mStreamService->mTimestampOffset;
    metadata.endOfBurst = (fifoFlags & STATUS_FLAG_RX_END) != 0;
    samplesCount = (std::min)(samplesCount, sampsPopped);

    if (stream->convertFloat)
    {
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto buffIn = stream->FIFOBuffers[i];
            auto buffOut = (std::complex<float> *)buffs[i];
            for (size_t j = 0; j < samplesCount; j++)
            {
                buffOut[j] = std::complex<float>(
                    float(buffIn[j].i)/2048,
                    float(buffIn[j].q)/2048);
            }
        }
    }

    return samplesCount;
}

int ConnectionSTREAM::WriteStream(const size_t streamID, const void * const *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata)
{
    auto *stream = (USBStreamServiceChannel *)streamID;

    //set the time enabled register if usage changed
    //TODO maybe the FPGA should check a flag in the pkt
    if (mStreamService->txTimeEnabled != metadata.hasTimestamp)
    {
        uint32_t regVal; this->ReadRegister(0x0005, regVal);
        if (metadata.hasTimestamp) regVal &= ~(1 << 5);
        else                       regVal |= (1 << 5);
        this->WriteRegister(0x0005, regVal);
        mStreamService->txTimeEnabled = metadata.hasTimestamp;
    }

    size_t samplesCount = std::min<size_t>(length, STREAM_MTU);

    const complex16_t **pushBuffer(nullptr);
    if (stream->convertFloat)
    {
        pushBuffer = (const complex16_t **)stream->FIFOBuffers.data();
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto buffIn = (const std::complex<float> *)buffs[i];
            auto bufOut = stream->FIFOBuffers[i];
            for (size_t j = 0; j < samplesCount; j++)
            {
                bufOut[j].i = int16_t(buffIn[j].real()*2048);
                bufOut[j].q = int16_t(buffIn[j].imag()*2048);
            }
        }
    }
    else pushBuffer = (const complex16_t **)buffs;

    auto ticks = metadata.timestamp - mStreamService->mTimestampOffset;
    size_t sampsPushed = mStreamService->GetTxFIFO()->push_samples(
        pushBuffer,
        samplesCount,
        stream->channelsCount,
        metadata.hasTimestamp?ticks:0,
        timeout_ms);
    samplesCount = (std::min)(samplesCount, sampsPushed);

    return samplesCount;
}

int ConnectionSTREAM::ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata &metadata)
{
    auto *stream = (USBStreamServiceChannel *)streamID;

    if (stream->isTx and mStreamService->mTxStatQueue.wait_and_pop(metadata, timeout_ms)) return 0;
    if (!stream->isTx and mStreamService->mRxStatQueue.wait_and_pop(metadata, timeout_ms)) return 0;
    return -1;
}

void ConnectionSTREAM::UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate)
{
    std::cout << "LMS_StreamBoard::ConfigurePLL(tx=" << txRate/1e6 << "MHz, rx=" << rxRate/1e6  << "MHz)" << std::endl;
    LMS_StreamBoard::ConfigurePLL(this, txRate/1e6, rxRate/1e6, 90);
    if (mStreamService) mStreamService->mHwCounterRate = rxRate;
}

void ConnectionSTREAM::EnterSelfCalibration(const size_t channel)
{
    if (mStreamService) mStreamService->stop();
}

void ConnectionSTREAM::ExitSelfCalibration(const size_t channel)
{
    if (mStreamService) mStreamService->start();
}

uint64_t ConnectionSTREAM::GetHardwareTimestamp(void)
{
    return mStreamService->mLastRxTimestamp + mStreamService->mTimestampOffset;
}

void ConnectionSTREAM::SetHardwareTimestamp(const uint64_t now)
{
    if (not mStreamService) mStreamService.reset(new USBStreamService(this));
    mStreamService->mTimestampOffset = int64_t(now)-int64_t(mStreamService->mLastRxTimestamp);
}

double ConnectionSTREAM::GetHardwareTimestampRate(void)
{
    return mStreamService->mHwCounterRate;
}
