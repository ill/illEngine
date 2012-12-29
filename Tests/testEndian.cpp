#include <cassert>
#include "tests.h"
#include "Util/endian.h"
#include "Logging/logging.h"

const uint16_t a16 = 0x1234;
const uint16_t b16 = 0x3412;

const uint32_t a32 = 0x12345678;
const uint32_t b32 = 0x78563412;

const uint64_t a64 = 0x0123456789ABCDEF;
const uint64_t b64 = 0xEFCDAB8967452301;

const float af = 128.0078125f;          //precisely represented 128 + 1/128
const double ad = 128.0078125;          //precisely represented 128 + 1/128

void testEndian() {
    //test 16 bit swap
    LOG_INFO("\n16Bit: %x\n swap: %x\n  res: %x\n swap: %x", a16, b16, swap16(b16), swap16(a16));
    assert(swap16(a16) == b16);
    assert(swap16(b16) == a16);

    //test 32 bit swap
    LOG_INFO("\n32Bit: %x\n swap: %x\n  res: %x\n swap: %x", a32, b32, swap32(b32), swap32(a32));
    assert(swap32(a32) == b32);
    assert(swap32(b32) == a32);

    //test 64 bit swap
    LOG_INFO("\n64Bit: %llx\n swap: %llx\n  res: %llx\n swap: %llx", a64, b64, swap64(b64), swap64(a64));
    assert(swap64(a64) == b64);
    assert(swap64(b64) == a64);

    //test float bit swap
    LOG_INFO("\nfloat: %010.10e\n swap: %010.10e\n  res: %010.10e", af, swapF(af), swapF(swapF(af)));
    {
        float swapped = swapF(af);
        assert(swapF(swapped) == af);
    }

    //test double bit swap
    LOG_INFO("\ndoubl: %010.10e\n swap: %010.10e\n  res: %010.10e", ad, swapD(ad), swapD(swapD(ad)));
    {
        double swapped = swapD(ad);
        assert(swapD(swapped) == ad);
    }

    //test endian functions
#if ILL_BYTEORDER == ILL_LIL_ENDIAN
    //16 bit
    assert(little16(a16) == a16);
    assert(big16(a16) == b16);

    //32 bit
    assert(little32(a32) == a32);
    assert(big32(a32) == b32);

    //64 bit
    assert(little64(a64) == a64);
    assert(big64(a64) == b64);

    //float
    assert(littleF(af) == af);
    assert(bigF(af) == swapF(af));

    //double
    assert(littleD(ad) == ad);
    assert(bigD(ad) == swapD(ad));
#else
    //16 bit
    assert(little16(a16) == b16);
    assert(big16(a16) == a16);

    //32 bit
    assert(little32(a32) == b32);
    assert(big32(a32) == a32);

    //64 bit
    assert(little64(a64) == b64);
    assert(big64(a64) == a64);

    //float
    assert(littleF(af) == swapF(af));
    assert(bigF(af) == af);

    //double
    assert(littleD(ad) == swapD(ad));
    assert(bigD(ad) == ad);
#endif
}
