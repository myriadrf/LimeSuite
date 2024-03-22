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
#include "limesuite/OpStatus.h"

#include <array>
#include <cstdarg>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace lime {
class ISPI;
class LMS7002M_RegistersMap;
class MCU_BD;

typedef double float_type;

/*! @brief Class for communicating with the LMS7002M chip.
 *
 * More information: https://limemicro.com/technology/lms7002m/
*/
class LIME_API LMS7002M
{
  public:
    /// @brief The maximum frequency of the onboard clock generator.
    static constexpr double CGEN_MAX_FREQ = 640e6;

    /// @brief The IDs of the clocks on the chip.
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

    /*!
     * The constructor for the LMS7002M chip.
     * @param port The connection interface
     */
    LMS7002M(std::shared_ptr<ISPI> port);

    /*!
     * Set the connection for the LMS7002M driver.
     * @param port The connection interface
     */
    void SetConnection(std::shared_ptr<ISPI> port);

    /*!
     * @brief Get the current connection to the device.
     * @return The connection to the device.
     */
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
     *
     * @param ch The channel to set the chip to.
     */
    void SetActiveChannel(const Channel ch);

    /*!
     * Get the selected channel (MAC).
     * The API calls will reflect this channel.
     * @param fromChip Whether to read directly from the chip or use the cache (true = device, false = cache)
     * @return The currently active channel.
     */
    Channel GetActiveChannel(bool fromChip = true);

    /*!
     * Gets the index of the current selected channel (MAC)
     * @param fromChip Whether to read directly from the chip or use the cache (true = device, false = cache)
     * @return The index of the currently active channel (0 - Channel A, 1 - Channel B)
     */
    size_t GetActiveChannelIndex(bool fromChip = true);

    /*!
     * Enable/disable the selected channel.
     * This powers on or off all of the respective hardware
     * for a given channel A or B: TSP, BB, and RF sections.
     * @param dir Rx or Tx
     * @param channel true for the transmit size, false for receive
     * @param enable true to enable, false to disable
     * @return The status of the operation
     */
    OpStatus EnableChannel(TRXDir dir, const uint8_t channel, const bool enable);

    /*!
     * @brief Writes all registers from host to chip
     * @return The status of the operation
     */
    OpStatus UploadAll();

    /*!
     * @brief Reads all registers from the chip to host
     * @return The status of the operation
     */
    OpStatus DownloadAll();

    /*!
     * @brief Reads all chip configuration and checks if it matches with local registers copy.
     * @return Whether the cached value is synced with the device or not
    */
    bool IsSynced();

    /**
     * @brief Copies all the channel specific registers from one channel to another one.
     * @param src The channel to copy from.
     * @param dest The channel to copy to.
     * @param copySX Whether to copy the SX registers or not.
     * @return The status of the operation
     */
    OpStatus CopyChannelRegisters(const Channel src, const Channel dest, bool copySX);

    /*!
     * @brief Sends reset signal to chip, after reset enables B channel controls
     * @return The status of the operation
     */
    OpStatus ResetChip();

    /*!
     * Perform soft-reset sequence over SPI
     * @return The status of the operation
     */
    OpStatus SoftReset();

    /*!
     * @brief Resets the logic registers to their default state.
     * @return The status of the operation
     */
    OpStatus ResetLogicRegisters();

    /*!
     * @brief Reads configuration file and uploads registers to chip
     * @param filename Configuration source file
     * @param tuneDynamicValues Whether to tune the dynamic values or not
     * @return The status of the operation
     */
    OpStatus LoadConfig(const std::string& filename, bool tuneDynamicValues = true);

    /*!
     * @brief Reads all registers from chip and saves to file
     * @param filename destination filename
     * @return The status of the operation
     */
    OpStatus SaveConfig(const std::string& filename);

    /*!
     * @brief Returns given parameter value from chip register
     * @param param LMS7002M control parameter
     * @param fromChip read directly from chip
     * @return parameter value
    */
    uint16_t Get_SPI_Reg_bits(const LMS7Parameter& param, bool fromChip = false);

    /*!
     * @brief Returns given parameter value from chip register
     * @param address register address
     * @param msb most significant bit index
     * @param lsb least significant bit index
     * @param fromChip read directly from chip
     * @return register bits from selected interval, shifted to right by lsb bits
    */
    uint16_t Get_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, bool fromChip = false);

    /*!
     * @brief Change given parameter value
     * @param param LMS7002M control parameter
     * @param fromChip read initial value directly from chip
     * @param value new parameter value
     * @return The status of the operation
     */
    OpStatus Modify_SPI_Reg_bits(const LMS7Parameter& param, const uint16_t value, bool fromChip = false);

    /*!
     * @brief Change given parameter value
     * @param address register address
     * @param msb Most significant bit index
     * @param lsb Least significant bit index
     * @param value new bits value, the value is shifted left by lsb bits
     * @param fromChip read initial value directly from chip
     * @return The status of the operation
     */
    OpStatus Modify_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, uint16_t value, bool fromChip = false);

    /*!
     * @brief Write given data value to whole register
     * @param address SPI address
     * @param data new register value
     * @param toChip whether we're writing to the chip or not
     * @return The status of the operation
     */
    OpStatus SPI_write(uint16_t address, uint16_t data, bool toChip = false);

    /*!
     * @brief Reads whole register value from given address
     * @param address SPI address
     * @param fromChip read value directly from chip
     * @param status The operation success status (optional).
     * @return register value
    */
    uint16_t SPI_read(uint16_t address, bool fromChip = false, OpStatus* status = 0);

    /*!
     * @brief Performs registers test by writing known data and confirming readback data
     * @param fileName The name of the file to write the test output to.
     * @return The operation status.
     */
    OpStatus RegistersTest(const std::string& fileName = "registersTest.txt");

    /*!
     * @brief Get parameter by name
     * @param name The name of the parameter to get.
     * @return A constant reference to the parameter
     */
    static const LMS7Parameter& GetParam(const std::string& name);

    /*!
     * @brief Calibrates Receiver. DC offset, IQ gains, IQ phase correction
     * @param bandwidth_Hz The bandwidth to calibrate the device for (in Hz)
     * @param useExtLoopback Whether to use external loopback or not.
     * @return The status of the operation
     */
    OpStatus CalibrateRx(float_type bandwidth_Hz, const bool useExtLoopback = false);

    /*!
     * @brief Calibrates Transmitter. DC correction, IQ gains, IQ phase correction
     * @param bandwidth_Hz The bandwidth to calibrate the device for (in Hz)
     * @param useExtLoopback Whether to use external loopback or not.
     * @return The status of the operation
     */
    OpStatus CalibrateTx(float_type bandwidth_Hz, const bool useExtLoopback = false);

    /**
     * @brief Tunes the Low Pass Filter for the TX direction.
     * @param tx_lpf_freq_RF The frequency (in Hz) to tune the filter to.
     * @return The status of the operation
     */
    OpStatus TuneTxFilter(const float_type tx_lpf_freq_RF);

    /**
     * @brief Tunes the Low Pass Filter for the RX direction.
     * @param rx_lpf_freq_RF The frequency (in Hz) to tune the filter to.
     * @return The status of the operation
     */
    OpStatus TuneRxFilter(const float_type rx_lpf_freq_RF);

    /*!
     * @brief Calibrates the internal Analog to Digital Converter on the chip.
     * @param clkDiv The clock division ratio for measurement loop.
     * @return The status of the operation
     */
    OpStatus CalibrateInternalADC(int clkDiv = 32);

    /*!
     * @brief Calibrates the RP_BIAS of the chip.
     * @return The status of the operation
     */
    OpStatus CalibrateRP_BIAS();

    /*!
     * @brief Calibrates the TX gain.
     * @return The status of the operation
     */
    OpStatus CalibrateTxGain();

    /**
     * @brief Calibrates the Analog RSSI
     * @return The status of the operation
     */
    OpStatus CalibrateAnalogRSSI_DC_Offset();

    /*!
     * Set the RX PGA gain in dB
     * @param gain In dB range -12.0, 19.0 dB
     * @param channel The channel to set it for
     * @return The status of the operation
     */
    OpStatus SetRBBPGA_dB(const float_type gain, const Channel channel);

    /*!
     * Gets the RX PGA gain in dB
     * @param channel The channel to get it from
     * @return The RX PGA gain (in dB)
     */
    float_type GetRBBPGA_dB(const Channel channel);

    /*!
     * Set the RX LNA gain in dB
     * @param gain In dB range 0.0, 30.0 dB
     * @param channel The channel to set it for
     * @return The status of the operation
     */
    OpStatus SetRFELNA_dB(const float_type gain, const Channel channel);

    /*!
     * Gets the RX LNA gain in dB
     * @param channel The channel to get it from
     * @return The RX LNA gain (in dB)
     */
    float_type GetRFELNA_dB(const Channel channel);

    /*!
     * Set the RX loopback LNA gain in dB
     * @param gain In dB range 0.0, 40.0 dB
     * @param channel The channel to set it for
     * @return The status of the operation
     */
    OpStatus SetRFELoopbackLNA_dB(const float_type gain, const Channel channel);

    /*!
     * Get the actual RX loopback LNA gain in dB
     * @param channel The channel to get it from
     * @return The actual RX loopback LNA gain (in dB)
     */
    float_type GetRFELoopbackLNA_dB(const Channel channel);

    /*!
     * Set the RX TIA gain in dB
     * @param gain In dB range 0.0, 12.0 dB
     * @param channel The channel to set it for
     * @return The status of the operation
     */
    OpStatus SetRFETIA_dB(const float_type gain, const Channel channel);

    /*!
     * Get the RX TIA gain in dB
     * @param channel The channel to get it from
     * @return The RX TIA gain in dB
     */
    float_type GetRFETIA_dB(const Channel channel);

    /*!
     * Set the TX PAD gain in dB
     * @param gain In dB range -52.0, 0.0 dB
     * @param channel The channel to set it for
     * @return The status of the operation
     */
    OpStatus SetTRFPAD_dB(const float_type gain, const Channel channel);

    /*!
     * @brief Gets the actual TX PAD gain.
     * @param channel The channel to get the gain from.
     * @return The actual TX PAD gain (in dB).
     */
    float_type GetTRFPAD_dB(const Channel channel);

    /*!
     * Set the TBB frontend gain in dB
     * @param gain In dB relative to optimal gain (0 - optimal gain, >0 may cause saturation)
     * @param channel The channel to set it for
     * @return The status of the operation
     */
    OpStatus SetTBBIAMP_dB(const float_type gain, const Channel channel);

    /*!
     * @brief Gets the TBB frontend gain.
     * @param channel The channel to get the gain from.
     * @return The actual TBB frontend gain (in dB).
     */
    float_type GetTBBIAMP_dB(const Channel channel);

    /*!
     * Set the TX loopback PAD gain in dB
     * @param gain In dB range -4.3, 0.0 dB
     * @param channel The channel to set it for
     * @return The status of the operation
     */
    OpStatus SetTRFLoopbackPAD_dB(const float_type gain, const Channel channel);

    /*!
     * @brief Gets the actual TX loopback PAD gain.
     * @param channel The channel to get the gain from.
     * @return The actual TX loopback PAD gain (in dB).
     */
    float_type GetTRFLoopbackPAD_dB(const Channel channel);

    /// @brief The possible antennae on the chip.
    enum class PathRFE : uint8_t {
        NONE,
        LNAH,
        LNAL,
        LNAW,
        LB1,
        LB2,
    };

    /*!
     * @brief Sets the RFE input path.
     * @param path The enumeration value of the path to set it to
     * @return The status of the operation
     */
    OpStatus SetPathRFE(PathRFE path);

    /*!
     * @brief Gets the currently set RFE path
     * @return The enumerator value of the currently selected RFE path.
     */
    PathRFE GetPathRFE(void);

    /*!
     * Set the TRF Band selection.
     * @param band 1 or 2
     * @return The status of the operation
     */
    OpStatus SetBandTRF(const int band);

    /*!
     * Get the TRF Band selection.
     * @return the band 1 or 2
     */
    int GetBandTRF(void);

    /*!
     * @brief Sets the antenna to use on the device
     * @param direction The direction to set the antenna for.
     * @param channel The channel to set the antenna for.
     * @param path The index of the antenna to use.
     * @return The status of the operation
     */
    OpStatus SetPath(TRXDir direction, uint8_t channel, uint8_t path);

    /*!
     * @brief Sets the reference clock of the SX
     * @param dir Rx/Tx module selection
     * @param freq_Hz The frequency to set the reference clock to (in Hz)
     * @return The status of the operation
     */
    OpStatus SetReferenceClk_SX(TRXDir dir, float_type freq_Hz);

    /*!
     * @brief Returns reference clock in Hz used for SXT or SXR.
	 * @param dir transmitter or receiver selection.
     * @return The reference clock speed (in Hz).
    */
    float_type GetReferenceClk_SX(TRXDir dir);

    /*!
     * Returns the curernt frequency of the clock generator.
     * @return Current CLKGEN frequency in Hz.
     * Returned frequency depends on reference clock used for Receiver.
    */
    float_type GetFrequencyCGEN();

    /*!
     * @brief Sets CLKGEN frequency, calculations use receiver'r reference clock
     * @param freq_Hz desired frequency in Hz
     * @param retainNCOfrequencies recalculate NCO coefficients to keep currently set frequencies
     * @param output if not null outputs calculated CGEN parameters
     * @return The status of the operation
     */
    OpStatus SetFrequencyCGEN(float_type freq_Hz, const bool retainNCOfrequencies = false, CGEN_details* output = nullptr);

    /*!
     * @brief Gets whether the VCO comparators of the clock generator are locked or not.
     * @return A value indicating whether the VCO comparators of the clock generator are locked or not.
     */
    bool GetCGENLocked(void);

    /*!
     * @brief Returns currently set SXR/SXT frequency
     * @param dir Rx/Tx module selection
	 * @return SX frequency Hz
     */
    float_type GetFrequencySX(TRXDir dir);

    /*!
     * @brief Sets SX frequency
     * @param dir Rx/Tx module selection
     * @param freq_Hz desired frequency in Hz
     * @param output if not null outputs intermediate calculation values
     * @return The status of the operation
     */
    OpStatus SetFrequencySX(TRXDir dir, float_type freq_Hz, SX_details* output = nullptr);

    /*!
     * @brief Sets SX frequency with Reference clock spur cancelation
     * @param dir Rx/Tx module selection
     * @param freq_Hz desired frequency in Hz
     * @param BW The bandwidth (in Hz)
     * @return The status of the operation
     */
    OpStatus SetFrequencySXWithSpurCancelation(TRXDir dir, float_type freq_Hz, float_type BW);

    /*!
     * @brief Gets whether the VCO comparators of the LO synthesizer are locked or not.
     * @param dir The direction to read from.
     * @return A value indicating whether the VCO comparators of the clock generator are locked or not.
     */
    bool GetSXLocked(TRXDir dir);

    ///VCO modules available for tuning
    enum class VCO_Module : uint8_t { VCO_CGEN, VCO_SXR, VCO_SXT };

    /*!
     * @brief Performs VCO tuning operations for CLKGEN
     * @return The status of the operation
     */
    OpStatus TuneCGENVCO();

    /*!
     * @brief Performs VCO tuning operations for CLKGEN, SXR, SXT modules
     * @param module module selection for tuning 0-cgen, 1-SXR, 2-SXT
     * @return The status of the operation
     */
    OpStatus TuneVCO(VCO_Module module);

    /*!
     * @brief Loads given DC_REG values into registers
     * @param dir TxTSP or RxTSP selection
     * @param I DC_REG I value
     * @param Q DC_REG Q value
     * @return The status of the operation
     */
    OpStatus LoadDC_REG_IQ(TRXDir dir, int16_t I, int16_t Q);

    /*!
     * @brief Sets chosen NCO's frequency
     * @param dir transmitter or receiver selection
     * @param index NCO index from 0 to 15
     * @param freq_Hz desired NCO frequency
     * @return The status of the operation
     */
    OpStatus SetNCOFrequency(TRXDir dir, uint8_t index, float_type freq_Hz);

    /*!
     * @brief Returns chosen NCO's frequency in Hz
     * @param dir transmitter or receiver selection
     * @param index NCO index from 0 to 15
     * @param fromChip read frequency directly from chip or local registers
     * @return NCO frequency in Hz
     */
    float_type GetNCOFrequency(TRXDir dir, uint8_t index, bool fromChip = true);

    /*!
     * @brief Sets chosen NCO phase offset angle when memory table MODE is 0
     * @param dir transmitter or receiver selection
     * @param angle_deg phase offset angle in degrees
     * @return The status of the operation
     */
    OpStatus SetNCOPhaseOffsetForMode0(TRXDir dir, float_type angle_deg);

    /*!
     * @brief Sets chosen NCO's phase offset angle
     * @param dir transmitter or receiver selection
     * @param index PHO index from 0 to 15
     * @param angle_deg phase offset angle in degrees
     * @return The status of the operation
     */
    OpStatus SetNCOPhaseOffset(TRXDir dir, uint8_t index, float_type angle_deg);

    /*!
     * @brief Returns chosen NCO's phase offset angle in radians
     * @param dir transmitter or receiver selection
     * @param index PHO index from 0 to 15
     * @return phase offset angle in degrees
     */
    float_type GetNCOPhaseOffset_Deg(TRXDir dir, uint8_t index);

    /*!
     * @brief Returns TSP reference frequency
     * @param dir TxTSP or RxTSP selection
     * @return TSP reference frequency in Hz
     */
    float_type GetReferenceClk_TSP(TRXDir dir);

    /*!
     * @brief Returns currently loaded FIR coefficients.
     * @param dir Transmitter or receiver selection.
     * @param gfirIndex GFIR index from 0 to 2.
     * @param coef Array of returned coefficients (normalized from -1 to 1)
     * @param coefCount Number of coefficients to read.
     * @return The status of the operation.
     */
    OpStatus GetGFIRCoefficients(TRXDir dir, uint8_t gfirIndex, float_type* coef, uint8_t coefCount);

    /*!
     * @brief Uploads given FIR coefficients to chip
     * @param dir Transmitter or receiver selection
     * @param gfirIndex GFIR index from 0 to 2
     * @param coef array of coefficients (normalized from -1 to 1)
     * @param coefCount number of coefficients
     * @return The status of the operation
     *
     * This function does not change GFIR*_L or GFIR*_N parameters, they have to be set manually
     */
    OpStatus SetGFIRCoefficients(TRXDir dir, uint8_t gfirIndex, const float_type* coef, uint8_t coefCount);

    /*!
     * @brief Sets up the GFIR filter.
     * @param dir The direction for which to set up the filter.
     * @param ch The channel for which to set up the filter
     * @param enabled Whether to enable or disable the GFIR filter
     * @param bandwidth The bandwidth (in Hz) to set the filter for.
     * @return The status of the operation
     */
    OpStatus SetGFIRFilter(TRXDir dir, Channel ch, bool enabled, double bandwidth);

    /*!
     * @brief Sets the frequencies of the NCO.
     * @param dir The direction for which to set the frequencies.
     * @param freq_Hz The frequency array to set.
     * @param count The amount of frequencies to set (max 16)
     * @param phaseOffset The phase offset of the NCO to set.
     * @return The status of the operation
     */
    OpStatus SetNCOFrequencies(TRXDir dir, const float_type* freq_Hz, uint8_t count, float_type phaseOffset);

    /*!
     * @brief Gets the current frequencies of the NCO.
     * @param dir The direction to receive the frequencies of.
     * @param phaseOffset Returns the phase offset in here if it's not nullptr.
     * @return The frequencies of the NCO (in Hz).
     */
    std::vector<float_type> GetNCOFrequencies(TRXDir dir, float_type* phaseOffset = nullptr);

    /*!
     * @brief Sets the phases of the NCO (up to 16 values)
     * @param dir The direction to which to set the phases.
     * @param angles_deg The angles of the NCO to set (in degrees).
     * @param count The amount of angles to set (max 16)
     * @param frequencyOffset The offset of the NCO (in Hz).
     * @return The status of the operation
     */
    OpStatus SetNCOPhases(TRXDir dir, const float_type* angles_deg, uint8_t count, float_type frequencyOffset);

    /*!
     * @brief Gets the current phases of the NCO (currently returns an empty vector)
     * @param dir The direction from which to get the phases.
     * @param frequencyOffset The offset of the NCO (in Hz).
     * @return The phases of the NCO (in degrees)
     */
    std::vector<float_type> GetNCOPhases(TRXDir dir, float_type* frequencyOffset = nullptr);

    /*!
     * @brief Configures interfaces for desired frequency
     * Sets interpolation and decimation, changes MCLK sources and TSP clock dividers accordingly to selected interpolation and decimation
     * @param cgen_freq_Hz The clock frequency to set (in Hz)
     * @param interpolation The HBI interpolation ratio (actual ratio is pow(2, interpolation + 1), 7 for bypass)
     * @param decimation The HBD decimation ratio (actual ratio is pow(2, decimation + 1), 7 for bypass)
     * @return The status of the operation
     */
    OpStatus SetInterfaceFrequency(float_type cgen_freq_Hz, const uint8_t interpolation, const uint8_t decimation);

    /*!
     * @brief Gets the current sample rate of the specified channel.
     * @param dir The direction for which to get the sample rate.
     * @param ch The channel from which to get the sample rate.
     * @return The current sample rate (in Hz)
     */
    float_type GetSampleRate(TRXDir dir, Channel ch);

    /*!
     * @brief Gets the current sample rate of the currently active channel.
     * @param dir The direction for which to get the sample rate.
     * @return The current sample rate (in Hz)
     */
    float_type GetSampleRate(TRXDir dir);

    /// @brief The sample source for the LML.
    enum class LMLSampleSource : uint8_t {
        AI,
        AQ,
        BI,
        BQ,
    };

    /*!
     * @brief Set the LML sample positions in the RF to baseband direction.
     * @param s0
     * @param s1
     * @param s2
     * @param s3
     */
    void ConfigureLML_RF2BB(const LMLSampleSource s0, const LMLSampleSource s1, const LMLSampleSource s2, const LMLSampleSource s3);

    /*!
     * @brief Set the LML sample positions in the baseband to RF direction.
     * @param s0
     * @param s1
     * @param s2
     * @param s3
     */
    void ConfigureLML_BB2RF(const LMLSampleSource s0, const LMLSampleSource s1, const LMLSampleSource s2, const LMLSampleSource s3);

    /*!
     * @brief Enables or disables the DC corrector bypass
     * @param enable Enables or disables the
     * @return The status of the operation
     */
    OpStatus SetRxDCRemoval(const bool enable);

    /*!
     * Get the RX DC removal filter enabled.
     * @return Whether the DC corrector bypass is enabled or not.
     */
    bool GetRxDCRemoval(void);

    /*!
     * Enables/disables TDD mode
     * @param enable true - use same PLL for Tx and Rx, false - us seperate PLLs
     * @return The status of the operation
     */
    OpStatus EnableSXTDD(bool enable);

    /*!
     * Set the TX DC offset adjustment.
     * @param dir true for tx, false for rx
     * @param I the real adjustment [+1.0, -1.0]
     * @param Q the imaginary adjustment [+1.0, -1.0]
     * @return The status of the operation
     */
    OpStatus SetDCOffset(TRXDir dir, const float_type I, const float_type Q);

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
     * @return The status of the operation
     */
    OpStatus SetIQBalance(const TRXDir dir, const float_type phase, const float_type gainI, const float_type gainQ);

    /*!
     * Get the IQ imbalance correction.
     * @param dir true for tx, false for rx
     * @param [out] phase the phase adjustment [+pi, -pi]
     * @param [out] gainI the real gain adjustment [+1.0, 0.0]
     * @param [out] gainQ the imaginary gain adjustment [+1.0, 0.0]
     */
    void GetIQBalance(const TRXDir dir, float_type& phase, float_type& gainI, float_type& gainQ);

    /*!
     * @brief Gets the frequency of the selected clock.
     * @param clk_id The enumerator value of the clock to get.
     * @return The current frequency of that clock (in Hz).
     */
    double GetClockFreq(ClockID clk_id);

    /*!
     * @brief Sets the frequency of the selected clock.
     * @param clk_id The enumerator value of the clock to set.
     * @param freq The frequency (in Hz) to set the clock to.
     * @return The operation success status.
     */
    OpStatus SetClockFreq(ClockID clk_id, double freq);

    /*!
     * @brief Modifies the defaults of this instance's Register map.
     * @param registerValues A vector of address value pairs to modify the defaults to.
     */
    void ModifyRegistersDefaults(const std::vector<std::pair<uint16_t, uint16_t>>& registerValues);

    /*!
     * @brief Sets whether a local registers cache is being used or not.
     * @param enabled Whether to enable the cache or not.
     */
    void EnableValuesCache(bool enabled = true);

    /*!
     * @brief Returns whether register value caching on the host is enabled or not.
     * @return True - cache is being used, false - device values only.
     */
    bool IsValuesCacheEnabled() const;

    /*!
     * @brief Gets the class to control the MCU on the chip.
     * @return A pointer to the class responsible for controlling the MCU.
     */
    MCU_BD* GetMCUControls() const;

    /*!
     * @brief Gets the of the chip
     * @return The current temperature of the chip (in degrees Celsius)
     */
    float_type GetTemperature();

    /// @brief The available log types on the LMS7002M
    enum class LogType : uint8_t { LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DATA };
    /*!
     * @brief Sets the function to call for any logs to be written to.
     * @param callback The function to call on LMS7002M logs
     */
    void SetLogCallback(std::function<void(const char*, LogType)> callback);

    /*!
     * @brief Batches multiple register writes into least amount of transactions
     * @param spiAddr spi register addresses to be written
     * @param spiData registers data to be written
     * @param cnt number of registers to write
     * @param toChip force write to chip
     * @return The status of the operation
     */
    OpStatus SPI_write_batch(const uint16_t* spiAddr, const uint16_t* spiData, uint16_t cnt, bool toChip = false);

    /*!
     * @brief Batches multiple register reads into least amount of transactions
     * @param spiAddr SPI addresses to read
     * @param spiData array for read data
     * @param cnt number of registers to read
     * @return The status of the operation
     */
    OpStatus SPI_read_batch(const uint16_t* spiAddr, uint16_t* spiData, uint16_t cnt);

    /// @brief The clock generator change callback type
    typedef OpStatus (*CGENChangeCallbackType)(void* userData);

    /*!
     * @brief Sets the function that gets executed when the CGEN changes.
     * @param callback The function to call on CGEN change.
     * @param userData The data to pass to the function being called.
     */
    void SetOnCGENChangeCallback(CGENChangeCallbackType callback, void* userData = nullptr);

  private:
    ///enumeration to indicate module registers intervals
    enum class MemorySection : uint8_t {
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
    };

    /*!
     * @brief Sets given module registers to default values
     * @return The status of the operation
     */
    virtual OpStatus SetDefaults(MemorySection module);

    LMS7002M_RegistersMap* BackupRegisterMap(void);
    void RestoreRegisterMap(LMS7002M_RegistersMap* backup);

    CGENChangeCallbackType mCallback_onCGENChange;
    void* mCallback_onCGENChange_userData;

    MCU_BD* mcuControl;
    bool useCache;
    LMS7002M_RegistersMap* mRegistersMap;

    struct ReadOnlyRegister {
        uint16_t address;
        uint16_t mask;
    };

    static const std::vector<ReadOnlyRegister> readOnlyRegisters;

    static const std::map<MemorySection, std::array<uint16_t, 2>> MemorySectionAddresses;
    static const std::array<std::array<float_type, 2>, 3> gVCO_frequency_table;
    static const std::array<float_type, 2> gCGEN_VCO_frequencies;

    uint32_t GetRSSI();
    void SetRxDCOFF(int8_t offsetI, int8_t offsetQ);

    OpStatus CalibrateTxGainSetup();

    OpStatus RegistersTestInterval(uint16_t startAddr, uint16_t endAddr, uint16_t pattern, std::stringstream& ss);

    OpStatus Modify_SPI_Reg_mask(const uint16_t* addr, const uint16_t* masks, const uint16_t* values, uint8_t start, uint8_t stop);

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
    OpStatus LoadConfigLegacyFile(const std::string& filename);
};
} // namespace lime
#endif
