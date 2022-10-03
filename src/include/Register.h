#ifndef LIME_REGISTER_H
#define LIME_REGISTER_H

#include <stdint.h>

namespace lime {

inline constexpr uint32_t bitMask(uint8_t msb, uint8_t lsb)
{
    return (~(~0u << (msb - lsb + 1))) << (lsb);
}

struct Register {
    Register() : address(0), defaultValue(0), msb(15), lsb(0), twoComplement(false) {};
    constexpr Register(uint16_t address, uint8_t msb, uint8_t lsb, uint16_t defaultValue, bool twocomplement)
        : address(address), defaultValue(defaultValue), msb(msb), lsb(lsb), twoComplement(twocomplement)
    {};
    uint16_t address;
    uint16_t defaultValue;
    uint8_t msb;
    uint8_t lsb;
    bool twoComplement;
};

}
#endif // LIME_REGISTER_H