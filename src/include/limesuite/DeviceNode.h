#pragma once

namespace lime
{

struct DeviceNode
{
    DeviceNode(){};
    DeviceNode(const std::string& name, const std::string& klass, void* ptr)
        : name(name), klass(klass), ptr(ptr) {}
    std::string name;
    std::string klass;
    void* ptr;
    std::vector<DeviceNode*> childs;
};

}
