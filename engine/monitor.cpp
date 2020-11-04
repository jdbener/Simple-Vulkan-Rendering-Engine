#include "monitor.hpp"

// GLFW
#include <GLFW/glfw3.h>

// Current monitor constant
const Monitor Monitor::Current = nullptr;
// Current VideoMode constant (set to an invalid value)
const Monitor::VideoMode Monitor::VideoMode::Current = {-1, -1, -1, -1, -1, -1};

const Monitor::VideoMode* Monitor::videoMode() const {
    return (VideoMode*) glfwGetVideoMode(monitor);
}

/// Returns the xPosition, yPosition, width, and height of the monitor
std::tuple<int, int, int, int> Monitor::workArea() const{
    int xpos, ypos, width, height;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
    return {xpos, ypos, width, height};
}

std::pair<int, int> Monitor::position() const {
    auto [xpos, ypos, width, height] = workArea();
    return {xpos, ypos};
}

std::pair<int, int> Monitor::size() const {
    auto [xpos, ypos, width, height] = workArea();
    return {width, height};
}

std::pair<float, float> Monitor::contentScale() const {
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    return {xscale, yscale};
}

str Monitor::name() const {
    // Should enforce a copy?
    return glfwGetMonitorName(monitor);
}

Monitor Monitor::Primary(){
    return glfwGetPrimaryMonitor();
}

std::vector<Monitor> Monitor::Monitors(){
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    return {monitors, monitors + count};
}
