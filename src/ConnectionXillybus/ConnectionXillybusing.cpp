/**
    @file ConnectionXillybusing.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection (streaming API)
*/

#include "ConnectionXillybus.h"
#include "fifo.h"
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>
#include <ciso646>
#include <FPGA_common.h>
#include <ErrorReporting.h>
#include "Logger.h"

using namespace std;
using namespace lime;

int ConnectionXillybus::UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz, const double txPhase, const double rxPhase)
{
    lime::fpga::FPGA_PLL_clock clocks[2];

    if (channel == 2)
    {
        clocks[0].index = 0;
        clocks[0].outFrequency = rxRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = txRate_Hz;
        return lime::fpga::SetPllFrequency(this, 4, 30.72e6, clocks, 2);
    }

    const float txInterfaceClk = 2 * txRate_Hz;
    const float rxInterfaceClk = 2 * rxRate_Hz;
    mExpectedSampleRate = rxRate_Hz;
    const int pll_ind = (channel == 1) ? 2 : 0;

    clocks[0].index = 0;
    clocks[0].outFrequency = rxInterfaceClk;
    clocks[1].index = 1;
    clocks[1].outFrequency = rxInterfaceClk;
    clocks[1].phaseShift_deg = rxPhase;
    if (lime::fpga::SetPllFrequency(this, pll_ind+1, rxInterfaceClk, clocks, 2)!=0)
        return -1;

    clocks[0].index = 0;
    clocks[0].outFrequency = txInterfaceClk;
    clocks[1].index = 1;
    clocks[1].outFrequency = txInterfaceClk;
    clocks[1].phaseShift_deg = txPhase;
    if (lime::fpga::SetPllFrequency(this, pll_ind, txInterfaceClk, clocks, 2)!=0)
        return -1;

    return 0;
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int ConnectionXillybus::UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz)
{
    const float txInterfaceClk = 2 * txRate_Hz;
    const float rxInterfaceClk = 2 * rxRate_Hz;
    const int pll_ind = (channel == 1) ? 2 : 0;
    int status = 0;
    uint32_t reg20;
    const double rxPhC1[] = { 91.08, 89.46 };
    const double rxPhC2[] = { -1 / 6e6, 1.24e-6 };
    const double txPhC1[] = { 89.75, 89.61 };
    const double txPhC2[] = { -3.0e-7, 2.71e-7 };

    const std::vector<uint32_t> spiAddr = { 0x021, 0x022, 0x023, 0x024, 0x027, 0x02A,
                                            0x400, 0x40C, 0x40B, 0x400, 0x40B, 0x400};
    const int bakRegCnt = spiAddr.size() - 4;
    auto info = GetInfo();
    bool phaseSearch = false;
    if (!(mStreamers[channel] && (mStreamers[channel]->rxRunning || mStreamers[channel]->txRunning)))
        if (this->chipVersion == 0x3841 && info.device == LMS_DEV_LIMESDR_PCIE) //0x3840 LMS7002Mr2, 0x3841 LMS7002Mr3
            if(rxInterfaceClk >= 5e6 || txInterfaceClk >= 5e6)
                phaseSearch = true;
    mExpectedSampleRate = rxRate_Hz;

    std::vector<uint32_t> dataWr;
    std::vector<uint32_t> dataRd;

    if (phaseSearch)
    {
        dataWr.resize(spiAddr.size());
        dataRd.resize(spiAddr.size());
        //backup registers
        dataWr[0] = (uint32_t(0x0020) << 16);
        ReadLMS7002MSPI(dataWr.data(), &reg20, 1, channel);

        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
        WriteLMS7002MSPI(dataWr.data(), 1, channel);

        for (int i = 0; i < bakRegCnt; ++i)
            dataWr[i] = (spiAddr[i] << 16);
        ReadLMS7002MSPI(dataWr.data(),dataRd.data(), bakRegCnt, channel);
    }

    if(rxInterfaceClk >= 5e6 || info.hardware < 3)
    {
        if (phaseSearch)
        {
            const std::vector<uint32_t> spiData = { 0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0086,
                                                    0x028D, 0x00FF, 0x5555, 0x02CD, 0xAAAA, 0x02ED};
            //Load test config
            const int setRegCnt = spiData.size();
            for (int i = 0; i < setRegCnt; ++i)
                dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
            WriteLMS7002MSPI(dataWr.data(), setRegCnt, channel);
        }
        lime::fpga::FPGA_PLL_clock clocks[2];
        clocks[0].index = 0;
        clocks[0].outFrequency = rxInterfaceClk;
        clocks[1].index = 1;
        clocks[1].outFrequency = rxInterfaceClk;
        if (this->chipVersion == 0x3841)
            clocks[1].phaseShift_deg = rxPhC1[1] + rxPhC2[1] * rxInterfaceClk;
        else
            clocks[1].phaseShift_deg = rxPhC1[0] + rxPhC2[0] * rxInterfaceClk;

        if (phaseSearch)
            clocks[1].findPhase = true;
        status = lime::fpga::SetPllFrequency(this, pll_ind+1, rxInterfaceClk, clocks, 2);
    }
    else
        status = lime::fpga::SetDirectClocking(this, pll_ind+1, rxInterfaceClk, 90);

    if(txInterfaceClk >= 5e6 || info.hardware < 3)
    {
        if (phaseSearch)
        {
            const std::vector<uint32_t> spiData = {0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0484};
            WriteRegister(0x000A, 0x0000);
            //Load test config
            const int setRegCnt = spiData.size();
            for (int i = 0; i < setRegCnt; ++i)
                dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
            WriteLMS7002MSPI(dataWr.data(), setRegCnt, channel);
        }

        lime::fpga::FPGA_PLL_clock clocks[2];
        clocks[0].index = 0;
        clocks[0].outFrequency = txInterfaceClk;
        clocks[1].index = 1;
        clocks[1].outFrequency = txInterfaceClk;
        if (this->chipVersion == 0x3841)
            clocks[1].phaseShift_deg = txPhC1[1] + txPhC2[1] * txInterfaceClk;
        else
            clocks[1].phaseShift_deg = txPhC1[0] + txPhC2[0] * txInterfaceClk;

        if (phaseSearch)
        {
            clocks[1].findPhase = true;
            WriteRegister(0x000A, 0x0200);
        }
        status = lime::fpga::SetPllFrequency(this, pll_ind, txInterfaceClk, clocks, 2);
    }
    else
        status = lime::fpga::SetDirectClocking(this, pll_ind, txInterfaceClk, 90);

    if (phaseSearch)
    {
        //Restore registers
        for (int i = 0; i < bakRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRd[i]; //msbit 1=SPI write
        WriteLMS7002MSPI(dataWr.data(), bakRegCnt, channel);
        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20; //msbit 1=SPI write
        WriteLMS7002MSPI(dataWr.data(), 1, channel);
        WriteRegister(0x000A, 0);
    }

    return status;
}

int ConnectionXillybus::ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms)
{
    WriteRegister(0xFFFF, 1 << epIndex);
    fpga::StopStreaming(this);
    ResetStreamBuffers();
    WriteRegister(0x0008, 0x0100 | 0x2);
    WriteRegister(0x0007, 1);
    fpga::StartStreaming(this);
    int totalBytesReceived = ReceiveData(buffer, length, epIndex, timeout_ms);
    fpga::StopStreaming(this);
    AbortReading(epIndex);
    return totalBytesReceived;
}