/**
@file   LMS7002M.h
@author Lime Microsystems (www.limemicro.com)
@brief  LMS7002M transceiver configuration interface
*/

#ifndef LMS7API_H
#define LMS7API_H

#include "limesuite/commonTypes.h"
#include "limesuite/config.h"
#include "limesuite/LMS7002M_parameters.h"

#include <cstdarg>
#include <cstdint>
#include <functional>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace lime {
class ISPI;
class LMS7002M_RegistersMap;
class MCU_BD;

struct RSSI_measurements {
    void clear()
    {
        amplitudeFFT.clear();
        amplitudeGeortzelF.clear();
        amplitudeGeortzelFPGA.clear();
    }

    std::vector<float> amplitudeFFT;
    std::vector<float> amplitudeGeortzelF;
    std::vector<float> amplitudeGeortzelFPGA;
};

typedef double float_type;

/** @brief Class for communicating with the LMS7002M chip. */
class LIME_API LMS7002M
{
  public:
    static constexpr double CGEN_MAX_FREQ = 640e6;

    enum class ClockID : uint8_t {
        CLK_REFERENCE = 0, ///< Reference clock
        CLK_SXR = 1, ///< RX LO clock
        CLK_SXT = 2, ///< TX LO clock
        CLK_CGEN = 3, ///< Clock generator clock
        CLK_RXTSP = 4, ///< RXTSP reference clock (read-only)
        CLK_TXTSP = 5 ///< TXTSP reference clock (read-only)
    };

    /** @brief LMS7002M's clock generator details */
    struct CGEN_details {
        float_type frequency;
        float_type frequencyVCO;
        float_type referenceClock;
        uint32_t INT;
        uint32_t FRAC;
        uint8_t div_outch_cgen;
        uint16_t csw;
        bool success;
    };

    struct SX_details {
        float_type frequency;
        float_type frequencyVCO;
        float_type referenceClock;
        uint32_t INT;
        uint32_t FRAC;
        uint8_t div_loch;
        bool en_div2_divprog;
        uint16_t sel_vco;
        uint16_t csw;
        bool success;
    };

    LMS7002M(std::shared_ptr<ISPI> port);

    /*!
     * Set the connection for the LMS7002M driver.
     * @param port the connection interface
     */
    void SetConnection(std::shared_ptr<ISPI> port);

    std::shared_ptr<ISPI> GetConnection(void) const { return controlPort; }

    virtual ~LMS7002M();

    /*!
     * Enum for configuring the channel selection.
     * @see MAC register
     */
    enum class Channel : uint8_t {
        ChA = 1U, ///< Channel A
        ChB = 2U, ///< Channel B
        ChAB = 3U, ///< Both channels
        ChSXR = 1U, ///< SXR register space
        ChSXT = 2U, ///< SXT register space
    };

    /*!
     * Set the selected channel (MAC).
     * The API calls will reflect this channel.
     */
    void SetActiveChannel(const Channel ch);

    /*!
     * Get the selected channel (MAC).
     * The API calls will reflect this channel.
     */
    Channel GetActiveChannel(bool fromChip = true);

    size_t GetActiveChannelIndex(bool fromChip = true);

    /*!
     * Enable/disable the selected channel.
     * This powers on or off all of the respective hardware
     * for a given channel A or B: TSP, BB, and RF sections.
     * @param dir Rx or Tx
     * @param channel true for the transmit size, false for receive
     * @param enable true to enable, false to disable
     */
    int EnableChannel(TRXDir dir, const uint8_t channel, const bool enable);

    int UploadAll();
    int DownloadAll();
    bool IsSynced();
    int CopyChannelRegisters(const Channel src, const Channel dest, bool copySX);

    int ResetChip();

    /*!
     * Perform soft-reset sequence over SPI
     */
    int SoftReset();
    int ResetLogicregisters();

    int LoadConfig(const std::string& filename, bool tuneDynamicValues = true);
    int SaveConfig(const std::string& filename);

    uint16_t Get_SPI_Reg_bits(const LMS7Parameter& param, bool fromChip = false);
    uint16_t Get_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, bool fromChip = false);
    int Modify_SPI_Reg_bits(const LMS7Parameter& param, const uint16_t value, bool fromChip = false);
    int Modify_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, uint16_t value, bool fromChip = false);
    int SPI_write(uint16_t address, uint16_t data, bool toChip = false);
    uint16_t SPI_read(uint16_t address, bool fromChip = false, int* status = 0);
    int RegistersTest(const std::string& fileName = "registersTest.txt");
    static const LMS7Parameter& GetParam(const std::string& name);

    int CalibrateRx(float_type bandwidth, const bool useExtLoopback = false);
    int CalibrateTx(float_type bandwidth, const bool useExtLoopback = false);

    int TuneTxFilter(const float_type bandwidth);
    int TuneRxFilter(const float_type rx_lpf_freq_RF);

    int CalibrateInternalADC(int clkDiv = 32);
    int CalibrateRP_BIAS();
    int CalibrateTxGain(float maxGainOffset_dBFS, float* actualGain_dBFS);
    int CalibrateAnalogRSSI_DC_Offset();

    /*!
     * Set the RX PGA gain in dB
     * @param gain in dB range -12.0, 19.0 dB
     * @return 0 for success, else error
     */
    int SetRBBPGA_dB(const float_type gain, const Channel channel);

    float_type GetRBBPGA_dB(const Channel channel);

    /*!
     * Set the RX LNA gain in dB
     * @param gain in dB range 0.0, 30.0 dB
     * @return 0 for success, else error
     */
    int SetRFELNA_dB(const float_type gain, const Channel channel);

    float_type GetRFELNA_dB(const Channel channel);

    /*!
     * Set the RX loopback LNA gain in dB
     * @param gain in dB range 0.0, 40.0 dB
     * @return 0 for success, else error
     */
    int SetRFELoopbackLNA_dB(const float_type gain, const Channel channel);

    //! Get the actual RX loopback LNA gain in dB
    float_type GetRFELoopbackLNA_dB(const Channel channel);

    /*!
     * Set the RX TIA gain in dB
     * @param gain in dB range 0.0, 12.0 dB
     * @return 0 for success, else error
     */
    int SetRFETIA_dB(const float_type gain, const Channel channel);

    float_type GetRFETIA_dB(const Channel channel);

    /*!
     * Set the TX PAD gain in dB
     * @param gain in dB range -52.0, 0.0 dB
     * @return 0 for success, else error
     */
    int SetTRFPAD_dB(const float_type gain, const Channel channel);

    //! Get the actual TX PAD gain in dB
    float_type GetTRFPAD_dB(const Channel channel);

    /*!
     * Set the TBB frontend gain in dB
     * @param gain in dB relative to optimal gain (0 - optimal gain, >0 may cause saturation)
     * @return 0 for success, else error
     */
    int SetTBBIAMP_dB(const float_type gain, const Channel channel);

    //! Get the TBB frontend gain in dB
    float_type GetTBBIAMP_dB(const Channel channel);

    /*!
     * Set the TX loopback PAD gain in dB
     * @param gain in dB range -4.3, 0.0 dB
     * @return 0 for success, else error
     */
    int SetTRFLoopbackPAD_dB(const float_type gain, const Channel channel);

    //! Get the actual TX loopback PAD gain in dB
    float_type GetTRFLoopbackPAD_dB(const Channel channel);

    enum class PathRFE : uint8_t {
        NONE,
        LNAH,
        LNAL,
        LNAW,
        LB1,
        LB2,
    };

    //! Set the RFE input path.
    int SetPathRFE(PathRFE path);

    //! Get the currently set RFE path
    PathRFE GetPathRFE(void);

    /*!
     * Set the TRF Band selection.
     * @param band 1 or 2
     */
    int SetBandTRF(const int band);

    /*!
     * Get the TRF Band selection.
     * @return the band 1 or 2
     */
    int GetBandTRF(void);

    int SetPath(TRXDir direction, uint8_t channel, uint8_t path);

    int SetReferenceClk_SX(TRXDir dir, float_type freq_Hz);
    float_type GetReferenceClk_SX(TRXDir dir);
    float_type GetFrequencyCGEN();
    int SetFrequencyCGEN(float_type freq_Hz, const bool retainNCOfrequencies = false, CGEN_details* output = nullptr);
    bool GetCGENLocked(void);
    float_type GetFrequencySX(TRXDir dir);
    int SetFrequencySX(TRXDir dir, float_type freq_Hz, SX_details* output = nullptr);
    int SetFrequencySXWithSpurCancelation(TRXDir dir, float_type freq_Hz, float_type BW);
    bool GetSXLocked(TRXDir dir);

    ///VCO modules available for tuning
    enum class VCO_Module : uint8_t { VCO_CGEN, VCO_SXR, VCO_SXT };
    int TuneCGENVCO();
    int TuneVCO(VCO_Module module);

    int LoadDC_REG_IQ(TRXDir dir, int16_t I, int16_t Q);
    int SetNCOFrequency(TRXDir dir, uint8_t index, float_type freq_Hz);
    float_type GetNCOFrequency(TRXDir dir, uint8_t index, bool fromChip = true);
    int SetNCOPhaseOffsetForMode0(TRXDir dir, float_type angle_Deg);
    int SetNCOPhaseOffset(TRXDir dir, uint8_t index, float_type angle_Deg);
    float_type GetNCOPhaseOffset_Deg(TRXDir dir, uint8_t index);
    int SetGFIRCoefficients(TRXDir dir, uint8_t GFIR_index, const int16_t* coef, uint8_t coefCount);
    int GetGFIRCoefficients(TRXDir dir, uint8_t GFIR_index, int16_t* coef, uint8_t coefCount);
    float_type GetReferenceClk_TSP(TRXDir dir);

    int WriteGFIRCoefficients(TRXDir dir, uint8_t gfirIndex, const float_type* coef, uint8_t coefCount);
    int SetGFIRFilter(TRXDir dir, unsigned ch, bool enabled, double bandwidth);

    int SetNCOFrequencies(TRXDir dir, const float_type* freq_Hz, uint8_t count, float_type phaseOffset);

    std::vector<float_type> GetNCOFrequencies(TRXDir dir, float_type* phaseOffset = nullptr);
    int SetNCOPhases(TRXDir dir, const float_type* angles_deg, uint8_t count, float_type frequencyOffset);

    std::vector<float_type> GetNCOPhases(TRXDir dir, float_type* frequencyOffset = nullptr);

    int SetInterfaceFrequency(float_type cgen_freq_Hz, const uint8_t interpolation, const uint8_t decimation);

    float_type GetSampleRate(TRXDir dir, Channel ch);
    float_type GetSampleRate(TRXDir dir);

    enum class LMLSampleSource : uint8_t {
        AI,
        AQ,
        BI,
        BQ,
    };

    /*!
     * Set the LML sample positions in the RF to baseband direction.
     */
    void ConfigureLML_RF2BB(const LMLSampleSource s0, const LMLSampleSource s1, const LMLSampleSource s2, const LMLSampleSource s3);

    /*!
     * Set the LML sample positions in the baseband to RF direction.
     */
    void ConfigureLML_BB2RF(const LMLSampleSource s0, const LMLSampleSource s1, const LMLSampleSource s2, const LMLSampleSource s3);

    int SetRxDCRemoval(const bool enable);

    /*!
     * Get the RX DC removal filter enabled.
     */
    bool GetRxDCRemoval(void);

    /*!
     * Enables/disables TDD mode
     * @param enable true - use same pll for Tx and Rx, false - us seperate PLLs
     * @return 0 for success for error condition
     */
    int EnableSXTDD(bool enable);

    /*!
     * Set the TX DC offset adjustment.
     * @param dir true for tx, false for rx
     * @param I the real adjustment [+1.0, -1.0]
     * @param Q the imaginary adjustment [+1.0, -1.0]
     * @return 0 for success for error condition
     */
    int SetDCOffset(TRXDir dir, const float_type I, const float_type Q);

    /*!
     * Readback the TX DC offset adjustment.
     * @param dir true for tx, false for rx
     * @param [out] I the real adjustment [+1.0, -1.0]
     * @param [out] Q the imaginary adjustment [+1.0, -1.0]
     */
    void GetDCOffset(TRXDir dir, float_type& I, float_type& Q);

    /*!
     * Set the IQ imbalance correction.
     * @param dir true for tx, false for rx
     * @param phase the phase adjustment [+pi, -pi]
     * @param gainI the real gain adjustment [+1.0, 0.0]
     * @param gainQ the imaginary gain adjustment [+1.0, 0.0]
     */
    int SetIQBalance(const TRXDir dir, const float_type phase, const float_type gainI, const float_type gainQ);

    /*!
     * Get the IQ imbalance correction.
     * @param dir true for tx, false for rx
     * @param [out] phase the phase adjustment [+pi, -pi]
     * @param [out] gainI the real gain adjustment [+1.0, 0.0]
     * @param [out] gainQ the imaginary gain adjustment [+1.0, 0.0]
     */
    void GetIQBalance(const TRXDir dir, float_type& phase, float_type& gainI, float_type& gainQ);

    double GetClockFreq(ClockID clk_id, uint8_t channel);
    void SetClockFreq(ClockID clk_id, double freq, uint8_t channel);

    ///enumeration to indicate module registers intervals
    enum MemorySection {
        LimeLight = 0,
        EN_DIR,
        AFE,
        BIAS,
        XBUF,
        CGEN,
        LDO,
        BIST,
        CDS,
        TRF,
        TBB,
        RFE,
        RBB,
        SX,
        TRX_GAIN,
        TxTSP,
        TxNCO,
        TxGFIR1,
        TxGFIR2,
        TxGFIR3a,
        TxGFIR3b,
        TxGFIR3c,
        RxTSP,
        RxNCO,
        RxGFIR1,
        RxGFIR2,
        RxGFIR3a,
        RxGFIR3b,
        RxGFIR3c,
        RSSI_DC_CALIBRATION,
        RSSI_PDET_TEMP_CONFIG,
        RSSI_DC_CONFIG,
        MEMORY_SECTIONS_COUNT
    };
    virtual int SetDefaults(MemorySection module);
    void ModifyRegistersDefaults(const std::vector<std::pair<uint16_t, uint16_t>>& registerValues);

    static float_type gVCO_frequency_table[3][2];
    static float_type gCGEN_VCO_frequencies[2];

    void EnableValuesCache(bool enabled = true);
    bool IsValuesCacheEnabled();
    MCU_BD* GetMCUControls() const;
    void EnableCalibrationByMCU(bool enabled);
    float_type GetTemperature();

    enum class LogType : uint8_t { LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DATA };
    void SetLogCallback(std::function<void(const char*, LogType)> callback);
    LMS7002M_RegistersMap* BackupRegisterMap(void);
    void RestoreRegisterMap(LMS7002M_RegistersMap* backup);

    int SPI_write_batch(const uint16_t* spiAddr, const uint16_t* spiData, uint16_t cnt, bool toChip = false);
    int SPI_read_batch(const uint16_t* spiAddr, uint16_t* spiData, uint16_t cnt);

    typedef int (*CGENChangeCallbackType)(void* userData);
    void SetOnCGENChangeCallback(CGENChangeCallbackType callback, void* userData = nullptr);

  protected:
    CGENChangeCallbackType mCallback_onCGENChange;
    void* mCallback_onCGENChange_userData;

    bool mCalibrationByMCU;
    MCU_BD* mcuControl;
    bool useCache;
    LMS7002M_RegistersMap* mRegistersMap;

    struct ReadOnlyRegister {
        uint16_t address;
        uint16_t mask;
    };

    static const std::vector<ReadOnlyRegister> readOnlyRegisters;

    std::array<std::array<uint16_t, 2>, MEMORY_SECTIONS_COUNT> MemorySectionAddresses;

    uint32_t GetRSSI(RSSI_measurements* measurements = nullptr);
    void SetRxDCOFF(int8_t offsetI, int8_t offsetQ);

    int CalibrateTxGainSetup();

    int RegistersTestInterval(uint16_t startAddr, uint16_t endAddr, uint16_t pattern, std::stringstream& ss);

    int Modify_SPI_Reg_mask(const uint16_t* addr, const uint16_t* masks, const uint16_t* values, uint8_t start, uint8_t stop);

    virtual void Log(const char* text, LogType type);

    void Log(LogType type, const char* format, ...)
    {
        va_list argList;
        va_start(argList, format);
        Log(type, format, argList);
        va_end(argList);
    }

    std::function<void(const char*, LogType)> log_callback;
    void Log(LogType type, const char* format, va_list argList);

    std::shared_ptr<ISPI> controlPort;
    std::array<int, 2> opt_gain_tbb;
    double _cachedRefClockRate;
    int LoadConfigLegacyFile(const std::string& filename);
};
} // namespace lime
#endif
