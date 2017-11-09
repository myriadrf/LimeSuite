/**
    @file Connection_uLimeSDRing.cpp
    @author Lime Microsystems
    @brief Implementation of uLimeSDR board connection (streaming API)
*/

#include "Connection_uLimeSDR.h"
#include "fifo.h"
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>
#include <ciso646>
#include <vector>
#include <FPGA_common.h>
#include "ErrorReporting.h"
#include "Logger.h"

using namespace lime;
using namespace std;

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int Connection_uLimeSDR::UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate, const double txPhase, const double rxPhase)
{
    const float txInterfaceClk = 2 * txRate;
    const float rxInterfaceClk = 2 * rxRate;
    int status = 0;

    mExpectedSampleRate = rxRate;

    lime::fpga::FPGA_PLL_clock clocks[4];

    clocks[0].bypass = false;
    clocks[0].index = 0;
    clocks[0].outFrequency = txInterfaceClk;
    clocks[0].phaseShift_deg = 0;
    clocks[0].findPhase = false;
    clocks[1].bypass = false;
    clocks[1].index = 1;
    clocks[1].outFrequency = txInterfaceClk;
    clocks[1].findPhase = false;
    clocks[1].phaseShift_deg = txPhase;
    clocks[2].bypass = false;
    clocks[2].index = 2;
    clocks[2].outFrequency = rxInterfaceClk;
    clocks[2].phaseShift_deg = 0;
    clocks[2].findPhase = false;
    clocks[3].bypass = false;
    clocks[3].index = 3;
    clocks[3].outFrequency = rxInterfaceClk;
    clocks[3].findPhase = false;
    clocks[3].phaseShift_deg = rxPhase;

    status = lime::fpga::SetPllFrequency(this, 0, rxInterfaceClk, clocks, 4);

    return status;
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int Connection_uLimeSDR::UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz)
{
    const float txInterfaceClk = 2 * txRate_Hz;
    const float rxInterfaceClk = 2 * rxRate_Hz;
    int status = 0;
    uint32_t reg20;
    const double rxPhC1[] = { 91.08, 89.46 };
    const double rxPhC2[] = { -1 / 6e6, 1.24e-6 };
    const double txPhC1[] = { 89.75, 89.61 };
    const double txPhC2[] = { -3.0e-7, 2.71e-7 };

    const std::vector<uint32_t> spiAddr = { 0x0021, 0x0022, 0x0023, 0x0024,
        0x0027, 0x002A, 0x0400, 0x040C,
        0x040B, 0x0400, 0x040B, 0x0400 };
    const int bakRegCnt = spiAddr.size() - 4;
    auto info = GetDeviceInfo();
    const int addrLMS7002M = info.addrsLMS7002M.at(0);
    bool phaseSearch = false;
    //if (this->chipVersion == 0x3841) //0x3840 LMS7002Mr2, 0x3841 LMS7002Mr3
    /*if (rxInterfaceClk >= 5e6 || txInterfaceClk >= 5e6)
        phaseSearch = true;*/
    mExpectedSampleRate = rxRate_Hz;
    std::vector<uint32_t> dataWr;
    std::vector<uint32_t> dataRd;

    if (phaseSearch)
    {
        dataWr.resize(spiAddr.size());
        dataRd.resize(spiAddr.size());
        //backup registers
        dataWr[0] = (uint32_t(0x0020) << 16);
        TransactSPI(addrLMS7002M, dataWr.data(), &reg20, 1);

        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
        TransactSPI(addrLMS7002M, dataWr.data(), nullptr, 1);

        for (int i = 0; i < bakRegCnt; ++i)
            dataWr[i] = (spiAddr[i] << 16);
        TransactSPI(addrLMS7002M, dataWr.data(), dataRd.data(), bakRegCnt);
    }

    if ((txInterfaceClk >= 5e6) && (rxInterfaceClk >= 5e6))
    {
        lime::fpga::FPGA_PLL_clock clocks[4];

        clocks[0].bypass = false;
        clocks[0].index = 0;
        clocks[0].outFrequency = txInterfaceClk;
        clocks[0].phaseShift_deg = 0;
        clocks[0].findPhase = false;
        clocks[1].bypass = false;
        clocks[1].index = 1;
        clocks[1].outFrequency = txInterfaceClk;
        clocks[1].findPhase = false;
        if (this->chipVersion == 0x3841)
            clocks[1].phaseShift_deg = txPhC1[1] + txPhC2[1] * txInterfaceClk;
        else
            clocks[1].phaseShift_deg = txPhC1[0] + txPhC2[0] * txInterfaceClk;
        clocks[2].bypass = false;
        clocks[2].index = 2;
        clocks[2].outFrequency = rxInterfaceClk;
        clocks[2].phaseShift_deg = 0;
        clocks[2].findPhase = false;
        clocks[3].bypass = false;
        clocks[3].index = 3;
        clocks[3].outFrequency = rxInterfaceClk;
        clocks[3].findPhase = false;
        if (this->chipVersion == 0x3841)
            clocks[3].phaseShift_deg = rxPhC1[1] + rxPhC2[1] * rxInterfaceClk;
        else
            clocks[3].phaseShift_deg = rxPhC1[0] + rxPhC2[0] * rxInterfaceClk;

        if (phaseSearch)
        {
            {
                clocks[3].findPhase = true;
                const std::vector<uint32_t> spiData = { 0x0E9F, 0x07FF, 0x5550, 0xE4E4,
                    0xE4E4, 0x0086, 0x028D, 0x00FF, 0x5555, 0x02CD, 0xAAAA, 0x02ED };
                //Load test config
                const int setRegCnt = spiData.size();
                for (int i = 0; i < setRegCnt; ++i)
                    dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
                TransactSPI(addrLMS7002M, dataWr.data(), nullptr, setRegCnt);
                status = lime::fpga::SetPllFrequency(this, 0, rxInterfaceClk, clocks, 4);
            }
            {
                clocks[3].findPhase = false;
                const std::vector<uint32_t> spiData = { 0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0484 };
                WriteRegister(0x000A, 0x0000);
                //Load test config
                const int setRegCnt = spiData.size();
                for (int i = 0; i < setRegCnt; ++i)
                    dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
                TransactSPI(addrLMS7002M, dataWr.data(), nullptr, setRegCnt);
                clocks[1].findPhase = true;
                WriteRegister(0x000A, 0x0200);

            }
        }
        status = lime::fpga::SetPllFrequency(this, 0, rxInterfaceClk, clocks, 4);
    }
    else
    {
        status = lime::fpga::SetDirectClocking(this, 0, rxInterfaceClk, 90);
        if (status == 0)
            status = lime::fpga::SetDirectClocking(this, 1, rxInterfaceClk, 90);
    }

    if (phaseSearch)
    {
        //Restore registers
        for (int i = 0; i < bakRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRd[i]; //msbit 1=SPI write
        TransactSPI(addrLMS7002M, dataWr.data(), nullptr, bakRegCnt);
        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20; //msbit 1=SPI write
        TransactSPI(addrLMS7002M, dataWr.data(), nullptr, 1);
        WriteRegister(0x000A, 0);
    }
    return status;
}


int Connection_uLimeSDR::ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms)
{
    int totalBytesReceived = 0;
    fpga::StopStreaming(this);

    //ResetStreamBuffers();
    WriteRegister(0x0008, 0x0100 | 0x2);
    WriteRegister(0x0007, 1);

    fpga::StartStreaming(this);

    int handle = BeginDataReading(buffer, length, 0);
    if (WaitForReading(handle, timeout_ms))
        totalBytesReceived = FinishDataReading(buffer, length, handle);

    AbortReading(0);
    fpga::StopStreaming(this);

    return totalBytesReceived;
}

int Connection_uLimeSDR::ResetStreamBuffers()
{
    rxSize = 0;
    txSize = 0;
#ifndef __unix__
    if (FT_AbortPipe(mFTHandle, mStreamRdEndPtAddr)!=FT_OK)
        return -1;
    if (FT_AbortPipe(mFTHandle, mStreamWrEndPtAddr)!=FT_OK)
        return -1;
    if (FT_FlushPipe(mFTHandle, mStreamRdEndPtAddr)!=FT_OK)
        return -1;
#else
    return FT_FlushPipe(mStreamRdEndPtAddr);
#endif
    return 0;
}