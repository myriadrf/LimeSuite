#include "spi.h"
#include "LMS7002_REGx51.h"

void SPI_transferVariable(unsigned short value)
{								  
	uint8_t spiIter;
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=0;	//set Clock low
		ucSDIN = value & 0x8000; //if current bit is 1 set Output High
		value <<= 1; //shift mask to right
		ucSCLK=1; 	//set Clock high
	}
	ucSCLK=0;	//set Clock low
}

void SPI_write(const unsigned short spiAddrReg, const unsigned short spiDataReg)
{
	ucSCLK=0;
	ucSEN=0;
	//write addr
	SPI_transferVariable(spiAddrReg | 0x8000); //set write bit
	//write data
	SPI_transferVariable(spiDataReg);
	ucSEN=1;
	ucSDIN=1;
}

unsigned short SPI_read (const unsigned short spiAddrReg)
{
	uint8_t spiIter;	
	uint16_t spiDataReg = 0;
	ucSCLK=0;
	ucSEN=0;
	//write addr
	SPI_transferVariable(spiAddrReg & ~0x8000);	//clear write bit
	ucSDIN=1;
	//read data
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=1; 	//set Clock high
		spiDataReg <<= 1;
		if (ucSDOUT)
			spiDataReg |= 1;
		ucSCLK=0;	//set Clock low
	}
	ucSEN=1;
	ucSDIN=1;
	return spiDataReg;
}
/*
void Modify_SPI_Reg_bits_WrOnly(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data, const uint16_t spiDataReg)
{
    const uint16_t spiMask = (~(~0 << ((bits>>4)-(bits&0xF)+1))) << (bits&0xF); // creates bit mask
   	SPI_write(SPI_reg_addr, (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask)); //write modified data back to SPI reg
}
*/
void Modify_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits, const uint16_t new_bits_data)
{
	uint16_t spiDataReg = SPI_read(SPI_reg_addr); //read current SPI reg data
	const uint16_t spiMask = (~(~0 << ((bits>>4)-(bits&0xF)+1))) << (bits&0xF); // creates bit mask
	spiDataReg = (spiDataReg & (~spiMask)) | ((new_bits_data << (bits&0xF)) & spiMask) ;//clear bits
	SPI_write(SPI_reg_addr, spiDataReg); //write modified data back to SPI reg
}

uint16_t Get_SPI_Reg_bits(const uint16_t SPI_reg_addr, const uint8_t bits)
{
	return (SPI_read(SPI_reg_addr) & (~(~0<<((bits>>4)+1)))) >> (bits&0xF); //shift bits to LSB
}

void Delay()
{
    volatile uint16_t i = 0xFF;
    while(i>0)
        --i;
}

void SPI_transferVariableSlow(unsigned short value)
{								  
	uint8_t spiIter;
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=0;	//set Clock low
        Delay();
		ucSDIN = value & 0x8000; //if current bit is 1 set Output High
		value <<= 1; //shift mask to right
        Delay();
		ucSCLK=1; 	//set Clock high
        Delay(); 
	}
    Delay();
	ucSCLK=0;	//set Clock low
}

void SPI_writeSlow(const unsigned short spiAddrReg, const unsigned short spiDataReg)
{
	ucSCLK=0;
	ucSEN=0;
    Delay();
	//write addr
	SPI_transferVariableSlow(spiAddrReg | 0x8000); //set write bit
	//write data
	SPI_transferVariableSlow(spiDataReg);
	ucSEN=1;
	ucSDIN=1;
}

unsigned short SPI_readSlow (const unsigned short spiAddrReg)
{
	uint8_t spiIter;	
	uint16_t spiDataReg = 0;
	ucSCLK=0;
	ucSEN=0;
    Delay();
	//write addr
	SPI_transferVariableSlow(spiAddrReg & ~0x8000);	//clear write bit
	ucSDIN=1;
	//read data
	for(spiIter = 16; spiIter>0; spiIter--) //MSB First
	{
		ucSCLK=1; 	//set Clock high
        Delay();
		spiDataReg <<= 1;
		if (ucSDOUT)
			spiDataReg |= 1;
        Delay();
		ucSCLK=0;	//set Clock low
	}
    Delay();
	ucSEN=1;
	ucSDIN=1;
	return spiDataReg;
}