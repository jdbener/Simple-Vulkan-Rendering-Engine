#include "window.hpp"

// GLFW
#include <GLFW/glfw3.h>

bool Window::glfwInitalized = false;
size_t Window::windowCount = 0;

// Callback which occurs when the window is closed, does the bookkeeping to
//  track its closure.
void _WindowCloseCallback(GLFWwindow* _window){
    Window* window = (Window*) glfwGetWindowUserPointer(_window);
    // Process any pending events
    Window::pollEvents();
    // Explicitly destroy the window
    window->destroy();
}

// Callback which updates the flag signifying that the main loop should pause while
//  minimized
void _WindowMinimizedCallback(GLFWwindow* _window, int minimized){
    Window* window = (Window*) glfwGetWindowUserPointer(_window);
    // Stop this window's rendering loop while the window is suspended, restart
    //  it when restored
    window->pauseLoop = minimized;
    if(window->pauseLoop) { dlg_info("Window '" + str(window->getName()) + "'s main loop suspended."); }
    else dlg_info("Window '" + str(window->getName()) + "'s main loop resumed.");
}

// Creates a new window with an attached vulkan rendering surface
Window::Window(vpp::Instance& instance, int width, int height, str _name, DeviceCreateInfo deviceInfo, std::vector<std::pair<int, int>> windowCreationHints) : name(_name) {
    // Initialize GLFW if necessary
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
    // Register the minimize callback
    glfwSetWindowIconifyCallback(window, &_WindowMinimizedCallback);
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

// Destroy the window
bool Window::destroy(){
    // Destroy the window if it exists
    if(!window) return false;
    glfwDestroyWindow(window);
    window = nullptr; // Note to the rest of the class that the window no longer exists
    windowCount--;

    return true;
}

// Forcibly changes the size of the window (similar to if the user had resized it)
void Window::setSize(int width, int height){
    if(!window) throw WindowNotFound(name);
    glfwSetWindowSize(window, width, height);
}

// Gets the total size of the window
//  The returned result is measured in screen coordinates and is not guaranteed
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

// Makes the window fullscreened.
//  If no monitor is specified the primary one is used.
//  If no mode is specified the window is considered windowed fullscreen and just
//      takes on the settings of the monitor it is attached to
void Window::makeFullscreen(Monitor monitor, const Monitor::VideoMode* mode){
    if(!window) throw WindowNotFound(name);
    // Save the dimensions of the old window
    savedWindowDimensions = getTotalSize();
    // Make the window fullscreened on the specified monitor
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

// Makes the window no longer fullscreened.
//  If width and height aren't specified, the size of the window before it became
//      fullscreened is used instead.
void Window::makeWindowed(int width, int height){
    if(!window) throw WindowNotFound(name);
    // If we haven't specified the requested dimensions
    if(width < 0 || height < 0){
        // Use the cached dimensions
        if(savedWindowDimensions) std::tie(width, height) = *savedWindowDimensions;
        // Or error if they haven't been set
        else throw std::runtime_error("Tried to unfullscreen a window '" + name + "' which was never fullscreened.");
    }

    // Clear out any saved dimensions
    savedWindowDimensions.reset();
    // Make the window windowed
    glfwSetWindowMonitor(window, nullptr, 0, 0, width, height, /* Don't care about refresh rate*/-1);
}

// Brings the window to the front and sets it as input focused
void Window::focus() const {
    if(!window) throw WindowNotFound(name);
    glfwFocusWindow(window);
}

// Requests that the user interact with the window
void Window::pingUser() const {
    if(!window) throw WindowNotFound(name);
    glfwRequestWindowAttention(window);
}

// Toggles weather or not the window is minimized
//  Signals that the main loop should stop running while minimized
void Window::minimize() {
    if(!window) throw WindowNotFound(name);
    int minimized = glfwGetWindowAttrib(window, GLFW_ICONIFIED);

    if(!minimized) glfwIconifyWindow(window);
    else restore();
}

// Toggles weather or not the window is maximized
void Window::maximize() {
    if(!window) throw WindowNotFound(name);
    int maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

    if(!maximized) glfwMaximizeWindow(window);
    else restore();
}

// Restores the window to its normal state if it is maximized or minimized
void Window::restore(){
    if(!window) throw WindowNotFound(name);
    glfwRestoreWindow(window);
}

// Recreates the swapchain
void Window::recreateSwapchain(){
    if(!window) throw WindowNotFound(name);

    vk::Extent2D newSize;
    auto [width, height] = getFrameSize();
    newSize.width = width; newSize.height = height;

    RenderState::recreateSwapchain({}, newSize);
}

// Returns true if the window has been destroyed
bool Window::isDestroyed() const {
    return window == nullptr;
}

// Alias of isDestroyed
bool Window::isClosed() const {
    return isDestroyed();
}

// Returns true if the window has been closed (by calling close or clicking the
//  X in the os.)
bool Window::shouldClose() const{
    return glfwWindowShouldClose(window);
}

// Close the window (the window will still exist until it goes out of scope or
//  is destroyed but can't be used any further.)
bool Window::close(){
    if(!window) return false;
    glfwWindowShouldClose(window);
    return true;
}

// ---------------------
// Statics
// ---------------------
// Process any window events and then continue
void Window::pollEvents(){
    if (!glfwInitalized) throw std::runtime_error("Can't process events before GLFW has been initialized!");
    glfwPollEvents();
}

// Wait for a window event to process and then continue
void Window::waitEvents(){
    if (!glfwInitalized) throw std::runtime_error("Can't process events before GLFW has been initialized!");
    glfwWaitEvents();
}

// Returns a list of vulkan extensions which need to be enabled to create
//  window surfaces
std::vector<const char*> Window::requiredVulkanExtensions(){
    // Initialize GLFW if necessary
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
bool Window::allWindowsClosed(){
    return windowCount == 0;
}
