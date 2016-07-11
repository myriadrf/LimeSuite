#include "ConnectionSTREAM.h"

using namespace lime;

ConnectionSTREAM::StreamChannel::StreamChannel(lime::IConnection* port) :
    mActive(false)
{
    this->port = dynamic_cast<ConnectionSTREAM*>(port);
    fifo = new LMS_SamplesFIFO(1024*8, 1);
}

ConnectionSTREAM::StreamChannel::~StreamChannel()
{
    delete fifo;
}

int ConnectionSTREAM::StreamChannel::Read(void* samples, const uint32_t count, Metadata* meta, const int32_t timeout_ms)
{
    complex16_t** ptr = (complex16_t**)&samples;
    int popped = fifo->pop_samples(ptr, count, 1, &meta->timestamp, timeout_ms, &meta->flags);
    return popped;
}

int ConnectionSTREAM::StreamChannel::Write(const void* samples, const uint32_t count, const Metadata *meta, const int32_t timeout_ms)
{
    const complex16_t** ptr = (const complex16_t**)&samples;
    int pushed = fifo->push_samples(ptr, count, 1, meta->timestamp, timeout_ms, meta->flags);
    return pushed;
}

IStreamChannel::Info ConnectionSTREAM::StreamChannel::GetInfo()
{
    Info stats;
    LMS_SamplesFIFO::BufferInfo info = fifo->GetInfo();
    stats.fifoSize = info.size;
    stats.fifoItemsCount = info.itemsFilled;
    stats.active = mActive;
    if(config.isTx)
        stats.linkRate = port->txDataRate_Bps.load();
    else
        stats.linkRate = port->rxDataRate_Bps.load();
    return stats;
}

bool ConnectionSTREAM::StreamChannel::IsActive() const
{
    return mActive;
}

int ConnectionSTREAM::StreamChannel::Start()
{
    mActive = true;
    return port->UpdateThreads();
}

int ConnectionSTREAM::StreamChannel::Stop()
{
    mActive = false;
    return port->UpdateThreads();
}
