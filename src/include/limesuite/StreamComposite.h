#pragma once

#include <vector>
#include <memory>
#include "limesuite/config.h"
#include "limesuite/complex.h"
#include "limesuite/SDRDevice.h"

namespace lime {

struct LIME_API StreamAggregate {
    SDRDevice* device;
    std::vector<int32_t> channels;
    int32_t streamIndex;
};

/** @brief Class for managing streaming from multiple devices at the same time. */
class LIME_API StreamComposite
{
  public:
    StreamComposite() = delete;
    StreamComposite(const std::vector<StreamAggregate>& aggregate);

    OpStatus StreamSetup(const SDRDevice::StreamConfig& config);
    void StreamStart();
    void StreamStop();

    template<class T> int StreamRx(T** samples, uint32_t count, SDRDevice::StreamMeta* meta);
    template<class T> int StreamTx(const T* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);

  private:
    std::vector<SDRDevice::StreamConfig> SplitAggregateStreamSetup(const SDRDevice::StreamConfig& cfg);
    std::vector<StreamAggregate> mAggregate;
    std::vector<StreamAggregate> mActiveAggregates;
};

} // namespace lime