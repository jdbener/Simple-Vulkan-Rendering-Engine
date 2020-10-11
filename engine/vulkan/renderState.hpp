#pragma once

#include "common.hpp"

class RenderState {
public:
    // Struct storing the data needed for each framebuffer
    struct RenderBuffer {
		//bool valid {};
		//unsigned int id {};
		vk::Image image {};
		vk::CommandBuffer commandBuffer {};
		vpp::ImageView imageView;
        //vpp::ViewableImage imageView;
		vpp::Framebuffer framebuffer;
		vpp::Semaphore semaphore;
	};

    // Struct used when (re)creating the swapchain from partial device information
    struct DeviceCreateInfo {
        enum Valid {NO, YES, NO_INITAL};

        Valid valid = NO;
        vk::DeviceCreateInfo info;
        vk::PhysicalDevice device;

        DeviceCreateInfo() : valid(NO) {}
        DeviceCreateInfo(vk::DeviceCreateInfo _info, vk::PhysicalDevice _device) : valid(YES), info(_info), device(_device) {}
    };

private:
    // Used to store the memory of the vulkan logical device we create for this window
    std::unique_ptr<vpp::Device> _device = nullptr;

public:
    vpp::Surface surface;
    vpp::Swapchain swapchain;
    vpp::RenderPass renderPass;
    vpp::Pipeline pipeline;
    std::vector<RenderBuffer> renderBuffers;

public:
    const vpp::Device& device() { return *_device; }

    vk::Extent2D swapchainExtent(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    vk::SurfaceFormatKHR swapchainFormat(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    vk::PresentModeKHR swapchainPresentMode(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    uint32_t swapchainImageCount(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    vk::SwapchainCreateInfoKHR swapchainProperties(const vk::PhysicalDevice pd, const vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain = {});

    // Helper to create a simple graphics focused renderpass
    void createGraphicsRenderPass(std::vector<vk::ImageLayout> colorAttachments = {vk::ImageLayout::colorAttachmentOptimal}, std::vector<vk::ImageLayout> inputAttachments = {});
    void recreateSwapchain(DeviceCreateInfo deviceInfo = {}, vk::Extent2D = {});
    void recreateRenderBuffers();
};
