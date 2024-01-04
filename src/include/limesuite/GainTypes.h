#ifndef LIME_GAINTYPES_H
#define LIME_GAINTYPES_H

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

static const std::unordered_map<eGainTypes, const std::string> GAIN_TYPES_TEXT{
    { eGainTypes::LNA, "LNA" },
    { eGainTypes::LoopbackLNA, "LB_LNA" },
    { eGainTypes::PGA, "PGA" },
    { eGainTypes::TIA, "TIA" },
    { eGainTypes::PAD, "PAD" },
    { eGainTypes::LoopbackPAD, "LB_PAD" },
    { eGainTypes::IAMP, "IAMP" },
    { eGainTypes::PA, "PA" },
    { eGainTypes::UNKNOWN, "" },
};

} // namespace lime

#endif // LIME_GAINTYPES_H
