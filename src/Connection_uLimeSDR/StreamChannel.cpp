#include "Connection_uLimeSDR.h"

using namespace lime;

Connection_uLimeSDR::StreamChannel::StreamChannel(lime::IConnection* port) :
    mActive(false)
{
    this->port = dynamic_cast<Connection_uLimeSDR*>(port);
    fifo = new RingFIFO(1024*8);
}

Connection_uLimeSDR::StreamChannel::~StreamChannel()
{
    delete fifo;
}

int Connection_uLimeSDR::StreamChannel::Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms)
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
            samplesFloat[i] = samplesShort[i]/2048.0;
    }
    //else if(config.format == StreamConfig::STREAM_12_BIT_IN_16)
    else
    {
        complex16_t* ptr = (complex16_t*)samples;
        popped = fifo->pop_samples(ptr, count, 1, &meta->timestamp, timeout_ms, &meta->flags);
    }
    return popped;
}

int Connection_uLimeSDR::StreamChannel::Write(const void* samples, const uint32_t count, const Metadata *meta, const int32_t timeout_ms)
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

IStreamChannel::Info Connection_uLimeSDR::StreamChannel::GetInfo()
{
    Info stats;
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

bool Connection_uLimeSDR::StreamChannel::IsActive() const
{
    return mActive;
}

int Connection_uLimeSDR::StreamChannel::Start()
{
    mActive = true;
    fifo->Clear();
    return port->UpdateThreads();
}

int Connection_uLimeSDR::StreamChannel::Stop()
{
    mActive = false;
    return port->UpdateThreads();
}
