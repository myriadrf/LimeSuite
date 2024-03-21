#ifndef LIME_SAMPLES_CONVERSION_H
#define LIME_SAMPLES_CONVERSION_H

#include <stdint.h>
#include "limesuite/complex.h"
#include "limesuite/config.h"

namespace lime {

void LIME_API complex16_to_complex32f(complex32f_t* dest, const complex16_t* src, uint32_t count);
void LIME_API complex16_to_complex32f_unzip(complex32f_t* destA, complex32f_t* destB, const complex16_t* src, uint32_t srcCount);

void LIME_API complex32f_to_complex16(complex16_t* dest, const complex32f_t* src, uint32_t count);
void LIME_API complex32f_to_complex16_zip(complex16_t* dest, const complex32f_t* srcA, const complex32f_t* srcB, uint32_t srcCount);

} // namespace lime

#endif
