#include "samplesConversion.h"
#include "string.h"
#include <assert.h>

namespace lime {

// generated vectorized assembly would be better if loop count would be compile time const.
void complex16_to_complex32f(complex32f_t* __restrict__ dest, const complex16_t* __restrict__ src, uint32_t srcCount)
{
    assert((srcCount & 0x3) == 0); // count should be multiple of 4
    const float scale = 1.0f/32768.0f;
    for(uint32_t i=0; i<(srcCount); i+=4)
    {
        dest[i].i = src[i].i * scale;
        dest[i].q = src[i].q * scale;
        dest[i+1].i = src[i+1].i * scale;
        dest[i+1].q = src[i+1].q * scale;
        dest[i+2].i = src[i+2].i * scale;
        dest[i+2].q = src[i+2].q * scale;
        dest[i+3].i = src[i+3].i * scale;
        dest[i+3].q = src[i+3].q * scale;
    }
}

void complex16_to_complex32f_unzip(complex32f_t* __restrict__ destA, complex32f_t* __restrict__ destB, const complex16_t* __restrict__ src, uint32_t srcCount)
{
    assert(srcCount > 0);
    assert((srcCount & 0x7) == 0); // count should be multiple of 8

    // loop increment needs to be 8 for the Arm compiler to auto vectorize to Neon instructions
    const float scale = 1.0f/32768.0f;
    for(uint32_t i=0; i<srcCount; i+=8)
    {
        destA->i = src[i].i * scale;
        destA->q = src[i].q * scale;
        ++destA;

        destB->i = src[i+1].i * scale;
        destB->q = src[i+1].q * scale;
        ++destB;

        destA->i = src[i+2].i * scale;
        destA->q = src[i+2].q * scale;
        ++destA;

        destB->i = src[i+3].i * scale;
        destB->q = src[i+3].q * scale;
        ++destB;

        destA->i = src[i+4].i * scale;
        destA->q = src[i+4].q * scale;
        ++destA;

        destB->i = src[i+5].i * scale;
        destB->q = src[i+5].q * scale;
        ++destB;

        destA->i = src[i+6].i * scale;
        destA->q = src[i+6].q * scale;
        ++destA;

        destB->i = src[i+7].i * scale;
        destB->q = src[i+7].q * scale;
        ++destB;
    }
}

void complex32f_to_complex16(complex16_t* __restrict__ dest, const complex32f_t* __restrict__ src, uint32_t srcCount)
{
    assert((srcCount & 0x3) == 0); // count should be multiple of 4
    for(uint32_t i=0; i<srcCount; i+=4)
    {
        dest[i].i = src[i].i * 32767;
        dest[i].q = src[i].q * 32767;
        dest[i+1].i = src[i+1].i * 32767;
        dest[i+1].q = src[i+1].q * 32767;
        dest[i+2].i = src[i+2].i * 32767;
        dest[i+2].q = src[i+2].q * 32767;
        dest[i+3].i = src[i+3].i * 32767;
        dest[i+3].q = src[i+3].q * 32767;
    }
}

void complex32f_to_complex16_zip(complex16_t* __restrict__ dest, const complex32f_t* __restrict__ srcA, const complex32f_t* __restrict__ srcB, uint32_t srcCount)
{
    assert(srcCount > 0);
    assert((srcCount & 0x7) == 0); // count should be multiple of 8
    // loop increment needs to be 8 for the Arm compiler to auto vectorize to Neon instructions

    srcCount *= 2;
    const float* A = reinterpret_cast<const float*>(srcA);
    const float* B = reinterpret_cast<const float*>(srcB);
    int16_t* d = reinterpret_cast<int16_t*>(dest);

    const int16_t scale = 32767;
    for(uint32_t i=0; i<(srcCount & (~0x7)); ++i)
    {
        *d++ = A[i] * scale;
        *d++ = B[i] * scale;
    }
}

}
