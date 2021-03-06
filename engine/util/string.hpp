#ifndef __STRING_H__
#define __STRING_H__

#include "../common.hpp"
#include "string_view.hpp"

#include <sstream>

#include <nytl/utf.hpp>

// String class which allows anything implementing << to be easily converted to a string
class str: public std::string {
public:
    using std::string::string;
    str() : std::string("") {}
    str(std::string& other) : std::string(other) {}
    str(std::string&& other) : std::string(std::move(other)) {}

    /// Create a str from an str_view
    str(str_view other) : std::string(other.data(), other.data() + other.size()) {}

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

    // UTF converters (TODO: necessary?)
    str(std::u16string_view o){ *this = nytl::toUtf8(o); }
    str(std::u32string_view o){ *this = nytl::toUtf8(o); }

    /// Override to ensure that we don't use the expensive stringstream process when
    ///  we are simply assigning a normal string type.
    using std::string::operator=;

    /// Replaces whatever is in this string with the next line of the input stream.
    ///  Returns true if the operation was successful
    bool setToLine(std::istream& s){
        bool state = s.good();
        std::getline(s, *this);
        return state && s.good();
    }

    /// Checks if the string contains the specified value
    template <typename T> FORCE_INLINE bool contains(T& o) const { return find(str(o)) != std::string::npos; }
    template <typename T> FORCE_INLINE bool contains(T&& o) const { return contains(o); }

    /// Creates a string_view of a sub section of the string
    FORCE_INLINE str_view substr_view(size_t start, size_t len) const { return {data() + start, len}; }
    FORCE_INLINE str_view substr_view(size_t len) const { return {data(), len}; }

    /// Gets the number of utf-8 characters
    size_t utf8Size() const { return nytl::charCount(*this); }

    // FIXME: there is probably a better way of implementing this
    /// Gets the <i>th utf-8 character
    std::array<char, 5> utf8At(size_t i) const { return nytl::nth(*this, i); }

    /// Tokenize the string.
    ///  A token will be split at any character in delimiters
    template <class T = str_view>
    std::vector<T> split(const str_view delimiters = " ") const {
        size_t start = 0, end;
        std::vector<T> out;
        do{
            end = find_first_of(delimiters, start);
            // If there are no elements to be found return the whole string
            if (end == str::npos) end = size();

            T sub = substr_view(start, end - start);
            if(!sub.empty()) out.push_back(sub);
            start = end + 1;
        } while (end < size());

        return out;
    }

    /// Removes all of the specified characters from the beginning of the string
    template <class T = str_view>
    FORCE_INLINE T lstrip(const str_view characters = " \t\n\r") const {
        return str_view(*this).lstrip(characters);
    }
    /// Removes all of the specified characters from the end of the string
    template <class T = str_view>
    FORCE_INLINE T rstrip(const str_view characters = " \t\n\r") const {
        return str_view(*this).rstrip(characters);
    }
    /// Removes all of the specified characters from the beginning and end of a copy of the string
    template <class T = str_view>
    FORCE_INLINE T strip(const str_view characters = " \t\n\r") const { return str_view(*this).strip(characters); }


    // Returns true if this string begins with the provided string
    bool beginsWith(const str_view what) const {
        return str_view(*this).beginsWith(what);
    }
    // Returns true if this string ends with the provided string
    bool endsWith(const str_view what) const {
        return str_view(*this).endsWith(what);
    }

    /// Replaces the first instance of the specified match with the specified replacement in a copy of the string
    str replaceFirst(const str_view match, const str_view replacement){
        std::string out = *this;
        if(size_t start = out.find(match); start != str::npos)
            out.replace(start, replacement.size(), replacement);
        return out;
    }
    /// Replaces all instances of the specified match with the specified replacement in a copy of the string
    str replace(const str_view match, const str_view replacement) const {
        std::string out = *this;
        size_t start;
        while((start = out.find(match)) != str::npos)
            out.replace(start, match.size(), replacement);

        return out;
    }
    /// Replaces all instances of each of the specified strings with the specified replacement in a copy of the string
    str replace(const std::vector<str>& matches, const str_view replacement) const {
        str out = *this;
        for(const str& match: matches)
            out = out.replace(match, replacement);

        return out;
    }

    /// Returns an all uppercase copy of the string
    str toupper() const {
        str out = *this;
        for(char& c: out) c = std::toupper(c);
        return out;
    }
    /// Returns an all lowercase copy of the string
    str tolower() const {
        str out = *this;
        for(char& c: out) c = std::tolower(c);
        return out;
    }

    // Ways to convert the string to a number
    FORCE_INLINE long long num(int base) const { return std::stoll(*this, nullptr, base); }
    FORCE_INLINE unsigned long long unum(int base) const { return std::stoull(*this, nullptr, base); }
    template <typename T = double> FORCE_INLINE T num() const { return std::stod(*this); }

    // Automatic conversion to a char*
    FORCE_INLINE operator const char*() const { return c_str(); }
    // Automatic conversion to a string_view
    FORCE_INLINE operator str_view() const { return {data(), size()}; }

    // Number casts
    FORCE_INLINE explicit operator int() const { return num(10); }
    FORCE_INLINE explicit operator unsigned int() const { return unum(10); }
    FORCE_INLINE explicit operator float() const { return num<float>(); }
    FORCE_INLINE explicit operator double() const { return num<double>(); }
public:
    /// Creates a string from a double, ensuring it has the specified precision
    static str precision(double other, int precision){
        std::stringstream s;
        s << std::fixed << std::setprecision(precision) << other;
        return s.str();
    }

    /// Creates a string from an int, saves the string in the specified base
    static str base(long long other, int base){
        std::stringstream s;
        s << std::setbase(base) << other;
        return s.str();
    }

    /// Creates a string from an input stream
    ///  Allows partitioning of a subset of the input stream.
    ///  By default the entire input stream will be stored in the string.
    static str stream(std::istream& s, size_t start = 0, long len = -1){
        auto data = readStream(s, start, len);
        return {(const char*) data.data(), data.size()};
    }

    /// Creates a string by getting the next line of the input file
    static str getLine(std::istream& s){
        str out;
        std::getline(s, out);
        return out;
    }
};

#endif //__STRING_H__
