#ifndef SPI_H
#define SPI_H

#include "typedefs.h"

void SPI_write(uint16_t spiAddrReg, uint16_t spiDataReg);
uint16_t SPI_read(uint16_t spiAddrReg);

void Modify_SPI_Reg_bits_WrOnly(
    const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data, const uint16_t initialValue);
void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data);
uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits);

void SPI_read_batch(const uint16_t* addr, uint16_t* values, uint8_t cnt);
void SPI_write_batch(const uint16_t* addr, const uint16_t* values, uint8_t cnt);

void Modify_SPI_Reg_mask(const uint16_t* addr, const uint16_t* values, const uint16_t* masks, uint8_t cnt);

#endif
