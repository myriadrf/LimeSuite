#ifndef LIME_COMPLEX_H
#define LIME_COMPLEX_H

#include <cstdint>
#include <type_traits>
#include <complex>

namespace lime {

/** @brief Structure to hold a 12 bit packed integer complex number.
    Used only for data transfering to hardware.
*/
struct complex12packed_t {
    constexpr complex12packed_t()
        : complex12packed_t(0, 0)
    {
    }
    constexpr complex12packed_t(int16_t i, int16_t q)
        : data{ uint8_t(i), uint8_t((q << 4) | ((i >> 8) & 0x0F)), uint8_t(q >> 4) }
    {
    }

    constexpr int16_t real() const
    {
        int16_t value = data[0];
        value |= (data[1] << 8);
        // shifting to fill sign
        value <<= 4;
        value >>= 4;
        return value;
    }

    constexpr void real(int16_t value)
    {
        data[0] = value;
        data[1] = (data[1] & 0xF0) | ((value >> 8) & 0x0F);
    }

    constexpr int16_t imag() const
    {
        int16_t value = data[1];
        value |= (data[2] << 8);
        value >>= 4;
        return value;
    }

    constexpr void imag(int16_t value)
    {
        data[1] = (value << 4) | (data[1] & 0x0F);
        data[2] = value >> 4;
    }

    constexpr void Set(int16_t i, int16_t q)
    {
        data[0] = i;
        data[1] = (q << 4) | ((i >> 8) & 0x0F);
        data[2] = q >> 4;
    }

  private:
    uint8_t data[3];
};
static_assert(std::is_trivially_copyable<complex12packed_t>::value == true);

// complex number structure for plain data types (int, float...)
template<class T> struct POD_complex_t {
    constexpr POD_complex_t()
        : POD_complex_t(0, 0)
    {
    }
    constexpr POD_complex_t(T real, T imag)
        : i(real)
        , q(imag)
    {
    }

    constexpr T real() const { return i; }
    constexpr void real(T value) { i = value; }
    constexpr T imag() const { return q; }
    constexpr void imag(T value) { q = value; }

    constexpr void Set(T ival, T qval)
    {
        i = ival;
        q = qval;
    }

    T i;
    T q;
};

/** @brief Structure to hold a 16 bit integer complex number. */
using complex16_t = POD_complex_t<int16_t>;
static_assert(std::is_trivially_copyable<complex16_t>::value == true);

/** @brief Structure to hold a 12 bit integer complex number.
    Stored as 16bit, but the actual used values range should be of 12bits
*/
// inheriting complex16_t instead of using alias of POD_complex_t<int16_t> to
// differentiate types for templating
class complex12_t : public complex16_t
{
    constexpr complex12_t()
        : complex16_t(){};
    constexpr complex12_t(int16_t real, int16_t imag)
        : complex16_t(real, imag){};
};
static_assert(std::is_trivially_copyable<complex12_t>::value == true);

/** @brief Structure to hold a 32 bit float complex number. */
using complex32f_t = POD_complex_t<float>;
static_assert(std::is_trivially_copyable<complex32f_t>::value == true);

/** @brief Structure to hold a 64 bit float complex number. */
using complex64f_t = POD_complex_t<double>;
static_assert(std::is_trivially_copyable<complex64f_t>::value == true);

} // namespace lime

#endif
