#pragma once

#include "engine/util/string.hpp"

// Vulkan
#include <vpp/vpp.hpp>

std::vector<const char*> validateInstanceLayers(std::vector<const char*> layers);
std::vector<const char*> validateInstanceExtensions(std::vector<const char*> extensions);

// Small utility to add some extra functionality to devices
class VulkDevice: public vpp::Device {
public:
    using vpp::Device::Device;
    //using vpp::Device::operator=;

    void waitIdle() const { vk::deviceWaitIdle(vkHandle()); }

    void waitForFence(vk::Fence fence, uint64_t timeout = UINT64_MAX) const { waitForFences({&fence, 1}, false, timeout); }
    void waitForFences(nytl::span<vk::Fence> fences, bool waitAll = true, uint64_t timeout = UINT64_MAX) const {
        vk::waitForFences(vkHandle(), fences, (waitAll ? VK_TRUE : VK_FALSE), timeout);
        vk::resetFences(vkHandle(), fences); // Reset the fence so that we don't just automatically bypass it
    }

// Macro which defines how exception checking works
#define exceptFunction(func) auto out = func();\
    if(!out) throw vk::VulkanError(vk::Result::errorInitializationFailed, "Failed to find the specified " #func " queue!");\
    return out
    /// Returns the graphics queue.
    const vpp::Queue* graphicsQueue() const { return queue(vk::QueueBits::graphics); }
    /// Returns the graphics queue.
    ///     Throws a VulkanError if it can't be found
    const vpp::Queue* graphicsQueueExcept() const { exceptFunction(graphicsQueue); }
    /// Returns the presentation queue.
    const vpp::Queue* presentQueue() const { return queue(vk::QueueBits::graphics | vk::QueueBits::transfer); }
    /// Returns the presentation queue.
    ///     Throws a VulkanError if it can't be found
    const vpp::Queue* presentQueueExcept() const { exceptFunction(presentQueue); }
    /// Returns the transfer queue.
    const vpp::Queue* transferQueue() const { return queue(vk::QueueBits::transfer); }
    /// Returns the transfer queue.
    ///     Throws a VulkanError if it can't be found
    const vpp::Queue* transferQueueExcept() const { exceptFunction(transferQueue); }
    /// Returns the compute queue.
    const vpp::Queue* computeQueue() const { return queue(vk::QueueBits::compute); }
    /// Returns the compute queue.
    ///     Throws a VulkanError if it can't be found
    const vpp::Queue* computeQueueExcept() const { exceptFunction(computeQueue); }
};
