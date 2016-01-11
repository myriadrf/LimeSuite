/**
@file	LMS7002M.h
@author Lime Microsystems (www.limemicro.com)
@brief	LMS7002M transceiver configuration interface
*/

#ifndef LMS7API_H
#define LMS7API_H

#include "LMS7002M_statuses.h"
#include "LMS7002M_parameters.h"
#include "typedefs.h"

#include <sstream>

class IConnection;
class LMS7002M_RegistersMap;

class LMS7002M
{
public:
    enum
    {
        Rx, Tx
    };

	LMS7002M();

    /*!
     * Set the connection for the LMS7002M driver.
     * \param port the connection interface
     * \param devIndex which RFIC index (default 0 for most devices)
     */
    void SetConnection(IConnection* port, const size_t devIndex = 0);

	virtual ~LMS7002M();

    /*!
     * Enum for configuring the channel selection.
     * @see MAC register
     */
    enum Channel
    {
        ChA = 1,
        ChB = 2,
        ChAB = 3,
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

    /*!
     * Get the channel selected by the RFIC index (devIndex),
     * and by the currently selected RF channel A/B (MAC).
     * Example when devIndex == 0, return 0 for chA, 1 for chB.
     */
    size_t GetActiveChannelIndex(void);

    ///@name Registers writing and reading
    liblms7_status UploadAll();
    liblms7_status DownloadAll();
    bool IsSynced();

	liblms7_status ResetChip();
	liblms7_status LoadConfig(const char* filename);
	liblms7_status SaveConfig(const char* filename);
    ///@}

    ///@name Registers writing and reading
    uint16_t Get_SPI_Reg_bits(const LMS7Parameter &param, bool fromChip = true);
    uint16_t Get_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, bool fromChip = true);
    liblms7_status Modify_SPI_Reg_bits(const LMS7Parameter &param, const uint16_t value, bool fromChip = true);
    liblms7_status Modify_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, uint16_t value, bool fromChip = true);
    liblms7_status SPI_write(uint16_t address, uint16_t data);
    uint16_t SPI_read(uint16_t address, bool fromChip = true, liblms7_status *status = 0);
    liblms7_status RegistersTest();
    ///@}

    ///@name Transmitter, Receiver calibrations
	liblms7_status CalibrateRx(float_type bandwidth_MHz);
	liblms7_status CalibrateTx(float_type bandwidth_MHz);
    ///@}

    ///@name Filters tuning
	enum TxFilter
	{
		TX_LADDER, TX_REALPOLE, TX_HIGHBAND
	};
    enum RxFilter
    {
        RX_TIA, RX_LPF_LOWBAND, RX_LPF_HIGHBAND
    };
	liblms7_status TuneTxFilter(TxFilter filterType, float_type bandwidth_MHz);
	liblms7_status TuneTxFilterLowBandChain(float_type ladder_bw_MHz, float_type realpole_bw_MHz);
	liblms7_status TuneRxFilter(RxFilter filterType, float_type bandwidth_MHz);
    ///@}

    ///@name RF selection
    enum PathRFE
    {
        PATH_RFE_NONE = 0,
        PATH_RFE_LNAH = int('H'),
        PATH_RFE_LNAL = int('L'),
        PATH_RFE_LNAW = int('W'),
        PATH_RFE_LB1 = 1,
        PATH_RFE_LB2 = 2,
    };

    //! Set the RFE input path.
    liblms7_status SetPathRFE(PathRFE path);

    //! Get the currently set RFE path
    PathRFE GetPathRFE(void);

    /*!
     * Set the TRF Band selection.
     * @param band 1 or 2
     */
    liblms7_status SetBandTRF(const int band);

    /*!
     * Get the TRF Band selection.
     * @return the band 1 or 2
     */
    int GetBandTRF(void);

    /*!
     * Update the external band selection by calling
     * UpdateExternalBandSelect() on the connection object.
     * This is called automatically by the LMS7002M driver,
     * but can also be called manually by the user.
     */
    void UpdateExternalBandSelect(void);

    ///@}

    ///@name CGEN and PLL
	float_type GetReferenceClk_SX(bool tx);
	float_type GetFrequencyCGEN_MHz();
	liblms7_status SetFrequencyCGEN(float_type freq_MHz);
	float_type GetFrequencySX_MHz(bool tx, float_type refClk_MHz);
	liblms7_status SetFrequencySX(bool tx, float_type freq_MHz, float_type refClk_MHz);
    ///VCO modules available for tuning
    enum VCO_Module
    {
        VCO_CGEN, VCO_SXR, VCO_SXT
    };
    liblms7_status TuneVCO(VCO_Module module);
    ///@}

    ///@name TSP
	liblms7_status LoadDC_REG_IQ(bool tx, int16_t I, int16_t Q);
	liblms7_status SetNCOFrequency(bool tx, uint8_t index, float_type freq_MHz);
	float_type GetNCOFrequency_MHz(bool tx, uint8_t index, float_type refClk_MHz, bool fromChip = true);
    liblms7_status SetNCOPhaseOffsetForMode0(bool tx, float_type angle_Deg);
	liblms7_status SetNCOPhaseOffset(bool tx, uint8_t index, float_type angle_Deg);
	float_type GetNCOPhaseOffset_Deg(bool tx, uint8_t index);
	liblms7_status SetGFIRCoefficients(bool tx, uint8_t GFIR_index, const int16_t *coef, uint8_t coefCount);
	liblms7_status GetGFIRCoefficients(bool tx, uint8_t GFIR_index, int16_t *coef, uint8_t coefCount);
    float_type GetReferenceClk_TSP_MHz(bool tx);
    ///@}

    liblms7_status SetInterfaceFrequency(float_type cgen_freq_MHz, const uint8_t interpolation, const uint8_t decimation);

    ///enumeration to indicate module registers intervals
    enum MemorySection
    {
        LimeLight = 0, EN_DIR, AFE, BIAS, XBUF, CGEN, LDO, BIST, CDS,
        TRF, TBB, RFE, RBB, SX, TxTSP,
        TxNCO, TxGFIR1, TxGFIR2, TxGFIR3a, TxGFIR3b, TxGFIR3c,
        RxTSP, RxNCO, RxGFIR1, RxGFIR2, RxGFIR3a, RxGFIR3b, RxGFIR3c,
        MEMORY_SECTIONS_COUNT
    };
    virtual liblms7_status SetDefaults(MemorySection module);

    static const float_type gLadder_lower_limit;
    static const float_type gLadder_higher_limit;
    static const float_type gRealpole_lower_limit;
    static const float_type gRealpole_higher_limit;
    static const float_type gHighband_lower_limit;
    static const float_type  gHighband_higher_limit;

    static const float_type gRxTIA_higher_limit;
    static const float_type gRxTIA_lower_limit_g1;
    static const float_type gRxTIA_lower_limit_g23;
    static const float_type gRxLPF_low_lower_limit;
    static const float_type gRxLPF_low_higher_limit;
    static const float_type gRxLPF_high_lower_limit;
    static const float_type gRxLPF_high_higher_limit;

    static float_type gVCO_frequency_table[3][2];
    static float_type gCGEN_VCO_frequencies[2];

protected:
    LMS7002M_RegistersMap *mRegistersMap;
    static const uint16_t readOnlyRegisters[];
    static const uint16_t readOnlyRegistersMasks[];

    uint16_t MemorySectionAddresses[MEMORY_SECTIONS_COUNT][2];
    ///@name Algorithms functions
    void BackupAllRegisters();
    void RestoreAllRegisters();
    uint32_t GetRSSI();
    void SetRxDCOFF(int8_t offsetI, int8_t offsetQ);
    uint32_t FindMinRSSI_Gain(const LMS7Parameter &param, uint16_t *foundValue);
    uint32_t FindMinRSSI(const LMS7Parameter &param, const int16_t startValue, int16_t *result, const uint8_t scanWidth, const uint8_t twoCompl, int8_t stepMult = 1);
    uint32_t FindMinRSSI(const uint16_t addr, const uint8_t msb, const uint8_t lsb, const int16_t startValue, int16_t *result, const uint8_t scanWidth, const uint8_t twoCompl, int8_t stepMult = 1);
    void CalibrateRxDC_RSSI();
    liblms7_status CalibrateTxSetup(float_type bandwidth_MHz);
    liblms7_status CalibrateRxSetup(float_type bandwidth_MHz);
    liblms7_status FixRXSaturation();
    void FilterTuning_AdjustGains();
    liblms7_status TuneTxFilterSetup(TxFilter type, float_type cutoff_MHz);
    liblms7_status TuneRxFilterSetup(RxFilter type, float_type cutoff_MHz);
    liblms7_status RFE_TIA_Calibration(float_type TIA_freq_MHz);
    liblms7_status RxLPFLow_Calibration(float_type RxLPFL_freq_MHz);
    liblms7_status RxLPFHigh_Calibration(float_type RxLPFH_freq_MHz);

    liblms7_status RegistersTestInterval(uint16_t startAddr, uint16_t endAddr, uint16_t pattern, std::stringstream &ss);
    liblms7_status SPI_write_batch(const uint16_t* spiAddr, const uint16_t* spiData, uint16_t cnt);
    liblms7_status SPI_read_batch(const uint16_t* spiAddr, uint16_t* spiData, uint16_t cnt);
    liblms7_status Modify_SPI_Reg_mask(const uint16_t *addr, const uint16_t *masks, const uint16_t *values, uint8_t start, uint8_t stop);
    ///@}
    ///Reference clock used for Receiver frequency calculations
    float_type mRefClkSXR_MHz;
    ///Reference clock used for Transmitter frequency calculations
    float_type mRefClkSXT_MHz;

    enum LogType
    {
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_DATA
    };
    virtual void Log(const char* text, LogType type);

    ///port used for communicating with LMS7002M
    IConnection* controlPort;
    int addrLMS7002M;
    size_t mdevIndex;

    liblms7_status LoadConfigLegacyFile(const char* filename);
};
#endif
