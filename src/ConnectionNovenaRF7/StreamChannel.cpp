#include "ConnectionNovenaRF7.h"

using namespace lime;

ConnectionNovenaRF7::StreamChannel::StreamChannel(lime::IConnection* port) :
    mActive(false)
{
    this->port = dynamic_cast<ConnectionNovenaRF7*>(port);
    fifo = new RingFIFO(1024*8);
}

ConnectionNovenaRF7::StreamChannel::~StreamChannel()
{
    delete fifo;
}

int ConnectionNovenaRF7::StreamChannel::Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms)
{
    int popped = 0;
    if(config.format == StreamConfig::FMT_FLOAT32 && !config.isTx)
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

int ConnectionNovenaRF7::StreamChannel::Write(const void* samples, const uint32_t count, const Metadata *meta, const int32_t timeout_ms)
{
    int pushed = 0;
    if(config.format == StreamConfig::FMT_FLOAT32 && config.isTx)
    {
        const float* samplesFloat = (const float*)samples;
        int16_t* samplesShort = new int16_t[2*count];
        for(size_t i=0; i<2*count; ++i)
            samplesShort[i] = samplesFloat[i]*2047;
        const complex16_t* ptr = (const complex16_t*)samplesShort ;
        pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
        delete[] samplesShort;
    }
    //else if(config.format == StreamConfig::STREAM_12_BIT_IN_16)
    else
    {
        const complex16_t* ptr = (const complex16_t*)samples;
        pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
    }
    return pushed;
}

IStreamChannel::Info ConnectionNovenaRF7::StreamChannel::GetInfo()
{
    Info stats;
    RingFIFO::BufferInfo info = fifo->GetInfo();
    stats.fifoSize = info.size;
    stats.fifoItemsCount = info.itemsFilled;
    stats.active = mActive;
    if(config.isTx)
        stats.linkRate = 0;
    else
        stats.linkRate = port->rxDataRate_Bps.load();
    return stats;
}

bool ConnectionNovenaRF7::StreamChannel::IsActive() const
{
    return mActive;
}

int ConnectionNovenaRF7::StreamChannel::Start()
{
    mActive = true;
    fifo->Clear();
    return port->UpdateThreads();
}

int ConnectionNovenaRF7::StreamChannel::Stop()
{
    mActive = false;
    return port->UpdateThreads();
}
