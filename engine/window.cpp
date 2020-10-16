#include "window.hpp"

// GLFW
#include <GLFW/glfw3.h>

bool Window::glfwInitalized = false;
size_t Window::windowCount = 0;

// Callback which occures when the window is closed, does the bookkeeping to
//  track its closure.
void _WindowCloseCallback(GLFWwindow* _window){
    Window* window = (Window*) glfwGetWindowUserPointer(_window);
    // Process any pending events
    Window::pollEvents();
    window->destroy();
}

// Callback which occures when the window is resized, resizes the swapchain to
//  match the new size
void _WindowResizeCallback(GLFWwindow* _window, int, int){
    Window* window = (Window*) glfwGetWindowUserPointer(_window);
    window->recreateSwapchain();
    window->rerecordCommandBuffers();
}

// Creates a new window with an attached vulkan rendering surface
Window::Window(vpp::Instance& instance, int width, int height, str _name, DeviceCreateInfo deviceInfo, std::vector<std::pair<int, int>> windowCreationHints) : name(_name) {
    // Initalize GLFW if nessicary
    if(!glfwInitalized){
        dlg_info("Initalizing GLFW");
        if(!glfwInit()) throw std::runtime_error("Failed to initalize GLFW!");
        else glfwInitalized = true;
    }

    // Rest the window hints back to default, (don't bring any settings from the
    //  previous windows over to this one.)
    glfwDefaultWindowHints ();

    // Apply any window hints which were passed in.
    for(auto& [hint, value]: windowCreationHints)
        glfwWindowHint(hint, value);

    // No OpenGL context... we will create one with vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Creates the window
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if(window == nullptr) throw std::runtime_error("Failed to create window: " + name);
    else windowCount++;

    // Create the vulkan surface for us to render too
    VkSurfaceKHR surf;
    if(auto result = (vk::Result) glfwCreateWindowSurface((VkInstance) instance.vkHandle(), window, nullptr, &surf);
      result != vk::Result::success)
        throw std::runtime_error("Failed to create window '" + name + "' surface: " + vk::name(result));
    // Convert it to a vpp surface
    surface = vpp::Surface(instance, (vk::SurfaceKHR) surf);

    // Create the swapchain which will feed images to the surface
    if(deviceInfo.valid == RenderState::DeviceCreateInfo::NO) deviceInfo.valid = RenderState::DeviceCreateInfo::NO_INITAL;
    RenderState::recreateSwapchain(deviceInfo);

    // Create a command pool for this window, from the queue with support for both graphics and transfer
    try {
        commandPool = vpp::CommandPool(device(), {{}, device().presentQueueExcept()->family()});
    // Falls back to creating from the queue with graphics support
    } catch (vk::VulkanError& e){
        dlg_warn("Falling back to graphics queue for window '" + name + "'");
        commandPool = vpp::CommandPool(device(), {{}, device().graphicsQueue()->family()});
    }

    // Create a renderpass for the window with a single color attachment which will be drawn to the surface
    RenderState::createGraphicsRenderPass({vk::ImageLayout::colorAttachmentOptimal});

    // Create the objects we need to render each frame
    RenderState::recreateRenderBuffers();

    // Give the window a reference to this class, so that callbacks can use it.
    glfwSetWindowUserPointer(window, this);
    // Register the close callback
    glfwSetWindowCloseCallback(window, &_WindowCloseCallback);
    // Register the resize callback
    glfwSetWindowSizeCallback(window, &_WindowResizeCallback);
}

// Destroy the window when it goes out of scope
Window::~Window(){
    destroy();

    // If this was the last open window, shutdown GLFW (if not already closed)
    if(windowCount <= 0 && glfwInitalized){
        dlg_info("Terminating GLFW");
        glfwInitalized = false;
        glfwTerminate();
    }
}

// Forcibly changes the size of the window (similar to if the user had resized it)
void Window::setSize(int width, int height){
    if(!window) throw WindowNotFound(name);
    glfwSetWindowSize(window, width, height);
}

// Gets the total size of the window
//  The returned result is measured in screen choordinates and is not garunteed
//  to match the pixel value.
std::pair<int, int> Window::getTotalSize() const{
    if(!window) throw WindowNotFound(name);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return std::make_pair(width, height);
}

// Gets the size of our framebuffer (the size of the image we need to create)
//   The returned result is measure in pixels
std::pair<int, int> Window::getFrameSize() const {
    if(!window) throw WindowNotFound(name);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return std::make_pair(width, height);
}

// Returns the current name of the window
std::string_view Window::getName() const{
    if(!window) throw WindowNotFound(name);
    return name;
}

// Changes the name of the window
void Window::setName(str& _name){
    if(!window) throw WindowNotFound(name);
    name = _name;
    glfwSetWindowTitle(window, name.c_str());
}

// Recreates the swapchain
void Window::recreateSwapchain(){
    if(!window) throw WindowNotFound(name);

    vk::Extent2D newSize;
    auto [width, height] = getFrameSize();
    newSize.width = width; newSize.height = height;

    RenderState::recreateSwapchain({}, newSize);
}

// Swap the queued framebuffer with the currently visible framebuffer.
//  A.k.a show the next image.
// void Window::swapBuffers(){
//     if(!window) throw WindowNotFound(name);
//
//     glfwSwapBuffers(window);
// }

// Returns true if the window has been closed (by calling close or clicking the
//  X in the os.)
bool Window::shouldClose() const{
    return glfwWindowShouldClose(window);
}

// Returns true if the window has been destroyed
bool Window::isDestroyed() const {
    return window == nullptr;
}

// Alias of isDestroyed
bool Window::isClosed() const {
    return isDestroyed();
}

// Close the window (the window will still exist until it goes out of scope or
//  is destroyed by can't be used any further.)
bool Window::close(){
    if(!window) return false;
    glfwWindowShouldClose(window);
    return true;
}

// Destroy the window
bool Window::destroy(){
    // Destroy the window if it exists
    if(!window) return false;
    glfwDestroyWindow(window);
    window = nullptr; // Note to the rest of the class that the window no longer exists
    windowCount--;

    return true;
}

// ---------------------
// Statics
// ---------------------
// Process any window events and then continue
void Window::pollEvents(){
    if (!glfwInitalized) throw std::runtime_error("Can't process events before GLFW has been initalized!");
    glfwPollEvents();
}

// Wait for a window event to process and then continue
void Window::waitEvents(){
    if (!glfwInitalized) throw std::runtime_error("Can't process events before GLFW has been initalized!");
    glfwWaitEvents();
}

// Returns a list of vulkan extensions which need to be enabled to create
//  window surfaces
std::vector<const char*> Window::requiredVulkanExtensions(){
    // Initalize GLFW if nessicary
    if(!glfwInitalized){
        dlg_info("Initalizing GLFW");
        if(!glfwInit()) throw std::runtime_error("Failed to initalize GLFW!");
        else glfwInitalized = true;
    }

    uint32_t count;
    const char** data = glfwGetRequiredInstanceExtensions(&count);
    return {data, data + count};
}

// Returns true if all windows which have been opened so far have been closed.
//  Intenteded to be used to determine when the main loop should end.
bool Window::allWindowsClosed(){
    return windowCount == 0;
}
