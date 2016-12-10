#include "ILimeSDRStreaming.h"
#include "ErrorReporting.h"
#include <assert.h>
#include "FPGA_common.h"
#include "LMS7002M.h"
#include <ciso646>

using namespace lime;

ILimeSDRStreaming::ILimeSDRStreaming()
{
    rxRunning = false;
    txRunning = false;
}
ILimeSDRStreaming::~ILimeSDRStreaming()
{

}

int ILimeSDRStreaming::SetupStream(size_t& streamID, const StreamConfig& config)
{
    if(rxRunning.load() == true || txRunning.load() == true)
        return ReportError(EPERM, "All streams must be stopped before doing setups");
    streamID = ~0;
    StreamChannel* stream = new StreamChannel(this);
    stream->config = config;
    //TODO check for duplicate streams
    if(config.isTx)
        mTxStreams.push_back(stream);
    else
        mRxStreams.push_back(stream);
    streamID = size_t(stream);
    return 0; //success
}

int ILimeSDRStreaming::CloseStream(const size_t streamID)
{
    if(rxRunning.load() == true || txRunning.load() == true)
        return ReportError(EPERM, "All streams must be stopped before closing");
    StreamChannel *stream = (StreamChannel*)streamID;
    for(auto i=mRxStreams.begin(); i!=mRxStreams.end(); ++i)
    {
        if(*i==stream)
        {
            delete *i;
            mRxStreams.erase(i);
            break;
        }
    }
    for(auto i=mTxStreams.begin(); i!=mTxStreams.end(); ++i)
    {
        if(*i==stream)
        {
            delete *i;
            mTxStreams.erase(i);
            break;
        }
    }
    return 0;
}
size_t ILimeSDRStreaming::GetStreamSize(const size_t streamID)
{
    uint16_t channelEnables = 0;
    for(uint8_t i=0; i<mRxStreams.size(); ++i)
        channelEnables |= (1 << mRxStreams[i]->config.channelID);
    for(uint8_t i=0; i<mTxStreams.size(); ++i)
        channelEnables |= (1 << mTxStreams[i]->config.channelID);
    uint8_t uniqueChannelCount = 0;
    for(uint8_t i=0; i<16; ++i)
    {
        uniqueChannelCount += (channelEnables & 0x1);
        channelEnables >>= 1;
    }
    //if no channels are setup return smallest number of samples in packet
    if(uniqueChannelCount == 0)
        return 680;
    else
        return 1360/uniqueChannelCount;
}

int ILimeSDRStreaming::ControlStream(const size_t streamID, const bool enable)
{
    auto *stream = (IStreamChannel* )streamID;
    assert(stream != nullptr);

    if(enable)
        return stream->Start();
    else
        return stream->Stop();
}

int ILimeSDRStreaming::ReadStream(const size_t streamID, void* buffs, const size_t length, const long timeout_ms, StreamMetadata& metadata)
{
    assert(streamID != 0);
    lime::IStreamChannel* channel = (lime::IStreamChannel*)streamID;
    lime::IStreamChannel::Metadata meta;
    meta.flags = 0;
    meta.flags |= metadata.hasTimestamp ? lime::IStreamChannel::Metadata::SYNC_TIMESTAMP : 0;
    meta.timestamp = metadata.timestamp;
    int status = channel->Read(buffs, length, &meta, timeout_ms);
    metadata.hasTimestamp = true;
    metadata.timestamp = meta.timestamp;
    return status;
}

int ILimeSDRStreaming::WriteStream(const size_t streamID, const void* buffs, const size_t length, const long timeout_ms, const StreamMetadata& metadata)
{
    assert(streamID != 0);
    lime::IStreamChannel* channel = (lime::IStreamChannel*)streamID;
    lime::IStreamChannel::Metadata meta;
    meta.flags = 0;
    meta.flags |= metadata.hasTimestamp ? lime::IStreamChannel::Metadata::SYNC_TIMESTAMP : 0;
    meta.timestamp = metadata.timestamp;
    int status = channel->Write(buffs, length, &meta, timeout_ms);
    return status;
}

int ILimeSDRStreaming::ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata& metadata)
{
    assert(streamID != 0);
    lime::IStreamChannel* channel = (lime::IStreamChannel*)streamID;
    IStreamChannel::Info info = channel->GetInfo();
    metadata.hasTimestamp = true;
    metadata.timestamp = info.timestamp;
    metadata.lateTimestamp = info.underrun > 0;
    metadata.packetDropped = info.droppedPackets > 0;
    return 0;
}

void ILimeSDRStreaming::EnterSelfCalibration(const size_t channel)
{
    if(not rxRunning)
        return;
    generateData.store(true);
    std::unique_lock<std::mutex> lck(streamStateLock);
    //wait untill all existing USB transfers complete
    safeToConfigInterface.wait_for(lck, std::chrono::milliseconds(250));
}

void ILimeSDRStreaming::ExitSelfCalibration(const size_t channel)
{
    generateData.store(false);
}

uint64_t ILimeSDRStreaming::GetHardwareTimestamp(void)
{
    if(not rxRunning.load() and not txRunning.load())
    {
        //stop streaming just in case the board has not been configured
        fpga::StopStreaming(this);
        fpga::ResetTimestamp(this);
        mTimestampOffset = 0;
        return 0;
    }
    else
    {
        return rxLastTimestamp.load()+mTimestampOffset;
    }
}

void ILimeSDRStreaming::SetHardwareTimestamp(const uint64_t now)
{
    mTimestampOffset = now - rxLastTimestamp.load();
}

double ILimeSDRStreaming::GetHardwareTimestampRate(void)
{
    return mExpectedSampleRate;
}

int ILimeSDRStreaming::UpdateThreads()
{
    bool needTx = false;
    bool needRx = false;

    //check which threads are needed
    for(auto i : mRxStreams)
        if(i->IsActive())
        {
            needRx = true;
            break;
        }
    for(auto i : mTxStreams)
        if(i->IsActive())
        {
            needTx = true;
            break;
        }

    //stop threads if not needed
    if(not needTx and txRunning.load())
    {
        terminateTx.store(true);
        txThread.join();
        txRunning.store(false);
    }
    if(not needRx and rxRunning.load())
    {
        terminateRx.store(true);
        rxThread.join();
        rxRunning.store(false);
    }

    //configure FPGA on first start, or disable FPGA when not streaming
    if((needTx or needRx) && (not rxRunning.load() and not txRunning.load()))
    {
        //enable FPGA streaming
        fpga::StopStreaming(this);
        fpga::ResetTimestamp(this);
        rxLastTimestamp.store(0);
        //USB FIFO reset
        // TODO : USB FIFO reset command for IConnection
        LMS64CProtocol::GenericPacket ctrPkt;
        ctrPkt.cmd = CMD_USB_FIFO_RST;
        ctrPkt.outBuffer.push_back(0x00);
        TransferPacket(ctrPkt);

        //enable MIMO mode, 12 bit compressed values
        StreamConfig config;
        config.linkFormat = StreamConfig::STREAM_12_BIT_COMPRESSED;
        //by default use 12 bit compressed, adjust link format for stream

        for(auto i : mRxStreams)
        {
            if(i->config.format == StreamConfig::STREAM_12_BIT_IN_16)
            {
                config.linkFormat = StreamConfig::STREAM_12_BIT_IN_16;
                break;
            }
        }
        for(auto i : mTxStreams)
        {
            if(i->config.format == StreamConfig::STREAM_12_BIT_IN_16)
            {
                config.linkFormat = StreamConfig::STREAM_12_BIT_IN_16;
                break;
            }
        }
        for(auto i : mRxStreams)
            i->config.linkFormat = config.linkFormat;
        for(auto i : mTxStreams)
            i->config.linkFormat = config.linkFormat;

        uint16_t smpl_width; // 0-16 bit, 1-14 bit, 2-12 bit
        if(config.linkFormat == StreamConfig::STREAM_12_BIT_IN_16)
            smpl_width = 0x0;
        else if(config.linkFormat == StreamConfig::STREAM_12_BIT_COMPRESSED)
            smpl_width = 0x2;
        else
            smpl_width = 0x2;
        WriteRegister(0x0008, 0x0100 | smpl_width);

        uint16_t channelEnables = 0;
        for(uint8_t i=0; i<mRxStreams.size(); ++i)
            channelEnables |= (1 << mRxStreams[i]->config.channelID);
        for(uint8_t i=0; i<mTxStreams.size(); ++i)
            channelEnables |= (1 << mTxStreams[i]->config.channelID);
        WriteRegister(0x0007, channelEnables);

        LMS7002M lmsControl;
        lmsControl.SetConnection(this);
        bool fromChip = true;
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_FIDM), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_FIDM), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE1), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE1), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0, fromChip);

        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S0S), 1, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S1S), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S2S), 3, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S3S), 2, fromChip);

        if(channelEnables & 0x2) //enable MIMO
        {
            uint16_t macBck = lmsControl.Get_SPI_Reg_bits(LMS7param(MAC), fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(MAC), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(MAC), macBck, fromChip);
        }

        fpga::StartStreaming(this);
    }
    else if(not needTx and not needRx)
    {
        //disable FPGA streaming
        fpga::StopStreaming(this);
    }

    //FPGA should be configured and activated, start needed threads
    if(needRx and not rxRunning.load())
    {
        ThreadData args;
        args.terminate = &terminateRx;
        args.dataPort = this;
        args.dataRate_Bps = &rxDataRate_Bps;
        args.channels = mRxStreams;
        args.generateData = &generateData;
        args.safeToConfigInterface = &safeToConfigInterface;
        args.lastTimestamp = &rxLastTimestamp;;

        rxRunning.store(true);
        terminateRx.store(false);
        rxThread = std::thread(RxLoopFunction, args);
    }
    if(needTx and not txRunning.load())
    {
        ThreadData args;
        args.terminate = &terminateTx;
        args.dataPort = this;
        args.dataRate_Bps = &txDataRate_Bps;
        args.channels = mTxStreams;
        args.generateData = &generateData;
        args.safeToConfigInterface = &safeToConfigInterface;
        args.lastTimestamp = nullptr;

        txRunning.store(true);
        terminateTx.store(false);
        txThread = std::thread(TxLoopFunction, args);
    }
    return 0;
}


//-----------------------------------------------------------------------------
ILimeSDRStreaming::StreamChannel::StreamChannel(lime::IConnection* port) :
    mActive(false)
{
    this->port = dynamic_cast<ILimeSDRStreaming*>(port);
    fifo = new RingFIFO(1024*8);
}

ILimeSDRStreaming::StreamChannel::~StreamChannel()
{
    delete fifo;
}

int ILimeSDRStreaming::StreamChannel::Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms)
{
    int popped = 0;
    if(config.format == StreamConfig::STREAM_COMPLEX_FLOAT32 && !config.isTx)
    {
        //in place conversion
        complex16_t* ptr = (complex16_t*)samples;
        int16_t* samplesShort = (int16_t*)samples;
        float* samplesFloat = (float*)samples;
        popped = fifo->pop_samples(ptr, count, 1, &meta->timestamp, timeout_ms, &meta->flags);
        for(int i=2*popped-1; i>=0; --i)
            samplesFloat[i] = (float)samplesShort[i]/2048.0;
    }
    //else if(config.format == StreamConfig::STREAM_12_BIT_IN_16)
    else
    {
        complex16_t* ptr = (complex16_t*)samples;
        popped = fifo->pop_samples(ptr, count, 1, &meta->timestamp, timeout_ms, &meta->flags);
    }
    return popped;
}

int ILimeSDRStreaming::StreamChannel::Write(const void* samples, const uint32_t count, const Metadata *meta, const int32_t timeout_ms)
{
    int pushed = 0;
    if(config.format == StreamConfig::STREAM_COMPLEX_FLOAT32 && config.isTx)
    {
        const float* samplesFloat = (const float*)samples;
        int16_t* samplesShort = new int16_t[2*count];
        for(size_t i=0; i<2*count; ++i)
            samplesShort[i] = samplesFloat[i]*2047;
        const complex16_t* ptr = (const complex16_t*)samplesShort ;
        pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
        delete samplesShort;
    }
    //else if(config.format == StreamConfig::STREAM_12_BIT_IN_16)
    else
    {
        const complex16_t* ptr = (const complex16_t*)samples;
        pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
    }
    return pushed;
}

IStreamChannel::Info ILimeSDRStreaming::StreamChannel::GetInfo()
{
    Info stats;
    memset(&stats,0,sizeof(stats));
    RingFIFO::BufferInfo info = fifo->GetInfo();
    stats.fifoSize = info.size;
    stats.fifoItemsCount = info.itemsFilled;
    stats.active = mActive;
    if(config.isTx)
        stats.linkRate = port->txDataRate_Bps.load();
    else
        stats.linkRate = port->rxDataRate_Bps.load();
    return stats;
}

bool ILimeSDRStreaming::StreamChannel::IsActive() const
{
    return mActive;
}

int ILimeSDRStreaming::StreamChannel::Start()
{
    mActive = true;
    fifo->Clear();
    return port->UpdateThreads();
}

int ILimeSDRStreaming::StreamChannel::Stop()
{
    mActive = false;
    return port->UpdateThreads();
}
