#ifndef ILL_ENDIAN_H_
#define ILL_ENDIAN_H_

#include <stdint.h>

/**
Contains useful platform independent byte swapping functions for reading binary data from files and from the network.

When communicating over the network always convert binary values from native to big endian order when sending, 
and convert from big endian back to native order when receiving.

When reading from a binary file convert from whatever matches the file specification into native order,
and when writing convert from native order to whatever matches the file specification.

Network order is big endian.
Intel x86 PC's are typically little endian.
ARM devices like Android and iOS are typically little endian by default but Android can switch to big endian or something.
Usually just google what endianness the device you are compiling for is if you're really interested or the ILL_BYTEORDER macro below isn't being set correctly.
*/


/* This byteorder stuff was lifted from PhysFS http://icculus.org/physfs/ which they say was lifted from SDL. http://www.libsdl.org/ */
#define ILL_LIL_ENDIAN  1234
#define ILL_BIG_ENDIAN  4321

#if  defined(__i386__) || defined(__ia64__) || \
     defined(_M_IX86) || defined(_M_IA64) || defined(_M_X64) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || defined(ARM) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
#define ILL_BYTEORDER    ILL_LIL_ENDIAN
#else
#define ILL_BYTEORDER    ILL_BIG_ENDIAN
#endif



///Swaps the byte order in a 16 bit binary number.
inline uint16_t swap16(uint16_t val) {
    return 
        (val >> 8) | 
        (val << 8);
}

///Swaps the byte order in a 32 bit binary number.
inline uint32_t swap32(uint32_t val) {
    return 
        (val >> 24) |
        ((val << 8) & 0x00FF0000) |
        ((val >> 8) & 0x0000FF00) |
        (val << 24);
}

///Swaps the byte order in a 64 bit binary number.
inline uint64_t swap64(uint64_t val) {
    return 
        (val >> 56) |
        ((val << 40) & 0x00FF000000000000) |
        ((val << 24) & 0x0000FF0000000000) |
        ((val << 8) & 0x000000FF00000000) |
        ((val >> 8) & 0x00000000FF000000) |
        ((val >> 24) & 0x0000000000FF0000) |
        ((val >> 40) & 0x000000000000FF00) |
        (val << 56);
}

///Swaps the byte order in a float. (Can't use the same bit flipping optimizations as with ints for various complicated reasons)
inline float swapF(float val) {
    //taken from the Doom 3 source, Hell Yeah!
    union {
        float	f;
        int8_t	b[4];
    } dat1, dat2;
    
    dat1.f = val;
    dat2.b[0] = dat1.b[3];
    dat2.b[1] = dat1.b[2];
    dat2.b[2] = dat1.b[1];
    dat2.b[3] = dat1.b[0];

    return dat2.f;
}

///Swaps the byte order in a double. (Can't use the same bit flipping optimizations as with ints for various complicated reasons)
inline double swapD(double val) {
    //taken from the Doom 3 source, Hell Yeah!  Except I modified this for doubles.
    union {
        double	f;
        int8_t	b[8];
    } dat1, dat2;
    
    dat1.f = val;
    dat2.b[0] = dat1.b[7];
    dat2.b[1] = dat1.b[6];
    dat2.b[2] = dat1.b[5];
    dat2.b[3] = dat1.b[4];
    dat2.b[4] = dat1.b[3];
    dat2.b[5] = dat1.b[2];
    dat2.b[6] = dat1.b[1];
    dat2.b[7] = dat1.b[0];

    return dat2.f;
}

///Converts a 16 bit number from little endian to native order or from native order to little endian
inline uint16_t little16(uint16_t val) {    
#if ILL_BYTEORDER == ILL_LIL_ENDIAN
    return val;
#else
    return swap16(val);
#endif
}

///Converts a 16 bit number from big endian to native order or from native order to big endian
inline uint16_t big16(uint16_t val) {
#if ILL_BYTEORDER == ILL_BIG_ENDIAN
    return val;
#else
    return swap16(val);
#endif
}

///Converts a 32 bit number from little endian to native order or from native order to little endian
inline uint32_t little32(uint32_t val) {
#if ILL_BYTEORDER == ILL_LIL_ENDIAN
    return val;
#else
    return swap32(val);
#endif
}

///Converts a 32 bit number from big endian to native order or from native order to big endian
inline uint32_t big32(uint32_t val) {
#if ILL_BYTEORDER == ILL_BIG_ENDIAN
    return val;
#else
    return swap32(val);
#endif
}

///Converts a 64 bit number from little endian to native order or from native order to little endian
inline uint64_t little64(uint64_t val) {
#if ILL_BYTEORDER == ILL_LIL_ENDIAN
    return val;
#else
    return swap64(val);
#endif
}

///Converts a 64 bit number from big endian to native order or from native order to big endian
inline uint64_t big64(uint64_t val) {
#if ILL_BYTEORDER == ILL_BIG_ENDIAN
    return val;
#else
    return swap64(val);
#endif
}

///Converts a 32 bit float precision number from little endian to native order or from native order to little endian
inline float littleF(float val) {
#if ILL_BYTEORDER == ILL_LIL_ENDIAN
    return val;
#else
    return swapF(val);
#endif
}

///Converts a 32 bit float precision number from big endian to native order or from native order to big endian
inline float bigF(float val) {
#if ILL_BYTEORDER == ILL_BIG_ENDIAN
    return val;
#else
    return swapF(val);
#endif
}

///Converts a 64 bit double precision number from little endian to native order or from native order to little endian
inline double littleD(double val) {
#if ILL_BYTEORDER == ILL_LIL_ENDIAN
    return val;
#else
    return swapD(val);
#endif
}

///Converts a 64 bit double precision number from big endian to native order or from native order to big endian
inline double bigD(double val) {
#if ILL_BYTEORDER == ILL_BIG_ENDIAN
    return val;
#else
    return swapD(val);
#endif
}

#endif
