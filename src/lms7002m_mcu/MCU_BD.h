/**
@file	MCU_BD.h
@author	Lime Microsystems
@brief	Header for MCU_BD.cpp
*/

#ifndef MCU_BD_H
#define MCU_BD_H

#include <atomic>
#include <string>
#include "IConnection.h"

namespace lime{

class LMS64CProtocol;
class IConnection;

class MCU_BD
{
    public:
        enum OperationStatus
        {
            SUCCESS = 0,
            FAILURE,
            TIMEOUT,
        };

        struct ProgressInfo
        {
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
        enum MCU_Parameter
        {
            MCU_REF_CLK,
            MCU_BW,
        };
        void SetParameter(MCU_Parameter param, float value);
        int WaitForMCU(uint32_t timeout_ms);

        static const int cMaxFWSize = 1024 * 16;

    protected:
        std::string mLoadedProgramFilename;
        std::atomic_ushort stepsDone;
        std::atomic_ushort stepsTotal;
        std::atomic_bool aborted;
        int WaitUntilWritten();
        int ReadOneByte(unsigned char * data);
        int One_byte_command(unsigned short data1, unsigned char * rdata1);
        unsigned int formREG2command(int m_iExt5, int m_iExt4, int m_iExt3, int m_iExt2, int m_iMode1, int m_iMode0);
        IConnection* m_serPort;
        int m_bLoadedDebug;
        int m_bLoadedProd;
        int byte_array_size;
        unsigned mChipID;

    public:
        uint8_t ReadMCUProgramID();
        OperationStatus SetDebugMode(bool enabled, IConnection::MCU_PROG_MODE mode);
        OperationStatus readIRAM(const uint8_t *addr, uint8_t* values, const uint8_t count);
        OperationStatus writeIRAM(const uint8_t *addr, const uint8_t* values, const uint8_t count);

        void Wait_CLK_Cycles(int data);
        // The IRAM content
        unsigned char m_IRAM[256];
        // The SFR content
        unsigned char m_SFR[256];
        // The program memory code
        unsigned char byte_array[cMaxFWSize];

        void mSPI_write(unsigned short addr_reg,unsigned short data_reg);
        unsigned short mSPI_read(unsigned short addr_reg);
        int Three_byte_command(unsigned char data1,unsigned char data2,unsigned char data3,
        unsigned char * rdata1,unsigned char * rdata2,unsigned char * rdata3);
        int GetProgramCode(const char *inFileName, bool bin = false);
        int Change_MCUFrequency(unsigned char data);
        int Read_IRAM();
        int Erase_IRAM();
        int Read_SFR();
        int Program_MCU(int m_iMode1, int m_iMode0);
        int Program_MCU(const uint8_t* binArray, const IConnection::MCU_PROG_MODE mode);
        void Reset_MCU();
        void RunTest_MCU(int m_iMode1, int m_iMode0, unsigned short test_code, int m_iDebug);
        int RunProductionTest_MCU();
        void RunFabTest_MCU(int m_iMode1, int m_iMode0, int m_iDebug);
        // debug mode functions
        void DebugModeSet_MCU(int m_iMode1, int m_iMode0);
        void DebugModeExit_MCU(int m_iMode1, int m_iMode0);
        int ResetPC_MCU();
        int RunInstr_MCU(unsigned short * pPCVAL);
        void Initialize(IConnection* pSerPort, unsigned chipID = 0, unsigned rom_size = 0);
        lime::IConnection::ProgrammingCallback callback;
};
}
#endif // MCU_BD_H
