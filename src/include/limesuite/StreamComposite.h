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

    /// @brief Constructs the StreamComposite object.
    /// @param aggregate The list of streams to aggregate into one stream.
    StreamComposite(const std::vector<StreamAggregate>& aggregate);

    /// @brief Sets up the streams with the given configuration.
    /// @param config The configuration to set up the streams with.
    /// @return The status of the operation (0 on success).
    int StreamSetup(const SDRDevice::StreamConfig& config);

    /// @brief Starts all of the aggregated streams.
    void StreamStart();

    /// @brief Ends all of the aggregated streams.
    void StreamStop();

    /// @copydoc TRXLooper::StreamRx()
    /// @tparam T The type of streams to send.
    template<class T> uint32_t StreamRx(T** samples, uint32_t count, SDRDevice::StreamMeta* meta);

    /// @copydoc TRXLooper::StreamTx()
    /// @tparam T The type of streams to receive.
    template<class T> uint32_t StreamTx(const T* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);

  private:
    std::vector<SDRDevice::StreamConfig> SplitAggregateStreamSetup(const SDRDevice::StreamConfig& cfg);
    std::vector<StreamAggregate> mAggregate;
    std::vector<StreamAggregate> mActiveAggregates;
};

} // namespace lime