# Simple Vulkan Rendering Engine

This was a small project I did to learn more about the nuts and bolts of the vulkan framework.
It was supposed to be the basis for a card game thus why I based it off a hardware accelerated path tracing library. However I figured Godot would be the better medium.
I will be leaving this here as a log.

To build the repository you must use meson, it was built against clang so:
```
CC=clang CXX=clang++ meson build
cd build
meson compile
```
