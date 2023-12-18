#ifndef LIME_MEMORYDEVICES_H
#define LIME_MEMORYDEVICES_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace lime {

enum class eMemoryDevice : uint8_t { FPGA_RAM = 0, FPGA_FLASH, EEPROM, COUNT };

static const std::unordered_map<eMemoryDevice, std::string> MEMORY_DEVICES_TEXT{
    { eMemoryDevice::FPGA_RAM, "FPGA RAM" },
    { eMemoryDevice::FPGA_FLASH, "FPGA FLASH" },
    { eMemoryDevice::EEPROM, "EEPROM" },
};

static const auto STRING_TO_MEMORY_DEVICES = []() {
    std::unordered_map<std::string, eMemoryDevice> map;

    for (auto device : MEMORY_DEVICES_TEXT)
    {
        map[device.second] = device.first;
    }

    return map;
}();

} // namespace lime

#endif // LIME_MEMORYDEVICES_H
