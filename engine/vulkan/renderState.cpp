#include "renderState.hpp"
#include <map>

vk::Extent2D RenderState::swapchainExtent(vk::PhysicalDevice pd, bool ignoreCache){
    static std::map<vk::SurfaceKHR, vk::Extent2D> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    vk::SurfaceCapabilitiesKHR capabilities = vk::getPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface.vkHandle());

    vk::Extent2D chosenExtent = capabilities.currentExtent;

    // Clamp the width and height with in acceptable bounds if our window manager allows us to tweak the bounds.
    if(chosenExtent.width == UINT32_MAX){
        chosenExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, chosenExtent.width));
        chosenExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, chosenExtent.height));
    }

    cache[surface] = chosenExtent;
    return chosenExtent;
}

vk::SurfaceFormatKHR RenderState::swapchainFormat(vk::PhysicalDevice pd, bool ignoreCache){
    static std::map<vk::SurfaceKHR, vk::SurfaceFormatKHR> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    std::vector<vk::SurfaceFormatKHR> formats = vk::getPhysicalDeviceSurfaceFormatsKHR(pd, surface.vkHandle());

    vk::SurfaceFormatKHR chosenFormat = formats[0];

    // Choose the format we want if available
    for(vk::SurfaceFormatKHR& format: formats)
        if(format.format == vk::Format::a8b8g8r8SrgbPack32 && format.colorSpace == vk::ColorSpaceKHR::srgbNonlinear){
            chosenFormat = format;
            break;
        }

    cache[surface] = chosenFormat;
    return chosenFormat;
}

vk::PresentModeKHR RenderState::swapchainPresentMode(vk::PhysicalDevice pd, bool ignoreCache){
    static std::map<vk::SurfaceKHR, vk::PresentModeKHR> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    std::vector<vk::PresentModeKHR> modes = vk::getPhysicalDeviceSurfacePresentModesKHR(pd, surface.vkHandle());

    if(!pd) pd = swapchain.vkPhysicalDevice();

    vk::PresentModeKHR chosenMode = vk::PresentModeKHR::fifo;

    // Choose the present mode we want if available
    for(vk::PresentModeKHR& mode: modes)
        // Mailbox will overwite images in the presentation queue if we render too fast
        if(mode == vk::PresentModeKHR::mailbox){
            chosenMode = mode;
            break;
        }

    cache[surface] = chosenMode;
    return chosenMode;
}

uint32_t RenderState::swapchainImageCount(vk::PhysicalDevice pd, bool ignoreCache){
    static std::map<vk::SurfaceKHR, uint32_t> cache;
    if(!ignoreCache && cache.find(surface) != cache.end()) return cache[surface];

    if(!pd) pd = swapchain.vkPhysicalDevice();
    vk::SurfaceCapabilitiesKHR capabilities = vk::getPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface.vkHandle());

    // +1 to avoid buffering
    uint32_t imageCount = capabilities.minImageCount + 1;

    // Clamp the image count (if it has a maximum)
    if(capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    cache[surface] = imageCount;
    return imageCount;
}

/// Bind an already existing custom pipeline
void RenderState::bindPipeline(vpp::Pipeline&& _pipeline) {
    pipeline = std::move(_pipeline);

    // Once the pipeline is bound, rerecord the command buffers
    rerecordCommandBuffers();
}

/// Bind a default pipeline based on the specified shader program
void RenderState::bindPipeline(vpp::ShaderProgram&& program, nytl::Span<const vk::DescriptorSetLayout> layouts, nytl::Span<const vk::PushConstantRange> ranges) {
    vpp::PipelineLayout layout(device(), layouts, ranges);
    vpp::GraphicsPipelineInfo pipelineInfo(renderPass, layout.vkHandle(), std::move(program));
    bindPipeline({device(), pipelineInfo.info()});
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
        dlg_info("Creating swapchain from specified device");
        _device = std::make_unique<VulkDevice>(surface.vkInstance(), deviceInfo.device, deviceInfo.info);
        swapchain = vpp::Swapchain(device(), swapchainProperties(device().vkPhysicalDevice(), surface.vkHandle()));

    // We haven't been given a device, just pick the "best" one
    } else if(deviceInfo.valid == DeviceCreateInfo::NO_INITAL){
        dlg_info("Creating swapchain, picking 'best' device.");
        const vpp::Queue* present;
        _device = std::make_unique<VulkDevice>(surface.vkInstance(), surface, present);
        swapchain = vpp::Swapchain(device(), swapchainProperties(device().vkPhysicalDevice(), surface.vkHandle()));

    // We just need to resize the swapchain
    } else {
        dlg_info("Resizing swapchain");
        vk::SwapchainCreateInfoKHR properties = swapchainProperties(device().vkPhysicalDevice(), surface.vkHandle(), swapchain.vkHandle());
        swapchain.resize(newSize, properties);

        // Automatically recreate the render buffers
        recreateRenderBuffers();
    }
}

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
        /* dependancy */ 0, nullptr}
    };
}

// Function which records the command buffer
//  This function will likely need to be overloaded to support more complex elements
void RenderState::recreateRenderBuffers(){
    // Invalidate the recordings in our command pool
    device().waitIdle();
    vk::resetCommandPool(device().vkDevice(), commandPool.vkHandle());

    // Resize the list of buffers to match the list of images
    std::vector<vk::Image> images = swapchain.images();
    renderBuffers.resize(images.size());

    for(size_t i = 0; i < images.size(); i++){
        RenderBuffer& buffer = renderBuffers[i];
        // Save the image
        buffer.image = images[i];
        // Create an image view
        buffer.imageView = {device(), {/*flags*/ {}, images[i], vk::ImageViewType::e2d, swapchainFormat().format,
            // Mapping each color channel (rgba) to itself
            {vk::ComponentSwizzle::identity, vk::ComponentSwizzle::identity, vk::ComponentSwizzle::identity, vk::ComponentSwizzle::identity},
            // This view will be used for color, with no steroscopic layers or mipmaping
            {vk::ImageAspectBits::color, 0, 1, 0, 1}
        }};

        // Recreate the framebuffer
        vk::Extent2D extent = swapchainExtent();
        buffer.framebuffer = { device(), {/*flags*/ {}, renderPass, 1, &buffer.imageView.vkHandle(), extent.width, extent.height, /*layers*/ 1} };

        // Allocate a command buffer if one doesn't yet exist
        if(!buffer.commandBuffer) buffer.commandBuffer = {commandPool, vk::CommandBufferLevel::primary};

        // Create a new semaphore if one doesn't yet exist
        if(!buffer.semaphore) buffer.semaphore = {device()};
    }
}

void RenderState::rerecordCommandBuffers(){
    vk::Extent2D extent = swapchainExtent();
    vk::Viewport viewport{0, 0, (float) extent.width, (float) extent.height, 0, 1};
    vk::Rect2D scissor{{0, 0}, {extent.width, extent.height}};
    for(RenderState::RenderBuffer& buffer: renderBuffers){
        vk::beginCommandBuffer(buffer.commandBuffer, {});
        defer(vk::endCommandBuffer(buffer.commandBuffer);, be) // Stop recording at end of loop

        vk::ClearValue clearValue = {0, 0, 0, 1}; // full opacity black
        vk::cmdBeginRenderPass(buffer.commandBuffer,
            {renderPass, buffer.framebuffer, {/*offset*/{0, 0}, extent}, 1, &clearValue}, vk::SubpassContents::eInline);
        defer(vk::cmdEndRenderPass(buffer.commandBuffer);, re) // End the render pass at end of loop

        vk::cmdBindPipeline(buffer.commandBuffer, vk::PipelineBindPoint::graphics, pipeline);
        vk::cmdSetViewport(buffer.commandBuffer, 0, 1, viewport);
        vk::cmdSetScissor(buffer.commandBuffer, 0, 1, scissor);
        vk::cmdDraw(buffer.commandBuffer, /*vertCount*/ 3, /*instanceCount*/ 1, /*firstVertex*/ 0, /*firstInstance*/ 0);
    }
}
