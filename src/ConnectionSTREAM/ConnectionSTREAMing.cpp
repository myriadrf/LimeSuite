/**
    @file ConnectionSTREAMing.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection (streaming API)
*/

#include "ConnectionSTREAM.h"
#include "StreamerLTE.h"
#include "fifo.h" //from StreamerLTE
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>
#include <ciso646>

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

        //configure LML clocking
        LMS7002M rfic;
        rfic.SetConnection(dataPort);
        rfic.Modify_SPI_Reg_bits(FCLK1_INV, 1, true);
        rfic.Modify_SPI_Reg_bits(FCLK2_INV, 1, true);
        rfic.Modify_SPI_Reg_bits(LML1_FIDM, 0, true); //Frame start=0
        rfic.Modify_SPI_Reg_bits(LML2_FIDM, 0, true); //Frame start=0
        rfic.Modify_SPI_Reg_bits(LML1_MODE, 0, true); //TRXIQ
        rfic.Modify_SPI_Reg_bits(LML2_MODE, 0, true); //TRXIQ

        //switch off Rx/Tx
        uint16_t interface_ctrl_000A = Reg_read(dataPort, 0x000A);
        Reg_write(dataPort, 0x000A, interface_ctrl_000A & ~0x3);

        //reset hardware timestamp to 0
        uint16_t interface_ctrl_0009 = Reg_read(dataPort, 0x0009);
        Reg_write(dataPort, 0x0009, interface_ctrl_0009 & ~0x3);
        Reg_write(dataPort, 0x0009, interface_ctrl_0009 | 0x3);
        Reg_write(dataPort, 0x0009, interface_ctrl_0009 & ~0x3);

        //enable MIMO mode, 12 bit compressed values
        uint16_t smpl_width = 0x2; // 0-16 bit, 1-14 bit, 2-12 bit
        if (channelsCount == 2)
        {
            Reg_write(dataPort, 0x0007, 0x0003); //channel enables
            Reg_write(dataPort, 0x0008, 0x0100 | smpl_width);
        }
        else
        {
            Reg_write(dataPort, 0x0007, 0x0001); //channel enables
            Reg_write(dataPort, 0x0008, 0x0000 | smpl_width);
        }

        //USB FIFO reset
        //TODO direction specific reset using isTx
        LMS64CProtocol::GenericPacket ctrPkt;
        ctrPkt.cmd = CMD_USB_FIFO_RST;
        ctrPkt.outBuffer.push_back(0x01);
        dataPort->TransferPacket(ctrPkt);
        ctrPkt.outBuffer[0] = 0x00;
        dataPort->TransferPacket(ctrPkt);

        LMS64CProtocol::GenericPacket pkt;
        pkt.cmd = CMD_ANALOG_VAL_WR;
        pkt.outBuffer.push_back( 1 ); //select tcxo channel
        pkt.outBuffer.push_back( 0 ); // raw value without units
        //TODO tcxo value is different for different boards
        uint16_t tcxo_value = 126; // txco value to be set
        pkt.outBuffer.push_back(tcxo_value >> 8);
        pkt.outBuffer.push_back(tcxo_value & 0xFF);

        dataPort->TransferPacket(pkt);

        //switch on Rx
        interface_ctrl_000A = Reg_read(mDataPort, 0x000A);
        Reg_write(mDataPort, 0x000A, interface_ctrl_000A | 0x1);
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
        uint16_t interface_ctrl_000A = Reg_read(mDataPort, 0x000A);
        Reg_write(mDataPort, 0x000A, interface_ctrl_000A | 0x1);

        this->updateThreadState();
    }

    void stop(void)
    {
        //stop threads first, then disable FPGA
        this->updateThreadState(true);
        //stop Tx Rx if they were active
        uint16_t interface_ctrl_000A = Reg_read(mDataPort, 0x000A);
        Reg_write(mDataPort, 0x000A, interface_ctrl_000A & ~0x3);
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

        if (isTx)
        {
            if (metadata.lateTimestamp and txTimeEnabled)
            {
                std::cout << "L" << std::flush;
                mTxStatQueue.push(metadata);
            }
        }
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
        convertFloat(convertFloat),
        sampsRemaining(0),
        bufferOffset(0),
        nextTimestamp(0),
        currentFifoFlags(0)
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

    size_t sampsRemaining;
    size_t bufferOffset;
    uint64_t nextTimestamp;
    uint32_t currentFifoFlags;
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
    auto s3 = pos1isA?LMS7002M::AQ:LMS7002M::BQ;
    auto s2 = pos1isA?LMS7002M::AI:LMS7002M::BI;
    auto s1 = pos0isA?LMS7002M::AQ:LMS7002M::BQ;
    auto s0 = pos0isA?LMS7002M::AI:LMS7002M::BI;

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

    //first read into the intermediate buffer
    //intermediate buffer can be removed with different fifo implementation
    if (stream->sampsRemaining == 0)
    {
        stream->bufferOffset = 0;
        //first receive full STREAM_MTU with no timeout
        stream->sampsRemaining = mStreamService->GetRxFIFO()->pop_samples(
            stream->FIFOBuffers.data(),
            STREAM_MTU,
            stream->channelsCount,
            &stream->nextTimestamp,
            0, //no timeout
            &stream->currentFifoFlags);

        //otherwise perform requested receive with timeout
        if (stream->sampsRemaining == 0) stream->sampsRemaining = mStreamService->GetRxFIFO()->pop_samples(
            stream->FIFOBuffers.data(),
            std::min<size_t>(STREAM_MTU, length),
            stream->channelsCount,
            &stream->nextTimestamp,
            timeout_ms,
            &stream->currentFifoFlags);

        if (stream->sampsRemaining == 0) return 0; //timeout
    }

    //std::cout << "RX TIME " << stream->nextTimestamp << std::endl;

    //convert into the output buffer
    size_t samplesCount = std::min<size_t>(length, stream->sampsRemaining);
    if (stream->convertFloat)
    {
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto buffIn = stream->FIFOBuffers[i]+stream->bufferOffset;
            auto buffOut = (std::complex<float> *)buffs[i];
            for (size_t j = 0; j < samplesCount; j++)
            {
                buffOut[j] = std::complex<float>(
                    float(buffIn[j].i)/2048,
                    float(buffIn[j].q)/2048);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto buffIn = stream->FIFOBuffers.data()[i]+stream->bufferOffset;
            std::memcpy(buffs[i], buffIn, samplesCount*sizeof(complex16_t));
        }
    }

    //metadata
    metadata.timestamp = stream->nextTimestamp + mStreamService->mTimestampOffset;
    metadata.hasTimestamp = true;
    metadata.endOfBurst = (stream->currentFifoFlags & STATUS_FLAG_RX_END) != 0;

    //setup for next call
    stream->bufferOffset += samplesCount;
    stream->sampsRemaining -= samplesCount;
    stream->nextTimestamp += samplesCount;
    if (stream->sampsRemaining != 0) metadata.endOfBurst = false;

    return samplesCount;
}

int ConnectionSTREAM::WriteStream(const size_t streamID, const void * const *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata)
{
    auto *stream = (USBStreamServiceChannel *)streamID;
    //TODO check fifo has space with timeout

    if (stream->sampsRemaining == 0)
    {
        stream->bufferOffset = 0;
        stream->sampsRemaining = STREAM_MTU;
    }

    //first convert into the intermediate buffer
    //intermediate buffer can be removed with different fifo implementation
    size_t samplesCount = std::min<size_t>(length, stream->sampsRemaining);
    if (stream->convertFloat)
    {
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto buffIn = (const std::complex<float> *)buffs[i];
            auto bufOut = stream->FIFOBuffers[i]+stream->bufferOffset;
            for (size_t j = 0; j < samplesCount; j++)
            {
                bufOut[j].i = int16_t(buffIn[j].real()*2048);
                bufOut[j].q = int16_t(buffIn[j].imag()*2048);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto bufOut = stream->FIFOBuffers[i]+stream->bufferOffset;
            std::memcpy(bufOut, buffs[i], samplesCount*sizeof(complex16_t));
        }
    }

    //metadata
    if (metadata.hasTimestamp)
    {
        stream->nextTimestamp = metadata.timestamp - mStreamService->mTimestampOffset;
        stream->nextTimestamp -= stream->bufferOffset; //applies to front of usb packet
        //std::cout << "TX TIME " << stream->nextTimestamp << std::endl;
    }

    //setup for next call
    stream->bufferOffset += samplesCount;
    stream->sampsRemaining -= samplesCount;

    //input was 100% consumed and this is a end of burst
    bool actualEob = metadata.endOfBurst and samplesCount == length;

    //memset any remaining samples to zero
    if (actualEob and stream->sampsRemaining != 0)
    {
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto bufOut = stream->FIFOBuffers[i]+stream->bufferOffset;
            std::memset(bufOut, 0, stream->sampsRemaining*sizeof(complex16_t));
        }
        stream->sampsRemaining = 0;
    }

    //push when the intermediate buffer is full
    if (stream->sampsRemaining == 0)
    {
        uint32_t statusFlags = 0;
        if (mStreamService->txTimeEnabled) statusFlags |= STATUS_FLAG_TX_TIME;
        if (actualEob) statusFlags |= STATUS_FLAG_TX_END;
        size_t sampsPushed = mStreamService->GetTxFIFO()->push_samples(
            (const complex16_t **)stream->FIFOBuffers.data(),
            STREAM_MTU,
            stream->channelsCount,
            stream->nextTimestamp,
            timeout_ms,
            statusFlags);
        stream->nextTimestamp += sampsPushed;
    }

    return samplesCount;
}

int ConnectionSTREAM::ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata &metadata)
{
    auto *stream = (USBStreamServiceChannel *)streamID;

    if (stream->isTx and mStreamService->mTxStatQueue.wait_and_pop(metadata, timeout_ms)) return 0;
    if (!stream->isTx and mStreamService->mRxStatQueue.wait_and_pop(metadata, timeout_ms)) return 0;
    return -1;
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
void ConnectionSTREAM::UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz)
{
    std::cout << "ConnectionSTREAM::ConfigureFPGA_PLL(tx=" << txRate_Hz/1e6 << "MHz, rx=" << rxRate_Hz/1e6 << "MHz)" << std::endl;
    ConfigureFPGA_PLL(0, 2 * txRate_Hz, 90);
    ConfigureFPGA_PLL(1, 2 * rxRate_Hz, 90);
    if(mStreamService) mStreamService->mHwCounterRate = rxRate_Hz;
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
