#include "ILimeSDRStreaming.h"
#include "ErrorReporting.h"
#include <assert.h>
#include "FPGA_common.h"
#include "LMS7002M.h"
#include <ciso646>
#include "Logger.h"

using namespace lime;

static const int MAX_CHANNEL_COUNT = 6;

ILimeSDRStreaming::ILimeSDRStreaming()
{
    for (int i = 0; i < MAX_CHANNEL_COUNT/2; i++)
    	mStreamers.push_back(new Streamer(this));
}
ILimeSDRStreaming::~ILimeSDRStreaming()
{
    for (unsigned i = 0; i < mStreamers.size() ; i++)
        delete mStreamers[i];
}

int ILimeSDRStreaming::SetupStream(size_t& streamID, const StreamConfig& config)
{
    if ( config.channelID >= MAX_CHANNEL_COUNT)
        return -1;
    unsigned index = config.channelID/2;
    while (index >= mStreamers.size())
       mStreamers.push_back(new Streamer(this));
    return mStreamers[index]->SetupStream(streamID,config);
}

int ILimeSDRStreaming::CloseStream(const size_t streamID)
{
    auto *stream = (StreamChannel* )streamID;
    return stream->mStreamer->CloseStream(streamID);
}
size_t ILimeSDRStreaming::GetStreamSize(const size_t streamID)
{
    auto *stream = (StreamChannel* )streamID;
    return stream->mStreamer->GetStreamSize();
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
    meta.flags = metadata.hasTimestamp ? lime::IStreamChannel::Metadata::SYNC_TIMESTAMP : 0;
    meta.flags |= metadata.endOfBurst ? lime::IStreamChannel::Metadata::END_BURST : 0;
    meta.timestamp = metadata.timestamp;
    int status = channel->Write(buffs, length, &meta, timeout_ms);
    return status;
}

int ILimeSDRStreaming::ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata& metadata)
{
    assert(streamID != 0);
    StreamChannel* channel = (StreamChannel*)streamID;

    //support late timestamp reporting
    auto txLastLateTime = channel->mStreamer->txLastLateTime.exchange(0);
    if (txLastLateTime != 0)
    {
        metadata.hasTimestamp = true;
        metadata.timestamp = txLastLateTime;
        metadata.lateTimestamp = true;
        metadata.packetDropped = true;
        return 0;
    }

    IStreamChannel::Info info = channel->GetInfo();
    metadata.hasTimestamp = true;
    metadata.timestamp = info.timestamp;
    metadata.lateTimestamp = info.underrun > 0;
    metadata.packetDropped = info.droppedPackets > 0;
    return 0;
}

void ILimeSDRStreaming::EnterSelfCalibration(const size_t channel)
{
    if (mStreamers.size() > channel/2)
        mStreamers[channel/2]->EnterSelfCalibration();
}

void ILimeSDRStreaming::ExitSelfCalibration(const size_t channel)
{
    if (mStreamers.size() > channel/2)
        mStreamers[channel/2]->ExitSelfCalibration();
}

uint64_t ILimeSDRStreaming::GetHardwareTimestamp(void)
{
    return mStreamers[0]->GetHardwareTimestamp();
}

void ILimeSDRStreaming::SetHardwareTimestamp(const uint64_t now)
{
    mStreamers[0]->SetHardwareTimestamp(now);
}

double ILimeSDRStreaming::GetHardwareTimestampRate(void)
{
    return mExpectedSampleRate;
}

int ILimeSDRStreaming::ReceiveData(char* buffer, int length, int epIndex, int timeout)
{
    return ReportError("Function not supported");
}

int ILimeSDRStreaming::SendData(const char* buffer, int length, int epIndex, int timeout)
{
    return ReportError("Function not supported");
}

int ILimeSDRStreaming::UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex)
{
    bool comp = true;

    switch (GetInfo().device)
    {
        case LMS_DEV_STREAM:
        case LMS_DEV_LIMESDR:
        case LMS_DEV_LIMESDR_PCIE:
        case LMS_DEV_LMS7002M_ULTIMATE_EVB:
            break;
        case LMS_DEV_LIMESDR_QPCIE:
            if (epIndex == 2)
                comp = (format == StreamConfig::STREAM_12_BIT_COMPRESSED);
            break;
        default: return ReportError("UploadWFM not supported");
    }

    const int samplesInPkt = comp ? 1360 : 1020;
    WriteRegister(0xFFFF, 1 << epIndex);
    WriteRegister(0x000C, chCount == 2 ? 0x3 : 0x1); //channels 0,1
    WriteRegister(0x000E, comp ? 0x2 : 0x0); //16bit samples

    uint16_t regValue = 0;
    ReadRegister(0x000D,regValue);
    regValue |= 0x4;
    WriteRegister(0x000D, regValue);

    lime::FPGA_DataPacket pkt;
    size_t samplesUsed = 0;
    int cnt = sample_count;

    const complex16_t* const* src = (const complex16_t* const*)samples;
    const lime::complex16_t** batch = new const lime::complex16_t*[chCount];
    lime::complex16_t** samplesShort = new lime::complex16_t*[chCount];
    for(unsigned i=0; i<chCount; ++i)
        samplesShort[i] = nullptr;


    if (format == StreamConfig::STREAM_12_BIT_IN_16 && comp == true)
    {
        for(unsigned i=0; i<chCount; ++i)
            samplesShort[i] = new lime::complex16_t[sample_count];
        for (int ch = 0; ch < chCount; ch++)
            for(size_t i=0; i < sample_count; ++i)
            {
                samplesShort[ch][i].i = src[ch][i].i >> 4;
                samplesShort[ch][i].q = src[ch][i].q >> 4;
            }
        src = samplesShort;
    }
    else if(format == StreamConfig::STREAM_COMPLEX_FLOAT32)
    {
        const float mult = comp ? 2047.0f : 32767.0f;
        for(unsigned i=0; i<chCount; ++i)
            samplesShort[i] = new lime::complex16_t[sample_count];

        const float* const* samplesFloat = (const float* const*)samples;
        for (int ch = 0; ch < chCount; ch++)
            for(size_t i=0; i < sample_count; ++i)
            {
                samplesShort[ch][i].i = samplesFloat[ch][2*i]*mult;
                samplesShort[ch][i].q = samplesFloat[ch][2*i+1]*mult;
            }
        src = samplesShort;
    }

    while(cnt > 0)
    {
        pkt.counter = 0;
        pkt.reserved[0] = 0;
        int samplesToSend = cnt > samplesInPkt/chCount ? samplesInPkt/chCount : cnt;

        for(unsigned i=0; i<chCount; ++i)
            batch[i] = &src[i][samplesUsed];
        samplesUsed += samplesToSend;

        int bufPos = lime::fpga::Samples2FPGAPacketPayload(batch, samplesToSend, chCount==2, comp, pkt.data);
        int payloadSize = (bufPos / 4) * 4;
        if(bufPos % 4 != 0)
            lime::error("Packet samples count not multiple of 4");
        pkt.reserved[2] = (payloadSize >> 8) & 0xFF; //WFM loading
        pkt.reserved[1] = payloadSize & 0xFF; //WFM loading
        pkt.reserved[0] = 0x1 << 5; //WFM loading

        long bToSend = 16+payloadSize;
        if (SendData((const char*)&pkt,bToSend,epIndex,500)!=bToSend)
            break;
        cnt -= samplesToSend;
    }
    delete[] batch;
    for(unsigned i=0; i<chCount; ++i)
        if (samplesShort[i])
            delete [] samplesShort[i];
    delete[] samplesShort;

    /*Give FX3 some time to load samples to FPGA*/
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if(cnt == 0)
        return 0;
    else
        return ReportError(-1, "Failed to upload waveform");
}


//-----------------------------------------------------------------------------
ILimeSDRStreaming::StreamChannel::StreamChannel(Streamer* streamer, StreamConfig conf) :
    mActive(false)
{
    mStreamer = streamer;
    this->config = conf;
    overflow = 0;
    underflow = 0;
    pktLost = 0;

    if (this->config.bufferLength == 0) //default size
        this->config.bufferLength = 1024*8*SamplesPacket::maxSamplesInPacket;
    else
    {
        size_t fifoSize = 64;
        while(fifoSize < conf.bufferLength/SamplesPacket::maxSamplesInPacket)
            fifoSize <<= 1;
        this->config.bufferLength = fifoSize*SamplesPacket::maxSamplesInPacket;
    }
    fifo = new RingFIFO(this->config.bufferLength);
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
            samplesFloat[i] = (float)samplesShort[i]/32767.0f;
    }
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
    if (config.isTx && mActive && mStreamer->txRunning.load() == false)
        mStreamer->UpdateThreads();
    if(config.format == StreamConfig::STREAM_COMPLEX_FLOAT32 && config.isTx)
    {
        const float* samplesFloat = (const float*)samples;
        int16_t* samplesShort = new int16_t[2*count];
        for(size_t i=0; i<2*count; ++i)
            samplesShort[i] = samplesFloat[i]*32767.0f;
        const complex16_t* ptr = (const complex16_t*)samplesShort ;
        pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
        delete[] samplesShort;
    }
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
    stats.droppedPackets = pktLost;
    stats.overrun = overflow;
    stats.overrun = underflow;
    pktLost = 0;
    overflow = 0;
    underflow = 0;
    if(config.isTx)
        stats.linkRate = mStreamer->txDataRate_Bps.load();
    else
        stats.linkRate = mStreamer->rxDataRate_Bps.load();
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
    overflow = 0;
    underflow = 0;
    pktLost = 0;
    return mStreamer->UpdateThreads();
}

int ILimeSDRStreaming::StreamChannel::Stop()
{
    mActive = false;
    return mStreamer->UpdateThreads();
}

ILimeSDRStreaming::Streamer::Streamer(ILimeSDRStreaming* port)
{
    dataPort = port;
    rxRunning = false;
    txRunning = false;
    mTimestampOffset = 0;
    rxLastTimestamp = 0;
    terminateRx = false;
    terminateTx = false;
    rxRunning = false;
    txRunning = false;
    rxDataRate_Bps = 0;
    txDataRate_Bps = 0;
    txBatchSize = 1;
    rxBatchSize = 1;
    mChipID = dataPort->mStreamers.size();
}

ILimeSDRStreaming::Streamer::~Streamer()
{
    for(auto i : mTxStreams)
        CloseStream((size_t)i);
    for(auto i : mRxStreams)
        CloseStream((size_t)i);
    terminateTx.store(true);
    if (txThread.joinable())
        txThread.join();
    terminateRx.store(true);
    if (rxThread.joinable())
        rxThread.join();
}

int ILimeSDRStreaming::Streamer::SetupStream(size_t& streamID, const StreamConfig& config)
{
    /*if(rxRunning.load() == true || txRunning.load() == true)
        return ReportError(EPERM, "All streams must be stopped before doing setups");*/
    streamID = ~0;
    StreamChannel* stream = new StreamChannel(this,config);
    //TODO check for duplicate streams
    if(config.isTx)
        mTxStreams.push_back(stream);
    else
        mRxStreams.push_back(stream);
    streamID = size_t(stream);
    LMS7002M lms;
    lms.SetConnection(dataPort, mChipID);
    double rate = lms.GetSampleRate(config.isTx,LMS7002M::ChA)/1e6;
    int size = (config.isTx) ?  mRxStreams.size(): mRxStreams.size();

    if (config.performanceLatency < 0.5)
        rate = 0.5 + rate * config.performanceLatency * 2.0 * size;
    else
        rate += (config.performanceLatency - 0.5) * 40.0 * size;

    for (int batch = 1; batch < rate; batch <<= 1)
        if (config.isTx)
            txBatchSize = batch;
        else
            rxBatchSize = batch;

    return 0; //success
}

int ILimeSDRStreaming::Streamer::CloseStream(const size_t streamID)
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

size_t ILimeSDRStreaming::Streamer::GetStreamSize()
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

void ILimeSDRStreaming::Streamer::EnterSelfCalibration()
{
    if(not rxRunning)
        return;
    UpdateThreads(true);
}

void ILimeSDRStreaming::Streamer::ExitSelfCalibration()
{
    UpdateThreads();
}

uint64_t ILimeSDRStreaming::Streamer::GetHardwareTimestamp(void)
{
    if(not rxRunning.load() and not txRunning.load())
    {
        //stop streaming just in case the board has not been configured
        dataPort->WriteRegister(0xFFFF, 1 << mChipID);
        fpga::StopStreaming(dataPort);
        fpga::ResetTimestamp(dataPort);
        mTimestampOffset = 0;
        return 0;
    }
    else
    {
        return rxLastTimestamp.load()+mTimestampOffset;
    }
}

void ILimeSDRStreaming::Streamer::SetHardwareTimestamp(const uint64_t now)
{
    mTimestampOffset = now - rxLastTimestamp.load();
}

int ILimeSDRStreaming::Streamer::UpdateThreads(bool stopAll)
{
    bool needTx = false;
    bool needRx = false;

    //check which threads are needed
    if (!stopAll)
    {
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
    dataPort->WriteRegister(0xFFFF, 1 << mChipID);
    //configure FPGA on first start, or disable FPGA when not streaming
    if((needTx or needRx) && (not rxRunning.load() and not txRunning.load()))
    {
        LMS7002M lmsControl;
        lmsControl.SetConnection(dataPort, mChipID);
        //enable FPGA streaming
        fpga::StopStreaming(dataPort);
        fpga::ResetTimestamp(dataPort);
        rxLastTimestamp.store(0);
        //Clear device stream buffers
        dataPort->ResetStreamBuffers();

        //enable MIMO mode, 12 bit compressed values
        StreamConfig config;
        config.linkFormat = StreamConfig::STREAM_12_BIT_COMPRESSED;
        //by default use 12 bit compressed, adjust link format for stream

        for(auto i : mRxStreams)
        {
            if(i->config.format != StreamConfig::STREAM_12_BIT_COMPRESSED)
            {
                config.linkFormat = StreamConfig::STREAM_12_BIT_IN_16;
                break;
            }
        }
        for(auto i : mTxStreams)
        {
            if(i->config.format != StreamConfig::STREAM_12_BIT_COMPRESSED)
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
        uint16_t mode;
        if(config.linkFormat == StreamConfig::STREAM_12_BIT_IN_16)
            smpl_width = 0x0;
        else if(config.linkFormat == StreamConfig::STREAM_12_BIT_COMPRESSED)
            smpl_width = 0x2;
        else
            smpl_width = 0x0;

        if (lmsControl.Get_SPI_Reg_bits(LMS7param(LML1_SISODDR),true))
            mode = 0x0040;
        else if (lmsControl.Get_SPI_Reg_bits(LMS7param(LML1_TRXIQPULSE),true))
            mode = 0x0180;
        else
            mode = 0x0100;

        dataPort->WriteRegister(0x0008, mode | smpl_width);

        uint16_t channelEnables = 0;
        for(uint8_t i=0; i<mRxStreams.size(); ++i)
            channelEnables |= (1 << (mRxStreams[i]->config.channelID&1));
        for(uint8_t i=0; i<mTxStreams.size(); ++i)
            channelEnables |= (1 << (mTxStreams[i]->config.channelID&1));
        dataPort->WriteRegister(0x0007, channelEnables);

        bool fromChip = true;
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_FIDM), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_FIDM), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE1), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE1), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0, fromChip);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0, fromChip);

        if (lmsControl.Get_SPI_Reg_bits(LMS7_MASK, true) == 0)
        {
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S0S), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S1S), 0, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S2S), 3, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S3S), 2, fromChip);
        }
        else
        {
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S0S), 0, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S1S), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S2S), 2, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_S3S), 3, fromChip);
        }

        if(channelEnables & 0x2) //enable MIMO
        {
            uint16_t macBck = lmsControl.Get_SPI_Reg_bits(LMS7param(MAC), fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(MAC), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1, fromChip);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(MAC), macBck, fromChip);
        }

        fpga::StartStreaming(dataPort);
    }
    else if(not needTx and not needRx)
    {
        //disable FPGA streaming
        fpga::StopStreaming(dataPort);
    }

    //FPGA should be configured and activated, start needed threads
    if(needRx and not rxRunning.load())
    {
        rxRunning.store(true);
        terminateRx.store(false);
        rxThread = std::thread(dataPort->RxLoopFunction, this);
    }
    if(needTx and not txRunning.load())
    {
        if (txThread.joinable())
            txThread.join();
        txRunning.store(true);
        terminateTx.store(false);
        txThread = std::thread(dataPort->TxLoopFunction, this);
    }
    return 0;
}
