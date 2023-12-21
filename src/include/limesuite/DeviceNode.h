#pragma once

#include "DeviceNodeClass.h"

#include <memory>

namespace lime {

struct DeviceNode {
    DeviceNode(){};
    DeviceNode(const std::string& name, eDeviceNodeClass nodeClass, void* ptr)
        : name(name)
        , deviceNodeClass(nodeClass)
        , ptr(ptr)
    {
    }
    std::string name;
    eDeviceNodeClass deviceNodeClass;
    void* ptr;
    std::vector<std::shared_ptr<DeviceNode>> children;
};

} // namespace lime
