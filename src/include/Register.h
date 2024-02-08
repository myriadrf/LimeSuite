#ifndef LIME_REGISTER_H
#define LIME_REGISTER_H

#include <cstdint>

namespace lime {

/**
 * @brief Gets the bit mask for the register
 * @param msb The index of the most significant bit
 * @param lsb The index of the least significant bit
 * @return The mask of the bits of the register.
 */
constexpr uint32_t bitMask(uint8_t msb, uint8_t lsb)
{
    return (~(~0u << (msb - lsb + 1))) << (lsb);
}

/**
 * @brief A structure for holding information about a register.
 */
struct Register {
    /// @brief Constructs the register with default values.
    constexpr Register()
        : address(0)
        , defaultValue(0)
        , msb(15)
        , lsb(0)
        , twoComplement(false){};

    /// @brief Constructs the register with the given values.
    /// @param address The memory address of the register.
    /// @param msb The index of the most significant bit of the register.
    /// @param lsb The index of the least significant bit of the register.
    /// @param defaultValue The default value of the register.
    /// @param twocomplement Whether the register is represented in a Two's Complement way.
    constexpr Register(uint16_t address, uint8_t msb, uint8_t lsb, uint16_t defaultValue, bool twocomplement)
        : address(address)
        , defaultValue(defaultValue)
        , msb(msb)
        , lsb(lsb)
        , twoComplement(twocomplement){};

    uint16_t address; ///< The memory address.
    uint16_t defaultValue; ///< The default value of the register.
    uint8_t msb; ///< The index of the most significant bit of the register.
    uint8_t lsb; ///< The index of the least significant bit of the register.
    bool twoComplement; ///< Indicates if the register is represented in a Two's Complement way.
};

} // namespace lime
#endif // LIME_REGISTER_H
