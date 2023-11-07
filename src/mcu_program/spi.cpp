#include <ciso646>
#include <stdint.h>
#include "limesuite/LMS7002M.h"
#include "limesuite/commonTypes.h"

static lime::LMS7002M* serPort;

extern float bandwidthRF;
extern float RefClk;

void SetupCalibrations(lime::LMS7002M* chip, double BW)
{
    serPort = chip;
    bandwidthRF = BW;
    RefClk = chip->GetReferenceClk_SX(lime::TRXDir::Rx);
}

#include <vector>
#include <string>
#include <fstream>
using namespace std;

//spiAddrReg might not have SPI write bit, add it here if necessary
void SPI_write(unsigned short spiAddrReg, unsigned short spiDataReg)
{
    serPort->SPI_write(spiAddrReg, spiDataReg);
}

unsigned short SPI_read(unsigned short spiAddrReg)
{
    return serPort->SPI_read(spiAddrReg);
}

void Modify_SPI_Reg_bits_WrOnly(
    const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data, const uint16_t spiDataReg)
{
    const uint16_t spiMask = (~(~0u << ((bits >> 4) - (bits & 0xF) + 1))) << (bits & 0xF); // creates bit mask
    //spiDataReg = (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask) ;//clear bits
    SPI_write(SPI_reg_addr,
        (spiDataReg & (~spiMask)) | ((new_bits_data << (bits & 0xF)) & spiMask)); //write modified data back to SPI reg
}

void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data)
{
    uint16_t spiDataReg = SPI_read(SPI_reg_addr); //read current SPI reg data
    const uint16_t spiMask = (~(~0u << ((bits >> 4) - (bits & 0xF) + 1))) << (bits & 0xF); // creates bit mask
    spiDataReg = (spiDataReg & (~spiMask)) | ((new_bits_data << (bits & 0xF)) & spiMask); //clear bits

    SPI_write(SPI_reg_addr, spiDataReg); //write modified data back to SPI reg
}

uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits)
{
    return (SPI_read(SPI_reg_addr) & (~(~0u << ((bits >> 4) + 1)))) >> (bits & 0xF); //shift bits to LSB
}

void SPI_read_batch(const uint16_t* addr, uint16_t* values, uint8_t cnt)
{
    serPort->SPI_read_batch(addr, values, cnt);
}
void SPI_write_batch(const uint16_t* addr, const uint16_t* values, uint8_t cnt)
{
    serPort->SPI_write_batch(addr, values, cnt);
}

/** @brief Modifies given registers with values applied using masks
    @param addr array of register addresses
    @param masks array of applied masks
    @param values array of values to be written
    @param start starting index of given arrays
    @param stop end index of given arrays
*/
void Modify_SPI_Reg_mask(const uint16_t* addr, const uint16_t* values, const uint16_t* masks, uint8_t cnt)
{
    uint8_t i;
    for (i = 0; i < cnt; ++i)
        SPI_write(addr[i], (SPI_read(addr[i]) & ~masks[i]) | (values[i] & masks[i]));
}
