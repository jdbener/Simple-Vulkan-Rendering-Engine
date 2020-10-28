// Include guard
#pragma once

#include "defs.hpp"

#include <iostream>         // cout
#include <iomanip>
#include <exception>

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

// Enable writing vectors out to ostreams
template <typename T>
std::ostream& operator<<(std::ostream& s, std::vector<T> out){
    s << "[";
    for(size_t i = 0; i < out.size(); i++){
        s << out[i];
        if(i < out.size() - 1) s << ", ";
    }
    return s << "]";
}

std::vector<std::byte> readStream(std::istream& f, size_t start = 0, long len = -1);


namespace nytl {
    // Makes the specified variable into a span of length 1
    template <typename T>
    nytl::span<T> make_span(T& what){
        return {&what, 1};
    }
}
