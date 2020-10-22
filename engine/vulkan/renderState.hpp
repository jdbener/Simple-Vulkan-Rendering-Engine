#pragma once

#include "common.hpp"

/// Class which stores all of the variables common to a graphics or compute pipeline
class VulkanState {
private:
    // State ID tracking for debugging purposes
    static uint16_t nextID;
    uint16_t _id;

public:
    // Struct storing the data needed for each framebuffer
    struct StateBuffer {
		vpp::CommandBuffer commandBuffer;
        vpp::Fence fence;
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

protected:
    // Used to store the memory of the vulkan logical device we create for this window
    std::unique_ptr<VulkDevice> _device = nullptr;

public:
    vpp::Pipeline pipeline;
    vpp::CommandPool commandPool;

public:
    // Track id initalization in the constructer
    VulkanState() : _id(nextID) { nextID++; }
    virtual ~VulkanState() {}

    /// Gets the id of this state
    const uint16_t id() const { return _id; }
    /// Gets the device stored in this state
    const VulkDevice& device() const { return *_device; }

    /// Bind an already existing custom pipeline
    void bindPipeline(vpp::Pipeline&&);

    /// Function which recordes to the buffers
    ///     Is automatically called after a pipeline is bound
    virtual bool rerecordCommandBuffers() = 0;
    /// Function to be called by the main loop every frame
    ///     Implementaion needs to handle the case where this object is no longer valid
    virtual bool mainLoop(uint64_t frame) = 0;
};

/// Class which stores all of the variables needed to render to the screen
class RenderState: public VulkanState {
public:
    struct RenderBuffer: public StateBuffer {
        vk::Image image {};
		vpp::ImageView imageView;
        //vpp::ViewableImage imageView;
		vpp::Framebuffer framebuffer;
		vpp::Semaphore acquired, finished;
    };
public:
    vpp::Surface surface;
    vpp::Swapchain swapchain;
    vpp::RenderPass renderPass;
    std::vector<RenderBuffer> renderBuffers;

protected:
    /// Creates a swapchain create info from the specified surface
    ///     Requires <surface> already be set
    vk::SwapchainCreateInfoKHR swapchainProperties(const vk::PhysicalDevice pd, const vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain = {});

public:
    using VulkanState::VulkanState;

    /// Gets the width and height of the swapchain.
    ///     Requires <surface> already be set.
    ///     If pd is ommitted uses the one bound to the <swapchain>
    vk::Extent2D swapchainExtent(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    /// Gets the image format of the swapchain.
    ///     Requires <surface> already be set.
    ///     If pd is ommitted uses the one bound to the <swapchain>
    vk::SurfaceFormatKHR swapchainFormat(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    /// Gets the presentation mode of the swapchain.
    ///     Requires <surface> already be set.
    ///     If pd is ommitted uses the one bound to the <swapchain>
    vk::PresentModeKHR swapchainPresentMode(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    /// Gets the number of images which can be rendering at once.
    ///     Requires <surface> already be set.
    ///     If pd is ommitted uses the one bound to the <swapchain>
    uint32_t swapchainImageCount(vk::PhysicalDevice pd = {}, bool ignoreCache = false);

    /// Bind a default pipeline based on the specified shader program
    void bindPipeline(vpp::ShaderProgram&&, nytl::Span<const vk::DescriptorSetLayout> layouts = {}, nytl::Span<const vk::PushConstantRange> ranges = {});

    /// Recreates the swapchain.
    ///  If a valid deviceInfo is passed in, the swapchain will be recreated with the new physical device.
    ///  If a special version of deviceInfo passed in from the constructor is found, it will let vpp pick a physical device.
    ///  If neither of these is the case, it will resize the swapchain to be the same size as the GLFW framebuffer.
    ///  Automatically recreates the render buffers when the swapchain is resized.
    void recreateSwapchain(DeviceCreateInfo deviceInfo = {}, vk::Extent2D = {});

    /// Helper to create a simple graphics focused renderpass
    void createGraphicsRenderPass(std::vector<vk::ImageLayout> colorAttachments = {vk::ImageLayout::colorAttachmentOptimal}, std::vector<vk::ImageLayout> inputAttachments = {});
    /// Function which sets up all of the data stored in the <renderBuffers>
    void recreateRenderBuffers();

    /// Function which recordes to the buffers
    ///     Is automatically called after a pipeline is bound
    virtual bool rerecordCommandBuffers();
    /// Function to be called by the main loop every frame
    ///     Implementaion needs to handle the case where this object is no longer valid
    ///     Automatically resizes the swapchain when it becomes outdated (ex window resized)
    virtual bool mainLoop(uint64_t frame);
};
