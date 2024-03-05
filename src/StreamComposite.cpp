#include "limesuite/StreamComposite.h"
#include <assert.h>
namespace lime {

StreamComposite::StreamComposite(const std::vector<StreamAggregate>& aggregate)
{
    mAggregate = aggregate;
}

OpStatus StreamComposite::StreamSetup(const SDRDevice::StreamConfig& config)
{
    mActiveAggregates.clear();
    SDRDevice::StreamConfig subConfig = config;

    std::size_t rxNeed = config.channels.at(TRXDir::Rx).size();
    std::size_t txNeed = config.channels.at(TRXDir::Tx).size();

    for (auto& aggregate : mAggregate)
    {
        subConfig.channels.at(TRXDir::Rx).clear();
        subConfig.channels.at(TRXDir::Tx).clear();
        assert(aggregate.device);
        const SDRDevice::Descriptor& desc = aggregate.device->GetDescriptor();
        std::size_t aggregateChannelCount = aggregate.channels.size();

        std::size_t channelCount = std::min(aggregateChannelCount, rxNeed);
        for (std::size_t j = 0; j < channelCount; ++j)
        {
            if (aggregate.channels[j] >= desc.rfSOC[aggregate.streamIndex].channelCount)
                return OpStatus::OUT_OF_RANGE;

            subConfig.channels.at(TRXDir::Rx).push_back(aggregate.channels[j]);
        }
        rxNeed -= channelCount;

        channelCount = std::min(aggregateChannelCount, txNeed);
        for (std::size_t j = 0; j < channelCount; ++j)
        {
            if (aggregate.channels[j] >= desc.rfSOC[aggregate.streamIndex].channelCount)
                return OpStatus::OUT_OF_RANGE;

            subConfig.channels.at(TRXDir::Tx).push_back(aggregate.channels[j]);
        }
        txNeed -= channelCount;

        OpStatus status = aggregate.device->StreamSetup(subConfig, aggregate.streamIndex);
        if (status != OpStatus::SUCCESS)
            return status;

        mActiveAggregates.push_back(aggregate);

        if (rxNeed == 0 && txNeed == 0)
        {
            break;
        }
    }
    return OpStatus::SUCCESS;
}

void StreamComposite::StreamStart()
{
    std::unordered_map<SDRDevice*, std::vector<uint8_t>> groups;
    for (auto& a : mActiveAggregates)
        groups[a.device].push_back(a.streamIndex);
    for (auto& g : groups)
        g.first->StreamStart(g.second);
}

void StreamComposite::StreamStop()
{
    std::unordered_map<SDRDevice*, std::vector<uint8_t>> groups;
    for (auto& a : mActiveAggregates)
        groups[a.device].push_back(a.streamIndex);
    for (auto& g : groups)
        g.first->StreamStop(g.second);
}

template<class T> int StreamComposite::StreamRx(T** samples, uint32_t count, SDRDevice::StreamMeta* meta)
{
    T** dest = samples;
    for (auto& a : mActiveAggregates)
    {
        int ret = a.device->StreamRx(a.streamIndex, dest, count, meta);
        if (ret != static_cast<int>(count))
            return ret;

        dest += a.channels.size();
    }
    return static_cast<int>(count);
}

template<class T> int StreamComposite::StreamTx(const T* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta)
{
    const T* const* src = samples;
    for (auto& a : mActiveAggregates)
    {
        int ret = a.device->StreamTx(a.streamIndex, src, count, meta);
        if (ret != static_cast<int>(count))
            return ret;

        src += a.channels.size();
    }
    return static_cast<int>(count);
}

// force instantiate functions with these types
template LIME_API int StreamComposite::StreamRx<lime::complex16_t>(
    lime::complex16_t** samples, uint32_t count, SDRDevice::StreamMeta* meta);
template LIME_API int StreamComposite::StreamRx<lime::complex32f_t>(
    lime::complex32f_t** samples, uint32_t count, SDRDevice::StreamMeta* meta);
template LIME_API int StreamComposite::StreamTx<lime::complex16_t>(
    const lime::complex16_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);
template LIME_API int StreamComposite::StreamTx<lime::complex32f_t>(
    const lime::complex32f_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);

} // namespace lime
