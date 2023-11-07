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
    CDCM_Y0Y1 = 0, ///< Integer LMS2_DAC1 LMS2_DAC2
    CDCM_Y2Y3, ///< Integer LMS2_DAC1 LMS2_DAC2 (unused)
    CDCM_Y4, ///< Fractional LMS2_ADC1
    CDCM_Y5, ///< Fractional LMS2_ADC2
    CDCM_Y6, ///< Fractional LMS3_ADC1
    CDCM_Y7, ///< Fractional LMS3_ADC2
} cdcm_output_t;

/** @brief The information about the clock generator's voltage-controlled oscillator */
struct CDCM_VCO {
    bool valid{ false }; ///< Whether the configuration is valid or not

    double prim_freq{ 30.72e6 }; ///< The primary frequency
    double sec_freq{ 30.72e6 }; ///< The secondary frequency

    int input_mux{ 0 }; ///< The currently selected input

    double output_freq{ 0 }; ///< The output frequency
    double freq_err{ 0 }; ///< The frequency error
    double min_freq{ 0 }; ///< The minimum frequency
    double max_freq{ 0 }; ///< The maximum frequency

    int R_div{ 1 }; ///< The R divider
    int M_div{ 1 }; ///< The M divider

    int N_mul_full{ 1 }; ///< The full value of the N multiplier
    int N_mul_10bit{ 1 }; ///< The 10-bit value of the N multiplier
    int N_mul_8bit{ 1 }; ///< The 8-bit value of the N multiplier

    int prescaler_A{ 4 }; ///< The value of the A prescaler
    int prescaler_B{ 4 }; ///< The value of the B prescaler

    int version{ 0 }; ///< The version of the clock generator
};

/** @brief The output settings of the clock generator */
struct CDCM_Output {
    bool isFrac; ///< Whether the clock generator is fractional or not
    bool used; ///< Whether the clock is being used

    double divider_val; ///< The value of the divider

    int integer_part; ///< The integer part of the number
    int fractional_part; ///< The fractional part of the number
    int prescaler; ///< The value of the prescaler

    double requested_freq; ///< The requested frequency of the clock
    double output_freq; ///< The output frequency of the clock
};

/** @brief The outputs of the clock generator */
struct CDCM_Outputs {
    CDCM_Output Y0Y1{ false, false, 0, 0, 0, 0, 30.72e6, 0 }; ///< The value of the Y0Y1 output
    CDCM_Output Y2Y3{ false, false, 0, 0, 0, 0, 30.72e6, 0 }; ///< The value of the Y2Y3 output
    CDCM_Output Y4{ false, false, 0, 0, 0, 0, 30.72e6, 0 }; ///< The value of the Y4 output
    CDCM_Output Y5{ false, false, 0, 0, 0, 0, 30.72e6, 0 }; ///< The value of the Y5 output
    CDCM_Output Y6{ false, false, 0, 0, 0, 0, 30.72e6, 0 }; ///< The value of the Y6 output
    CDCM_Output Y7{ false, false, 0, 0, 0, 0, 30.72e6, 0 }; ///< The value of the Y7 output
};

/** @brief Class for communicating with the CDCM6208 2:8 Clock Generator, Jitter Cleaner With Fractional Dividers */
class LIME_API CDCM_Dev
{
  public:
    CDCM_Dev(std::shared_ptr<lime::ISPI> comms, uint16_t SPI_BASE_ADDR);

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

    void SetOutputs(CDCM_Outputs outputs);
    /**
      @brief Get the outputs object.
      @return The available outputs of the clock generator.
     */
    CDCM_Outputs GetOutputs() { return outputs; };

    int UploadConfiguration();
    int DownloadConfiguration();

    bool IsLocked();
    int GetVersion();
    int SetVersion(uint8_t version);

    void UpdateOutputFrequencies();
    double SolveFracDiv(double target, CDCM_Output* Output);

    /**
      @brief Sets the base address of the SPI.
      @param SPI_BASE_ADDR The new address of the SPI.
     */
    void SetSPIBaseAddr(uint16_t SPI_BASE_ADDR) { this->SPI_BASE_ADDR = SPI_BASE_ADDR; }

    /**
      @brief Gets the base address of the SPI.
      @return The address of the SPI.
     */
    uint16_t GetSPIBaseAddr() { return SPI_BASE_ADDR; }

  private:
    int WriteRegister(uint16_t addr, uint16_t val);
    uint16_t ReadRegister(uint16_t addr);

    int SolveN(int Target, int* Mult8bit, int* Mult10bit);
    void CalculateFracDiv(CDCM_Output* Output);
    double DecToFrac(double target, int* num, int* den);

    /**
      @brief Checks if the given double is an integer.      
      @param var The double to check.
      @return Whether the double is an integer or not.
     */
    bool IsInteger(double var) { return var == (int64_t)var; }
    std::vector<CDCM_VCO> FindValidVCOFreqs(double lcm, int version);
    uint64_t FindGCD(uint64_t a, uint64_t b);
    int GetLowestFreqErr(std::vector<CDCM_VCO> input);
    int FindLowestPSAOutput(std::vector<CDCM_VCO> input);
    int FindBestVCOConfigIndex(std::vector<CDCM_VCO>& input, int num_errors);
    CDCM_VCO FindVCOConfig();
    int PrepareToReadRegs();

    std::shared_ptr<lime::ISPI> comms; ///< The communication with the device endpoint
    CDCM_VCO VCO; ///< The voltage-controlled oscillator information
    CDCM_Outputs outputs; ///< The outputs the device can handle
    uint16_t SPI_BASE_ADDR; ///< The base address of the stored information on the device
    bool is_locked; ///< Indicates whether the voltage-controlled oscillator phase lock loop is locked
};
