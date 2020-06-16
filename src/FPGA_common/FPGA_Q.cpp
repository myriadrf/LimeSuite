#include "FPGA_Q.h"
#include "IConnection.h"
#include "Logger.h"
#include "LMS64CProtocol.h"
#include <ciso646>
#include <vector>
#include <map>
#include <math.h>

namespace lime
{

FPGA_Q::FPGA_Q() : FPGA(){}
/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int FPGA_Q::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, double txPhase, double rxPhase, int channel)
{
    lime::FPGA::FPGA_PLL_clock clocks[3];

    if (channel == 2)
    {
        clocks[0].index = 0;
        clocks[0].outFrequency = rxRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = txRate_Hz;
        return SetPllFrequency(4, 30.72e6, clocks, 2);
    }

    const int pll_ind = (channel == 1) ? 2 : 0;
    if ((txRate_Hz >= 5e6) && (rxRate_Hz >= 5e6))
    {
        clocks[0].index = 0;
        clocks[0].outFrequency = rxRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = rxRate_Hz;
        clocks[1].phaseShift_deg = rxPhase;
        if (SetPllFrequency(pll_ind+1, rxRate_Hz, clocks, 2)!=0)
            return -1;

        clocks[0].index = 0;
        clocks[0].outFrequency = txRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = txRate_Hz;
        clocks[1].phaseShift_deg = txPhase;
        clocks[2].index = 2;
        clocks[2].outFrequency = 2*txRate_Hz;
        clocks[2].bypass = false;
        if (SetPllFrequency(pll_ind, txRate_Hz, clocks, 3)!=0)
            return -1;
    }
    else
    {
        if(SetDirectClocking(pll_ind) != 0)
            return -1;
        if(SetDirectClocking(pll_ind + 1) != 0)
            return -1;
    }

    return 0;
}

int FPGA_Q::WriteRegisters(const uint32_t *addrs, const uint32_t *data, unsigned cnt)
{
    return connection->WriteRegisters(addrs, data, cnt);
}
int FPGA_Q::ReadRegisters(const uint32_t *addrs, uint32_t *data, unsigned cnt)
{
    return connection->ReadRegisters(addrs, data, cnt);
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int FPGA_Q::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, int channel)
{
    const int pll_ind = (channel == 1) ? 2 : 0;
    int status = 0;
    uint32_t reg20;
    const double rxPhC1 =  89.46;
    const double rxPhC2 =  1.24e-6;
    const double txPhC1 =  89.61;
    const double txPhC2 =  2.71e-7;

    const std::vector<uint32_t> spiAddr = { 0x021, 0x022, 0x023, 0x024, 0x027, 0x02A, 0x82,
                                            0x400, 0x40C, 0x40B, 0x400, 0x40B, 0x400};
    const int bakRegCnt = spiAddr.size() - 4;

    bool phaseSearch = false;
    //if (!(mStreamers.size() > channel && (mStreamers[channel]->rxRunning || mStreamers[channel]->txRunning)))
    //if(rxRate_Hz >= 5e6 && txRate_Hz >= 5e6)
    {
        uint32_t addr[3] = {0, 1, 2};
        uint32_t vals[3];
        ReadRegisters(addr,vals,3);
        vals[1] = (vals[1]<<8)|vals[2];
        if ((vals[0]==0xE && vals[1]>0x20E)||(vals[0]==0xF && vals[1]>0x206)||(vals[0]==0x10 && vals[1]>0x102)||vals[0]==0x17)
            phaseSearch = true;
    }
    
    if (!phaseSearch)
        return SetInterfaceFreq(txRate_Hz, rxRate_Hz, txPhC1 + txPhC2 * txRate_Hz, rxPhC1 + rxPhC2 * rxRate_Hz, channel);

    if(rxRate_Hz < 5e6 && txRate_Hz < 5e6)
    {
        rxRate_Hz = 5e6;
        txRate_Hz = 5e6;
    }
    std::vector<uint32_t> dataRdA;
    std::vector<uint32_t> dataRdB;
    std::vector<uint32_t> dataWr;

    dataWr.resize(spiAddr.size());
    dataRdA.resize(bakRegCnt);
    dataRdB.clear();
    //backup registers
    dataWr[0] = (uint32_t(0x0020) << 16);
    connection->ReadLMS7002MSPI(dataWr.data(), &reg20, 1, channel);

    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);

    for (int i = 0; i < bakRegCnt; ++i)
        dataWr[i] = (spiAddr[i] << 16);
    connection->ReadLMS7002MSPI(dataWr.data(),dataRdA.data(), bakRegCnt, channel);


    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFE; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);

    for (int i = 0; i < bakRegCnt; ++i)
        if (spiAddr[i] >= 0x100)
            dataRdB.push_back(spiAddr[i] << 16);
    connection->ReadLMS7002MSPI(dataRdB.data(), dataRdB.data(), dataRdB.size(), channel);

    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFF; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);

    {
        std::vector<uint32_t> spiData = { 0x0E9F, 0x0FFF, 0x5550, 0xE4E4, 0xE4E4, 0x0086, 0x8001,
                                          0x028D, 0x00FF, 0x5555, 0x02CD, 0xAAAA, 0x02ED};
        //Load test config
        const int setRegCnt = spiData.size();
        for (int i = 0; i < setRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
        connection->WriteLMS7002MSPI(dataWr.data(), setRegCnt, channel);
    }

    lime::FPGA::FPGA_PLL_clock clocks[3];
    clocks[0].index = 1;
    clocks[0].outFrequency = rxRate_Hz;
    clocks[0].phaseShift_deg = rxPhC1 + rxPhC2 * rxRate_Hz;
    clocks[0].findPhase = true;
    clocks[1] = clocks[0];
    clocks[1].index = 0;
    if (SetPllFrequency(pll_ind+1, rxRate_Hz, clocks, 2)!=0)
    {
        status = -1;
        clocks[0].index = 0;
        clocks[0].phaseShift_deg = 0;
        clocks[0].findPhase = false;
        clocks[1].findPhase = false;
        SetPllFrequency(pll_ind+1, rxRate_Hz, clocks, 2);
    }

    {
        std::vector<uint32_t> spiData = {0x0E9F, 0x0FFF, 0x5550, 0xE4E4, 0xE4E4, 0x0484, 0x8001};
        WriteRegister(0xFFFF, 1 << channel);
        WriteRegister(0x000A, 0x0000);
        //Load test config
        const int setRegCnt = spiData.size();
        for (int i = 0; i < setRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
        connection->WriteLMS7002MSPI(dataWr.data(), setRegCnt, channel);
    }


    clocks[0].index = 2;
    clocks[0].phaseShift_deg = 0.0f;
    clocks[0].findPhase = false;
    clocks[0].outFrequency = 2*txRate_Hz;
    clocks[0].bypass = false;

    clocks[1].index = 0;
    clocks[1].outFrequency = txRate_Hz;
    clocks[1].phaseShift_deg = 0.0f;
    clocks[1].findPhase = false;
    clocks[1].bypass = false;

    clocks[2].index = 1;
    clocks[2].outFrequency = txRate_Hz;
    clocks[2].phaseShift_deg = txPhC1 + txPhC2 * txRate_Hz;
    clocks[2].findPhase = true;
    clocks[2].bypass = false;
    WriteRegister(0x000A, 0x0200);
    if (SetPllFrequency(pll_ind, txRate_Hz, &clocks[0], 3)!=0)
    {
        status = -1;
        /*clocks[1].index = 0;
        clocks[1].phaseShift_deg = 0;
        clocks[1].findPhase = false;
        clocks[2].findPhase = false;
        SetPllFrequency(pll_ind, txRate_Hz, &clocks[0], 3);*/
    }

    //Restore registers
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);
    for (int i = 0; i < bakRegCnt; ++i)
        dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRdA[i]; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), bakRegCnt, channel);
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFE; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);

    int k = 0;
    for (int i = 0; i < bakRegCnt; ++i)
        if (spiAddr[i] >= 0x100){
            dataWr[k] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRdB[k]; //msbit 1=SPI write
            k++;
        }
    connection->WriteLMS7002MSPI(dataWr.data(), k, channel);
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20; //msbit 1=SPI write
    connection->WriteLMS7002MSPI(dataWr.data(), 1, channel);
    WriteRegister(0x000A, 0);

    return status;
}



} //namespace lime
