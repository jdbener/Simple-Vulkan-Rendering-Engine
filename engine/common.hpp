// Include guard
#pragma once

#include "defs.hpp"

#include <iostream>         // cout
#include <iomanip>
#include <exception>
#include <variant>

#include <dlg/dlg.hpp>      // logger
#include <nytl/scope.hpp>   // Scopeguard
#include <nytl/span.hpp>    // Span

#include "util/repeat.h"    // repeat loop

/// Macro which sets up deferred function calls using nytl::ScopeGuard
///  What is the source code to be deferred till the end of the scope
///  ID is a unique ID (since a variable is created, the variable needs to have a unique name)
#define defer(what, id) nytl::ScopeGuard id([&]() { what });

// Enable appending vectors of the same type with the + operator
template <typename T>
std::vector<T> operator+(std::vector<T> a, std::vector<T> b){
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

// Enable appending std::arrays of the same type with the + operaor
template <typename T, size_t aSize, size_t bSize>
std::array<T, aSize + bSize> operator+ (std::array<T, aSize> a, std::array<T, bSize> b){
    std::array<T, aSize + bSize> out;
    // Copy a's data
    memcpy(out.data(), a.data(), aSize * sizeof(T));
    // Copy b's data
    memcpy(out.data() + aSize, b.data(), bSize * sizeof(T));
    return out;
}

/// Copies the provided array into an std::array
template <size_t size, typename T>
std::array<T, size> make_array(T* array){
    std::array<T, size> out;
    memcpy(out.data(), array, size * sizeof(T));
    return out;
}

// Enable writing std::vectors out to ostreams
template <typename T>
std::ostream& operator<<(std::ostream& s, std::vector<T> out){
    s << "[";
    for(size_t i = 0; i < out.size(); i++){
        s << out[i];
        if(i < out.size() - 1) s << ", ";
    }
    return s << "]";
}

// Enable writing std::variants out to ostreams
// https://stackoverflow.com/questions/47168477/how-to-stream-stdvariant
template <typename T0, typename ... Ts>
std::ostream & operator<< (std::ostream & s, std::variant<T0, Ts...> const & v)
    { std::visit([&](auto && arg){ s << arg;}, v); return s; }

/// Converts the provided stream into a byte array
std::vector<std::byte> readStream(std::istream& f, size_t start = 0, long len = -1);


namespace nytl {
    /// Makes the specified variable into a span of length 1
    template <typename T>
    FORCE_INLINE nytl::span<T> make_span(T& what){
        return {&what, 1};
    }
}
