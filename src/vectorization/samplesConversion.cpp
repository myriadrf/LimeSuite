#include "samplesConversion.h"
#include "string.h"
#include <assert.h>

namespace lime {

// templates with fixed iteration count allow to produce more efficient instructions

template<uint32_t srcCount>
static void fastPath_complex16_to_complex32f(complex32f_t* __restrict__ dest, const complex16_t* __restrict__ src)
{
    constexpr float scale = 1.0f / 32768.0f;
    for (uint16_t i = 0; i < srcCount; i += 2)
    {
        dest[i].i = src[i].i * scale;
        dest[i].q = src[i].q * scale;

        dest[i + 1].i = src[i + 1].i * scale;
        dest[i + 1].q = src[i + 1].q * scale;
    }
}

static void slowPath_complex16_to_complex32f(
    complex32f_t* __restrict__ dest, const complex16_t* __restrict__ src, uint32_t srcCount)
{
    constexpr float scale = 1.0f / 32768.0f;
    for (uint16_t i = 0; i < srcCount; i += 2)
    {
        dest[i].i = src[i].i * scale;
        dest[i].q = src[i].q * scale;

        dest[i + 1].i = src[i + 1].i * scale;
        dest[i + 1].q = src[i + 1].q * scale;
    }
}

void complex16_to_complex32f(complex32f_t* __restrict__ dest, const complex16_t* __restrict__ src, uint32_t srcCount)
{
    switch (srcCount)
    {
    case 512:
        fastPath_complex16_to_complex32f<512>(dest, src);
        break;
    case 256:
        fastPath_complex16_to_complex32f<256>(dest, src);
        break;
    case 128:
        fastPath_complex16_to_complex32f<128>(dest, src);
        break;
    case 64:
        fastPath_complex16_to_complex32f<64>(dest, src);
        break;
    default:
        slowPath_complex16_to_complex32f(dest, src, srcCount);
        break;
    }
}

template<uint32_t srcCount>
static void fastPath_complex16_to_complex32f_unzip(
    complex32f_t* __restrict__ destA, complex32f_t* __restrict__ destB, const complex16_t* __restrict__ src)
{
    constexpr float scale = 1.0f / 32768.0f;
    for (uint32_t i = 0; i < srcCount / 2; i++)
    {
        const uint16_t srcPos = 2 * i;
        destA[i].i = src[srcPos].i * scale;
        destA[i].q = src[srcPos].q * scale;

        destB[i].i = src[srcPos + 1].i * scale;
        destB[i].q = src[srcPos + 1].q * scale;
    }
}

static void slowPath_complex16_to_complex32f_unzip(
    complex32f_t* __restrict__ destA, complex32f_t* __restrict__ destB, const complex16_t* __restrict__ src, uint32_t srcCount)
{
    constexpr float scale = 1.0f / 32768.0f;
    for (uint32_t i = 0; i < srcCount / 2; i++)
    {
        const uint16_t srcPos = 2 * i;
        destA[i].i = src[srcPos].i * scale;
        destA[i].q = src[srcPos].q * scale;

        destB[i].i = src[srcPos + 1].i * scale;
        destB[i].q = src[srcPos + 1].q * scale;
    }
}

void complex16_to_complex32f_unzip(
    complex32f_t* __restrict__ destA, complex32f_t* __restrict__ destB, const complex16_t* __restrict__ src, uint32_t srcCount)
{
    switch (srcCount)
    {
    case 512:
        fastPath_complex16_to_complex32f_unzip<512>(destA, destB, src);
        break;
    case 256:
        fastPath_complex16_to_complex32f_unzip<256>(destA, destB, src);
        break;
    case 128:
        fastPath_complex16_to_complex32f_unzip<128>(destA, destB, src);
        break;
    case 64:
        fastPath_complex16_to_complex32f_unzip<64>(destA, destB, src);
        break;
    default:
        slowPath_complex16_to_complex32f_unzip(destA, destB, src, srcCount);
        break;
    }
}

template<uint32_t srcCount>
static void fastPath_complex32f_to_complex16(complex16_t* __restrict__ dest, const complex32f_t* __restrict__ src)
{
    const int16_t scale = 32767;
    for (uint32_t i = 0; i < srcCount; i++)
    {
        dest[i].i = src[i].i * scale;
        dest[i].q = src[i].q * scale;
    }
}

static void slowPath_complex32f_to_complex16(
    complex16_t* __restrict__ dest, const complex32f_t* __restrict__ src, uint32_t srcCount)
{
    const int16_t scale = 32767;
    for (uint32_t i = 0; i < srcCount; i++)
    {
        dest[i].i = src[i].i * scale;
        dest[i].q = src[i].q * scale;
    }
}

void complex32f_to_complex16(complex16_t* __restrict__ dest, const complex32f_t* __restrict__ src, uint32_t srcCount)
{
    switch (srcCount)
    {
    case 512:
        fastPath_complex32f_to_complex16<512>(dest, src);
        break;
    case 256:
        fastPath_complex32f_to_complex16<256>(dest, src);
        break;
    case 128:
        fastPath_complex32f_to_complex16<128>(dest, src);
        break;
    case 64:
        fastPath_complex32f_to_complex16<64>(dest, src);
        break;
    default:
        slowPath_complex32f_to_complex16(dest, src, srcCount);
        break;
    }
}

template<uint32_t srcCount>
static void fastPath_complex32f_to_complex16_zip(
    complex16_t* __restrict__ dest, const complex32f_t* __restrict__ srcA, const complex32f_t* __restrict__ srcB)
{
    constexpr int16_t scale = 32767;
    for (uint32_t i = 0; i < srcCount; i++)
    {
        const uint32_t destPos = 2 * i;
        dest[destPos].i = srcA[i].i * scale;
        dest[destPos].q = srcA[i].q * scale;
        dest[destPos + 1].i = srcB[i].i * scale;
        dest[destPos + 1].q = srcB[i].q * scale;
    }
}

static void slowPath_complex32f_to_complex16_zip(
    complex16_t* __restrict__ dest, const complex32f_t* __restrict__ srcA, const complex32f_t* __restrict__ srcB, uint32_t srcCount)
{
    constexpr int16_t scale = 32767;
    for (uint32_t i = 0; i < srcCount; i++)
    {
        const uint32_t destPos = 2 * i;
        dest[destPos].i = srcA[i].i * scale;
        dest[destPos].q = srcA[i].q * scale;
        dest[destPos + 1].i = srcB[i].i * scale;
        dest[destPos + 1].q = srcB[i].q * scale;
    }
}

void complex32f_to_complex16_zip(
    complex16_t* __restrict__ dest, const complex32f_t* __restrict__ srcA, const complex32f_t* __restrict__ srcB, uint32_t srcCount)
{
    switch (srcCount)
    {
    case 512:
        fastPath_complex32f_to_complex16_zip<512>(dest, srcA, srcB);
        break;
    case 256:
        fastPath_complex32f_to_complex16_zip<256>(dest, srcA, srcB);
        break;
    case 128:
        fastPath_complex32f_to_complex16_zip<128>(dest, srcA, srcB);
        break;
    case 64:
        fastPath_complex32f_to_complex16_zip<64>(dest, srcA, srcB);
        break;
    default:
        slowPath_complex32f_to_complex16_zip(dest, srcA, srcB, srcCount);
        break;
    }
}

} // namespace lime
