/*
* util.h
*
* Miscellaneous useful utility functions
*
*  Created on: Apr 3, 2010
*      Author: ilya
*/

#ifndef ILL_UTIL_H__
#define ILL_UTIL_H__

#include <cmath>
#include <climits>
#include <cstdlib>
#include <stdint.h>
#include <string>
#include <sstream>

#include <glm/glm.hpp>

/**
A useful little container that stores if a value is dirty.
Meaning if the value needs to be recomputed or if the one in there currently is good.
*/
template <typename T>
struct DirtyBitContainer {
    DirtyBitContainer()
        : m_dirty(true)
    {}

    T m_value;
    bool m_dirty;
};

/**
Snaps the value to dampTarget if it's within dampFactor of the value
e.g 0.3 and -0.3 will be snapped to 0.0 with dampTarget of 0.0 and dampFactor of 0.5.  -0.6 or 0.6 wouldn't.
*/
template <typename T>
inline T dampen (T value, const T& dampFactor, const T& dampTarget) {
    if(glm::abs(value - dampTarget) < dampFactor) {
        value = dampTarget;
    }

    return value;
}

/**
Does a high pass filter on a value
*/
template <typename T>
inline T highPassFilter(T value, const T& previousValue, T filterFactor) {
    return (value * filterFactor) + (previousValue * ((T)1 - filterFactor));
}

/**
Slowly drifts a value to destination if otherValue = destination with falloffFactor
falloffFactor needs to be greater than zero and less than one
*/
template <typename T>
inline T drift(T value, const T& otherValue, const T& destination, const T& falloffFactor) {
    if(otherValue == destination) {
        value -= (value - otherValue) * falloffFactor;
    }

    return value;
}

/**
Similar to sign except this one returns a 1 if the value is 0.
*/
template <typename T>
inline T signO(const T& value) {
    return value >= (T)0 ? (T)1 : (T)-1;
}

/**
Returns a 1 dimensional grid cell for a value and dimensions of a grid.
*/
template <typename T, typename R>
inline R grid(const T& value, const T& dimensions) {
    return (R) glm::floor(value / dimensions);
}

/**
Signed compare if one number is greater than the other.

@param num1 number on the left side of the inequality
@param num2 number on the right side of the inequality
@param sign If 0 or a positive number, just checks if num1 > num2
If a negative number, actually flips and checks if num1 < num2

@return The result of the inequality
*/
template <typename T>
inline bool gt(T num1, T num2, int8_t sign) {
    if(sign >= 0) {
        return num1 > num2;
    }
    else {
        return num1 < num2;
    }
}

/**
Signed compare if one number is greater than or equal to the other.

@param num1 number on the left side of the inequality
@param num2 number on the right side of the inequality
@param sign If 0 or a positive number, just checks if num1 >= num2
If a negative number, actually flips and checks if num1 <= num2

@return The result of the inequality
*/
template <typename T>
inline bool geq(T num1, T num2, int8_t sign) {
    if(sign >= 0) {
        return num1 >= num2;
    }
    else {
        return num1 <= num2;
    }
}

/**
Signed compare if one number is less than the other.

@param num1 number on the left side of the inequality
@param num2 number on the right side of the inequality
@param sign If 0 or a positive number, just checks if num1 < num2
If a negative number, actually flips and checks if num1 > num2

@return The result of the inequality
*/
template <typename T>
inline bool lt(T num1, T num2, int8_t sign) {
    if(sign >= 0) {
        return num1 < num2;
    }
    else {
        return num1 > num2;
    }
}

/**
Signed compare if one number is less than or equal to the other.

@param num1 number on the left side of the inequality
@param num2 number on the right side of the inequality
@param sign If 0 or a positive number, just checks if num1 <= num2
If a negative number, actually flips and checks if num1 >= num2

@return The result of the inequality
*/
template <typename T>
inline bool leq(T num1, T num2, int8_t sign) {
    if(sign >= 0) {
        return num1 <= num2;
    }
    else {
        return num1 >= num2;
    }
}

/**

*/
template<typename T>
inline bool eq(T num1, T num2, T delta = (T)0.0001) {
    return glm::abs(num1 - num2) <= delta;
}

/**
Outputs a formatted string similar to printf.
@param formatString The format string, like "Hello %f"
@param followed by a variable argument list with all the variables to add into the formatted string
*/
std::string formatString(const char * formatString, ...);

/**
Returns the next power of 2
*/
inline unsigned int nextPow2(uint32_t x) {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

/**
Returns if int1 is greater than int2 if both are unsigned ints.
Handles the case of rollover.
*/
inline bool uintGreater(unsigned int int1, unsigned int int2) {
    return ((int1 > int2) && (int1 - int2 <= UINT_MAX / 2)) || ((int2 > int1)
        && (int2 - int1 > UINT_MAX / 2));
}

/**
Returns the bit in a mask given the bit number.
Can be used for storing bit types in an enum.
*/
template <typename T>
inline T maskBit(T bitNumber) {
    return (T)1 << bitNumber;
}

#endif
