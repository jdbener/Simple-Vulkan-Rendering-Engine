#pragma once

#include "util/string.hpp"
#include "vulkan/common.hpp"
#include "vulkan/renderState.hpp"

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

class Window: public RenderState {
private:
    // Tracks if GLFW has been initalized
    static bool glfwInitalized;
    // Track the number of created windows
    static size_t windowCount;
    str name;

    friend void _WindowCloseCallback(GLFWwindow*);

protected:
    GLFWwindow* window = nullptr;

    bool destroy();

public:
    Window(vpp::Instance&, int width = 800, int height = 600, str name = "Project Delta", RenderState::DeviceCreateInfo deviceInfo = {}, std::vector<std::pair<int, int>> windowCreationHints = {});
    ~Window();

    // TODO: Window icon
    // TODO: Make fullscreen
    // TODO: Focus, demand attention, minimize, maximize

    void setSize(int width, int height);
    std::pair<int, int> getTotalSize() const;
    std::pair<int, int> getFrameSize() const;
    void setName(str&);
    void setName(str&& s) { setName(s); }
    std::string_view getName() const;

    void recreateSwapchain();
    // Overide to the main loop function which abandons the loop if the window has already been closed
    virtual bool mainLoop(uint64_t frame) { if(!window) return false; return RenderState::mainLoop(frame); }

    bool isDestroyed() const;
    bool isClosed() const;
    bool shouldClose() const;
    bool close();

public:
    static void pollEvents();
    static void waitEvents();

    static std::vector<const char*> requiredVulkanExtensions();

    static bool allWindowsClosed();
};
