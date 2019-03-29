/*
 * File:   LimeSDR_mini.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeNET_micro.h"
#include "Logger.h"
#include "FPGA_Mini.h"
#include "device_constants.h"

namespace lime
{

LMS7_LimeNET_micro::LMS7_LimeNET_micro(lime::IConnection* conn, LMS7_Device *obj):
    LMS7_LimeSDR_mini(conn, obj)
{
    if (lms_list[0]->GetReferenceClk_SX(false) < 0)
    {
        lime::info("Reference clock set to 30.72 MHz");
        lms_list[0]->SetReferenceClk_SX(false, 30.72e6);
    }
}

std::vector<std::string> LMS7_LimeNET_micro::GetPathNames(bool dir_tx, unsigned chan) const
{
    if (dir_tx)
        return {"NONE", "BAND1", "BAND2", "Auto"};
    else
	return {"NONE", "LNAH", "LNAL", "LNAW_NC", "Auto"};
}

int LMS7_LimeNET_micro::SetRFSwitch(bool isTx, unsigned path)
{
    int reg3 = fpga->ReadRegister(3);
    int bom_ver = reg3>>4;
    int hw_ver = reg3 & 0xF;
    if (isTx==false)
    {
        if (path==LMS_PATH_LNAW)
        {
            lime::warning("LNAW has no connection to RF ports");
        }
        else if (path==LMS_PATH_LNAL)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {
                value &= ~(0x0702);
                fpga->WriteRegister(0x17, value | 0x0502);
            }
            else
            {
                value &= ~(3<<8);
                fpga->WriteRegister(0x17, value | (1<<8));
            }
        }
        else if (path==LMS_PATH_LNAH)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {
                value &= ~(0x0702);
                fpga->WriteRegister(0x17, value | 0x0602);
            }
            else
            {
                value &= ~(3<<8);
                if (bom_ver == 0)
                    fpga->WriteRegister(0x17, value | (1<<8));
                else
                    fpga->WriteRegister(0x17, value | (2<<8));
            }
        }
    }
    else
    {
        if (path==LMS_PATH_TX1)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {
                value &= ~(0x7001);
                fpga->WriteRegister(0x17, value | 0x5000);
            }
            else
            {
                value &= ~(3<<12);
                fpga->WriteRegister(0x17, value | (1<<12));
            }
        }
        else if (path==LMS_PATH_TX2)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            if (hw_ver >= 3)
            {

                value &= ~(0x7001);
                fpga->WriteRegister(0x17, value | 0x6000);
            }
            else
            {
                value &= ~(3<<12);
                if (bom_ver == 0)
                    fpga->WriteRegister(0x17, value | (1<<12));
                else
                    fpga->WriteRegister(0x17, value | (2<<12));
            }
        }
    }
    return 0;
}

std::vector<std::string> LMS7_LimeNET_micro::GetProgramModes() const
{
    return {program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_LimeNET_micro::AutoRFPath(bool isTx, double f_Hz)
{
    int reg3 = fpga->ReadRegister(3);
    int bom_ver = reg3>>4;
    int hw_ver = reg3 & 0xF;
    if (hw_ver < 3 && bom_ver == 0)
        return 0;
    if ((!isTx) && (f_Hz < 1.7e9))
    {
        lime::info("Selected RX path: LNAL");
        int ret = SetPath(false, 0, LMS_PATH_LNAL);
        auto_rx_path = true;
        return ret;
    }
    return LMS7_LimeSDR_mini::AutoRFPath(isTx, f_Hz);
}

}//namespace lime


