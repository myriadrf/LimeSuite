#include "samplesConversion.h"
#include "string.h"
#include <assert.h>

namespace lime {

// templates with fixed iteration count allow to produce more efficient instructions

void complex12_to_complex16(complex16_t* dest, const complex12_t* src, uint32_t srcCount)
{
    PathSelection(dest, src, srcCount);
}

void complex12_to_complex16_unzip(complex16_t* destA, complex16_t* destB, const complex12_t* src, uint32_t srcCount)
{
    PathSelectionUnzip(destA, destB, src, srcCount);
}

void complex16_to_complex32f(complex32f_t* dest, const complex16_t* src, uint32_t srcCount)
{
    PathSelection(dest, src, srcCount);
}

void complex16_to_complex32f_unzip(complex32f_t* destA, complex32f_t* destB, const complex16_t* src, uint32_t srcCount)
{
    PathSelectionUnzip(destA, destB, src, srcCount);
}

void complex32f_to_complex16(complex16_t* dest, const complex32f_t* src, uint32_t srcCount)
{
    PathSelection(dest, src, srcCount);
}

void complex32f_to_complex16_zip(complex16_t* dest, const complex32f_t* srcA, const complex32f_t* srcB, uint32_t srcCount)
{
    PathSelectionZip(dest, srcA, srcB, srcCount);
}

} // namespace lime
