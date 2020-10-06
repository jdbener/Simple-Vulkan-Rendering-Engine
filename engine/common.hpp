// Include guard
#pragma once

#include "defs.hpp"

#include <iostream>
#include <sstream>
#include <dlg/dlg.hpp>

#include "util/repeat.h"

template <class T>
std::string toString(T& what){
    std::stringstream s;
    s << what;
    return s.str();
}
template <class T> FORCE_INLINE std::string toString(T&& what){ return toString(what); }

template <class T> FORCE_INLINE std::string str(T&& what){ return toString(what); }
template <class T> FORCE_INLINE std::string str(T& what){ return toString(what); }
