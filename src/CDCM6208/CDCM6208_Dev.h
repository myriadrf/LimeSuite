#pragma once

#include "limesuite/config.h"
#include "FPGA_common.h"

#define CDCM_VCO_MIN_V1 2390 * 1e6
#define CDCM_VCO_MAX_V1 2550 * 1e6
#define CDCM_VCO_MIN_V2 2940 * 1e6
#define CDCM_VCO_MAX_V2 3130 * 1e6

#define CDCM1_BASE_ADDR 0x120
#define CDCM2_BASE_ADDR 0x140

typedef enum {
    CDCM_Y0Y1 = 0, // integer LMS2_DAC1 LMS2_DAC2
    CDCM_Y2Y3, // integer LMS2_DAC1 LMS2_DAC2 (unused)
    CDCM_Y4, // fractional LMS2_ADC1
    CDCM_Y5, // fractional LMS2_ADC2
    CDCM_Y6, // fractional LMS3_ADC1
    CDCM_Y7 // fractional LMS3_ADC2
} cdcm_output_t;

struct CDCM_VCO {
    CDCM_VCO()
        : valid(false)
        , prim_freq(30.72e6)
        , sec_freq(30.72e6)
        , input_mux(0)
        , output_freq(0)
        , freq_err(0)
        , min_freq(0)
        , max_freq(0)
        , R_div(1)
        , M_div(1)
        , N_mul_full(1)
        , N_mul_10bit(1)
        , N_mul_8bit(1)
        , prescaler_A(4)
        , prescaler_B(4)
        , version(0)
    {
    }

    bool valid;
    double prim_freq;
    double sec_freq;
    int input_mux;

    double output_freq;
    double freq_err;

    double min_freq;
    double max_freq;

    int R_div;
    int M_div;

    int N_mul_full;
    int N_mul_10bit;
    int N_mul_8bit;

    int prescaler_A;
    int prescaler_B;

    int version;
};

struct CDCM_Output {
    bool isFrac;
    bool used;

    double divider_val;

    int integer_part;
    int fractional_part;
    int prescaler;

    double requested_freq;
    double output_freq;
};

struct CDCM_Outputs {
    CDCM_Output Y0Y1 = { false, false, 0, 0, 0, 0, 30.72e6, 0 };
    CDCM_Output Y2Y3 = { false, false, 0, 0, 0, 0, 30.72e6, 0 };
    CDCM_Output Y4 = { false, false, 0, 0, 0, 0, 30.72e6, 0 };
    CDCM_Output Y5 = { false, false, 0, 0, 0, 0, 30.72e6, 0 };
    CDCM_Output Y6 = { false, false, 0, 0, 0, 0, 30.72e6, 0 };
    CDCM_Output Y7 = { false, false, 0, 0, 0, 0, 30.72e6, 0 };
};

class LIME_API CDCM_Dev
{
  public:
    CDCM_Dev(lime::ISPI* comms, uint16_t SPI_BASE_ADDR);

    int Init(double primaryFreq, double secondaryFreq);
    int Reset(double primaryFreq, double secondaryFreq);

    int SetFrequency(cdcm_output_t output, double frequency, bool upload = true);
    double GetFrequency(cdcm_output_t output);

    double GetInputFreq();
    void SetPrimaryFreq(double freq);
    double GetPrimaryFreq();
    void SetSecondaryFreq(double freq);
    double GetSecondaryFreq();
    int RecalculateFrequencies();

    void SetVCO(CDCM_VCO VCO);
    CDCM_VCO GetVCO();
    void SetVCOInput(int input);
    int GetVCOInput();
    void SetVCOMultiplier(int value);

    void SetOutputs(CDCM_Outputs Outputs);
    CDCM_Outputs getOutputs() { return Outputs; };

    int UploadConfiguration();
    int DownloadConfiguration();

    bool IsLocked();
    int GetVersion();
    int SetVersion(uint8_t version);

    void UpdateOutputFrequencies();
    double SolveFracDiv(double target, CDCM_Output* Output);

    void SetSPIBaseAddr(uint16_t SPI_BASE_ADDR) { this->SPI_BASE_ADDR = SPI_BASE_ADDR; }
    uint16_t GetSPIBaseAddr() { return SPI_BASE_ADDR; }

  private:
    int WriteRegister(uint16_t addr, uint16_t val);
    uint16_t ReadRegister(uint16_t addr);

    int SolveN(int Target, int* Mult8bit, int* Mult10bit);
    void CalculateFracDiv(CDCM_Output* Output);
    double DecToFrac(double target, int* num, int* den);
    bool IsInteger(double var) { return var == (int64_t)var; }
    std::vector<CDCM_VCO> FindValidVCOFreqs(double lcm, int version);
    uint64_t FindGCD(uint64_t a, uint64_t b);
    int GetLowestFreqErr(std::vector<CDCM_VCO> input);
    int FindLowestPSAOutput(std::vector<CDCM_VCO> input);
    int FindBestVCOConfigIndex(std::vector<CDCM_VCO>& input, int num_errors);
    CDCM_VCO FindVCOConfig();
    int PrepareToReadRegs();

    lime::ISPI* comms;
    CDCM_VCO VCO;
    CDCM_Outputs Outputs;
    uint16_t SPI_BASE_ADDR;
    bool is_locked;
};
