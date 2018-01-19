#ifndef CONSTANTS_H
#define	CONSTANTS_H


namespace lime
{
    constexpr double cgenMax = 640e6;
    
    namespace program_mode
    {
        constexpr auto autoUpdate = "Automatic";
        constexpr auto fx3Flash = "FX3 FLASH";
        constexpr auto fx3RAM = "FX3 RAM";
        constexpr auto fx3Reset = "FX3 Reset";       
        constexpr auto fpgaFlash = "FPGA FLASH";
        constexpr auto fpgaRAM = "FPGA RAM";
        constexpr auto fpgaReset = "FPGA Reset";   
        constexpr auto mcuRAM = "LMSMCU SRAM";
        constexpr auto mcuEEPROM = "LMSMCU EEPROM";
        constexpr auto mcuReset = "LMSMCU Reset";  
    }
    
}


#endif	

