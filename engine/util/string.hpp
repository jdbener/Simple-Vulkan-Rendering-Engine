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

    // Replaces whatever is in this string with next line of the input stream.
    //  Returns true if the operation was successful
    bool setToLine(std::istream& s){
        bool state = s.good();
        std::getline(s, *this);
        return state && s.good();
    }

    // Checks if the string contains
    template <typename T> bool contains(T& o) const { return find(o) != std::string::npos; }
    template <typename T> bool contains(T&& o) const { return find(o) != std::string::npos; }

    // Creates a string_view of a sub section of the string
    std::string_view substr_view(size_t start, size_t len) const { return {c_str() + start, len}; }
    std::string_view substr_view(size_t len) const { return {c_str(), len}; }

    // Gets the number of utf-8 characters
    size_t utf8Size() const { return nytl::charCount(*this); }

    // Gets the <i>th utf-8 character
    // TODO: there is probably a better way of implementing this
    std::array<char, 5> utf8At(size_t i) const { return nytl::nth(*this, i); }

    // Ways to convert the string to a number
    long num(int base) const { return std::stol(*this, nullptr, base); }
    template <typename T = double> T num() const { return std::stod(*this); }

    // Automatic conversion to a char*
    operator const char*() const { return c_str(); }

    // Number casts
    explicit operator int() const { return num<int>(); }
    explicit operator unsigned int() const { return num<unsigned int>(); }
    explicit operator float() const { return num<float>(); }
    explicit operator double() const { return num<double>(); }
public:
    // Creates a string from a double, ensuring it has the specified precision
    static str precision(double other, int precision){
        std::stringstream s;
        s << std::fixed << std::setprecision(precision) << other;
        return s.str();
    }

    // Creates a string from an int, saves the string in the specified base
    static str base(long long other, int base){
        std::stringstream s;
        s << std::setbase(base) << other;
        return s.str();
    }

    // Creates a string from an input stream
    //  Allows partitioning of a subset of the input stream.
    //  By default the entire input stream will be stored in the string.
    static str stream(std::istream& s, size_t start = 0, long len = -1){
        return (char*) readStream(s, start, len).data();
    }

    // Creates a string by getting the next line of the input file
    static str getLine(std::istream& s){
        str out;
        std::getline(s, out);
        return out;
    }
};
