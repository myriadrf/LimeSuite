/**
@file	MCU_BD.h
@author	Lime Microsystems
@brief	Header for MCU_BD.cpp
*/

#ifndef MCU_BD_H
#define MCU_BD_H

#include <atomic>
#include <string>
#include <functional>
#include <memory>

#include "mcu_programs.h"

namespace lime {

class ISPI;

class MCU_BD
{
  public:
    enum class MCU_PROG_MODE : uint8_t { RESET, EEPROM_AND_SRAM, SRAM, BOOT_SRAM_FROM_EEPROM };
    enum MCU_ERROR_CODES {
        MCU_NO_ERROR = 0,
        MCU_ERROR,
        MCU_CGEN_TUNE_FAILED,
        MCU_SXR_TUNE_FAILED,
        MCU_SXT_TUNE_FAILED,
        MCU_LOOPBACK_SIGNAL_WEAK,
        MCU_INVALID_RX_PATH,
        MCU_INVALID_TX_BAND,
        MCU_RX_LPF_OUT_OF_RANGE,
        MCU_RX_INVALID_TIA,
        MCU_TX_LPF_OUT_OF_RANGE,
        MCU_PROCEDURE_DISABLED,
        MCU_R_CTL_LPF_LIMIT_REACHED,
        MCU_CFB_TIA_RFE_LIMIT_REACHED,
        MCU_RCAL_LPF_LIMIT_REACHED,

        MCU_ERROR_CODES_COUNT
    };

    enum class OperationStatus : uint8_t {
        SUCCESS,
        FAILURE,
        TIMEOUT,
    };

    /** @brief Structure containing the information about the current progress. */
    struct ProgressInfo {
        unsigned short stepsDone;
        unsigned short stepsTotal;
        bool aborted;
    };
    ProgressInfo GetProgressInfo() const;

    MCU_BD();
    virtual ~MCU_BD();
    int m_iLoopTries;
    std::string GetProgramFilename() const;

    void RunProcedure(uint8_t id);
    enum class MCU_Parameter : uint8_t {
        MCU_REF_CLK,
        MCU_BW,
        MCU_EXT_LOOPBACK_PAIR,
    };
    void SetParameter(MCU_Parameter param, float value);
    int WaitForMCU(uint32_t timeout_ms);
    static const char* MCUStatusMessage(const uint8_t code);

    static const int cMaxFWSize = 1024 * 16;

    /*!
     * Callback from programming processes
     * @param bsent number of bytes transferred
     * @param btotal total number of bytes to send
     * @param progressMsg string describing current progress state
     * @return 0-continue programming, 1-abort operation
     */
    typedef std::function<bool(int bsent, int btotal, const char* progressMsg)> ProgrammingCallback;

    uint8_t ReadMCUProgramID();
    OperationStatus SetDebugMode(bool enabled, MCU_PROG_MODE mode);
    OperationStatus readIRAM(const std::byte* addr, std::byte* values, const uint8_t count);
    OperationStatus writeIRAM(const std::byte* addr, const std::byte* values, const uint8_t count);

    void Wait_CLK_Cycles(int data);
    // The IRAM content
    std::array<std::byte, 256> m_IRAM;
    // The SFR content
    std::array<std::byte, 256> m_SFR;
    // The program memory code
    std::array<std::byte, cMaxFWSize> byte_array;

    void mSPI_write(unsigned short addr_reg, unsigned short data_reg);
    unsigned short mSPI_read(unsigned short addr_reg);
    int Three_byte_command(
        std::byte data1, std::byte data2, std::byte data3, std::byte& rdata1, std::byte& rdata2, std::byte& rdata3);
    int GetProgramCode(const char* inFileName, bool bin = false);
    int Change_MCUFrequency(std::byte data);
    int Read_IRAM();
    int Erase_IRAM();
    int Read_SFR();
    int Program_MCU(int m_iMode1, int m_iMode0);
    int Program_MCU(const std::array<std::byte, MCU_PROGRAM_SIZE>& buffer, const MCU_PROG_MODE mode);
    void Reset_MCU();
    void RunTest_MCU(int m_iMode1, int m_iMode0, unsigned short test_code, int m_iDebug);
    int RunProductionTest_MCU();
    void RunFabTest_MCU(int m_iMode1, int m_iMode0, int m_iDebug);
    // debug mode functions
    void DebugModeSet_MCU(int m_iMode1, int m_iMode0);
    void DebugModeExit_MCU(int m_iMode1, int m_iMode0);
    int ResetPC_MCU();
    int RunInstr_MCU(unsigned short* pPCVAL);
    void Initialize(std::shared_ptr<ISPI> pSerPort, unsigned rom_size = 0);
    ProgrammingCallback callback;

  protected:
    std::string mLoadedProgramFilename;
    std::atomic_ushort stepsDone;
    std::atomic_ushort stepsTotal;
    std::atomic_bool aborted;
    int WaitUntilWritten();
    int ReadOneByte(std::byte& data);
    int One_byte_command(unsigned short data1, std::byte& rdata1);
    unsigned int formREG2command(int m_iExt5, int m_iExt4, int m_iExt3, int m_iExt2, int m_iMode1, int m_iMode0);
    std::shared_ptr<ISPI> m_serPort;
    int m_bLoadedDebug;
    int m_bLoadedProd;
    int byte_array_size;
};
} // namespace lime
#endif // MCU_BD_H
