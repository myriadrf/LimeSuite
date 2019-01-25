/*
 * File:   LimeSDR.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR.h"
#include "FPGA_common.h"
#include "device_constants.h"

namespace lime
{

LMS7_LimeSDR::LMS7_LimeSDR(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Generic(conn, obj)
{
}

std::vector<std::string> LMS7_LimeSDR::GetProgramModes() const
{
    return {program_mode::autoUpdate,
            program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::fx3Flash, program_mode::fx3Reset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_LimeSDR::SetRate(double f_Hz, int oversample)
{
    bool bypass = (oversample == 1) || (oversample == 0 && f_Hz > 62e6);

    for (unsigned i = 0; i < GetNumChannels(false);i++)
    {
        if (rx_channels[i].cF_offset_nco != 0.0 || tx_channels[i].cF_offset_nco != 0.0)
        {
            bypass = false;
            break;
        }
    }

    lime::LMS7002M* lms = lms_list[0];

    if (!bypass)
        return LMS7_Device::SetRate(f_Hz, oversample);

    if ((lms->SetFrequencyCGEN(f_Hz*4) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 7) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), 7) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1) != 0)
       || (lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), 7, 7) != 0))
       return -1;

    return SetFPGAInterfaceFreq(7, 7);
}

int LMS7_LimeSDR::Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const
{
    int ret = LMS7_Device::Program(mode, data, len, callback);

    if ((mode == program_mode::fx3Flash) || (mode == program_mode::fpgaFlash))
        connection->ProgramWrite(nullptr, 0, 0, 1, nullptr);
    return ret;
}

LMS7_CoreSDR::LMS7_CoreSDR(lime::IConnection* conn, LMS7_Device *obj) : LMS7_LimeSDR(conn, obj)
{
}

std::vector<std::string> LMS7_CoreSDR::GetProgramModes() const
{
    return {program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::fx3Flash, program_mode::fx3Reset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_CoreSDR::SetPath(bool tx, unsigned chan, unsigned path)
{
    if (path >= GetPathNames(tx, chan).size())
        return -1;

    int ret = LMS7_Device::SetPath(tx, chan, path);

    if (tx==false)
    {
        uint16_t value = fpga->ReadRegister(0x17);
        int bit_offset = chan ? 4 : 0;
        value &= ~(0xF<<bit_offset);
        if (path==LMS_PATH_LNAH)
            value |= (1<<bit_offset);
        else if (path==LMS_PATH_LNAW)
            value |= (2<<bit_offset);
        else if (path==LMS_PATH_LNAL)
            value |= (4<<bit_offset);
        ret |= fpga->WriteRegister(0x17, value);
    }
    else
    {
        int bit_offset = chan ? 12 : 8;
        if (path==LMS_PATH_TX1)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(0xF<<bit_offset);
            ret |= fpga->WriteRegister(0x17, value | (1<<bit_offset));
        }
        else if (path==LMS_PATH_TX2)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(0xF<<bit_offset);
            ret |= fpga->WriteRegister(0x17, value);
        }
    }

    return ret;
}

int LMS7_CoreSDR::SetRFSwitch(bool isTx, unsigned path, int chan)
{

    return 0;
}

LMS7_LimeSDR_PCIE::LMS7_LimeSDR_PCIE(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Generic(conn, obj)
{
}

std::vector<std::string> LMS7_LimeSDR_PCIE::GetProgramModes() const
{
    return {program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_LimeSDR_PCIE::SetRate(double f_Hz, int oversample)
{
    bool bypass = (oversample == 1) || (oversample == 0 && f_Hz > 62e6);

    for (unsigned i = 0; i < GetNumChannels(false);i++)
    {
        if (rx_channels[i].cF_offset_nco != 0.0 || tx_channels[i].cF_offset_nco != 0.0)
        {
            bypass = false;
            break;
        }
    }

    lime::LMS7002M* lms = lms_list[0];

    if (!bypass)
        return LMS7_Device::SetRate(f_Hz, oversample);

    if ((lms->SetFrequencyCGEN(f_Hz*4) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 7) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), 7) != 0)
       || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1) != 0)
       || (lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), 7, 7) != 0))
       return -1;

    return SetFPGAInterfaceFreq(7, 7);
}

}









