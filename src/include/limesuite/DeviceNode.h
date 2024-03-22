#pragma once

#include "DeviceNodeClass.h"

#include <memory>
#include <string>
#include <vector>

namespace lime {

/// @brief Structure describing a device node in the device node tree.
struct DeviceNode {
    /// @brief Default constructor for the node.
    DeviceNode(){};

    /// @brief The constructor for the device node.
    /// @param name The name of the node.
    /// @param nodeClass The device class of the node.
    /// @param ptr The pointer to the device.
    DeviceNode(const std::string& name, eDeviceNodeClass nodeClass, void* ptr)
        : name(name)
        , deviceNodeClass(nodeClass)
        , ptr(ptr)
    {
    }
    std::string name; ///< The name of the node.
    eDeviceNodeClass deviceNodeClass; ///< The device class of the node.
    void* ptr; ///< The pointer to the device.
    std::vector<std::shared_ptr<DeviceNode>> children; ///< The children of this node in the device tree.
};

} // namespace lime
