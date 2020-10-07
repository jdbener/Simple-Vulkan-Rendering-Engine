#include "window.hpp"

// GLFW
#include <GLFW/glfw3.h>

bool Window::glfwInitalized = false;
size_t Window::windowCount = 0;
size_t Window::openCount = 0;

// Callback which occures when the window is closed, does the bookkeeping to
//  track its closure.
void _WindowCloseCallback(GLFWwindow* _window){
    Window* window = (Window*) glfwGetWindowUserPointer(_window);
    window->closed = true;
    Window::openCount--;
}

// Creates a new window with an attached vulkan rendering surface
Window::Window(vpp::Instance& instance, int width, int height, str _name, std::vector<std::pair<int, int>> windowCreationHints) : name(_name) {
    // Initalize GLFW if nessicary
    if(!glfwInitalized){
        dlg_info("Initalizing GLFW");
        if(!glfwInit()) throw std::runtime_error("Failed to initalize GLFW!");
        else glfwInitalized = true;
    }

    // Apply any window hints which were passed in.
    for(auto& [hint, value]: windowCreationHints)
        glfwWindowHint(hint, value);

    // No OpenGL context... we will create one with vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Creates the window
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if(window == nullptr) throw std::runtime_error("Failed to create window: " + name);
    else {
        windowCount++;
        openCount++;
    }

    // Give the window a reference to this class, so that callbacks can use it.
    glfwSetWindowUserPointer(window, this);
    // Register the close callback
    glfwSetWindowCloseCallback(window, &_WindowCloseCallback);
}

// Destroy the window when it goes out of scope
Window::~Window(){
    destroy();
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
    if(!window || closed) throw WindowNotFound(name);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return std::make_pair(width, height);
}

// Returns the current name of the window
std::string_view Window::getName() const{
    if(!window || closed) throw WindowNotFound(name);
    return name;
}

// Changes the name of the window
void Window::setName(str& _name){
    if(!window || closed) throw WindowNotFound(name);
    name = _name;
    glfwSetWindowTitle(window, name.c_str());
}

// Returns true if the window has been closed (by calling close or clicking the
//  X in the os.)
bool Window::shouldClose() const{
    return glfwWindowShouldClose(window);
}

// Swap the queued framebuffer with the currently visible framebuffer.
//  A.k.a show the next image.
void Window::swapBuffers(){
    if(!window || closed) throw WindowNotFound(name);

    glfwSwapBuffers(window);
}

// Returns true if the window has been destroyed
bool Window::isDestroyed() const {
    return window == nullptr;
}

// Returns true if the window has been closed
bool Window::isClosed() const{
    return closed;
}

// Close the window (the window will still exist until it goes out of scope or
//  is destroyed by can't be used any further.)
bool Window::close(){
    if(!window || closed) return false;
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

    // If this window wasn't close through another means, mark that it has been closed.
    if(!closed) openCount--;

    // If this was the last open window, shutdown GLFW
    if(windowCount <= 0){
        dlg_info("Terminating GLFW");
        glfwTerminate();
        glfwInitalized = false;
    }

    return true;
}

// ---------------------
// Statics
// ---------------------
// Process any window events and then continue
void Window::pollEvents(){
    if (!glfwInitalized) throw std::runtime_error("Can't poll events before GLFW has been initalized!");
    glfwPollEvents();
}

// Wait for a window event to process and then continue
void Window::waitEvents(){
    if (!glfwInitalized) throw std::runtime_error("Can't poll events before GLFW has been initalized!");
    glfwWaitEvents();
}

// Returns a list of vulkan extensions which need to be enabled to create
//  window surfaces
std::vector<const char*> Window::vulkanExtensions(){
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
    return openCount == 0;
}
