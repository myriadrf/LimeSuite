#include "FPGA_Mini.h"
#include "Logger.h"
#include <ciso646>
#include <vector>
#include <map>
#include <math.h>
#include <iostream>

#include "Register.h"

namespace lime {

FPGA_Mini::FPGA_Mini(std::shared_ptr<ISPI> fpgaSPI, std::shared_ptr<ISPI> lms7002mSPI)
    : FPGA(fpgaSPI, lms7002mSPI)
{
}

OpStatus FPGA_Mini::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, double txPhase, double rxPhase, int channel)
{
    OpStatus status = OpStatus::SUCCESS;

    std::vector<FPGA_PLL_clock> clocks(4);
    if ((txRate_Hz >= 5e6) && (rxRate_Hz >= 5e6))
    {
        clocks[0].bypass = false;
        clocks[0].index = 0;
        clocks[0].outFrequency = txRate_Hz;
        clocks[0].phaseShift_deg = 0;
        clocks[0].findPhase = false;
        clocks[1].bypass = false;
        clocks[1].index = 1;
        clocks[1].outFrequency = txRate_Hz;
        clocks[1].findPhase = false;
        clocks[1].phaseShift_deg = txPhase;
        clocks[2].bypass = false;
        clocks[2].index = 2;
        clocks[2].outFrequency = rxRate_Hz;
        clocks[2].phaseShift_deg = 0;
        clocks[2].findPhase = false;
        clocks[3].bypass = false;
        clocks[3].index = 3;
        clocks[3].outFrequency = rxRate_Hz;
        clocks[3].findPhase = false;
        clocks[3].phaseShift_deg = rxPhase;

        status = SetPllFrequency(0, rxRate_Hz, clocks);
    }
    else
    {
        status = SetDirectClocking(0);

        if (status == OpStatus::SUCCESS)
        {
            status = SetDirectClocking(1);
        }
    }

    return status;
}

OpStatus FPGA_Mini::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, int channel)
{
    uint32_t reg20;
    const double rxPhC1 = 89.46;
    const double rxPhC2 = 1.24e-6;
    const double txPhC1 = 89.61;
    const double txPhC2 = 2.71e-7;

    const std::vector<uint32_t> spiAddr = {
        0x0021, 0x0022, 0x0023, 0x0024, 0x0027, 0x002A, 0x0400, 0x040C, 0x040B, 0x0400, 0x040B, 0x0400
    };
    const int bakRegCnt = spiAddr.size() - 4;

    bool phaseSearch = false;
    if (rxRate_Hz >= 5e6 && txRate_Hz >= 5e6)
    {
        phaseSearch = true;
    }

    if (!phaseSearch)
    {
        return SetInterfaceFreq(txRate_Hz, rxRate_Hz, txPhC1 + txPhC2 * txRate_Hz, rxPhC1 + rxPhC2 * rxRate_Hz, 0);
    }

    std::vector<uint32_t> dataRd;
    std::vector<uint32_t> dataWr;
    dataWr.resize(spiAddr.size());
    dataRd.resize(spiAddr.size());

    //backup registers
    dataWr[0] = 0x0020;
    lms7002mPort->SPI(dataWr.data(), &reg20, 1);
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
    lms7002mPort->SPI(dataWr.data(), nullptr, 1);
    lms7002mPort->SPI(spiAddr.data(), dataRd.data(), bakRegCnt);

    { //Config Rx
        const std::vector<uint32_t> spiData = {
            0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0086, 0x028D, 0x00FF, 0x5555, 0x02CD, 0xAAAA, 0x02ED
        };
        const int setRegCnt = spiData.size();

        for (int i = 0; i < setRegCnt; ++i)
        {
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
        }

        lms7002mPort->SPI(dataWr.data(), nullptr, setRegCnt);
    }

    bool rxPhaseSearchSuccess = false;
    bool txPhaseSearchSuccess = false;
    std::vector<FPGA_PLL_clock> clocks(4);

    const int pllRetryConfigCount = 2;
    for (int i = 0; i < pllRetryConfigCount; i++) // attempt multiple times
    {
        clocks[0].index = 3;
        clocks[0].outFrequency = rxRate_Hz;
        clocks[0].phaseShift_deg = rxPhC1 + rxPhC2 * rxRate_Hz;
        clocks[0].findPhase = true;
        clocks[1] = clocks[0];
        clocks[2] = clocks[0];
        clocks[3] = clocks[0];
        if (SetPllFrequency(0, rxRate_Hz, clocks) == OpStatus::SUCCESS)
        {
            rxPhaseSearchSuccess = true;
            break;
        }
    }

    if (rxPhaseSearchSuccess)
    {
        //Config TX
        const std::vector<uint32_t> spiData = { 0x0E9F, 0x07FF, 0x5550, 0xE4E4, 0xE4E4, 0x0484 };
        WriteRegister(0x000A, 0x0000);
        const int setRegCnt = spiData.size();

        for (int i = 0; i < setRegCnt; ++i)
        {
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
        }

        lms7002mPort->SPI(dataWr.data(), nullptr, setRegCnt);

        for (int i = 0; i < pllRetryConfigCount; i++)
        {
            clocks[0].index = 1;
            clocks[0].outFrequency = txRate_Hz;
            clocks[0].phaseShift_deg = txPhC1 + txPhC2 * txRate_Hz;
            clocks[0].findPhase = true;
            clocks[1] = clocks[0];
            clocks[2] = clocks[0];
            clocks[3] = clocks[0];
            WriteRegister(0x000A, 0x0200);
            if (SetPllFrequency(0, txRate_Hz, clocks) == OpStatus::SUCCESS)
            {
                txPhaseSearchSuccess = true;
                break;
            }
        }

        if (!txPhaseSearchSuccess)
        {
            lime::error("LML TX phase search FAIL");
        }
    }
    else
    {
        lime::error("LML RX phase search FAIL");
    }

    //Restore registers
    for (int i = 0; i < bakRegCnt; ++i)
    {
        dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRd[i]; //msbit 1=SPI write
    }

    lms7002mPort->SPI(dataWr.data(), nullptr, bakRegCnt);

    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20; //msbit 1=SPI write
    lms7002mPort->SPI(dataWr.data(), nullptr, 1);

    WriteRegister(0x000A, 0);

    if (!rxPhaseSearchSuccess || !txPhaseSearchSuccess)
        return SetInterfaceFreq(txRate_Hz, rxRate_Hz, txPhC1 + txPhC2 * txRate_Hz, rxPhC1 + rxPhC2 * rxRate_Hz, 0);
    return OpStatus::SUCCESS;
}

} //namespace lime
