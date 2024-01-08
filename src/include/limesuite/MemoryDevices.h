#ifndef LIME_MEMORYDEVICES_H
#define LIME_MEMORYDEVICES_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace lime {

enum class eMemoryDevice : uint8_t { FPGA_RAM = 0, FPGA_FLASH, EEPROM, COUNT };

extern const std::unordered_map<eMemoryDevice, std::string> MEMORY_DEVICES_TEXT;
extern const std::unordered_map<std::string, eMemoryDevice> STRING_TO_MEMORY_DEVICES;

} // namespace lime

#endif // LIME_MEMORYDEVICES_H
