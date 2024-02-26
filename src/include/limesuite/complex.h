#ifndef LIME_COMPLEX_H
#define LIME_COMPLEX_H

#include <cstdint>

namespace lime {

/// @brief Structure to hold a 12 bit integer complex number.
struct complex12compressed_t {
    constexpr complex12compressed_t()
        : complex12compressed_t(0, 0)
    {
    }

    /// @brief Constructs the 12 bit compressed complex number.
    /// @param i The I value of the number.
    /// @param q The Q value of the number.
    constexpr complex12compressed_t(int16_t i, int16_t q)
        : data{ 0, 0, 0 }
    {
        Set(i, q);
    }

    /// @copydoc POD_complex_t::real()
    constexpr int16_t real() const
    {
        int16_t value = data[0];
        value |= (data[1] << 8);
        // shifting to fill sign
        value <<= 4;
        value >>= 4;
        return value;
    }

    /// @copydoc POD_complex_t::real(T)
    constexpr void real(int16_t value)
    {
        data[0] = value;
        data[1] = (data[1] & 0xF0) | ((value >> 8) & 0x0F);
    }

    /// @copydoc POD_complex_t::imag()
    constexpr int16_t imag() const
    {
        int16_t value = data[1];
        value |= (data[2] << 8);
        value <<= 4;
        value >>= 4;
        return value;
    }

    /// @copydoc POD_complex_t::imag(T)
    constexpr void imag(int16_t value)
    {
        data[1] = (value << 4) | (data[1] & 0x0F);
        data[2] = value >> 4;
    }

    /// @copydoc POD_complex_t::Set()
    constexpr void Set(int16_t ival, int16_t qval)
    {
        data[0] = ival;
        data[1] = (qval << 4) | ((ival >> 8) & 0x0F);
        data[2] = qval >> 4;
    }

  private:
    uint8_t data[3];
};

/// @brief Complex number structure for plain data types (int, float...)
/// @tparam T The type of the number to hold.
template<class T> struct POD_complex_t {
    constexpr POD_complex_t()
        : POD_complex_t(0, 0)
    {
    }

    /// @brief Constructs the complex number with the specified values.
    /// @param real The I value of the number.
    /// @param imag The Q value of the number.
    constexpr POD_complex_t(T real, T imag)
        : i(real)
        , q(imag)
    {
    }

    /// @brief Gets the I (real) value of the complex number.
    /// @return The I value of the complex number.
    constexpr T real() const { return i; }

    /// @brief Sets the I (real) value of the complex number.
    /// @param value The value to set the complex number's I component to.
    constexpr void real(T value) { i = value; }

    /// @brief Gets the Q (imaginary) value of the complex number.
    /// @return The Q value of the complex number.
    constexpr T imag() const { return q; }

    /// @brief Sets the Q (imaginary) value of the complex number.
    /// @param value The value to set the complex number's Q component to.
    constexpr void imag(T value) { q = value; }

    /// @brief Sets both values of the complex number.
    /// @param ival The I (real) component of the complex number.
    /// @param qval The Q (imaginary) component of the complex number.
    constexpr void Set(T ival, T qval)
    {
        i = ival;
        q = qval;
    }

    T i; ///< The I component of the number.
    T q; ///< The Q component of the number.
};

/** @brief Structure to hold a 16 bit integer complex number. */
struct complex16_t : public POD_complex_t<int16_t> {
    constexpr complex16_t()
        : complex16_t(0, 0)
    {
    }

    /// @brief Constructs the 16 bit integer complex number.
    /// @param re The I (real) component of the number.
    /// @param im The Q (imaginary) component of the number.
    constexpr complex16_t(int16_t re, int16_t im)
        : POD_complex_t<int16_t>(re, im)
    {
    }
};

/** @brief Structure to hold a 32 bit float complex number. */
struct complex32f_t : public POD_complex_t<float> {
    constexpr complex32f_t()
        : complex32f_t(0, 0)
    {
    }

    /// @brief Constructs the 32 bit floating-point complex number.
    /// @param re The I (real) component of the number.
    /// @param im The Q (imaginary) component of the number.
    constexpr complex32f_t(float re, float im)
        : POD_complex_t<float>(re, im)
    {
    }
};

/** @brief Structure to hold a 64 bit float complex number. */
struct complex64f_t : public POD_complex_t<double> {
    constexpr complex64f_t()
        : complex64f_t(0, 0)
    {
    }

    /// @brief Constructs the 64 bit floating-point complex number.
    /// @param re The I (real) component of the number.
    /// @param im The Q (imaginary) component of the number.
    constexpr complex64f_t(double re, double im)
        : POD_complex_t<double>(re, im)
    {
    }
};

} // namespace lime

#endif
