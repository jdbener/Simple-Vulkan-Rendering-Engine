#include "renderState.hpp"
#include <map>

// Initialize the id list
uint16_t VulkanState::nextID = 0;

/// Bind an already existing custom pipeline
void VulkanState::bindPipeline(vpp::Pipeline&& _pipeline) {
    pipeline = std::move(_pipeline);

    // Once the pipeline is bound, re-record the command buffers
    rerecordCommandBuffers();
}

/// Set any custom steps which need to be recorded to the internal command buffer
///     The provided function will always be called right before the draw/compute call
///     Command buffers must be rerecorded when this is changed
void VulkanState::bindCustomCommandRecordingSteps(std::function<void (vpp::CommandBuffer&)> _new) {
    customCommandRecordingSteps = _new;
};


/// Gets the width and height of the swapchain.
///     Requires <surface> already be set
///     If pd is omitted uses the one bound to the <swapchain>
vk::Extent2D RenderState::swapchainExtent(vk::PhysicalDevice pd, bool ignoreCache){
    // Caching
    static std::unordered_map<vk::SurfaceKHR, vk::Extent2D> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    vk::SurfaceCapabilitiesKHR capabilities = vk::getPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface.vkHandle());

    // Default
    vk::Extent2D chosenExtent = capabilities.currentExtent;

    // Clamp the width and height within acceptable bounds if our window manager allows us to tweak the bounds.
    if(chosenExtent.width == UINT32_MAX){
        chosenExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, chosenExtent.width));
        chosenExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, chosenExtent.height));
    }

    // Update cache
    cache[surface] = chosenExtent;
    return chosenExtent;
}

/// Gets the image format of the swapchain.
///     Requires <surface> already be set
///     If pd is omitted uses the one bound to the <swapchain>
vk::SurfaceFormatKHR RenderState::swapchainFormat(vk::PhysicalDevice pd, bool ignoreCache){
    // Caching
    static std::unordered_map<vk::SurfaceKHR, vk::SurfaceFormatKHR> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    std::vector<vk::SurfaceFormatKHR> formats = vk::getPhysicalDeviceSurfaceFormatsKHR(pd, surface.vkHandle());

    // Default
    vk::SurfaceFormatKHR chosenFormat = formats[0];

    // Choose the format we want if available
    for(vk::SurfaceFormatKHR& format: formats)
        if(format.format == vk::Format::a8b8g8r8SrgbPack32 && format.colorSpace == vk::ColorSpaceKHR::srgbNonlinear){
            chosenFormat = format;
            break;
        }

    // Update cache
    cache[surface] = chosenFormat;
    return chosenFormat;
}

/// Gets the presentation mode of the swapchain.
///     Requires <surface> already be set.
///     If pd is omitted uses the one bound to the <swapchain>
vk::PresentModeKHR RenderState::swapchainPresentMode(vk::PhysicalDevice pd, bool ignoreCache){
    // Caching
    static std::unordered_map<vk::SurfaceKHR, vk::PresentModeKHR> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    std::vector<vk::PresentModeKHR> modes = vk::getPhysicalDeviceSurfacePresentModesKHR(pd, surface.vkHandle());

    // Default
    vk::PresentModeKHR chosenMode = vk::PresentModeKHR::fifo;

    // Choose the present mode we want if available
    for(vk::PresentModeKHR& mode: modes)
        // Mailbox will overwrite images in the presentation queue if we render too fast
        if(mode == vk::PresentModeKHR::mailbox){
            chosenMode = mode;
            break;
        }

    // Update cache
    cache[surface] = chosenMode;
    return chosenMode;
}

/// Gets the number of images which can be rendered at once.
///     Requires <surface> already be set
///     If pd is omitted uses the one bound to the <swapchain>
uint32_t RenderState::swapchainImageCount(vk::PhysicalDevice pd, bool ignoreCache){
    // Caching
    static std::unordered_map<vk::SurfaceKHR, uint32_t> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    vk::SurfaceCapabilitiesKHR capabilities = vk::getPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface.vkHandle());

    // +1 to avoid buffering
    uint32_t imageCount = capabilities.minImageCount + 1;

    // Clamp the image count (if it has a maximum)
    if(capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    // Update cache
    cache[surface] = imageCount;
    return imageCount;
}

/// Creates the pipeline create info for this state which can then be modified and bound
vpp::GraphicsPipelineInfo RenderState::createGraphicsPipelineInfo(vpp::ShaderProgram&& program, nytl::Span<const vk::DescriptorSetLayout> layouts, nytl::Span<const vk::PushConstantRange> ranges) {
    vk::PipelineLayout layout = vpp::PipelineLayout(device(), layouts, ranges).release();
    return {renderPass, layout, std::move(program)};
}

/// Bind a graphics pipeline based on the provided pipeline info
void RenderState::bindPipeline(vpp::GraphicsPipelineInfo& createInfo) {
    VulkanState::bindPipeline({device(), createInfo.info()});
    vk::destroyPipelineLayout(device(), createInfo.info().layout);
}

// Utility function used by recreate swapchain to pick the properties of the swapchain
//  NOTE: Referenced from https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
vk::SwapchainCreateInfoKHR RenderState::swapchainProperties(const vk::PhysicalDevice pd, const vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain){
    vk::SurfaceFormatKHR chosenFormat = swapchainFormat(pd, true);

    return {/* flags */ {},
        surface,
        swapchainImageCount(pd, true),
        chosenFormat.format,
        chosenFormat.colorSpace,
        swapchainExtent(pd, true),
        /* imageArrayLayers*/ 1,
        vk::ImageUsageBits::colorAttachment,
        vk::SharingMode::exclusive,
        /*queueFamilyIndexCount*/ 0,
        /*QueueFamilyIndices*/ nullptr,
        /* Pretransform */ vk::getPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface).currentTransform,
        vk::CompositeAlphaBitsKHR::opaque,
        swapchainPresentMode(pd, true),
        VK_TRUE,
        oldSwapchain};
}

// Recreates the swapchain
//  If a valid deviceInfo is passed in, the swapchain will be recreated with the new physical device
//  If a special version of deviceInfo passed in from the constructor is found, it will let vpp pick a physical device
//  If neither of these is the case, it will resize the swapchain to be the same size as the GLFW framebuffer
//  Automatically recreates the render buffers when the swapchain is resized
void RenderState::recreateSwapchain(DeviceCreateInfo deviceInfo, vk::Extent2D newSize){
    // We are given a new valid physical device from which we need to create a new Device
    if(deviceInfo.valid == DeviceCreateInfo::YES){
        dlg_info("State " + str(id()) + ": Creating swapchain from specified device");
        _device = std::make_unique<VulkDevice>(surface.vkInstance(), deviceInfo.device, deviceInfo.info);
        swapchain = vpp::Swapchain(device(), swapchainProperties(device().vkPhysicalDevice(), surface.vkHandle()));

    // We haven't been given a device, just pick the "best" one
    } else if(deviceInfo.valid == DeviceCreateInfo::NO_INITAL){
        dlg_info("State " + str(id()) + ": Creating swapchain, picking 'best' device.");
        const vpp::Queue* present;
        _device = std::make_unique<VulkDevice>(surface.vkInstance(), surface, present);
        swapchain = vpp::Swapchain(device(), swapchainProperties(device().vkPhysicalDevice(), surface.vkHandle()));

    // We just need to resize the swapchain
    } else {
        dlg_info("State " + str(id()) + ": Resizing swapchain");
        vk::SwapchainCreateInfoKHR properties = swapchainProperties(device().vkPhysicalDevice(), surface.vkHandle(), swapchain.vkHandle());
        swapchain.resize(newSize, properties);

        // Automatically recreate the render buffers
        recreateRenderBuffers();
    }
}

/// Helper to create a simple graphics focused renderpass
void RenderState::createGraphicsRenderPass(std::vector<vk::ImageLayout> _colorAttachments, std::vector<vk::ImageLayout> _inputAttachments){
    vk::AttachmentDescription attachment {/*flags*/ {}, swapchainFormat().format, vk::SampleCountBits::e1, vk::AttachmentLoadOp::clear, vk::AttachmentStoreOp::store, vk::AttachmentLoadOp::dontCare, vk::AttachmentStoreOp::dontCare,
        // Don't care what the pass looks like when we start, create a final pass suitable for screen presentation
        vk::ImageLayout::undefined, vk::ImageLayout::presentSrcKHR};

    uint32_t i = 0;
    std::vector<vk::AttachmentReference> colorAttachments;
    for(vk::ImageLayout layout: _colorAttachments) colorAttachments.push_back({i++, layout});
    std::vector<vk::AttachmentReference> inputAttachments;
    for(vk::ImageLayout layout: _inputAttachments) inputAttachments.push_back({i++, layout});

    vk::SubpassDescription subpass {/* flags */ {}, vk::PipelineBindPoint::graphics,
        (uint32_t) inputAttachments.size(), inputAttachments.data(),
        (uint32_t) colorAttachments.size(), colorAttachments.data(),
        /*resolveAttachments*/ nullptr, /*DepthStencilAttachment*/ nullptr,
        /*preserveAttachmentCount*/ 0, /*preserveAttachments*/ nullptr
    };

    renderPass = {
        device(), {/*flags*/ {},
        /* attachment*/ 1, &attachment,
        /* subpass*/ 1, &subpass,
        /* dependency */ 0, nullptr}
    };
}

/// Function which sets up all of the data stored in the <renderBuffers>
void RenderState::recreateRenderBuffers(){
    // Wait for everything currently queued to finish rendering
    device().waitIdle();
    // Invalidate the recordings in our command pool
    vk::resetCommandPool(device().vkDevice(), commandPool.vkHandle());

    // Resize the list of buffers to match the list of images
    std::vector<vk::Image> images = swapchain.images();
    renderBuffers.resize(images.size());

    for(size_t i = 0; i < images.size(); i++){
        // TODO: needed?
        // Save the image
        renderBuffers[i].image = images[i];
        // Create an image view
        renderBuffers[i].imageView = {device(), {/*flags*/ {}, images[i], vk::ImageViewType::e2d, swapchainFormat().format,
            // Mapping each color channel (rgba) to itself
            {vk::ComponentSwizzle::identity, vk::ComponentSwizzle::identity, vk::ComponentSwizzle::identity, vk::ComponentSwizzle::identity},
            // This view will be used for color, with no stereoscopic layers or mipmapping
            {vk::ImageAspectBits::color, 0, 1, 0, 1}
        }};

        // Recreate the framebuffer
        vk::Extent2D extent = swapchainExtent();
        renderBuffers[i].framebuffer = { device(), {/*flags*/ {}, renderPass, 1, &renderBuffers[i].imageView.vkHandle(), extent.width, extent.height, /*layers*/ 1} };

        // Allocate a command buffer if one doesn't yet exist
        if(!renderBuffers[i].commandBuffer) renderBuffers[i].commandBuffer = {commandPool, vk::CommandBufferLevel::primary};

        // Create a new semaphore if one doesn't yet exist
        if(!renderBuffers[i].acquired){
            renderBuffers[i].acquired = {device()};
            renderBuffers[i].finished = {device()};
            // Mark the fence as signaled so that we will bypass it on the first run
            renderBuffers[i].fence = {device(), {vk::FenceCreateBits::signaled}};
        }
    }
}

/// Function which records the command buffers
///     Is automatically called after a pipeline is bound
bool RenderState::rerecordCommandBuffers(){
    // Calculate the size and viewport
    vk::Extent2D extent = swapchainExtent();
    vk::Viewport viewport{0, 0, (float) extent.width, (float) extent.height, 0, 1};
    vk::Rect2D scissor{{0, 0}, {extent.width, extent.height}};
    // Specify the blank render color
    vk::ClearValue clearValue = {0, 0, 0, 1}; // full opacity black

    for(RenderBuffer& buffer: renderBuffers){
        vk::beginCommandBuffer(buffer.commandBuffer, {});
        defer(vk::endCommandBuffer(buffer.commandBuffer);, be) // Stop recording at end of loop

        vk::cmdBeginRenderPass(buffer.commandBuffer,
            {renderPass, buffer.framebuffer, {/*offset*/{0, 0}, extent}, 1, &clearValue}, vk::SubpassContents::eInline);
        defer(vk::cmdEndRenderPass(buffer.commandBuffer);, re) // End the render pass at end of loop

        vk::cmdBindPipeline(buffer.commandBuffer, vk::PipelineBindPoint::graphics, pipeline);
        vk::cmdSetViewport(buffer.commandBuffer, 0, 1, viewport);
        vk::cmdSetScissor(buffer.commandBuffer, 0, 1, scissor);

        // Any custom bindings (like vertex buffers)
        if(customCommandRecordingSteps) customCommandRecordingSteps(buffer.commandBuffer);

        vk::cmdDraw(buffer.commandBuffer, /*vertCount*/ 3, /*instanceCount*/ 1, /*firstVertex*/ 0, /*firstInstance*/ 0);
    }

    // If we made it this far nothing went wrong
    return true;
}

/// Function to be called by the main loop every frame
///     Implementation needs to handle the case where this object is no longer valid
///     Automatically resizes the swapchain when it becomes outdated (ex window resized)
bool RenderState::mainLoop(uint64_t frame){
    try{
        // Get the next image in the render queue
        uint32_t i = vk::acquireNextImageKHR(device().vkHandle(), swapchain.vkHandle(), /*timeout*/ UINT64_MAX, renderBuffers[frame % renderBuffers.size()].acquired.vkHandle(), {});

        // Wait for any previous rendering tasks on this image to finish
        device().waitForFence(renderBuffers[i].fence.vkHandle());

        // Render the image
        vk::PipelineStageFlags waitStage = vk::PipelineStageBits::colorAttachmentOutput;
        vk::queueSubmit(device().presentQueue()->vkHandle(), std::vector<vk::SubmitInfo>{ {1, &renderBuffers[frame % renderBuffers.size()].acquired.vkHandle(), &waitStage, 1, &renderBuffers[i].commandBuffer.vkHandle(), 1, &renderBuffers[i].finished.vkHandle() } }, renderBuffers[i].fence.vkHandle());
        // Once the image has been rendered put it into the swapchain's buffer
        vk::queuePresentKHR(device().presentQueue()->vkHandle(), {1, &renderBuffers[i].finished.vkHandle(), 1, &swapchain.vkHandle(), &i, nullptr});

        // Everything went fine
        return true;
    } catch (vk::VulkanError& e){
        // If we have an error saying the swapchain is the wrong size, recreate it
        if (e.error == vk::Result::errorOutOfDateKHR) {
            dlg_info("State " + str(id()) + ": Swapchain out of date, recreating");
            recreateSwapchain();
            rerecordCommandBuffers();
        // If we have an error we haven't handled, throw it further up
        } else
            throw e;
    }
    // If an exception was caught something didn't go right (but this wasn't fatal)
    return false;
}
