/**
@file	MCU_BD.h
@author	Lime Microsystems
@brief	Header for MCU_BD.cpp
*/

#ifndef MCU_BD_H
#define MCU_BD_H

#include <atomic>

class LMScomms;

class MCU_BD
{
    public:
        //GCC 4.8 fails to use not aligned and bigger than 8 bytes struct for atomic
        struct alignas(8) ProgressInfo
        {
            unsigned short stepsDone;
            unsigned short stepsTotal;
            bool aborted;
        };
        ProgressInfo GetProgressInfo() const;

        MCU_BD();
        virtual ~MCU_BD();
        int m_iLoopTries;

    protected:
        std::atomic<ProgressInfo> mProgressInfo;
        void Log(const char* msg);
        int WaitUntilWritten();
        int ReadOneByte(unsigned char * data);
        int One_byte_command(unsigned short data1, unsigned char * rdata1);
        LMScomms * m_serPort;

    public:
        void Wait_CLK_Cycles(int data);
        // The IRAM content
        unsigned char m_IRAM[256];
        // The SFR content
        unsigned char m_SFR[256];
        // The program memory code
        unsigned char byte_array[8192];

        void mSPI_write(unsigned short addr_reg,unsigned short data_reg);
        unsigned short mSPI_read(unsigned short addr_reg);
        int Three_byte_command(unsigned char data1,unsigned char data2,unsigned char data3,
                   unsigned char * rdata1,unsigned char * rdata2,unsigned char * rdata3);
        void GetProgramCode(const char *inFileName, bool bin = false);
        int Change_MCUFrequency(unsigned char data);
        int Read_IRAM();
        int Erase_IRAM();
        int Read_SFR();
        int Program_MCU(int m_iMode1, int m_iMode0);
        void Reset_MCU();
        void RunTest_MCU(int m_iMode1, int m_iMode0, unsigned short test_code, int m_iDebug);
        void RunFabTest_MCU(int m_iMode1, int m_iMode0, int m_iDebug);
        // debug mode functions
        void DebugModeSet_MCU(int m_iMode1, int m_iMode0);
        void DebugModeExit_MCU(int m_iMode1, int m_iMode0);
        int ResetPC_MCU();
        int RunInstr_MCU(unsigned short * pPCVAL);
        void Initialize(LMScomms* pSerPort);
};

#endif // MCU_BD_H
