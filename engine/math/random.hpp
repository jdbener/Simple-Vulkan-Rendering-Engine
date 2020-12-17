#pragma once

#include "math.hpp"

// Make algorithms work a little better with clang
#define PCG_USE_ZEROCHECK_ROTATE_IDIOM 1
#include <pcg_random.hpp>
#include <pcg_extras.hpp>
#include <random>

class Random: public pcg32 {
public:
    using pcg32::pcg32;
    /// Default constructor is seeded via an std::random_device
    Random() : pcg32(pcg_extras::seed_seq_from<std::random_device>{}) {}

    /// Generate a uniformly distribuited random number in the interval [min, max]
    template <class T = int>
    T generate(T min, T max){
        if constexpr(std::is_floating_point<T>()){
            std::uniform_real_distribution<T> distribution(min, max);
            return distribution(*this);
        } else {
            std::uniform_int_distribution<T> distribution(min, max);
            return distribution(*this);
        }
    }

    // TODO: noise
    // https://mrl.cs.nyu.edu/~perlin/noise/
};
