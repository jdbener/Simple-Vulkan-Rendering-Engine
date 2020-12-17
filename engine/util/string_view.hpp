#ifndef __STRING_VIEW_H__
#define __STRING_VIEW_H__

#include "../common.hpp"

#include <string_view>

class str_view: public std::string_view {
public:
    using std::string_view::string_view;
    str_view(std::string_view other) : std::string_view(other) {}

    /// Checks if the string contains the specified value
    template <typename T> bool contains(T& o) const { return find(o) != str_view::npos; }
    template <typename T> bool contains(T&& o) const { return contains(o); }

    /// Tokenize the string.
    ///  A token will be split at any character in delimiters
    std::vector<str_view> split(const str_view delimiters = " ") const {
        size_t start = 0, end;
        std::vector<str_view> out;
        do{
            end = find_first_of(delimiters, start);
            // If there are no elements to be found return the whole string
            if (end == str_view::npos) end = size();

            str_view sub = substr(start, end - start);
            if(!sub.empty()) out.push_back(sub);
            start = end + 1;
        } while (end < size());

        return out;
    }

    /// Removes all of the specified characters from the beginning of the string
    str_view lstrip(const str_view characters = " \t\n\r") const {
        size_t start = find_first_not_of(characters);
        if(start == str_view::npos) start = 1;

        return substr(start, size() - start);
    }
    /// Removes all of the specified characters from the end of the string
    str_view rstrip(const str_view characters = " \t\n\r") const {
        size_t end = size() - 1;

        // Find the first character (starting from the end) which isn't one of the matched characters
        while(characters.contains(at(end)) && end > 0) end--;
        if(end == 0) end = size() - 1;
        else end++;

        return substr(0, end);
    }
    /// Removes all of the specified characters from the beginning and end of a copy of the string
    str_view strip(const str_view characters = " \t\n\r") const { return lstrip(characters).rstrip(characters); }

    // Returns true if this string begins with the provided string
    bool beginsWith(const str_view what) const {
        // If the searched for string is longer than this string, this string can't begin with it
        if(what.size() > size()) return false;

        // Scan through the beginning of this string and determine if any chracters differ from what
        for(int i = 0; i < what.size(); i++)
            if(at(i) != what[i]) return false;

        // If all of the characters in the length match than this string begins with the other string
        return true;
    }
    // Returns true if this string ends with the provided string
    bool endsWith(const str_view what) const {
        // If the searched for string is longer than this string, this string can't begin with it
        if(what.size() > size()) return false;

        // Scan through the beginning of this string and determine if any chracters differ from what
        for(int i = 0; i < what.size(); i++)
            if(at(size() - what.size() + i) != what[i]) return false;

        // If all of the characters in the length match than this string begins with the other string
        return true;
    }
};

#endif //__STRING_VIEW_H__
