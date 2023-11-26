#include "limesuite/StreamComposite.h"

namespace lime {

StreamComposite::StreamComposite(const std::vector<StreamAggregate>& aggregate)
{
    mAggregate = aggregate;
}

int StreamComposite::StreamSetup(const SDRDevice::StreamConfig& config)
{
    mActiveAggregates.clear();
    SDRDevice::StreamConfig subConfig = config;
    subConfig.rxCount = 0;
    subConfig.txCount = 0;

    int rxNeed = config.rxCount;
    int txNeed = config.txCount;

    for (auto& a : mAggregate)
    {
        const SDRDevice::Descriptor& desc = a.device->GetDescriptor();
        int channelCount = a.channels.size();


        subConfig.rxCount = rxNeed > channelCount ? channelCount : rxNeed;
        for (int j=0; j<subConfig.rxCount; ++j)
        {
            if (a.channels[j] >= desc.rfSOC[a.streamIndex].channelCount)
                return -1;

            subConfig.rxChannels[j] = a.channels[j];
        }
        rxNeed -= subConfig.rxCount;

        subConfig.txCount = txNeed > channelCount ? channelCount : txNeed;
        for (int j=0; j<subConfig.txCount; ++j)
        {
            if (a.channels[j] >= desc.rfSOC[a.streamIndex].channelCount)
                return -1;

            subConfig.txChannels[j] = a.channels[j];
        }
        txNeed -= subConfig.txCount;

        int rez = a.device->StreamSetup(subConfig, a.streamIndex);
        if (rez != 0)
            return rez;
        mActiveAggregates.push_back(a);

        if (rxNeed == 0 && txNeed == 0)
            break;
    }
    return 0;
}

void StreamComposite::StreamStart()
{
    for (auto& a : mActiveAggregates)
        a.device->StreamStart(a.streamIndex);

    // TODO: synchronized start
}

void StreamComposite::StreamStop()
{
    for (auto& a : mActiveAggregates)
        a.device->StreamStop(a.streamIndex);
}

template<class T>
int StreamComposite::StreamRx(T** samples, uint32_t count, SDRDevice::StreamMeta* meta)
{
    T** dest = samples;
    for (auto& a : mActiveAggregates)
    {
        int ret = a.device->StreamRx(a.streamIndex, dest, count, meta);
        if (ret != count)
            return ret;

        dest += a.channels.size();
    }
    return count;
}

template<class T>
int StreamComposite::StreamTx(const T* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta)
{
    const T* const* src = samples;
    for (auto& a : mActiveAggregates)
    {
        int ret = a.device->StreamTx(a.streamIndex, src, count, meta);
        if (ret != count)
            return ret;

        src += a.channels.size();
    }
    return count;
}

// force instantiate functions with these types
template int StreamComposite::StreamRx<lime::complex16_t>(lime::complex16_t** samples, uint32_t count, SDRDevice::StreamMeta* meta);
template int StreamComposite::StreamRx<lime::complex32f_t>(lime::complex32f_t** samples, uint32_t count, SDRDevice::StreamMeta* meta);
template int StreamComposite::StreamTx<lime::complex16_t>(const lime::complex16_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);
template int StreamComposite::StreamTx<lime::complex32f_t>(const lime::complex32f_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);

}
