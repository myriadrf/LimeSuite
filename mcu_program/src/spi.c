#include "spi.h"
#include "LMS7002_REGx51.h"

extern void SPI_wait_clk();

void SPI_transferVariable(const unsigned short value)
{								  
	xdata uint8_t spiIter;
	xdata uint16_t spiMask=0x8000;
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=0;	//set Clock low
		ucSDIN = value & spiMask; //if current bit is 1 set Output High
		spiMask = spiMask >> 1; //shift mask to right
		SPI_wait_clk();
		ucSCLK=1; 	//set Clock high
		SPI_wait_clk();
		ucSCLK=0;	//set Clock low
	}
}

void SPI_write(unsigned short spiAddrReg, unsigned short spiDataReg)
{
	spiAddrReg |= 0x8000; //set write bit
	ucSCLK=0;
	ucSEN=0;
	SPI_wait_clk();
	//write addr
	SPI_transferVariable(spiAddrReg);
	//write data
	SPI_transferVariable(spiDataReg);
	ucSEN=1;
	ucSDIN=1;
	SPI_wait_clk();
}

unsigned short SPI_read (unsigned short spiAddrReg)
{
	uint8_t spiIter;	
	uint16_t spiDataReg;
	uint16_t spiMask;	
	ucSCLK=0;
	ucSEN=0;
	spiAddrReg &= ~0x8000; //clear write bit	
	SPI_wait_clk();
	
	//write addr
	SPI_transferVariable(spiAddrReg);
	ucSDIN=1;
	//read data
	spiDataReg = 0; //set all bits to 0
	spiMask=0x8000;
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		SPI_wait_clk();
		ucSCLK=1; 	//set Clock high
		if (ucSDOUT)
			spiDataReg |= spiMask;
		spiMask = spiMask >> 1; //shift mask to right
		SPI_wait_clk();
		ucSCLK=0;	//set Clock low
	}
	SPI_wait_clk();
	ucSEN=1;
	ucSDIN=1;
	SPI_wait_clk();
	return spiDataReg;
}

void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data)
{
	uint16_t spiDataReg = SPI_read(SPI_reg_addr); //read current SPI reg data
	uint16_t spiMask = (~(~0 << ((bits>>4)-(bits&0xF)+1))) << (bits&0xF); // creates bit mask
	spiDataReg = (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask) ;//clear bits
	SPI_write(SPI_reg_addr, spiDataReg); //write modified data back to SPI reg
}

uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits)
{
	return (SPI_read(SPI_reg_addr) & (~(~0<<((bits>>4)+1)))) >> (bits&0xF); //shift bits to LSB
}

/** @brief Modifies given registers with values applied using masks
    @param addr array of register addresses
    @param masks array of applied masks
    @param values array of values to be written
    @param start starting index of given arrays
    @param stop end index of given arrays
*/
/*
void Modify_SPI_Reg_mask(const uint16_t *addr, const uint16_t *masks, const uint16_t *values, uint8_t start, uint8_t stop)
{	
    while (start <= stop)
    {   
        SPI_write(addr[start], ( SPI_read(addr[start]) & ~masks[start] ) | ( values[start] & masks[start] ));
        ++start;
    }
}
*/