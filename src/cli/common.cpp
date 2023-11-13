#include "common.h"

using namespace lime;

SDRDevice* ConnectUsingNameHint(const std::string hintArguments)
{
    DeviceHandle handle(hintArguments);
    auto handles = DeviceRegistry::enumerate(handle);
    if (handles.size() > 1)
    {
        std::cerr << "Ambiguous device handle (" << handle.Serialize() << "), matches:\n";
        for (const auto& h : handles)
            std::cerr << "\t" << h.Serialize() << std::endl;
        return nullptr;
    }
    return DeviceRegistry::makeDevice(handle);
}
