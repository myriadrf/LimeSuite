#ifndef LIME_SAMPLES_CONVERSION_H
#define LIME_SAMPLES_CONVERSION_H

#include <stdint.h>
#include "limesuite/complex.h"

namespace lime {

void complex16_to_complex32f(complex32f_t* __restrict__ dest, const complex16_t* __restrict__ src, uint32_t count);
void complex16_to_complex32f_unzip(
    complex32f_t* __restrict__ destA, complex32f_t* __restrict__ destB, const complex16_t* __restrict__ src, uint32_t srcCount);

void complex32f_to_complex16(complex16_t* __restrict__ dest, const complex32f_t* __restrict__ src, uint32_t count);
void complex32f_to_complex16_zip(complex16_t* __restrict__ dest,
    const complex32f_t* __restrict__ srcA,
    const complex32f_t* __restrict__ srcB,
    uint32_t srcCount);

} // namespace lime

#endif