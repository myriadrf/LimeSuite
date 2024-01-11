#include <string>
#include <unordered_map>

#include "limesuite/GainTypes.h"
#include "limesuite/MemoryDevices.h"
#include "limesuite/MemoryRegions.h"

using namespace lime;

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

const std::unordered_map<eMemoryDevice, const std::string> lime::MEMORY_DEVICES_TEXT{
    { eMemoryDevice::FPGA_RAM, "FPGA RAM" },
    { eMemoryDevice::FPGA_FLASH, "FPGA FLASH" },
    { eMemoryDevice::EEPROM, "EEPROM" },
};

const std::unordered_map<std::string, eMemoryDevice> lime::STRING_TO_MEMORY_DEVICES = []() {
    std::unordered_map<std::string, eMemoryDevice> map;

    for (auto device : MEMORY_DEVICES_TEXT)
    {
        map[device.second] = device.first;
    }

    return map;
}();

const std::unordered_map<eMemoryRegion, const std::string> lime::MEMORY_REGIONS_TEXT{
    { eMemoryRegion::VCTCXO_DAC, "VCTCXO DAC (non-volatile)" },
};
