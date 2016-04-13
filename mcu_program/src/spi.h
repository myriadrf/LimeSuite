#ifndef SPI_H
#define SPI_H

#include "typedefs.h"

extern void SPI_write(unsigned short spiAddrReg, unsigned short spiDataReg);
extern unsigned short SPI_read (unsigned short spiAddrReg);
void Modify_SPI_Reg_mask(const uint16_t *addr, const uint16_t *masks, const uint16_t *values, uint8_t start, uint8_t stop);

extern void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data);
extern uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits);

void BeginBatch(const char* name);
void EndBatch();

#endif
