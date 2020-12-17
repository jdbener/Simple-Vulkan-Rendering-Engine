#pragma once

#include "engine/common.hpp"
#include "engine/vendor/glm.hpp"
#include <nytl/math.hpp>

// Defines the engine wide up direction
#define UP_AXIS glm::vec3{0, 1, 0}

// Print a vector to an output stream
template <typename T> std::ostream& operator<<(std::ostream& s, glm::vec<2, T> vec){ return s << "(" << vec.x << ", " << vec.y << ")"; }
template <typename T> std::ostream& operator<<(std::ostream& s, glm::vec<3, T> vec){ return s << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")"; }
template <typename T> std::ostream& operator<<(std::ostream& s, glm::vec<4, T> vec){ return s << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")"; }

namespace glm {
    // Converters which convert whole vectors to/from radians/degrees
    template <typename T> vec<3, T> radians(vec<3, T>& in){ return {radians(in.x), radians(in.y), radians(in.z)}; }
    template <typename T> vec<3, T> degrees(vec<3, T>& in){ return {degrees(in.x), degrees(in.y), degrees(in.z)}; }
    template <typename T> vec<4, T> radians(vec<4, T>& in){ return {radians(in.x), radians(in.y), radians(in.z), radians(in.w)}; }
    template <typename T> vec<4, T> degrees(vec<4, T>& in){ return {degrees(in.x), degrees(in.y), degrees(in.z), degrees(in.w)}; }
}
