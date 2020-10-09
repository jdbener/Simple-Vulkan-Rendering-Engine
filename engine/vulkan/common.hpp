#pragma once

#include "engine/util/string.hpp"

// Vulkan
#include <vpp/vpp.hpp>

std::vector<const char*> validateInstanceLayers(std::vector<const char*> layers);
std::vector<const char*> validateInstanceExtensions(std::vector<const char*> extensions);
