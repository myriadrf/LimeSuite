#ifndef LIME_GAINTYPES_H
#define LIME_GAINTYPES_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace lime {

enum class eGainTypes : uint8_t {
    LNA, ///< Low Noise Amplifier
    PGA, ///< Programmable Gain Amplifier
    TIA, ///< Trans Impedance Amplifier
    PAD,
    IAMP,
    PA, ///< On-board Power Amplifier
};

static const std::unordered_map<eGainTypes, std::string> GAIN_TYPES_TEXT{
    { eGainTypes::LNA, "LNA" },
    { eGainTypes::PGA, "PGA" },
    { eGainTypes::TIA, "TIA" },
    { eGainTypes::PAD, "PAD" },
    { eGainTypes::IAMP, "IAMP" },
    { eGainTypes::PA, "PA" },
};

} // namespace lime

#endif // LIME_GAINTYPES_H