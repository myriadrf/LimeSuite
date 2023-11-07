#include "CDCM6208_Dev.h"
#include <string>
#include <unordered_map>
#include <chrono>
#include <math.h>
#include <thread>

using namespace lime;

CDCM_Dev::CDCM_Dev(std::shared_ptr<ISPI> comms, uint16_t SPI_BASE_ADDR)
    : comms(comms)
    , SPI_BASE_ADDR(SPI_BASE_ADDR)
    , is_locked(false)
{
    VCO.version = 1;
    VCO.min_freq = CDCM_VCO_MIN_V2;
    VCO.max_freq = CDCM_VCO_MAX_V2;
}

/**
    @brief Initializes CDCM device parameters.
    @param primaryFreq Primary VCO input frequency.
    @param secondaryFreq Secondary VCO input frequency.
    @return 0 on success; -1 on error.
*/
int CDCM_Dev::Init(double primaryFreq, double secondaryFreq)
{
    VCO.prim_freq = primaryFreq;
    VCO.sec_freq = secondaryFreq;

    outputs.Y0Y1.requested_freq = 30.72e6;
    outputs.Y2Y3.requested_freq = 30.72e6;
    outputs.Y4.requested_freq = 30.72e6;
    outputs.Y5.requested_freq = 30.72e6;
    outputs.Y6.requested_freq = 30.72e6;
    outputs.Y7.requested_freq = 30.72e6;

    return DownloadConfiguration();
}

/**
    @brief Resets CDCM to the default configuration.
    @param primaryFreq Primary VCO input frequency.
    @param secondaryFreq Secondary VCO input frequency.
    @return 0 on success; -1 on error.
*/
int CDCM_Dev::Reset(double primaryFreq, double secondaryFreq)
{
    VCO.prim_freq = primaryFreq;
    VCO.sec_freq = secondaryFreq;

    outputs.Y0Y1.requested_freq = 30.72e6;
    outputs.Y2Y3.requested_freq = 30.72e6;
    outputs.Y4.requested_freq = 30.72e6;
    outputs.Y5.requested_freq = 30.72e6;
    outputs.Y6.requested_freq = 30.72e6;
    outputs.Y7.requested_freq = 30.72e6;

    struct regVal {
        uint16_t addr;
        uint16_t val;
    };
    std::map<int, regVal> CDCM_Regs = { { 1, { (uint16_t)(SPI_BASE_ADDR + 1), 0x0000 } },
        { 2, { (uint16_t)(SPI_BASE_ADDR + 2), 0x0018 } },
        { 3, { (uint16_t)(SPI_BASE_ADDR + 3), 0x00F0 } },
        { 4, { (uint16_t)(SPI_BASE_ADDR + 4), 0x20FF } },
        { 6, { (uint16_t)(SPI_BASE_ADDR + 6), 0x0018 } },
        { 8, { (uint16_t)(SPI_BASE_ADDR + 8), 0x0018 } },
        { 9, { (uint16_t)(SPI_BASE_ADDR + 9), 0x0003 } },
        { 10, { (uint16_t)(SPI_BASE_ADDR + 10), 0x0180 } },
        { 11, { (uint16_t)(SPI_BASE_ADDR + 11), 0x0000 } },
        { 12, { (uint16_t)(SPI_BASE_ADDR + 12), 0x0003 } },
        { 13, { (uint16_t)(SPI_BASE_ADDR + 13), 0x0180 } },
        { 14, { (uint16_t)(SPI_BASE_ADDR + 14), 0x0000 } },
        { 15, { (uint16_t)(SPI_BASE_ADDR + 15), 0x0003 } },
        { 16, { (uint16_t)(SPI_BASE_ADDR + 16), 0x0180 } },
        { 17, { (uint16_t)(SPI_BASE_ADDR + 17), 0x0000 } },
        { 18, { (uint16_t)(SPI_BASE_ADDR + 18), 0x0003 } },
        { 19, { (uint16_t)(SPI_BASE_ADDR + 19), 0x0180 } },
        { 20, { (uint16_t)(SPI_BASE_ADDR + 20), 0x0000 } },
        { 23, { (uint16_t)(SPI_BASE_ADDR + 23), 0x010A } } };

    for (auto reg : CDCM_Regs)
        if (WriteRegister(reg.second.addr, reg.second.val) != 0)
            return -1;

    // Load to CDCM
    if (WriteRegister(SPI_BASE_ADDR + 21, 1) != 0)
        return -1;

    if (PrepareToReadRegs() != 0)
        return -1;

    is_locked = !((ReadRegister(SPI_BASE_ADDR + 22) >> 2) & 1);

    return DownloadConfiguration();
}

/**
    @brief Sets primary frequency of VCO.
    @param freq The primary frequency to set the VCO to.
*/
void CDCM_Dev::SetPrimaryFreq(double freq)
{
    VCO.prim_freq = freq;
    if (GetVCOInput() == 2)
        UpdateOutputFrequencies();
}

/**
    @brief Gets the primary frequency of VCO.
    @return VCO primary frequency.
*/
double CDCM_Dev::GetPrimaryFreq()
{
    return VCO.prim_freq;
}

/**
    @brief Sets secondary frequency of VCO.
    @param freq The Secondary frequency to set the VCO to.
*/
void CDCM_Dev::SetSecondaryFreq(double freq)
{
    VCO.sec_freq = freq;
    if (GetVCOInput() == 1)
        UpdateOutputFrequencies();
}

/**
    @brief Gets secondary frequency of VCO.
    @return VCO secondary frequency.
*/
double CDCM_Dev::GetSecondaryFreq()
{
    return VCO.sec_freq;
}

/**
    @brief Returns current VCO input frequency value.
    @return Input frequency.
*/
double CDCM_Dev::GetInputFreq()
{
    if (GetVCOInput() == 1)
        return VCO.prim_freq / VCO.R_div;
    else
        return VCO.sec_freq;
}

/**
    @brief Selects VCO input.
    @param input 1 - primary, 2 - secondary.
*/
void CDCM_Dev::SetVCOInput(int input)
{
    VCO.input_mux = input;
    UpdateOutputFrequencies();
}

/**
    @brief Returns which VCO input is selected.
    @return 1 - primary, 2 - secondary.
*/
int CDCM_Dev::GetVCOInput()
{
    return VCO.input_mux;
}

/**
    @brief Sets all CDCM outputs.
    @param Outputs CDCM Outputs structure.
*/
void CDCM_Dev::SetOutputs(CDCM_Outputs Outputs)
{
    this->outputs = Outputs;
}

/**
    @brief Sets CDCM output frequency to desired value.
    @param output CDCM output to set frequency for.
    @param frequency The frequency to set.
    @param upload Whether upload the registers after calculation.
    @return 0 on success; -1 on error.
*/
int CDCM_Dev::SetFrequency(cdcm_output_t output, double frequency, bool upload)
{
    int return_val = -1;
    switch (output)
    {
    case CDCM_Y0Y1:
        outputs.Y0Y1.requested_freq = frequency;
        outputs.Y0Y1.used = true;
        break;
    case CDCM_Y2Y3:
        outputs.Y2Y3.requested_freq = frequency;
        outputs.Y2Y3.used = true;
        break;
    case CDCM_Y4:
        outputs.Y4.requested_freq = frequency;
        outputs.Y4.used = true;
        break;
    case CDCM_Y5:
        outputs.Y5.requested_freq = frequency;
        outputs.Y5.used = true;
        break;
    case CDCM_Y6:
        outputs.Y6.requested_freq = frequency;
        outputs.Y6.used = true;
        break;
    case CDCM_Y7:
        outputs.Y7.requested_freq = frequency;
        outputs.Y7.used = true;
        break;
    default:
        break;
    }

    CDCM_VCO VCOConfig = FindVCOConfig();
    if (VCOConfig.valid)
    {
        if (VCOConfig.output_freq > 0)
        {
            VCO = VCOConfig;
            SolveN(VCO.N_mul_full, &VCO.N_mul_8bit, &VCO.N_mul_10bit);

            VCO.output_freq = GetInputFreq();
            VCO.output_freq /= VCO.M_div;
            VCO.output_freq *= VCO.prescaler_A;
            VCO.output_freq *= VCO.N_mul_full;
        }
        return_val = 0;
    }

    if (outputs.Y0Y1.used)
        outputs.Y0Y1.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y0Y1.requested_freq);
    else
        outputs.Y0Y1.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y0Y1.output_freq);

    if (outputs.Y2Y3.used)
        outputs.Y2Y3.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y2Y3.requested_freq);
    else
        outputs.Y2Y3.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y2Y3.output_freq);

    if (outputs.Y4.used)
        outputs.Y4.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y4.requested_freq;
    else
        outputs.Y4.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y4.output_freq;

    SolveFracDiv(outputs.Y4.divider_val, &outputs.Y4);

    if (outputs.Y5.used)
        outputs.Y5.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y5.requested_freq;
    else
        outputs.Y5.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y5.output_freq;

    SolveFracDiv(outputs.Y5.divider_val, &outputs.Y5);

    if (outputs.Y6.used)
        outputs.Y6.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y6.requested_freq;
    else
        outputs.Y6.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y6.output_freq;

    SolveFracDiv(outputs.Y6.divider_val, &outputs.Y6);

    if (outputs.Y7.used)
        outputs.Y7.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y7.requested_freq;
    else
        outputs.Y7.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y7.output_freq;

    SolveFracDiv(outputs.Y7.divider_val, &outputs.Y7);

    UpdateOutputFrequencies();

    if (upload)
        if (UploadConfiguration() != 0)
            return_val = -1;

    return return_val;
}

/**
    @brief Updates VCO and Output frequencies based on requested ones.
    @return 0 on success; -1 on error.
*/
int CDCM_Dev::RecalculateFrequencies()
{
    int return_val = -1;
    CDCM_VCO VCOConfig = FindVCOConfig();
    if (VCOConfig.valid)
    {
        if (VCOConfig.output_freq > 0)
        {
            VCO = VCOConfig;
            SolveN(VCO.N_mul_full, &VCO.N_mul_8bit, &VCO.N_mul_10bit);

            VCO.output_freq = GetInputFreq();
            VCO.output_freq /= VCO.M_div;
            VCO.output_freq *= VCO.prescaler_A;
            VCO.output_freq *= VCO.N_mul_full;
        }
        return_val = 0;
    }

    if (outputs.Y0Y1.used)
        outputs.Y0Y1.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y0Y1.requested_freq);
    else
        outputs.Y0Y1.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y0Y1.output_freq);

    if (outputs.Y2Y3.used)
        outputs.Y2Y3.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y2Y3.requested_freq);
    else
        outputs.Y2Y3.divider_val = (int)round((VCO.output_freq / VCO.prescaler_A) / outputs.Y2Y3.output_freq);

    if (outputs.Y4.used)
        outputs.Y4.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y4.requested_freq;
    else
        outputs.Y4.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y4.output_freq;

    SolveFracDiv(outputs.Y4.divider_val, &outputs.Y4);

    if (outputs.Y5.used)
        outputs.Y5.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y5.requested_freq;
    else
        outputs.Y5.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y5.output_freq;

    SolveFracDiv(outputs.Y5.divider_val, &outputs.Y5);

    if (outputs.Y6.used)
        outputs.Y6.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y6.requested_freq;
    else
        outputs.Y6.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y6.output_freq;

    SolveFracDiv(outputs.Y6.divider_val, &outputs.Y6);

    if (outputs.Y7.used)
        outputs.Y7.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y7.requested_freq;
    else
        outputs.Y7.divider_val = (VCO.output_freq / VCO.prescaler_A) / outputs.Y7.output_freq;

    SolveFracDiv(outputs.Y7.divider_val, &outputs.Y7);

    UpdateOutputFrequencies();

    return return_val;
}

/**
    @brief Gets CDCM output frequency.
    @param output CDCM output to get frequency from.
    @return Output frequency.
*/
double CDCM_Dev::GetFrequency(cdcm_output_t output)
{
    switch (output)
    {
    case CDCM_Y0Y1:
        return outputs.Y0Y1.output_freq;
    case CDCM_Y2Y3:
        return outputs.Y2Y3.output_freq;
    case CDCM_Y4:
        return outputs.Y4.output_freq;
    case CDCM_Y5:
        return outputs.Y5.output_freq;
    case CDCM_Y6:
        return outputs.Y6.output_freq;
    case CDCM_Y7:
        return outputs.Y7.output_freq;
    default:
        return -1;
    }
}

/**
    @brief Gets current CDCM VCO structure.
    @return VCO structure.
*/
CDCM_VCO CDCM_Dev::GetVCO()
{
    return VCO;
}

/**
    @brief Sets CDCM VCO.
    @param VCO The voltage-controlled oscillator to use.
*/
void CDCM_Dev::SetVCO(CDCM_VCO VCO)
{
    this->VCO = VCO;
    UpdateOutputFrequencies();
}

/**
    @brief Sets VCO N multiplier value.
    @param value N multiplier value.
*/
void CDCM_Dev::SetVCOMultiplier(int value)
{

    VCO.N_mul_full = value;
    SolveN(value, &(VCO.N_mul_8bit), &(VCO.N_mul_10bit));
    UpdateOutputFrequencies();
}

/**
    @brief Sets CDCM version.
    @param version 0 - version 1; 1 - version 2.
    @return 0 on sucesss; -1 on error.
*/
int CDCM_Dev::SetVersion(uint8_t version)
{
    if (version != 0 && version != 1)
        return -1;

    VCO.version = version;
    if (VCO.version == 1)
    {
        VCO.min_freq = CDCM_VCO_MIN_V2;
        VCO.max_freq = CDCM_VCO_MAX_V2;
    }
    else
    {
        VCO.min_freq = CDCM_VCO_MIN_V1;
        VCO.max_freq = CDCM_VCO_MAX_V1;
    }
    return 0;
}

/**
    @brief Gets CDCM version.
    @return 0 - version 1; 1 - version 2.
*/
int CDCM_Dev::GetVersion()
{
    return VCO.version;
}

/**
    @brief Returns whether VCO PLL is locked.
    @return true - locked; false - not locked.
*/
bool CDCM_Dev::IsLocked()
{
    return is_locked;
}

/**
    @brief Uploads current CDCM configuration to FPGA registers.
    @return 0 on sucesss; -1 on error.
*/
int CDCM_Dev::UploadConfiguration()
{
    if (!comms)
        return -1;

    struct regVal {
        uint16_t addr;
        uint16_t val;
    };

    std::map<int, regVal> CDCM_Regs = { { 1, { (uint16_t)(SPI_BASE_ADDR + 1), 0 } },
        { 2, { (uint16_t)(SPI_BASE_ADDR + 2), 0 } },
        { 3, { (uint16_t)(SPI_BASE_ADDR + 3), 0 } },
        { 4, { (uint16_t)(SPI_BASE_ADDR + 4), 0 } },
        { 6, { (uint16_t)(SPI_BASE_ADDR + 6), 0 } },
        { 8, { (uint16_t)(SPI_BASE_ADDR + 8), 0 } },
        { 9, { (uint16_t)(SPI_BASE_ADDR + 9), 0 } },
        { 10, { (uint16_t)(SPI_BASE_ADDR + 10), 0 } },
        { 11, { (uint16_t)(SPI_BASE_ADDR + 11), 0 } },
        { 12, { (uint16_t)(SPI_BASE_ADDR + 12), 0 } },
        { 13, { (uint16_t)(SPI_BASE_ADDR + 13), 0 } },
        { 14, { (uint16_t)(SPI_BASE_ADDR + 14), 0 } },
        { 15, { (uint16_t)(SPI_BASE_ADDR + 15), 0 } },
        { 16, { (uint16_t)(SPI_BASE_ADDR + 16), 0 } },
        { 17, { (uint16_t)(SPI_BASE_ADDR + 17), 0 } },
        { 18, { (uint16_t)(SPI_BASE_ADDR + 18), 0 } },
        { 19, { (uint16_t)(SPI_BASE_ADDR + 19), 0 } },
        { 20, { (uint16_t)(SPI_BASE_ADDR + 20), 0 } } };

    for (int i = 3; i < 24; i += 3)
    {
        int reg = i == 6 ? 4 : i; // Read 4th register instead of 6th
        int val = ReadRegister(CDCM_Regs[reg].addr);
        if (val == -1)
            return -1;
        CDCM_Regs[reg].val = val;
    }

    CDCM_Regs[1].val = ((VCO.M_div - 1) << 2) | ((VCO.N_mul_10bit - 1) >> 8);
    CDCM_Regs[2].val = ((VCO.N_mul_10bit - 1) << 8) | (VCO.N_mul_8bit - 1);

    CDCM_Regs[3].val &= ~0xF;
    CDCM_Regs[3].val |= ((VCO.prescaler_A - 4) & 3);
    CDCM_Regs[3].val |= ((VCO.prescaler_B - 4) & 3) << 2;

    CDCM_Regs[4].val &= ~(0x1F00);
    CDCM_Regs[4].val |= ((VCO.R_div - 1) & 0xF) << 8;
    CDCM_Regs[4].val |= ((VCO.input_mux - 1) & 1) << 12;

    CDCM_Regs[6].val = 0 | ((uint16_t)(outputs.Y0Y1.divider_val - 1) & 0xFF);

    CDCM_Regs[8].val = 0 | ((uint16_t)(outputs.Y2Y3.divider_val - 1) & 0xFF);

    CDCM_Regs[9].val &= ~0x1E00;
    CDCM_Regs[9].val |= (((int)outputs.Y4.isFrac) << 9);
    CDCM_Regs[9].val |= ((outputs.Y4.prescaler - 2) & 7) << 10;

    CDCM_Regs[10].val = 0 | (((outputs.Y4.integer_part - 1) & 0xFF) << 4);
    CDCM_Regs[10].val |= (outputs.Y4.fractional_part >> 16) & 0xF;

    CDCM_Regs[11].val = outputs.Y4.fractional_part & 0xFFFF;

    CDCM_Regs[12].val &= ~0x1E00;
    CDCM_Regs[12].val |= (((int)outputs.Y5.isFrac) << 9);
    CDCM_Regs[12].val |= ((outputs.Y5.prescaler - 2) & 7) << 10;

    CDCM_Regs[13].val = 0 | (((outputs.Y5.integer_part - 1) & 0xFF) << 4);
    CDCM_Regs[13].val |= (outputs.Y5.fractional_part >> 16) & 0xF;

    CDCM_Regs[14].val = outputs.Y5.fractional_part & 0xFFFF;

    CDCM_Regs[15].val &= ~0x1E00;
    CDCM_Regs[15].val |= (((int)outputs.Y6.isFrac) << 9);
    CDCM_Regs[15].val |= ((outputs.Y6.prescaler - 2) & 7) << 10;

    CDCM_Regs[16].val = 0 | (((outputs.Y6.integer_part - 1) & 0xFF) << 4);
    CDCM_Regs[16].val |= (outputs.Y6.fractional_part >> 16) & 0xF;

    CDCM_Regs[17].val = outputs.Y6.fractional_part & 0xFFFF;

    CDCM_Regs[18].val &= ~0x1E00;
    CDCM_Regs[18].val |= (((int)outputs.Y7.isFrac) << 9);
    CDCM_Regs[18].val |= ((outputs.Y7.prescaler - 2) & 7) << 10;

    CDCM_Regs[19].val = 0 | (((outputs.Y7.integer_part - 1) & 0xFF) << 4);
    CDCM_Regs[19].val |= (outputs.Y7.fractional_part >> 16) & 0xF;

    CDCM_Regs[20].val = outputs.Y7.fractional_part & 0xFFFF;

    for (auto reg : CDCM_Regs)
        if (WriteRegister(reg.second.addr, reg.second.val) != 0)
            return -1;

    // Load to CDCM
    if (WriteRegister(SPI_BASE_ADDR + 21, 1) != 0)
        return -1;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (PrepareToReadRegs() != 0)
        return -1;

    is_locked = !((ReadRegister(SPI_BASE_ADDR + 22) >> 2) & 1);

    return 0;
}

/**
    @brief Downloads current CDCM configuration from FPGA registers.
    @return 0 on sucesss; -1 on error.
*/
int CDCM_Dev::DownloadConfiguration()
{
    if (!comms)
        return -1;

    struct regVal {
        uint16_t addr;
        uint16_t val;
    };

    std::map<int, regVal> CDCM_Regs = { { 1, { (uint16_t)(SPI_BASE_ADDR + 1), 0 } },
        { 2, { (uint16_t)(SPI_BASE_ADDR + 2), 0 } },
        { 3, { (uint16_t)(SPI_BASE_ADDR + 3), 0 } },
        { 4, { (uint16_t)(SPI_BASE_ADDR + 4), 0 } },
        { 6, { (uint16_t)(SPI_BASE_ADDR + 6), 0 } },
        { 8, { (uint16_t)(SPI_BASE_ADDR + 8), 0 } },
        { 9, { (uint16_t)(SPI_BASE_ADDR + 9), 0 } },
        { 10, { (uint16_t)(SPI_BASE_ADDR + 10), 0 } },
        { 11, { (uint16_t)(SPI_BASE_ADDR + 11), 0 } },
        { 12, { (uint16_t)(SPI_BASE_ADDR + 12), 0 } },
        { 13, { (uint16_t)(SPI_BASE_ADDR + 13), 0 } },
        { 14, { (uint16_t)(SPI_BASE_ADDR + 14), 0 } },
        { 15, { (uint16_t)(SPI_BASE_ADDR + 15), 0 } },
        { 16, { (uint16_t)(SPI_BASE_ADDR + 16), 0 } },
        { 17, { (uint16_t)(SPI_BASE_ADDR + 17), 0 } },
        { 18, { (uint16_t)(SPI_BASE_ADDR + 18), 0 } },
        { 19, { (uint16_t)(SPI_BASE_ADDR + 19), 0 } },
        { 20, { (uint16_t)(SPI_BASE_ADDR + 20), 0 } },
        { 22, { (uint16_t)(SPI_BASE_ADDR + 22), 0 } },
        { 23, { (uint16_t)(SPI_BASE_ADDR + 23), 0 } } };

    if (PrepareToReadRegs() != 0)
        return -1;

    for (auto& reg : CDCM_Regs)
    {
        int val = ReadRegister(reg.second.addr);
        if (val == -1)
            return -1;
        else
            reg.second.val = val;
    }

    VCO.M_div = (CDCM_Regs[1].val >> 2) + 1;

    VCO.N_mul_10bit = 0 | ((CDCM_Regs[1].val & 3) << 8);
    VCO.N_mul_10bit |= (CDCM_Regs[2].val >> 8);
    VCO.N_mul_10bit += 1;
    VCO.N_mul_8bit = (CDCM_Regs[2].val & 0xFF) + 1;
    VCO.N_mul_full = VCO.N_mul_8bit * VCO.N_mul_10bit;

    VCO.prescaler_A = (CDCM_Regs[3].val & 3) + 4;
    VCO.prescaler_B = ((CDCM_Regs[3].val >> 2) & 3) + 4;

    VCO.R_div = ((CDCM_Regs[4].val >> 8) & 0xF) + 1;

    VCO.input_mux = ((CDCM_Regs[4].val >> 12) & 1) + 1;

    outputs.Y0Y1.divider_val = (CDCM_Regs[6].val & 0xFF) + 1;
    outputs.Y2Y3.divider_val = (CDCM_Regs[8].val & 0xFF) + 1;

    outputs.Y4.isFrac = (bool)((CDCM_Regs[9].val >> 9) & 1);
    outputs.Y4.prescaler = ((CDCM_Regs[9].val >> 10) & 7) + 2;
    outputs.Y4.integer_part = ((CDCM_Regs[10].val >> 4) & 0xFF) + 1;
    outputs.Y4.fractional_part = 0 | ((CDCM_Regs[10].val & 0xF) << 16);
    outputs.Y4.fractional_part |= CDCM_Regs[11].val;
    CalculateFracDiv(&outputs.Y4);

    outputs.Y5.isFrac = (bool)((CDCM_Regs[12].val >> 9) & 1);
    outputs.Y5.prescaler = ((CDCM_Regs[12].val >> 10) & 7) + 2;
    outputs.Y5.integer_part = ((CDCM_Regs[13].val >> 4) & 0xFF) + 1;
    outputs.Y5.fractional_part = 0 | ((CDCM_Regs[13].val & 0xF) << 16);
    outputs.Y5.fractional_part |= CDCM_Regs[14].val;
    CalculateFracDiv(&outputs.Y5);

    outputs.Y6.isFrac = (bool)((CDCM_Regs[15].val >> 9) & 1);
    outputs.Y6.prescaler = ((CDCM_Regs[15].val >> 10) & 7) + 2;
    outputs.Y6.integer_part = ((CDCM_Regs[16].val >> 4) & 0xFF) + 1;
    outputs.Y6.fractional_part = 0 | ((CDCM_Regs[16].val & 0xF) << 16);
    outputs.Y6.fractional_part |= CDCM_Regs[17].val;
    CalculateFracDiv(&outputs.Y6);

    outputs.Y7.isFrac = (bool)((CDCM_Regs[18].val >> 9) & 1);
    outputs.Y7.prescaler = ((CDCM_Regs[18].val >> 10) & 7) + 2;
    outputs.Y7.integer_part = ((CDCM_Regs[19].val >> 4) & 0xFF) + 1;
    outputs.Y7.fractional_part = 0 | ((CDCM_Regs[19].val & 0xF) << 16);
    outputs.Y7.fractional_part |= CDCM_Regs[20].val;
    CalculateFracDiv(&outputs.Y7);

    SetVersion((CDCM_Regs[23].val >> 3) & 7);

    is_locked = !((CDCM_Regs[22].val >> 2) & 1);

    UpdateOutputFrequencies();

    return 0;
}

/** 
    @brief Finds fractional dividers integer and fractional parts values.
    @param target The requested fractional divider target.
    @param[out] Output Output to which fractional divider the value is found.
    @return Found divider value.
*/
double CDCM_Dev::SolveFracDiv(double target, CDCM_Output* Output)
{
    // Calculate max fractional val (integer value is offset by +1)
    // 8 bits integer value, 20 bits fractional value
    // 1111 1111  1111 1111 1111 1111 1111
    // \  int  /  \ fractional           /
    //    +1
    // 1 0000 0000 1111 1111 1111 1111 1111
    // \  int    / \ fractional           /

    double max_fractional_val = (((1 << 28) + ((1 << 20) - 1)) / (double)(1 << 20)) * 3;

    double result = target;
    // Check bounds
    if (result < 1)
        result = 1;
    else if (result > max_fractional_val)
        result = max_fractional_val;

    Output->isFrac = true; // This seems highly unecessary

    // Divider must be multiplied by 2 or 3
    double div2_result;
    double div3_result;
    int div2_fixed;
    int div3_fixed;
    div2_result = result / 2;
    div3_result = result / 3;

    // Cast float into 8.20 fixed point integer
    div2_fixed = (int)(div2_result * ((1 << 20) & ((1 << 28) - 1)));
    // Recast back
    div2_result = (float)div2_fixed / (1 << 20);
    // Cast float into 8.20 fixed point integer
    div3_fixed = (int)(div3_result * ((1 << 20) & ((1 << 28) - 1)));
    // Recast back
    div3_result = (float)div3_fixed / (1 << 20);
    // Find best match
    double div2_difference = abs(result - (div2_result * 2.0));
    double div3_difference = abs(result - (div3_result * 3.0));
    if (div2_difference < div3_difference)
    {
        Output->integer_part = (div2_fixed >> 20) & ((1 << 8) - 1);
        Output->fractional_part = div2_fixed & ((1 << 20) - 1);
        Output->prescaler = 2;
        result = div2_result * 2;
    }
    else
    {
        Output->integer_part = (div3_fixed >> 20) & ((1 << 8) - 1);
        Output->fractional_part = div3_fixed & ((1 << 20) - 1);
        Output->prescaler = 3;
        result = div3_result * 3;
    }

    return result;
}

/** 
    @brief Finds 8-bit and 10-bit multiplier values.
    @param Target The requested fractional divider target.
    @param[out] Mult8bit 8-bit multiplier value.
    @param[out] Mult10bit 10-bit multiplier value.
    @return 0 on success; -1 on failure.
*/
int CDCM_Dev::SolveN(int Target, int* Mult8bit, int* Mult10bit)
{
    // Find multiplier combination to get desired ratio
    double res;
    for (int i10 = 1; i10 <= 1024; i10++)
    {
        res = double(Target) / i10;
        if (res < (1 << 8)) // Check max value
        {
            // Check if res is integer
            if (IsInteger(res))
            {
                *Mult8bit = (int)res;
                *Mult10bit = i10;
                return 0;
            }
        }
    }
    return -1;
}

/** 
    @brief Calculates full divider value based on integer and fractional parts.
    @param[in] Output CDCM Output.
*/
void CDCM_Dev::CalculateFracDiv(CDCM_Output* Output)
{
    if (Output->isFrac)
    {
        Output->divider_val = Output->fractional_part;
        Output->divider_val = Output->divider_val / (1 << 20);
        Output->divider_val = Output->divider_val + Output->integer_part;
        Output->divider_val = Output->divider_val * Output->prescaler;
    }
    else
    {
        Output->divider_val = Output->integer_part;
    }
}

/** 
    @brief Calculates CDCM output frequencies based on current VCO configuration.
*/
void CDCM_Dev::UpdateOutputFrequencies()
{
    VCO.output_freq = GetInputFreq();

    VCO.output_freq /= VCO.M_div;
    VCO.output_freq *= VCO.prescaler_A;
    VCO.output_freq *= VCO.N_mul_full;

    outputs.Y0Y1.output_freq = (VCO.output_freq / VCO.prescaler_A) / outputs.Y0Y1.divider_val;
    outputs.Y4.output_freq = (VCO.output_freq / VCO.prescaler_A) / outputs.Y4.divider_val;
    outputs.Y5.output_freq = (VCO.output_freq / VCO.prescaler_A) / outputs.Y5.divider_val;

    outputs.Y2Y3.output_freq = (VCO.output_freq / VCO.prescaler_B) / outputs.Y2Y3.divider_val;
    outputs.Y6.output_freq = (VCO.output_freq / VCO.prescaler_B) / outputs.Y6.divider_val;
    outputs.Y7.output_freq = (VCO.output_freq / VCO.prescaler_B) / outputs.Y7.divider_val;
}

/** 
    @brief Prepare to read the FPGA registers.
    @return 0 on success; -1 on failure.
*/
int CDCM_Dev::PrepareToReadRegs()
{
    const auto timeout = std::chrono::milliseconds(200);
    uint16_t status = 0;

    WriteRegister(SPI_BASE_ADDR + 24, 1);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    do
    {
        status = ReadRegister(SPI_BASE_ADDR + 24);
        t2 = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } while (status != 2 && (t2 - t1) < timeout);

    if ((t2 - t1) > timeout)
        return -1;

    return 0;
}

/** 
    @brief Calculates numerator and denumenator values from decimal.
    @param decimal Decimal value.
    @param[out] num Numerator value.
    @param[out] den Denumenator value.
    @return Error value after conversion.
*/
double CDCM_Dev::DecToFrac(double decimal, int* num, int* den)
{
    // Constant for CDCM6208
    const int max_num_bits = 18;
    const int max_den_bits = 14;
    uint64_t l_num = 1;
    uint64_t l_den = 1;
    double l_target = decimal;

    while (IsInteger(decimal) == false)
    {
        l_den *= 10;
        decimal *= 10;
    }
    l_num = (uint64_t)decimal;

    uint64_t gcd = FindGCD(l_num, l_den);
    l_num /= gcd;
    l_den /= gcd;

    // Find out how many bits are used by numerator and denominator
    int num_bits = 0, den_bits = 0;
    for (int i = 0; i < 64; i++)
    {
        if ((l_num >> i) > 0)
            num_bits = i;
        if ((l_den >> i) > 0)
            den_bits = i;
    }
    // Offset by one
    num_bits++;
    den_bits++;
    // Truncate if needed
    while ((num_bits > max_num_bits) || (den_bits > max_den_bits))
    {
        l_den = l_den >> 1;
        l_num = l_num >> 1;
        num_bits--;
        den_bits--;
    }

    // Check if numerator value valid for CDCM6208
    int placeholder;
    while (SolveN(l_num, &placeholder, &placeholder) == -1)
    {
        l_den = l_den >> 1;
        l_num = l_num >> 1;
    }
    // Return values
    *num = (int)l_num;
    *den = (int)l_den;
    double result_freq = *num / (double)*den;
    return (fabs(1 - (l_target / result_freq)));
}

/** 
    @brief Find all available VCO configurations based on @p lcm value.
    @param lcm The least common multiple value.
    @param version VCO version.
    @return All available VCO configurations.
*/
std::vector<CDCM_VCO> CDCM_Dev::FindValidVCOFreqs(double lcm, int version)
{
    // Find number of valid vco freqs for each prescaler
    std::vector<CDCM_VCO> Config_vector;
    CDCM_VCO placeholder_struct;
    double lo_freq;
    double hi_freq;
    double frequency;

    placeholder_struct.version = version;
    placeholder_struct.min_freq = version == 0 ? CDCM_VCO_MIN_V1 : CDCM_VCO_MIN_V2;
    placeholder_struct.max_freq = version == 0 ? CDCM_VCO_MAX_V1 : CDCM_VCO_MAX_V2;

    for (int prescaler = 4; prescaler <= 6; prescaler++)
    {
        // Find low and high bounds for current prescaler
        lo_freq = placeholder_struct.min_freq / prescaler;
        hi_freq = placeholder_struct.max_freq / prescaler;
        frequency = ceil(lo_freq / lcm) * lcm;
        while (frequency < hi_freq)
        {
            placeholder_struct.prescaler_A = prescaler;
            placeholder_struct.prescaler_B = prescaler;
            placeholder_struct.output_freq = frequency;
            Config_vector.push_back(placeholder_struct);
            frequency += lcm;
        }
    }
    return Config_vector;
}

/** 
    @brief Euclidean method to find greatest common divisor between two integers.
    @param a First integer.
    @param b Second integer.
    @return Greatest common divisor value.
*/
uint64_t CDCM_Dev::FindGCD(uint64_t a, uint64_t b)
{
    if (b == 0)
        return a;
    return FindGCD(b, a % b);
}

/** 
    @brief Finds the index of VCO input vector which has lowest Prescaler A value.
    @param input Vector of VCO configurations.
    @return Index of lowest input.
*/
int CDCM_Dev::FindLowestPSAOutput(std::vector<CDCM_VCO> input)
{
    int min_nom = 0xFFFFFFF;
    int index = 0;
    int curr_val;
    for (size_t i = 0; i < input.size(); i++)
    {
        curr_val = input[i].N_mul_full * input[i].prescaler_A;
        if (curr_val < min_nom)
        {
            min_nom = curr_val;
            index = i;
        }
    }
    return index;
}

/** 
    @brief Finds the index of VCO input vector which has lowest frequency error.
    @param input Vector of VCO configurations.
    @return Index of lowest input.
*/
int CDCM_Dev::GetLowestFreqErr(std::vector<CDCM_VCO> input)
{
    double min_err = 100e6;
    int index = 0;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i].freq_err < min_err)
        {
            min_err = input[i].freq_err;
            index = i;
        }
    }
    return index;
}

/** 
    @brief Finds best VCO configuration index.
    @param input Vector of VCO configurations.
    @param num_errors How many errors there are in all configurations.
    @return Index of best VCO configuration.
*/
int CDCM_Dev::FindBestVCOConfigIndex(std::vector<CDCM_VCO>& input, int num_errors)
{
    int no_error = input.size() - num_errors;
    // All configs have freq. errors
    if (no_error == 0)
    {
        return GetLowestFreqErr(input);
    }
    // No configs have freq. errors
    else if (num_errors == 0)
    {
        return FindLowestPSAOutput(input);
    }
    else
    {
        for (int i = (input.size() - 1); i >= 0; i--)
        {
            if (input[i].freq_err > 0)
                input.erase(input.begin() + i);
        }
        return FindLowestPSAOutput(input);
    }
}

/** 
    @brief Finds VCO configuration based on requested output frequencies.
    @return VCO configuration.
*/
CDCM_VCO CDCM_Dev::FindVCOConfig()
{
    double l_Y0Y1 = outputs.Y0Y1.requested_freq;
    double l_Y2Y3 = outputs.Y2Y3.requested_freq;
    double input_shift = 1;
    // Eliminate fractional parts by shifting left (if any)
    while (!(IsInteger(l_Y0Y1) && IsInteger(l_Y2Y3)))
    {
        l_Y0Y1 *= 10;
        l_Y2Y3 *= 10;
        input_shift *= 10;
    }

    // Find lowest common multiple
    double int_lcm;
    bool do_vco_calc = true;

    if (outputs.Y0Y1.used && outputs.Y2Y3.used)
        int_lcm = (l_Y0Y1 * l_Y2Y3) / FindGCD((uint64_t)l_Y0Y1, (uint64_t)l_Y2Y3);
    else if (outputs.Y0Y1.used)
        int_lcm = l_Y0Y1;
    else if (outputs.Y2Y3.used)
        int_lcm = l_Y2Y3;
    else
        do_vco_calc = false;

    if (do_vco_calc)
    {
        // Find number of valid VCO freqs for each prescaler
        std::vector<CDCM_VCO> Config_vector;
        Config_vector = FindValidVCOFreqs(int_lcm, VCO.version);

        int have_error = 0;
        int max_r_div = GetVCOInput() ? 16 : 1;

        for (size_t i = 0; i < Config_vector.size(); i++)
        {
            Config_vector[i].prim_freq = VCO.prim_freq;
            Config_vector[i].sec_freq = VCO.sec_freq;
            double min_err = std::numeric_limits<double>::max();

            for (int r_div = 1; r_div <= max_r_div; r_div++)
            {
                double input_freq = ((GetVCOInput() == 1 ? VCO.prim_freq : VCO.sec_freq) / r_div) * input_shift;
                int n_mul = 1, m_div = 1;

                Config_vector[i].freq_err = DecToFrac(Config_vector[i].output_freq / input_freq, &n_mul, &m_div);

                if (min_err > Config_vector[i].freq_err)
                {
                    min_err = Config_vector[i].freq_err;
                    Config_vector[i].R_div = r_div;
                    Config_vector[i].N_mul_full = n_mul;
                    Config_vector[i].M_div = m_div;
                }
            }

            if (Config_vector[i].freq_err > 0)
                have_error++;
        }

        if (Config_vector.size() > 0)
        {
            // Find index of best config
            int best_index = FindBestVCOConfigIndex(Config_vector, have_error);
            Config_vector[best_index].valid = true;
            Config_vector[best_index].input_mux = GetVCOInput();
            return Config_vector[best_index];
        }
        else
        {
            CDCM_VCO vco_config;
            vco_config.valid = false;
            return vco_config;
        }
    }
    // No algorithm for fractional only frequency planning
    CDCM_VCO vco_config;
    vco_config.valid = true;
    return vco_config;
}

/**
  @brief Writes the given value to the given register.
  @param addr The address of where to write the value.
  @param val The value to write.
  @return 0 on success; -1 on error.
 */
int CDCM_Dev::WriteRegister(uint16_t addr, uint16_t val)
{
    const uint32_t mosi = (1 << 31) | (addr << 16) | val;
    try
    {
        comms->SPI(&mosi, nullptr, 1);
        return 0;
    } catch (...)
    {
        return -1;
    }
}

/**
  @brief Reads the value of a given address.
  @param addr The address of which value to read.
  @return The value of the given address; -1 on error.
 */
uint16_t CDCM_Dev::ReadRegister(uint16_t addr)
{
    const uint32_t mosi = (addr);
    uint32_t miso = 0;
    try
    {
        comms->SPI(&mosi, &miso, 1);
        return miso & 0xFFFF;
    } catch (...)
    {
        return -1;
    }
}
