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
    // Function pointer which stores a reference to extra command buffer recording steps
    std::function<void (vpp::CommandBuffer&)> customCommandRecordingSteps = {};

public:
    vpp::CommandPool commandPool;

public:
    // Track id initialization in the constructor
    VulkanState() : _id(nextID) { nextID++; }
    virtual ~VulkanState() {}

    /// Gets the id of this state
    const uint16_t id() const { return _id; }
    /// Gets the device stored in this state
    const VulkDevice& device() const { return *_device; }

    /// Set any custom steps which need to be recorded to the internal command buffer
    ///     The provided function will always be called right before the draw/compute call
    ///     Command buffers must be rerecorded when this is changed
    void bindCustomCommandRecordingSteps(std::function<void (vpp::CommandBuffer&)> _new);

    /// Function which records to the buffers.
    ///     Is automatically called after a pipeline is bound
    virtual bool rerecordCommandBuffers() = 0;
    /// Function to be called by the main loop every frame.
    ///     Implementation needs to handle the case where this object is no longer valid
    virtual bool mainLoop(uint64_t frame) = 0;


    /// Copies the provided data into the given buffer through a stagging buffer.
    ///     Submits the operation on the device attached to the provided state.
    ///     The input buffer must be marked as a vk::BufferUsageBits::transferDst.
    ///     If flagged to wait for the opperation to finish, it will do so; otherwise
    ///         it will return the submition ID for future syncronization.
    ///     If a command buffer is provided, that buffer will be overwritten and used;
    ///         otherwise a new buffer will be allocated and then destroyed.
    ///     The provided command buffer will never be destroyed.
    ///     If a command buffer is generated, the wait command will destroy it,
    ///         non waiting will not. (This is a memory leak! Externally manage the buffer!)
    template <typename T>
    uint64_t fillStaging(vpp::BufferSpan buffer, nytl::span<T> data, const bool wait = true, std::optional<std::reference_wrapper<vpp::CommandBuffer>> cb = {}){
        // Variable which stores the internal copy of the command buffer
        vpp::CommandBuffer internalCB;

        // At the end of the function, if we should release the command buffer (we
        //  didn't create it and thus shouldn't destroy it), release it
        // WARNING: WATCH THIS FOR BUGS
        bool release = cb.has_value();
        defer(if(release) internalCB.release();, cr);

        // Create a command buffer if one wasn't provided
        if(!cb) internalCB = commandPool.allocate(); // Invalid?
        // Or create a copy which won't touch the original if one was provided
        else internalCB = {cb->get().device(), cb->get().commandPool(), cb->get().vkHandle()};

        // Record the command buffer
        vk::beginCommandBuffer(internalCB, {});
        vpp::SubBuffer stagingBuff = vpp::fillStaging(internalCB, buffer, nytl::span<std::byte>{(std::byte*) data.data(), data.size() * sizeof(data[0])});
        vk::endCommandBuffer(internalCB);

        // Add the buffer to the submittion queue...
        vpp::QueueSubmitter& submitter = device().queueSubmitter();
        uint64_t out = submitter.add(internalCB);
        // And submit it then wait for it to finish...
        if(wait) return submitter.wait(out);

        // Or just submit it
        submitter.submit(out);
        // And release the internal reference (it can't be freed since the queue is
        //     running so just wait for the commandpool to clean it up.)
        release = true;
        return out;
    }
    template <typename T>
    uint64_t fillStaging(vpp::BufferSpan buffer, std::vector<T>& data, const bool wait = true, std::optional<std::reference_wrapper<vpp::CommandBuffer>> cb = {})
    { return fillStaging(buffer, nytl::span{data}, wait, std::move(cb)); }
};

/// Class which stores all of the variables needed to render to the screen
class GraphicsState: public VulkanState {
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
    /// Creates a swapchain create info from the specified surface.
    ///     Requires <surface> already be set
    vk::SwapchainCreateInfoKHR swapchainProperties(const vk::PhysicalDevice pd, const vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain = {});

public:
    using VulkanState::VulkanState;

    /// Gets the width and height of the swapchain.
    ///     Requires <surface> already be set.
    ///     If pd is omitted uses the one bound to the <swapchain>
    vk::Extent2D swapchainExtent(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    /// Gets the image format of the swapchain.
    ///     Requires <surface> already be set.
    ///     If pd is omitted uses the one bound to the <swapchain>
    vk::SurfaceFormatKHR swapchainFormat(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    /// Gets the presentation mode of the swapchain.
    ///     Requires <surface> already be set.
    ///     If pd is omitted uses the one bound to the <swapchain>
    vk::PresentModeKHR swapchainPresentMode(vk::PhysicalDevice pd = {}, bool ignoreCache = false);
    /// Gets the number of images which can be rendered at once.
    ///     Requires <surface> already be set.
    ///     If pd is omitted uses the one bound to the <swapchain>
    uint32_t swapchainImageCount(vk::PhysicalDevice pd = {}, bool ignoreCache = false);

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

    /// Function which records to the command buffers
    ///     Is automatically called after a pipeline is bound
    virtual bool rerecordCommandBuffers();
    /// Function to be called by the main loop every frame
    ///     Implementation needs to handle the case where this object is no longer valid
    ///     Automatically resizes the swapchain when it becomes outdated (ex window resized)
    virtual bool mainLoop(uint64_t frame);
};
