#pragma once

#include "util/string.hpp"

// Vulkan
#include <vpp/vpp.hpp>

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

class Window {
public:
    // Struct used to con
    struct DeviceCreateInfo {
        enum Valid {NO, YES, NO_INITAL};

        Valid valid = NO;
        vk::DeviceCreateInfo info;
        vk::PhysicalDevice device;

        DeviceCreateInfo() : valid(NO) {}
        DeviceCreateInfo(vk::DeviceCreateInfo _info, vk::PhysicalDevice _device) : valid(YES), info(_info), device(_device) {}
    };

private:
    // Tracks if GLFW has been initalized
    static bool glfwInitalized;
    // Track the number of created windows and currently open windows
    static size_t windowCount, openCount;
    // Used to store the memory of the vulkan logical device we create for this window
    std::unique_ptr<vpp::Device> device = nullptr;
    str name;
    bool closed = false;


    friend void _WindowCloseCallback(GLFWwindow*);
    friend void _WindowResizeCallback(GLFWwindow*, int, int);
protected:
    GLFWwindow* window = nullptr;
    vpp::Surface surface;
    vpp::Swapchain swapchain;

    bool destroy();

public:
    Window(vpp::Instance&, int width = 800, int height = 600, str name = "Project Delta", DeviceCreateInfo deviceInfo = {}, std::vector<std::pair<int, int>> windowCreationHints = {});
    ~Window();

    // TODO: Make fullscreen
    // TODO: void makeFullscreen();
    // TODO: void makeWindowed(int width, int height);
    // TODO: Focus, minimize, maximize

    void setSize(int width, int height);
    std::pair<int, int> getTotalSize() const;
    std::pair<int, int> getFrameSize() const;
    void setName(str&);
    std::string_view getName() const;

    vpp::Surface& getSurface(); // Needed?
    void recreateSurface(vpp::Instance&);
    vpp::Swapchain& getSwapchain();
    void recreateSwapchain(DeviceCreateInfo deviceInfo = {});

    void swapBuffers();

    bool isDestroyed() const;
    bool shouldClose() const;
    bool isClosed() const;
    bool close();

public:
    static void pollEvents();
    static void waitEvents();

    static std::vector<const char*> requiredVulkanExtensions();

    static bool allWindowsClosed();
};
