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
private:
    // Tracks if GLFW has been initalized
    static bool glfwInitalized;
    // Track the number of created windows and currently open windows
    static size_t windowCount, openCount;
    str name;
    bool closed = false;

    friend void _WindowCloseCallback(GLFWwindow*);
protected:
    GLFWwindow* window = nullptr;
    vpp::Surface surface;

    bool destroy();

public:
    Window(vpp::Instance&, int width = 800, int height = 600, str name = "Project Delta", std::vector<std::pair<int, int>> windowCreationHints = {});
    ~Window();

    // TODO: Make fullscreen
    // TODO: Focus, minimize, maximize

    // TODO: void setSize(int width, int height);
    std::pair<int, int> getTotalSize() const;
    std::pair<int, int> getFrameSize() const;
    void setName(str&);
    std::string_view getName() const;

    // TODO: void recreateSurface(vpp::Instance&);
    void swapBuffers();

    bool isDestroyed() const;
    bool shouldClose() const;
    bool isClosed() const;
    bool close();

public:
    static void pollEvents();
    static void waitEvents();
    static std::vector<const char*> vulkanExtensions();

    static bool allWindowsClosed();
};
