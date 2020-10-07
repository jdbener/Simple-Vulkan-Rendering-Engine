#pragma once

#include "../common.hpp"

#include <sstream>

#include <nytl/utf.hpp>

// String class which allows anything implementing << to be easily converted to a string
class str: public std::string {
public:
    using std::string::string;
    str(std::string& other) : std::string(other) {}
    str(std::string&& other) : std::string(other) {}

    // Ability to explicitly cast anything with << overloaded to a string
    template <typename T> explicit str(T& other){ *this = other; }
    template <typename T> explicit str(T&& other){ *this = other; }

    // Assign anything with << overloaded to the string
    template <typename T>
    str operator= (T& other){
        std::stringstream s;
        s << other;
        return *this = s.str();
    }
    template <typename T> str operator= (T&& other){ return *this = other; }

    // UTF converters (nessicary?)
    str(std::u16string_view& o){ *this = nytl::toUtf8(o); }
    str(std::u32string_view& o){ *this = nytl::toUtf8(o); }
    str(std::u16string_view&& o){ *this = nytl::toUtf8(o); }
    str(std::u32string_view&& o){ *this = nytl::toUtf8(o); }

    // Overide to ensure that we don't use the expensive stringstream process when
    //  we are simply assigning a normal string type.
    using std::string::operator=;

    // Checks if the string contains
    template <typename T> bool contains(T& o){ return find(o) != std::string::npos; }
    template <typename T> bool contains(T&& o){ return find(o) != std::string::npos; }

    std::string_view substr_view(size_t start, size_t len){ return {c_str() + start, len}; }
    std::string_view substr_view(size_t len){ return {c_str(), len}; }

    // Gets the number of utf-8 characters
    size_t utf8Size(){ return nytl::charCount(*this); }

    // Gets the <i>th utf-8 character
    // TODO: there is probably a better way of implementing this
    std::array<char, 5> utf8At(size_t i){ return nytl::nth(*this, i); }

    // Convert the string to a number
    long long num(int base){ return std::stoll(*this, nullptr, base); }
    template <typename T = double> T num(){ return std::stod(*this); }

    explicit operator int() { return num<int>(); }
    explicit operator unsigned int() { return num<unsigned int>(); }
    explicit operator float() { return num<float>(); }
    explicit operator double() { return num<double>(); }
public:
    static str precision(double other, int precision){
        std::stringstream s;
        s << std::fixed << std::setprecision(precision) << other;
        return s.str();
    }

    static str base(long long other, int base){
        std::stringstream s;
        s << std::setbase(base) << other;
        return s.str();
    }
};
