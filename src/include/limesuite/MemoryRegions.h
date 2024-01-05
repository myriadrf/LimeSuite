#ifndef LIME_MEMORYREGIONS_H
#define LIME_MEMORYREGIONS_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace lime {

enum class eMemoryRegion : uint8_t { VCTCXO_DAC, COUNT };

static const std::unordered_map<eMemoryRegion, const std::string> MEMORY_REGIONS_TEXT{
    { eMemoryRegion::VCTCXO_DAC, "VCTCXO DAC (non-volatile)" },
};

} // namespace lime

#endif // LIME_MEMORYREGIONS_H
