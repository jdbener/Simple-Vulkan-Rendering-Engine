#pragma once
// Disable the uninitialized variables in constexpr functions error
#pragma clang diagnostic ignored "-Wc++20-extensions"

// Defines the version of the engine
#define ENGINE_VERSION VK_MAKE_VERSION(0, 0, 1)

// Forces the specified function to be inlined
#define FORCE_INLINE __attribute__((always_inline))
