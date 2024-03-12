#ifndef LIME_SAMPLES_CONVERSION_H
#define LIME_SAMPLES_CONVERSION_H

#include <stdint.h>
#include "limesuite/complex.h"

namespace lime {

template<class Dest, class Src> constexpr float GetScalingRatio()
{
    if constexpr (std::is_same<Src, complex16_t>::value == true)
    {
        if constexpr (std::is_same<Dest, complex32f_t>::value == true || std::is_same<Dest, complex64f_t>::value == true)
            return 1.0f / 32768;
        else if constexpr (std::is_same<Dest, complex12_t>::value == true || std::is_same<Dest, complex12packed_t>::value == true)
            return 2048.0 / 32768.0;
    }
    else if constexpr (std::is_same<Src, complex12_t>::value == true || std::is_same<Src, complex12packed_t>::value == true)
    {
        if constexpr (std::is_same<Dest, complex32f_t>::value == true || std::is_same<Dest, complex64f_t>::value == true)
            return 1.0f / 2048;
        else if constexpr (std::is_same<Dest, complex16_t>::value == true)
            return 16;
        else if constexpr (std::is_same<Dest, complex12packed_t>::value == true)
            return 1;
    }
    else if constexpr (std::is_same<Src, complex32f_t>::value == true || std::is_same<Src, complex64f_t>::value == true)
    {
        if constexpr (std::is_same<Dest, complex16_t>::value == true)
            return 32767;
        else if constexpr (std::is_same<Dest, complex12_t>::value == true || std::is_same<Dest, complex12packed_t>::value == true)
            return 2047;
    }
    return 1;
}

// Generic case using multiplication
template<class D, class S> constexpr void Rescale(D& dest, const S& src)
{
    constexpr float ratio = GetScalingRatio<D, S>();
    dest.real(src.real() * ratio);
    dest.imag(src.imag() * ratio);
}

// Specialized cases to utilize bitshifting or direct copy
template<> constexpr void Rescale(complex12_t& dest, const complex16_t& src)
{
    dest.real(src.real() >> 4);
    dest.imag(src.imag() >> 4);
}

template<> constexpr void Rescale(complex16_t& dest, const complex12_t& src)
{
    dest.real(src.real() << 4);
    dest.imag(src.imag() << 4);
}

template<> constexpr void Rescale(complex12packed_t& dest, const complex12_t& src)
{
    dest.real(src.real());
    dest.imag(src.imag());
}

template<> constexpr void Rescale(complex12_t& dest, const complex12packed_t& src)
{
    dest.real(src.real());
    dest.imag(src.imag());
}

template<> constexpr void Rescale(complex12packed_t& dest, const complex16_t& src)
{
    dest.real(src.real() >> 4);
    dest.imag(src.imag() >> 4);
}

template<> constexpr void Rescale(complex16_t& dest, const complex12packed_t& src)
{
    dest.real(src.real() << 4);
    dest.imag(src.imag() << 4);
}

// compile time known iteration/element count
template<uint32_t srcCount, class DestT, class SrcT> static void fastPath_convert(DestT* dest, const SrcT* src)
{
    for (uint32_t i = 0; i < srcCount; ++i)
        Rescale(dest[i], src[i]);
}

// dynamic iteration count, will generate extra SIMD instuctions for various last iteration counts
template<class DestT, class SrcT> static void slowPath_convert(DestT* dest, const SrcT* src, uint32_t srcCount)
{
    for (uint16_t i = 0; i < srcCount; ++i)
        Rescale(dest[i], src[i]);
}

template<class DestT, class SrcT> static void PathSelection(DestT* dest, SrcT* src, uint32_t srcCount)
{
    switch (srcCount)
    {
    case 512:
        fastPath_convert<512>(dest, src);
        break;
    case 256:
        fastPath_convert<256>(dest, src);
        break;
    case 128:
        fastPath_convert<128>(dest, src);
        break;
    case 64:
        fastPath_convert<64>(dest, src);
        break;
    default:
        slowPath_convert(dest, src, srcCount);
        break;
    }
}

template<uint32_t srcCount, class DestT, class SrcT> static void fastPath_convert_unzip(DestT* destA, DestT* destB, const SrcT* src)
{
    for (uint32_t i = 0; i < srcCount / 2; i++)
    {
        const uint16_t srcPos = 2 * i;
        Rescale(destA[i], src[srcPos]);
        Rescale(destB[i], src[srcPos + 1]);
    }
}

template<class DestT, class SrcT> static void slowPath_convert_unzip(DestT* destA, DestT* destB, const SrcT* src, uint32_t srcCount)
{
    for (uint32_t i = 0; i < srcCount / 2; i++)
    {
        const uint16_t srcPos = 2 * i;
        Rescale(destA[i], src[srcPos]);
        Rescale(destB[i], src[srcPos + 1]);
    }
}

template<class DestT, class SrcT> static void PathSelectionUnzip(DestT* destA, DestT* destB, const SrcT* src, uint32_t srcCount)
{
    switch (srcCount)
    {
    case 512:
        fastPath_convert_unzip<512>(destA, destB, src);
        break;
    case 256:
        fastPath_convert_unzip<256>(destA, destB, src);
        break;
    case 128:
        fastPath_convert_unzip<128>(destA, destB, src);
        break;
    case 64:
        fastPath_convert_unzip<64>(destA, destB, src);
        break;
    default:
        slowPath_convert_unzip(destA, destB, src, srcCount);
        break;
    }
}

template<uint32_t srcCount, class DestT, class SrcT>
static void fastPath_convert_zip(DestT* dest, const SrcT* srcA, const SrcT* srcB)
{
    for (uint32_t i = 0; i < srcCount; i++)
    {
        const uint32_t destPos = 2 * i;
        Rescale(dest[destPos], srcA[i]);
        Rescale(dest[destPos + 1], srcB[i]);
    }
}

template<class DestT, class SrcT>
static void slowPath_convert_zip(DestT* dest, const SrcT* srcA, const SrcT* srcB, uint32_t srcCount)
{
    for (uint32_t i = 0; i < srcCount; i++)
    {
        const uint32_t destPos = 2 * i;
        Rescale(dest[destPos], srcA[i]);
        Rescale(dest[destPos + 1], srcB[i]);
    }
}

template<class DestT, class SrcT> static void PathSelectionZip(DestT* dest, SrcT* srcA, SrcT* srcB, uint32_t srcCount)
{
    switch (srcCount)
    {
    case 512:
        fastPath_convert_zip<512>(dest, srcA, srcB);
        break;
    case 256:
        fastPath_convert_zip<256>(dest, srcA, srcB);
        break;
    case 128:
        fastPath_convert_zip<128>(dest, srcA, srcB);
        break;
    case 64:
        fastPath_convert_zip<64>(dest, srcA, srcB);
        break;
    default:
        slowPath_convert_zip(dest, srcA, srcB, srcCount);
        break;
    }
}

} // namespace lime

#endif
