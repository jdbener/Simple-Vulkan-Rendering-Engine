#pragma once

#include "monitor.hpp"
#include "util/string.hpp"
#include "vulkan/common.hpp"
#include "vulkan/state.hpp"

class GLFWwindow;

class WindowNotFound: public std::runtime_error{
public:
    WindowNotFound(str name, str s = "") : std::runtime_error("") {
        if (s.empty()) *this = std::runtime_error("Window '" + name + "' not found: Already destroyed?");
        else *this = std::runtime_error("Window '" + name + "' not found: " + s);
    }

protected:
    WindowNotFound(std::runtime_error& o) : std::runtime_error(o) {}
    WindowNotFound(std::runtime_error&& o) : std::runtime_error(o) {}
};

class Window: public GraphicsState {
private:
    // Tracks if GLFW has been initialized
    static bool glfwInitalized;
    // Track the number of created windows
    static size_t windowCount;

    // Tracks the size to restore the window to if it is made fullscreen
    std::optional<std::pair<int, int>> savedWindowDimensions = {};
    // Tracks the name of the window
    str name;

    // Callbacks
    friend void _WindowCloseCallback(GLFWwindow*);
    friend void _WindowMinimizedCallback(GLFWwindow*, int minimized);

protected:
    /// Pointer to the GLFW window
    GLFWwindow* window = nullptr;
    /// Tracks weather or not the main loop should be paused
    bool pauseLoop = false;

    /// Destroy the window
    bool destroy();

public:
    /// Creates a new window with an attached vulkan rendering surface
    Window(vpp::Instance&, int width = 800, int height = 600, str name = "Project Delta", GraphicsState::DeviceCreateInfo deviceInfo = {}, std::vector<std::pair<int, int>> windowCreationHints = {});
    ~Window();

    // TODO: Window icon

    /// Forcibly changes the size of the window (similar to if the user had resized it)
    void setSize(int width, int height);
    /// Gets the total size of the window.
    ///  The returned result is measured in screen coordinates and is not guaranteed.
    ///  to match the pixel value.
    std::pair<int, int> getTotalSize() const;
    /// Gets the size of our framebuffer (the size of the image we need to create).
    ///   The returned result is measure in pixels
    std::pair<int, int> getFrameSize() const;
    /// Gets the position on the monitor of the window
    std::pair<int, int> getPosition() const;
    /// Changes the name of the window
    void setName(str&);
    void setName(str&& s) { setName(s); }
    /// Returns the current name of the window
    std::string_view getName() const;
    /// Returns the monitor the window is (most) on
    Monitor getCurrentMonitor();

    /// Makes the window fullscreened
    ///  If no monitor is specified the primary one is used
    ///  If no mode is specified the window is considered windowed fullscreen and just
    ///      takes on the settings of the monitor it is attached to
    void makeFullscreen(Monitor monitor = Monitor::Current, const Monitor::VideoMode* mode = &Monitor::VideoMode::Current);
    /// Makes the window no longer fullscreened.
    ///  If width and height aren't specified, the size of the window before it became
    ///      fullscreened is used instead.
    void makeWindowed(int width = -1, int height = -1);
    void makeWindowed(std::pair<int, int> dimensions = {-1, -1}) { makeWindowed(dimensions.first, dimensions.second); }

    /// Brings the window to the front and sets it as input focused.
    ///     Can be annoying so avoid using.
    void focus() const;
    /// Requests that the user interact with the window
    void pingUser() const;
    /// Toggles weather or not the window is minimized
    ///  Signals that the main loop should stop running while minimized
    void minimize();
    /// Toggles weather or not the window is maximized
    void maximize();
    /// Restores the window to its normal state if it is maximized or minimized
    void restore();

    void recreateSwapchain();
    // Override to the main loop function which abandons the loop if the window has already been closed or minimized
    virtual bool mainLoop(uint64_t frame) { if(!window || pauseLoop) return false; return GraphicsState::mainLoop(frame); }

    /// Returns true if the window has been destroyed
    bool isDestroyed() const;
    /// Returns true if the window has been destroyed
    bool isClosed() const;
    /// Returns true if the window has been closed (by calling close or clicking the
    ///  X in the os.)
    bool shouldClose() const;
    /// Close the window (the window will still exist until it goes out of scope or
    ///  is destroyed but can't be used any further.)
    bool close();

public:
    /// Process any pending window events and then continue
    static void pollEvents();
    /// Wait for a window event to process and then continue
    static void waitEvents();

    /// Returns a list of vulkan extensions which need to be enabled to create
    ///  window surfaces
    static std::vector<const char*> requiredVulkanExtensions();

    /// Returns true if all windows which have been opened so far have been closed.
    static bool allWindowsClosed();

    /// Closes out all connections to the window manager
    ///     Must be called after all window classes have finished being destroyed.
    static void terminate();
};
