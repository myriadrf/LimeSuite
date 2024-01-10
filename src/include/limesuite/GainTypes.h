#ifndef LIME_GAINTYPES_H
#define LIME_GAINTYPES_H

#include "limesuite/config.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace lime {

/// @brief Available gain types on the devices.
enum class eGainTypes : uint8_t {
    LNA, ///< Low Noise Amplifier
    LoopbackLNA,
    PGA, ///< Programmable Gain Amplifier
    TIA, ///< Trans Impedance Amplifier
    PAD,
    LoopbackPAD,
    IAMP,
    PA, ///< On-board Power Amplifier
    UNKNOWN
};

extern LIME_API const std::unordered_map<eGainTypes, const std::string> GAIN_TYPES_TEXT;
extern LIME_API const std::unordered_map<std::string, eGainTypes> STRING_TO_GAIN_TYPES;

} // namespace lime

#endif // LIME_GAINTYPES_H
