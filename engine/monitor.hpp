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
    };
private:
    GLFWmonitor* monitor;

public:
    Monitor(GLFWmonitor* _monitor = nullptr) : monitor(_monitor) {}
    operator GLFWmonitor* () const { return monitor; }

    const VideoMode* videoMode() const;
    std::pair<int, int> position() const;
    std::pair<int, int> size() const;
    std::pair<float, float> contentScale() const;
    str name() const;

public:
    static Monitor Primary();
    static std::vector<Monitor> Monitors();
};
