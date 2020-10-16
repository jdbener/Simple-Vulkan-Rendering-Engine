// Include guard
#pragma once

#include "defs.hpp"

#include <iostream>         // cout
#include <iomanip>
#include <exception>

#include <dlg/dlg.hpp>      // logger
#include <nytl/scope.hpp>   // Scopeguard

#include "util/repeat.h"    // repeat loop

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

#define defer(what, id) nytl::ScopeGuard id([&]() { what });
