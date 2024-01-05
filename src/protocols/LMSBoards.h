/**
@file	LMSBoards.h
@author Lime Microsystems
@brief	enumerations of available LMS7002M hardware
*/
#pragma once

#include <array>
#include <string>

namespace lime {

enum eLMS_DEV {
    LMS_DEV_UNKNOWN = 0,
    LMS_DEV_EVB6 = 1,
    LMS_DEV_DIGIGREEN = 2,
    LMS_DEV_DIGIRED = 3, //2x USB3, LMS6002,
    LMS_DEV_EVB7 = 4,
    LMS_DEV_ZIPPER = 5, //MyRiad bridge to FMC, HSMC bridge
    LMS_DEV_SOCKETBOARD = 6,
    LMS_DEV_EVB7V2 = 7,
    LMS_DEV_STREAM = 8, //Altera Cyclone IV, USB3, 2x 128 MB RAM, RFDIO, FMC
    LMS_DEV_NOVENA = 9, //Freescale iMX6 CPU
    LMS_DEV_DATASPARK = 10, //Altera Cyclone V, 2x 256 MB RAM, 2x FMC (HPC, LPC), USB3
    LMS_DEV_RFSPARK = 11, //LMS7002 EVB
    LMS_DEV_LMS6002USB = 12, //LM6002-USB (USB stick: FX3, FPGA, LMS6002, RaspberryPi con)
    LMS_DEV_RFESPARK = 13, //LMS7002 EVB
    LMS_DEV_LIMESDR = 14, //LimeSDR-USB, 32bit FX3, 2xRAM, LMS7
    LMS_DEV_LIMESDR_PCIE = 15,
    LMS_DEV_LIMESDR_QPCIE = 16, //2x LMS, 14 bit ADC and DAC
    LMS_DEV_LIMESDRMINI = 17, //FTDI + MAX10 + LMS
    LMS_DEV_USTREAM = 18, //with expansion booards (uMyriad)
    LMS_DEV_LIMESDR_SONY_PA = 19, //stand alone board with Sony PAs, tuners
    LMS_DEV_LIMESDR_USB_SP = 20,
    LMS_DEV_LMS7002M_ULTIMATE_EVB = 21,
    LMS_DEV_LIMENET_MICRO = 22, //Raspberry Pi CM3(L), Ethernet, MAX10, LMS7002,
    LMS_DEV_LIMESDR_CORE_SDR = 23, //LMS7002, Intel Cyclone 4, RAM, GNSS
    LMS_DEV_LIMESDR_CORE_HE = 24, //PA board
    LMS_DEV_LIMESDRMINI_V2 = 25, //FTDI + ECP5 + LMS
    LMS_DEV_LIMESDR_X3 = 26, // 3xLMS
    LMS_DEV_LIMESDR_XTRX = 27, // XTRX
    LMS_DEV_LIMESDR_MMX8 = 28,

    LMS_DEV_COUNT
};

const std::array<const std::string, LMS_DEV_COUNT> LMS_DEV_NAMES = { "UNKNOWN",
    "EVB6",
    "DigiGreen",
    "DigiRed",
    "EVB7",
    "ZIPPER",
    "Socket Board",
    "EVB7_v2",
    "Stream",
    "Novena",
    "DataSpark",
    "RF-Spark",
    "LMS6002-USB Stick",
    "RF-ESpark",
    "LimeSDR-USB",
    "LimeSDR-PCIe",
    "LimeSDR-QPCIe",
    "LimeSDR-Mini",
    "uStream",
    "LimeSDR SONY PA",
    "LimeSDR-USB SP",
    "LMS7002M Ultimate EVB",
    "LimeNET-Micro",
    "LimeSDR-Core",
    "LimeSDR-Core-HE",
    "LimeSDR-Mini_v2",
    "LimeSDR X3",
    "LimeSDR XTRX",
    "LimeSDR MMX8" };

static constexpr const std::string& GetDeviceName(const eLMS_DEV device)
{
    if (LMS_DEV_UNKNOWN < device && device < LMS_DEV_COUNT)
    {
        return LMS_DEV_NAMES.at(device);
    }

    return LMS_DEV_NAMES.at(LMS_DEV_UNKNOWN);
}

enum eEXP_BOARD {
    EXP_BOARD_UNKNOWN,
    EXP_BOARD_UNSUPPORTED,
    EXP_BOARD_NO,
    EXP_BOARD_MYRIAD1,
    EXP_BOARD_MYRIAD2,
    EXP_BOARD_MYRIAD_NOVENA,
    EXP_BOARD_HPM1000,
    EXP_BOARD_MYRIAD7,
    EXP_BOARD_HPM7,
    EXP_BOARD_MYRIAD7_NOVENA,

    EXP_BOARD_COUNT
};

const std::array<const std::string, EXP_BOARD_COUNT> EXP_BOARD_NAMES = {
    "UNKNOWN",
    "UNSUPPORTED",
    "NOT AVAILABLE",
    "Myriad1",
    "Myriad2",
    "Novena",
    "HPM1000",
    "Myriad7",
    "HMP7",
    "Myriad7 Novena",
};

static constexpr const std::string& GetExpansionBoardName(const eEXP_BOARD board)
{
    if (EXP_BOARD_UNKNOWN < board && board < EXP_BOARD_COUNT)
    {
        return EXP_BOARD_NAMES.at(board);
    }

    return EXP_BOARD_NAMES.at(EXP_BOARD_UNKNOWN);
}

} // namespace lime
