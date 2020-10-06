/*
    File: repeat.h
    Author: Joshua "Jdbener" Dahl
    Source: https://gist.github.com/jdbener/94d5209d502bd0c335deeb1c3c9b2e88
*/
/* -----------------------------------------------------------------------------
    SYNTAX EXAMPLE
----------------------------------------------------------------------------- */
/*
    repeat(x){
        // code here
    }

    Will repeat the provided code x times. (A counter "_i" storing the itteration
    index will always be present)

    Ex:
    repeat(5)
        std::cout << "This line will be repeated 5 times" << std::endl;
*/

/*
    repeat(x, index){
        // code here
    }

    Will repeat the provided code x times, storing the current index in a
    variable with the name of the second input.

    Ex:
    repeat(5, index)
        std::cout << "Current index: " << index << std::endl;
*/

/*
    repeat(x, type, index){
        // code here
    }

    Will repeat the provided code x times, storing the current index in a
    variable type dictated by the second input and a name provided by the third

    This option shouldn't be necessary for 99% of applications.
*/

/*
    frepeat(x){
        // code here
    }

    frepeat(x, index){
        // code here
    }

    frepeat(x, type, index){
        // code here
    }

    Along with forward_repeat, forwardRepeat, and ForwardRepeat variations all
    work the same as their repeat counterparts except they will always count
    from 0 to x - 1.

    By default repeat loops are forward repeat loops
*/

/*
    rrepeat(x){
        // code here
    }

    rrepeat(x, index){
        // code here
    }

    rrepeat(x, type, index){
        // code here
    }

    Along with reverse_repeat, reverseRepeat, and ReverseRepeat variations all
    work the same as their repeat counterparts except they will always count
    from x - 1 to 0.

    If you wish for the default repeat loop to be a reverse repeat loop you may
    define:
      #define REPEAT_REVERSE_DEFAULT
    before including this file.
*/
#ifndef _REPEAT_H_
#define _REPEAT_H_

// Macros defining Repeat Syntax
#define _REPEAT_TIMES_TYPE_(x, type, y) for(type y = x; y--;)
#define _FORWARD_REPEAT_TIMES_TYPE_(x, type, y) for(type y = 0; y < x; y++)
#define _REPEAT_TIMES_(x, y) _REPEAT_TIMES_TYPE_(x, size_t, y)
#define _FORWARD_REPEAT_TIMES_(x, y) _FORWARD_REPEAT_TIMES_TYPE_(x, size_t, y)
#define _REPEAT_(x) _REPEAT_TIMES_(x, _i)
#define _FORWARD_REPEAT_(x) _FORWARD_REPEAT_TIMES_(x, _i)
// Macro overloading from: https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
#define _GET_REPEAT_(_1,_2,_3, NAME,...) NAME

// Reverse (optimized) repeat
#define rrepeat(...) _GET_REPEAT_(__VA_ARGS__, _REPEAT_TIMES_TYPE_, _REPEAT_TIMES_, _REPEAT_)(__VA_ARGS__)
#define reverse_repeat(...) rrepeat(__VA_ARGS__)
#define reverseRepeat(...) rrepeat(__VA_ARGS__)
#define ReverseRepeat(...) rrepeat(__VA_ARGS__)

// Forward (index based) repeat
#define frepeat(...) _GET_REPEAT_(__VA_ARGS__, _FORWARD_REPEAT_TIMES_TYPE_, _FORWARD_REPEAT_TIMES_, _FORWARD_REPEAT_)(__VA_ARGS__)
#define forward_repeat(...) frepeat(__VA_ARGS__)
#define forwardRepeat(...) frepeat(__VA_ARGS__)
#define ForwardRepeat(...) frepeat(__VA_ARGS__)

// Default repeat direction
#ifdef REPEAT_REVERSE_DEFAULT
#   define repeat(...) rrepeat(__VA_ARGS__)
#else // REPEAT_REVERSE_DEFAULT
#   define repeat(...) frepeat(__VA_ARGS__)
#endif // REPEAT_REVERSE_DEFAULT
#endif // _REPEAT_H_
