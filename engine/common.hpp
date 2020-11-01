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

// Macro which sets up defered function calls using nytl::ScopeGuard
//  What is the source code to be defered till the end of the scope
//  ID is a unique ID (since a variable is created, the variable needs to have a unique name)
#define defer(what, id) nytl::ScopeGuard id([&]() { what });

// Enable appending vectors of the same type with the + operator
template <typename T>
std::vector<T> operator+(std::vector<T> a, std::vector<T> b){
    a.reserve(a.size() + b.size());
    for(T& cur: b) a.push_back(cur);
    return a;
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
    // Makes the specified variable into a span of length 1
    template <typename T>
    nytl::span<T> make_span(T& what){
        return {&what, 1};
    }
}
