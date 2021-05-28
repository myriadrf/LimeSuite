/* --------------------------------------------------------------------------------------------
FILE:		CFRcomm.cpp
DESCRIPTION     Crest Factor Reduction API
CONTENT:
AUTHOR:		Lime Microsystems LTD
DATE:		Jan 01, 2018
-------------------------------------------------------------------------------------------- */

#include "CFRcomm.h"
#include "nrc.h"
#include "math.h"
#include <iostream>

CFRcomm::CFRcomm()
{
    chA = 1;
    lmsControl = NULL;
}

CFRcomm::~CFRcomm()
{
}

void CFRcomm::Connect(lms_device_t *device)
{
    lmsControl = device;
}

CFRcomm::Register::Register()
    : address(0), msb(0), lsb(0), defaultValue(0), twocomplement(0)
{
}

CFRcomm::Register::Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue, unsigned short twocomplement)
    : address(address), msb(msb), lsb(lsb), defaultValue(defaultValue), twocomplement(twocomplement)
{
}

int CFRcomm::SPI_write(lms_device_t *lmsControl, uint16_t address, uint16_t data)
{

    int ret = 0;
    if (LMS_WriteFPGAReg(lmsControl, address, data) != 0)
        ret = -1;
    return ret;
}

void CFRcomm::SetCFRFilterOrder(int val, int interpolation)
{

    unsigned short mask;
    uint16_t regValue;
        
    int order = val;
    if (order > 40)
        order = 40; // max
    if (order < 1)
        order = 1; // min
    if (lmsControl == NULL)
        return;
    Register reg, reg2;

    if (chA == 1) {
        reg = Register(0x004A, 7, 0, 0, 0); // CFR_ORDER
        reg2 = Register(0x0041, 13, 12, 0, 0);  // INTER_CFR
    }
    else {
        reg = Register(0x004A, 15, 8, 0, 0); // CFR_ORDER_chB
        reg2 = Register(0x0041, 15, 14, 0, 0); // INTER_CFR_chB
    }

    mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content
    regValue &= ~mask;
    regValue |= (order << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0)
        ; // register write

    mask = (~(~0 << (reg2.msb - reg2.lsb + 1))) << reg2.lsb;
    LMS_ReadFPGAReg(lmsControl, reg2.address, &regValue); // read bit content
    regValue &= ~mask;
    regValue |= (interpolation << reg2.lsb) & mask;
    if (LMS_WriteFPGAReg(lmsControl, reg2.address, regValue) != 0)
        ; // register write

    UpdateHannCoeff(lmsControl, order, chA, interpolation);

    if (chA == 1)
    {
        //std::cout << "[INFO] CFR Ch.A parameters set:" <<  std::endl;
        std::cout << "[INFO] CFR Ch.A order=" << order << " interpolation=" << interpolation << std::endl;
    }
    else
    {
        //std::cout << "[INFO] CFR Ch.B parameters set:" <<  std::endl;
        std::cout << "[INFO] CFR Ch.B order=" << order << " interpolation=" << interpolation << std::endl;
    }
}

void CFRcomm::SetRegValue(Register reg, uint16_t newValue)
{

    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content
    regValue &= ~mask;
    regValue |= (newValue << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0)
        ; // register write
}

void CFRcomm::SetCFRThreshold(double threshold)
{

    double threshold1 = threshold;

    if (threshold1 > 1.0)
        threshold1 = 1.0; // max
    if (threshold1 < 0.4)
        threshold1 = 0.4; // min
    if (lmsControl == NULL)
        return;
    Register reg;

    if (chA == 1)
        reg = Register(0x0046, 15, 0, 0, 0); // thresholdSpin
    else
        reg = Register(0x0047, 15, 0, 0, 0); // thresholdSpin_chB

    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content

    int code = (int)(threshold1 * 65535);
    if (code >= 65535)
        code = 65535;
    if (code < 0)
        code = 0;

    regValue &= ~mask;
    regValue |= (code << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0)
        ; // register write
    //wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));
    if (chA == 1)
        std::cout << "[INFO] CFR Ch.A threshold= " << threshold1 << std::endl;
    else
        std::cout << "[INFO] CFR Ch.B threshold= " << threshold1 << std::endl;
}

void CFRcomm::SetCFRGain(double threshold)
{

    double threshold1 = threshold;
    if (threshold1 > 2.0)
        threshold1 = 2.0; // max
    if (threshold1 < 0.1)
        threshold1 = 0.1; // min
    if (lmsControl == NULL)
        return;
    Register reg;

    if (chA == 1)
        reg = Register(0x0048, 15, 0, 0, 0); // thresholdGain
    else
        reg = Register(0x0049, 15, 0, 0, 0); // thresholdGain

    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask
    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content

    int code = (int)(threshold * 8192);
    if (code >= 32767)
        code = 32767;
    if (code < 0)
        code = 0;

    regValue &= ~mask;
    regValue |= (code << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0)
        ; // register write

    if (chA == 1)
        std::cout << "[INFO] Gain CFR Ch.A = " << threshold1 << std::endl;
    else
        std::cout << "[INFO] Gain CFR Ch.B = " << threshold1 << std::endl;
}

void CFRcomm::UpdateHannCoeff(lms_device_t *lmsControl, uint16_t Filt_N, int chAA, int interpolation)
{
    Register reg, reg2, reg3, reg4;
    uint16_t msb, lsb = 0;
    uint16_t data = 0;
    uint16_t addr = 0;
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t offset = 0;
    uint16_t w[60];
    uint16_t maddressf0 = 0x07;
    uint16_t maddressf1 = 0x08;

    uint16_t NN = 3;
    uint16_t MaxFilt_N = 40;

    if (interpolation > 1)
        interpolation = 1; //limit
    else if (interpolation < 0)
        interpolation = 0; //limit

    if (interpolation == 2)
    {
        NN = 0;
        MaxFilt_N = 10;
    }
    else if (interpolation == 1)
    {
        NN = 1;
        MaxFilt_N = 20;
    }
    else
    {
        NN = 3;
        MaxFilt_N = 40;
    }

    if (Filt_N > MaxFilt_N)
        Filt_N = MaxFilt_N;

    for (i = 0; i < Filt_N; i++)
        w[i] = (uint16_t)(32768.0 * 0.25 * (1.0 - cos(2.0 * M_PI * i / (Filt_N - 1))));

    if (chAA == 1)
        reg = Register(0x0045, 0, 0, 0, 0); // [chkSLEEP_CFR]
    else
        reg = Register(0x0045, 8, 8, 0, 0); // [chkSLEEP_CFR_chB]

    if (chAA == 1)
        reg2 = Register(0x0045, 2, 2, 0, 0); //[[chkODD_CFR]
    else
        reg2 = Register(0x0045, 10, 10, 0, 0); //  [chkODD_CFR_chB]

    reg3 = Register(0x0041, 11, 11, 0, 0); //[chkResetN];

    if (chAA == 1)
        reg4 = Register(0x0045, 7, 7, 0, 0); //[chkDEL_HB];
    else
        reg4 = Register(0x0045, 15, 15, 0, 0); //[chkDEL_HB_chB];

    SetRegValue(reg, 1); // sleep <= '1';

    msb = lsb = 0;
    data = 0;
    i = 0;

    SetRegValue(reg3, 0); // sleep <= '0';
    SetRegValue(reg3, 1); // sleep <= '0';

    while (i < MaxFilt_N) //40
    {                     // reset all
        addr = (2 << 15) + (maddressf0 << 6) + (msb << 2) + lsb;
        SPI_write(lmsControl, addr, data);
        addr = (2 << 15) + (maddressf1 << 6) + (msb << 2) + lsb;
        SPI_write(lmsControl, addr, data);
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;
        i++;
    }

    msb = lsb = 0;
    i = j = 0;
    offset = 0;
    while (i <= (uint16_t)((Filt_N / 2) - 1))
    {
        addr = (2 << 15) + (maddressf1 << 6) + (msb << 2) + lsb;
        if (j >= offset)
            data = w[(uint16_t)((Filt_N + 1) / 2 + i)];
        else
            data = 0;
        SPI_write(lmsControl, addr, data);
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;

        if (j >= offset)
            i++;
        j++;
    }

    msb = lsb = 0;
    i = j = 0;
    offset = (MaxFilt_N / 2) - ((uint16_t)((Filt_N + 1) / 2));
    while (i < Filt_N)
    {
        addr = (2 << 15) + (maddressf0 << 6) + (msb << 2) + lsb;

        if (j >= offset)
            data = w[i];
        else
            data = 0;

        SPI_write(lmsControl, addr, data);
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;

        if (j >= offset)
            i++;
        j++;
    }
    if ((Filt_N % 2) == 1)
        SetRegValue(reg2, 1); // odd
    else
        SetRegValue(reg2, 0); // even

    if (interpolation == 1)
        SetRegValue(reg4, 1);
    else
        SetRegValue(reg4, 0);

    SetRegValue(reg, 0); // sleep <= '0';

    // software reset
    SetRegValue(reg3, 0); // reset_n <= '0';
    SetRegValue(reg3, 1); // reset_n <= '1';
}

void CFRcomm::BypassCFR(int val)
{

    if (lmsControl == NULL)
        return;
    int val1 = val;
    if (val1 > 1)
        val1 = 1; // max
    if (val1 < 0)
        val1 = 0; // min
    Register reg;

    if (chA == 1)
        reg = Register(0x0045, 1, 1, 0, 0); // chkBYPASS_CFR
    else
        reg = Register(0x0045, 9, 9, 0, 0); // chkBYPASS_CFR_chB

    SetRegValue(reg, val1); // sleep <= '0';

    if (chA == 1)
    {
        if (val1 == 1)
            std::cout << "[INFO] CFR Ch.A bypassed" << std::endl;
        else
            std::cout << "[INFO] CFR Ch.A on" << std::endl;
    }
    else
    {
        if (val1 == 1)
            std::cout << "[INFO] CFR Ch.B bypassed" << std::endl;
        else
            std::cout << "[INFO] CFR Ch.B on" << std::endl;
    }
}

void CFRcomm::BypassCFRGain(int val)
{

    if (lmsControl == NULL)
        return;
    int val1 = val;
    if (val1 > 1)
        val1 = 1; // max
    if (val1 < 0)
        val1 = 0; // min
    Register reg;
    if (chA == 1)
        reg = Register(0x0045, 3, 3, 0, 0); // chkBYPASS_CFR
    else
        reg = Register(0x0045, 11, 11, 0, 0); // chkBYPASS_CFR_chB

    SetRegValue(reg, val1); // sleep <= '0';
    if (chA == 1)
    {
        if (val1 == 1)
            std::cout << "[INFO] Gain CFR at Ch.A bypassed" << std::endl;
        else
            std::cout << "[INFO] Gain CFR at Ch.A on" << std::endl;
    }
    else
    {
        if (val1 == 1)
            std::cout << "[INFO] Gain CFR at Ch.B bypassed" << std::endl;
        else
            std::cout << "[INFO] Gain CFR at Ch.B on" << std::endl;
    }
}

void CFRcomm::SelectCHA(int val)
{

    int val1 = val;

    if (val1 > 1)
        val1 = 1; // max
    if (val1 < 0)
        val1 = 0; // min

    chA = val1;
    if (val1 == 1)
        std::cout << "[INFO] CFR Ch.A selected" << std::endl;
    else
        std::cout << "[INFO] CFR Ch.B selected" << std::endl;
}
