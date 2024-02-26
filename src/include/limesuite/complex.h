#ifndef LIME_COMPLEX_H
#define LIME_COMPLEX_H

#include <cstdint>

namespace lime {

/** @brief Structure to hold a 12 bit integer complex number. */
struct complex12compressed_t {
    complex12compressed_t()
        : complex12compressed_t(0, 0)
    {
    }
    complex12compressed_t(int16_t i, int16_t q) { Set(i, q); }

    int16_t real() const
    {
        int16_t value = data[0];
        value |= (data[1] << 8);
        // shifting to fill sign
        value <<= 4;
        value >>= 4;
        return value;
    }
    void real(int16_t value)
    {
        data[0] = value;
        data[1] = (data[1] & 0xF0) | ((value >> 8) & 0x0F);
    }

    int16_t imag() const
    {
        int16_t value = data[1];
        value |= (data[2] << 8);
        value <<= 4;
        value >>= 4;
        return value;
    }
    void imag(int16_t value)
    {
        data[1] = (value << 4) | (data[1] & 0x0F);
        data[2] = value >> 4;
    }

    void Set(int16_t i, int16_t q)
    {
        data[0] = i;
        data[1] = (q << 4) | ((i >> 8) & 0x0F);
        data[2] = q >> 4;
    }

  private:
    uint8_t data[3];
};

// complex number structure for plain data types (int, float...)
template<class T> struct POD_complex_t {
    POD_complex_t()
        : POD_complex_t(0, 0)
    {
    }
    POD_complex_t(T real, T imag)
        : i(real)
        , q(imag)
    {
    }

    T real() const { return i; }
    void real(T value) { i = value; }
    T imag() const { return q; }
    void imag(T value) { q = value; }

    void Set(T ival, T qval)
    {
        i = ival;
        q = qval;
    }

    T i;
    T q;
};

/** @brief Structure to hold a 16 bit integer complex number. */
struct complex16_t : public POD_complex_t<int16_t> {
    complex16_t()
        : complex16_t(0, 0)
    {
    }
    complex16_t(int16_t re, int16_t im)
        : POD_complex_t<int16_t>(re, im)
    {
    }
};

/** @brief Structure to hold a 32 bit float complex number. */
struct complex32f_t : public POD_complex_t<float> {
    complex32f_t()
        : complex32f_t(0, 0)
    {
    }
    complex32f_t(float re, float im)
        : POD_complex_t<float>(re, im)
    {
    }
};

/** @brief Structure to hold a 64 bit float complex number. */
struct complex64f_t {
    double i; ///< The I component of the number.
    double q; ///< The Q component of the number.
};

} // namespace lime

#endif
