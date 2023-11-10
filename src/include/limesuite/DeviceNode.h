#pragma once

#include <memory>

namespace lime {

struct DeviceNode {
    DeviceNode(){};
    DeviceNode(const std::string& name, const std::string& klass, void* ptr)
        : name(name)
        , klass(klass)
        , ptr(ptr)
    {
    }
    std::string name;
    std::string klass;
    void* ptr;
    std::vector<std::shared_ptr<DeviceNode>> children;
};

} // namespace lime
