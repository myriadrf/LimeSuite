#include "FPGA_common.h"
#include "IConnection.h"
#include "ErrorReporting.h"
#include "LMS64CProtocol.h"
#include <ciso646>
#include <vector>
#include <map>
#include <math.h>
using namespace std;

#ifndef NDEBUG
    #define LMS_VERBOSE_OUTPUT
#endif

namespace lime
{
namespace fpga
{

// 0x000A
const int RX_EN = 1; //controls both receiver and transmitter
const int TX_EN = 1 << 1; //used for wfm playback from fpga
const int STREAM_LOAD = 1 << 2;

// 0x0009
const int SMPL_NR_CLR = 1; // rising edge clears
const int TXPCT_LOSS_CLR = 1 << 1; // 0 - normal operation, 1-clear

int StartStreaming(IConnection* serPort)
{
    uint16_t interface_ctrl_000A;
    int status = serPort->ReadRegister(0x000A, interface_ctrl_000A);
    if(status != 0)
        return status;
    status = serPort->WriteRegister(0x000A, interface_ctrl_000A | RX_EN);
    return status;
}

int StopStreaming(IConnection* serPort)
{
    uint16_t interface_ctrl_000A;
    int status = serPort->ReadRegister(0x000A, interface_ctrl_000A);
    if(status != 0)
        return status;
    serPort->WriteRegister(0x000A, interface_ctrl_000A & ~(TX_EN | RX_EN));
    return status;
}

int ResetTimestamp(IConnection* serPort)
{
    int status;
#ifndef NDEBUG
    uint16_t interface_ctrl_000A;
    status = serPort->ReadRegister(0x000A, interface_ctrl_000A);
    if(status != 0)
        return 0;
    if(interface_ctrl_000A & RX_EN)
        return ReportError(EPERM, "Streaming must be stopped to reset timestamp");
#endif // NDEBUG
    //reset hardware timestamp to 0
    uint16_t interface_ctrl_0009;
    status = serPort->ReadRegister(0x0009, interface_ctrl_0009);
    if(status != 0)
        return 0;
    serPort->WriteRegister(0x0009, interface_ctrl_0009 & ~(TXPCT_LOSS_CLR | SMPL_NR_CLR));
    serPort->WriteRegister(0x0009, interface_ctrl_0009 | (TXPCT_LOSS_CLR | SMPL_NR_CLR));
    serPort->WriteRegister(0x0009, interface_ctrl_0009 & ~(TXPCT_LOSS_CLR | SMPL_NR_CLR));
    return status;
}

int InitializeStreaming(IConnection* serPort, const StreamConfig &config)
{
    int channelsCount = 2;
    StopStreaming(serPort);
    ResetTimestamp(serPort);

    //enable MIMO mode, 12 bit compressed values
    uint16_t smpl_width; // 0-16 bit, 1-14 bit, 2-12 bit
    if(config.linkFormat == StreamConfig::STREAM_12_BIT_IN_16)
        smpl_width = 0x0;
    else if(config.linkFormat == StreamConfig::STREAM_12_BIT_COMPRESSED)
        smpl_width = 0x2;
    else
        smpl_width = 0x2;
    serPort->WriteRegister(0x0008, 0x0100 | smpl_width);

    uint16_t channelEnables = 0;
    for(int i=0; i<config.channels.size(); ++i)
        channelEnables |= (1 << config.channels[i]);
    serPort->WriteRegister(0x0007, channelEnables);
    return 0;
}

/** @brief Configures board FPGA clocks
@param serPort communications port
@param pllIndex index of FPGA pll
@param clocks list of clocks to configure
@param clocksCount number of clocks to configure
@return 0-success, other-failure
*/
int SetPllFrequency(IConnection* serPort, uint8_t pllIndex, const double inputFreq, FPGA_PLL_clock* clocks, const uint8_t clockCount)
{
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    const auto timeout = chrono::seconds(3);
    const uint16_t busyAddr = 0x0021;

    if(not serPort)
        return ReportError(ENODEV, "ConfigureFPGA_PLL: connection port is NULL");
    if(not serPort->IsOpen())
        return ReportError(ENODEV, "ConfigureFPGA_PLL: configure FPGA PLL, device not connected");
    eLMS_DEV boardType = serPort->GetDeviceInfo().deviceName == GetDeviceName(LMS_DEV_QSPARK) ? LMS_DEV_QSPARK : LMS_DEV_UNKNOWN;

    if(pllIndex > 15)
        ReportError(ERANGE, "SetPllFrequency: PLL index(%i) out of range [0-15]", pllIndex);

    //check if all clocks are above 5MHz
    const double PLLlowerLimit = 5e6;
    if(inputFreq < PLLlowerLimit)
        return ReportError(ERANGE, "SetPllFrequency: input frequency must be >=%g MHz", PLLlowerLimit/1e6);
    for(int i=0; i<clockCount; ++i)
        if(clocks[i].outFrequency < PLLlowerLimit && not clocks[i].bypass)
            return ReportError(ERANGE, "SetPllFrequency: clock(%i) must be >=%g MHz", i, PLLlowerLimit/1e6);

    //disable direct clock source
    uint16_t drct_clk_ctrl_0005 = 0;
    serPort->ReadRegister(0x0005, drct_clk_ctrl_0005);
    serPort->WriteRegister(0x0005, drct_clk_ctrl_0005 & ~(1 << pllIndex));

    uint16_t reg23val = 0;
    if(serPort->ReadRegister(0x0003, reg23val) != 0)
        return ReportError(ENODEV, "SetPllFrequency: failed to read register");

    const uint16_t PLLCFG_START = 0x1;
    const uint16_t PHCFG_START = 0x2;
    const uint16_t PLLRST_START = 0x4;
    const uint16_t PHCFG_UPDN = 1 << 13;
    reg23val &= 0x1F << 3; //clear PLL index
    reg23val &= ~PLLCFG_START; //clear PLLCFG_START
    reg23val &= ~PHCFG_START; //clear PHCFG
    reg23val &= ~PLLRST_START; //clear PLL reset
    reg23val &= ~PHCFG_UPDN; //clear PHCFG_UpDn
    reg23val |= pllIndex << 3;

    uint16_t statusReg;
    bool done = false;
    uint8_t errorCode = 0;
    vector<uint32_t> addrs;
    vector<uint32_t> values;
    addrs.push_back(0x0023); values.push_back(reg23val); //PLL_IND
    addrs.push_back(0x0023); values.push_back(reg23val | PLLRST_START);
    serPort->WriteRegisters(addrs.data(), values.data(), values.size());
    addrs.clear(); values.clear();

    t1 = chrono::high_resolution_clock::now();
    if(boardType == LMS_DEV_QSPARK) do //wait for reset to activate
    {
        serPort->ReadRegister(busyAddr, statusReg);
        done = statusReg & 0x1;
        errorCode = (statusReg >> 7) & 0xFF;
        t2 = chrono::high_resolution_clock::now();
    } while(not done && errorCode == 0 && (t2-t1) < timeout);
    if(t2 - t1 > timeout)
        return ReportError(ENODEV, "SetPllFrequency: PLLRST timeout, busy bit is still 1");
    if(errorCode != 0)
        return ReportError(EBUSY, "SetPllFrequency: error resetting PLL");

    addrs.push_back(0x0023); values.push_back(reg23val & ~PLLRST_START);

    //configure FPGA PLLs
    const double vcoLimits_Hz[2] = { 600e6, 1300e6 };

    map< unsigned long, int> availableVCOs; //all available frequencies for VCO
    for(int i=0; i<clockCount; ++i)
    {
        unsigned long freq;
        freq = clocks[i].outFrequency*(int(vcoLimits_Hz[0]/clocks[i].outFrequency) + 1);
        while(freq >= vcoLimits_Hz[0] && freq <= vcoLimits_Hz[1])
        {
            //add all output frequency multiples that are in VCO interval
            availableVCOs.insert( pair<unsigned long, int>(freq, 0));
            freq += clocks[i].outFrequency;
        }
    }

    unsigned int bestScore = 0; //score shows how many outputs have integer dividers
    //calculate scores for all available frequencies
    unsigned long bestVCO = 0;
    for (auto &it : availableVCOs)
    {
        for(int i=0; i<clockCount; ++i)
        {
            if(clocks[i].outFrequency == 0 || clocks[i].bypass)
                continue;

            if( (int(it.first) % int(clocks[i].outFrequency)) == 0)
                it.second = it.second+1;
        }
        if(it.second > bestScore)
        {
            bestScore = it.second;
            bestVCO = it.first;
        }
    }
    int N, M;
    double bestDeviation = 1e9;
    double Fvco;
    for(auto it : availableVCOs)
    {
        if(it.second == bestScore)
        {
            float coef = (it.first / inputFreq);
            int Ntemp = 1;
            int Mtemp = int(coef + 0.5);
            while(inputFreq / (Ntemp + 1) > PLLlowerLimit)
            {
                ++Ntemp;
                Mtemp = int(coef*Ntemp + 0.5);
                if(Mtemp > 255)
                {
                    --Ntemp;
                    Mtemp = int(coef*Ntemp + 0.5);
                    break;
                }
            }
            double deviation = fabs(it.first - inputFreq*Mtemp / Ntemp);
            if(deviation < bestDeviation)
            {
                bestDeviation = deviation;
                Fvco = it.first;
                M = Mtemp;
                N = Ntemp;
            }
        }
    }

    int mlow = M / 2;
    int mhigh = mlow + M % 2;
    Fvco = inputFreq*M/N; //actual VCO freq
#ifdef LMS_VERBOSE_OUTPUT
    printf("----- FPGA PLL #%i CONFIG -----\n", pllIndex);
    printf("M=%i, N=%i, Fvco=%.3f MHz\n", M, N, Fvco / 1e6);
#endif
    if(Fvco < vcoLimits_Hz[0] || Fvco > vcoLimits_Hz[1])
        return ReportError(ERANGE, "SetPllFrequency: VCO(%g MHz) out of range [%g:%g] MHz", Fvco/1e6, vcoLimits_Hz[0]/1e6, vcoLimits_Hz[1]/1e6);

    uint16_t M_N_odd_byp = (M%2 << 3) | (N%2 << 1);
    if(M == 1)
        M_N_odd_byp |= 1 << 2; //bypass M
    if(N == 1)
        M_N_odd_byp |= 1; //bypass N
    addrs.push_back(0x0026); values.push_back(M_N_odd_byp);
    int nlow = N / 2;
    int nhigh = nlow + N % 2;
    addrs.push_back(0x002A); values.push_back(nhigh << 8 | nlow); //N_high_cnt, N_low_cnt
    addrs.push_back(0x002B); values.push_back(mhigh << 8 | mlow);

    uint16_t c7_c0_odds_byps = 0x5555; //bypass all C
    uint16_t c15_c8_odds_byps = 0x5555; //bypass all C

    //set outputs
    for(int i=0; i<clockCount; ++i)
    {
        int C = int(Fvco / clocks[i].outFrequency + 0.5);
        int clow = C / 2;
        int chigh = clow + C % 2;
        if(i < 8)
        {
            if(not clocks[i].bypass && C != 1)
                c7_c0_odds_byps &= ~(1 << (i*2)); //enable output
            c7_c0_odds_byps |= (C % 2) << (i*2+1); //odd bit
        }
        else
        {
            if(not clocks[i].bypass && C != 1)
                c15_c8_odds_byps &= ~(1 << ((i-8)*2)); //enable output
            c15_c8_odds_byps |= (C % 2) << ((i-8)*2+1); //odd bit
        }
        addrs.push_back(0x002E + i); values.push_back(chigh << 8 | clow);
        clocks[i].rd_actualFrequency = (inputFreq * M / N) / (chigh + clow);
    }
    addrs.push_back(0x0027); values.push_back(c7_c0_odds_byps);
    addrs.push_back(0x0028); values.push_back(c15_c8_odds_byps);
    addrs.push_back(0x0023); values.push_back(reg23val | PLLCFG_START);
    if(serPort->WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
        ReportError(EIO, "SetPllFrequency: PLL CFG, failed to write registers");
    addrs.clear(); values.clear();

    t1 = chrono::high_resolution_clock::now();
    if(boardType == LMS_DEV_QSPARK) do //wait for config to activate
    {
        serPort->ReadRegister(busyAddr, statusReg);
        done = statusReg & 0x1;
        errorCode = (statusReg >> 7) & 0xFF;
        t2 = chrono::high_resolution_clock::now();
    } while(not done && errorCode == 0 && (t2-t1) < timeout);
    if(t2 - t1 > timeout)
        return ReportError(ENODEV, "SetPllFrequency: PLLCFG timeout, busy bit is still 1");
    if(errorCode != 0)
        return ReportError(EBUSY, "SetPllFrequency: error configuring PLLCFG");

    addrs.push_back(0x0023); values.push_back(reg23val & ~PLLCFG_START);
    for(int i=0; i<clockCount; ++i)
    {
        int C = int(Fvco / clocks[i].outFrequency + 0.5);
        float fOut_MHz = inputFreq/1e6;
        float Fstep_us = 1 / (8 * fOut_MHz*C);
        float Fstep_deg = (360 * Fstep_us) / (1 / fOut_MHz);
        short nSteps = clocks[i].phaseShift_deg / Fstep_deg;
        addrs.push_back(0x0024); values.push_back(nSteps); //CNT_PHASE
        int cnt_ind = (clocks[i].index + 2) & 0x1F; //C0 index 2, C1 index 3...
	reg23val &= ~(0xF<<8);
        reg23val = reg23val | (cnt_ind << 8);
        if(clocks[i].phaseShift_deg >= 0)
            reg23val |= PHCFG_UPDN;
        else
            reg23val &= ~PHCFG_UPDN;
        addrs.push_back(0x0023); values.push_back(reg23val); //PHCFG_UpDn, CNT_IND
        addrs.push_back(0x0023); values.push_back(reg23val | PHCFG_START);
#ifdef LMS_VERBOSE_OUTPUT
        printf("C%i=%i, Fout=%3.3f MHz, CNT_IND=%i, nSteps=%i, phaseShift=%.1f\n", i, C, clocks[i].rd_actualFrequency / 1e6, cnt_ind, nSteps, nSteps*Fstep_deg);
#endif
        if(serPort->WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
            ReportError(EIO, "SetPllFrequency: PHCFG, failed to write registers");
        addrs.clear();
        values.clear();

        t1 = chrono::high_resolution_clock::now();
        if(boardType == LMS_DEV_QSPARK) do
        {
            serPort->ReadRegister(busyAddr, statusReg);
            done = statusReg & 0x1;
            errorCode = (statusReg >> 7) & 0xFF;
            t2 = chrono::high_resolution_clock::now();
        } while(!done && errorCode == 0 && (t2-t1) < timeout);
        if(t2 - t1 > timeout)
            return ReportError(ENODEV, "SetPllFrequency: PHCFG timeout, busy bit is still 1");
        if(errorCode != 0)
            return ReportError(EBUSY, "SetPllFrequency: error configuring PHCFG");
        addrs.push_back(0x0023); values.push_back(reg23val & ~PHCFG_START);
    }
    if(serPort->WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
        ReportError(EIO, "SetPllFrequency: configure FPGA PLL, failed to write registers");
    return 0;
}

int SetDirectClocking(IConnection* serPort, uint8_t clockIndex, const double inputFreq, const double phaseShift_deg)
{
    if(not serPort)
        return ReportError(ENODEV, "SetDirectClocking: connection port is NULL");
    if(not serPort->IsOpen())
        return ReportError(ENODEV, "SetDirectClocking: device not connected");

    uint16_t drct_clk_ctrl_0005 = 0;
    serPort->ReadRegister(0x0005, drct_clk_ctrl_0005);
    uint16_t drct_clk_ctrl_0006;
    serPort->ReadRegister(0x0006, drct_clk_ctrl_0006);

    vector<uint32_t> addres;
    vector<uint32_t> values;

    //enable direct clocking
    addres.push_back(0x0005); values.push_back(drct_clk_ctrl_0005 | (1 << clockIndex));
   //clear CNT_ID and CLK_IND
    drct_clk_ctrl_0006 = drct_clk_ctrl_0006 & ~0x3FF;
    const int cnt_ind = clockIndex << 5; // was 1<<5
    const int clk_ind = clockIndex;
    drct_clk_ctrl_0006 = drct_clk_ctrl_0006 | cnt_ind | clk_ind;
    addres.push_back(0x0006); values.push_back(drct_clk_ctrl_0006);
    const float oversampleClock_Hz = 100e6;
    const int registerChainSize = 128;
    const float oversampleClock_ns = 1e9 / oversampleClock_Hz;
    const float phaseStep_deg = 360 * oversampleClock_ns*(1e-9) / (1 / inputFreq);
    uint16_t phase_reg_select = (phaseShift_deg / phaseStep_deg)+0.5;
    const float actualPhaseShift_deg = 360 * inputFreq / (1 / (phase_reg_select * oversampleClock_ns*1e-9));
#ifdef LMS_VERBOSE_OUTPUT
    printf("########################################\n");
    printf("Direct clocking. clock index: %i\n", clockIndex);
    printf("phase_reg_select : %i\n", phase_reg_select);
    printf("input clock: %g MHz\n", inputFreq/1e6);
    printf("phase shift(desired/actual) : %.2f/%.2f\n", phaseShift_deg, actualPhaseShift_deg);
    printf("########################################\n");
#endif
    addres.push_back(0x0004); values.push_back(phase_reg_select);
    //LOAD_PH_REG = 1 << 10;
    addres.push_back(0x0006); values.push_back(drct_clk_ctrl_0006 | 1 << 10);
    addres.push_back(0x0006); values.push_back(drct_clk_ctrl_0006);
    if(serPort->WriteRegisters(addres.data(), values.data(), values.size()) != 0)
        return ReportError(EIO, "SetDirectClocking: failed to write registers");
    return 0;
}


} //namespace fpga
} //namespace lime
