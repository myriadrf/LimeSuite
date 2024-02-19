#include <string>
#include <type_traits>
#include <unordered_map>

#include "limesuite/GainTypes.h"
#include "limesuite/MemoryDevices.h"
#include "limesuite/MemoryRegions.h"

using namespace lime;

template<class OriginalKey, class OriginalValue>
const std::unordered_map<std::remove_cv_t<OriginalValue>, OriginalKey> SwapKeysAndValues(
    const std::unordered_map<OriginalKey, OriginalValue>& source)
{
    std::unordered_map<std::remove_cv_t<OriginalValue>, OriginalKey> map;

    for (const auto& pair : source)
    {
        map[pair.second] = pair.first;
    }

    return map;
}

const std::unordered_map<eGainTypes, const std::string> lime::GAIN_TYPES_TEXT{
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
const std::unordered_map<std::string, eGainTypes> lime::STRING_TO_GAIN_TYPES = SwapKeysAndValues(GAIN_TYPES_TEXT);

const std::unordered_map<eMemoryDevice, const std::string> lime::MEMORY_DEVICES_TEXT{
    { eMemoryDevice::FPGA_RAM, "FPGA RAM" },
    { eMemoryDevice::FPGA_FLASH, "FPGA FLASH" },
    { eMemoryDevice::EEPROM, "EEPROM" },
};
const std::unordered_map<std::string, eMemoryDevice> lime::STRING_TO_MEMORY_DEVICES = SwapKeysAndValues(MEMORY_DEVICES_TEXT);

const std::unordered_map<eMemoryRegion, const std::string> lime::MEMORY_REGIONS_TEXT{
    { eMemoryRegion::VCTCXO_DAC, "VCTCXO DAC (non-volatile)" },
};
