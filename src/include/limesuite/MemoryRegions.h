#ifndef LIME_MEMORYREGIONS_H
#define LIME_MEMORYREGIONS_H

#include "limesuite/config.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace lime {

enum class eMemoryRegion : uint8_t { VCTCXO_DAC, COUNT };

extern LIME_API const std::unordered_map<eMemoryRegion, std::string> MEMORY_REGIONS_TEXT;

} // namespace lime

#endif // LIME_MEMORYREGIONS_H
