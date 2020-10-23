#include "monitor.hpp"

// GLFW
#include <GLFW/glfw3.h>

const Monitor::VideoMode* Monitor::videoMode() const {
    return (VideoMode*) glfwGetVideoMode(monitor);
}

std::pair<int, int> Monitor::position() const {
    int xpos, ypos, width, height;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
    return {xpos, ypos};
}

std::pair<int, int> Monitor::size() const {
    int xpos, ypos, width, height;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
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
