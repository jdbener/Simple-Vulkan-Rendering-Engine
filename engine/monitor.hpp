#pragma once

#include "util/string.hpp"

class GLFWmonitor;

class Monitor {
public:
    struct VideoMode {
        int width;
        int height;
        int redBits;
        int greenBits;
        int blueBits;
        int refreshRate;

        static const VideoMode Current;
    };
private:
    GLFWmonitor* monitor;

public:
    Monitor(GLFWmonitor* _monitor = nullptr) : monitor(_monitor) {}
    operator GLFWmonitor* () const { return monitor; }

    /// Returns the video mode of the monitor
    const VideoMode* videoMode() const;
    /// Returns the xPosition, yPosition, width, and height of the monitor
    std::tuple<int, int, int, int> workArea() const;
    /// Returns the xPosition and yPosition of the monitor
    std::pair<int, int> position() const;
    /// Returns the width and height of the monitor
    std::pair<int, int> size() const;
    std::pair<float, float> contentScale() const;
    /// Returns the name of the monitor
    str name() const;

public:
    static Monitor Primary();
    static std::vector<Monitor> Monitors();

    /// Constant representing the current monitor the window is on
    static const Monitor Current;
};
