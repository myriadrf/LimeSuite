#ifndef LIME_COMPLEX_H
#define LIME_COMPLEX_H

#include <cstdint>

namespace lime {

/** @brief Structure to hold a 16 bit integer complex number. */
struct complex16_t {
    int16_t i; ///< The I component of the number.
    int16_t q; ///< The Q component of the number.
};

/** @brief Structure to hold a 32 bit float complex number. */
struct complex32f_t {
    float i; ///< The I component of the number.
    float q; ///< The Q component of the number.
};

/** @brief Structure to hold a 64 bit float complex number. */
struct complex64f_t {
    double i; ///< The I component of the number.
    double q; ///< The Q component of the number.
};

} // namespace lime

#endif
