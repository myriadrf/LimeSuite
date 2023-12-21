#include "FPGA_common.h"
#include "limesuite/IComms.h"
#include <vector>
#include <math.h>
#include <thread>
#include "Logger.h"
#include <algorithm>
#include <unordered_set>
#include <assert.h>
#include "LMSBoards.h"
using namespace std;

#ifndef NDEBUG
    #define ASSERT_WARNING(cond, message) \
        if (!cond) \
        printf("W: %s (%s)\n", message, #cond)
#else
    #define ASSERT_WARNING(cond, message)
#endif

#if 0
    #define verbose_printf(...) printf(__VA_ARGS__);
#else
    #define verbose_printf(...)
#endif

namespace lime {

/** @brief A class for writing a batch of registers into the FPGA. */
class WriteRegistersBatch
{
  public:
    WriteRegistersBatch(FPGA* fpga)
        : owner(fpga){};
    ~WriteRegistersBatch() { ASSERT_WARNING(addrs.size() == 0, "FPGA WriteRegistersBatch not flushed"); }
    int Flush()
    {
        int status = owner->WriteRegisters(addrs.data(), values.data(), addrs.size());
        addrs.clear();
        values.clear();
        return status;
    }
    void WriteRegister(uint16_t addr, uint16_t value)
    {
        addrs.push_back(addr);
        values.push_back(value);
    }

  private:
    FPGA* owner;
    std::vector<uint32_t> addrs;
    std::vector<uint32_t> values;
};

// 0x000A
const int RX_EN = 1; //controls both receiver and transmitter
const int TX_EN = 1 << 1; //used for wfm playback from fpga
const int STREAM_LOAD = 1 << 2;
const int RX_PTRN_EN = 1 << 8;
const int TX_PTRN_EN = 1 << 9;

// 0x0009
const int SMPL_NR_CLR = 1; // rising edge clears
const int TXPCT_LOSS_CLR = 1 << 1; // 0 - normal operation, 1-clear

// 0x0023
const uint16_t PLLCFG_START = 0x1;
const uint16_t PHCFG_START = 0x2;
const uint16_t PLLRST_START = 0x4;
const uint16_t PHCFG_UPDN = 1 << 13;
const uint16_t PHCFG_MODE = 1 << 14;

const uint16_t busyAddr = 0x0021;
static const std::chrono::milliseconds busyPollPeriod(10); // time between checking "done" bit

// does the fpga has "done" bit to indicate PLLCFG_START,PHCFG_START,PLLRST_START completion
static bool HasWaitForDone(uint8_t targetDevice)
{
    // TODO: list devices that don't have it, as it's most likely that future devices will support this
    switch (static_cast<eLMS_DEV>(targetDevice))
    {
    case LMS_DEV_LIMESDR_QPCIE:
    case LMS_DEV_LIMESDR_X3:
    case LMS_DEV_LIMESDR_XTRX:
        return true;
    default:
        return false;
    }
}

static bool HasFPGAClockPhaseSearch(uint8_t targetDevice, uint8_t version, uint8_t revision)
{
    const uint16_t ver_rev = version << 8 | revision;
    switch (static_cast<eLMS_DEV>(targetDevice))
    {
    case LMS_DEV_LIMESDR:
        return ver_rev > 0x20E;
    case LMS_DEV_LIMESDR_PCIE:
        return ver_rev > 0x206;
    case LMS_DEV_LIMESDR_QPCIE:
        return ver_rev > 0x102;
    case LMS_DEV_LIMESDR_CORE_SDR:
    case LMS_DEV_LIMESDRMINI:
    case LMS_DEV_LIMESDRMINI_V2:
    case LMS_DEV_LIMESDR_X3:
    case LMS_DEV_LIMESDR_XTRX:
        return true;
    default:
        return false;
    }
}

FPGA::FPGA(std::shared_ptr<ISPI> fpgaSPI, std::shared_ptr<ISPI> lms7002mSPI)
    : fpgaPort(fpgaSPI)
    , lms7002mPort(lms7002mSPI)
    , useCache(false)
{
}

void FPGA::EnableValuesCache(bool enabled)
{
    verbose_printf("Enable FPGA registers cache: %s\n", enabled ? "true" : "false");
    useCache = enabled;
    if (!useCache)
        regsCache.clear();
}

int FPGA::WriteRegister(uint32_t addr, uint32_t val)
{
    return WriteRegisters(&addr, &val, 1);
}

int FPGA::ReadRegister(uint32_t addr)
{
    uint32_t val;
    return ReadRegisters(&addr, &val, 1) != 0 ? -1 : val;
}

int FPGA::WriteRegisters(const uint32_t* addrs, const uint32_t* data, unsigned cnt)
{
    std::vector<uint32_t> spiBuffer;
    if (useCache)
    {
        static const int readonly_regs[] = { 0x000,
            0x001,
            0x002,
            0x003,
            0x021,
            0x022,
            0x065,
            0x067,
            0x069,
            0x06A,
            0x06B,
            0x06C,
            0x06D,
            0x06F,
            0x070,
            0x071,
            0x072,
            0x073,
            0x074,
            0x076,
            0x077,
            0x078,
            0x07A,
            0x07B,
            0x07C,
            0x0C2,
            0x100,
            0x101,
            0x102,
            0x103,
            0x104,
            0x105,
            0x106,
            0x107,
            0x108,
            0x109,
            0x10A,
            0x10B,
            0x10C,
            0x10D,
            0x10E,
            0x10F,
            0x110,
            0x111,
            0x114 };

        for (unsigned i = 0; i < cnt; i++)
        {
            auto endptr = readonly_regs + sizeof(readonly_regs) / sizeof(*readonly_regs);
            if (std::find(readonly_regs, endptr, addrs[i]) != endptr)
                continue;

            auto result = regsCache.find(addrs[i]);
            if (result != regsCache.end() && result->second == data[i])
                continue;
            spiBuffer.push_back((1 << 31) | (addrs[i]) << 16 | data[i]);
            regsCache[addrs[i]] = data[i];
        }
        if (spiBuffer.size())
            fpgaPort->SPI(spiBuffer.data(), nullptr, spiBuffer.size());
        return 0;
    }
    for (unsigned i = 0; i < cnt; i++)
        spiBuffer.push_back((1 << 31) | (addrs[i]) << 16 | data[i]);
    if (spiBuffer.size())
        fpgaPort->SPI(spiBuffer.data(), nullptr, spiBuffer.size());
    return 0;
}

int FPGA::WriteLMS7002MSPI(const uint32_t* data, uint32_t length)
{
#ifndef NDEBUG
    for (uint32_t i = 0; i < length; ++i)
        assert(data[i] & (1 << 31));
#endif
    lms7002mPort->SPI(data, nullptr, length);
    return 0;
}

int FPGA::ReadLMS7002MSPI(const uint32_t* writeData, uint32_t* readData, uint32_t length)
{
    lms7002mPort->SPI(writeData, readData, length);
    return 0;
}

int FPGA::ReadRegisters(const uint32_t* addrs, uint32_t* data, unsigned cnt)
{
    std::vector<uint32_t> spiBuffer;
    if (useCache)
    {
        static const int volatile_regs[] = { 0x021,
            0x022,
            0x060,
            0x065,
            0x067,
            0x069,
            0x06A,
            0x06B,
            0x06C,
            0x06D,
            0x06F,
            0x070,
            0x071,
            0x072,
            0x073,
            0x074,
            0x076,
            0x077,
            0x078,
            0x07A,
            0x07B,
            0x07C,
            0x0C2,
            0x100,
            0x101,
            0x102,
            0x103,
            0x104,
            0x105,
            0x106,
            0x107,
            0x108,
            0x109,
            0x10A,
            0x10B,
            0x10C,
            0x10D,
            0x10E,
            0x10F,
            0x110,
            0x111,
            0x114 };

        std::vector<uint32_t> reg_addr;
        for (unsigned i = 0; i < cnt; i++)
        {
            auto endptr = volatile_regs + sizeof(volatile_regs) / sizeof(*volatile_regs);
            if (std::find(volatile_regs, endptr, addrs[i]) == endptr)
            {
                auto result = regsCache.find(addrs[i]);
                if (result != regsCache.end())
                    continue;
            }
            spiBuffer.push_back(addrs[i]);
        }

        if (spiBuffer.size())
        {
            std::vector<uint32_t> reg_val(spiBuffer.size());
            fpgaPort->SPI(spiBuffer.data(), reg_val.data(), spiBuffer.size());
            for (unsigned i = 0; i < spiBuffer.size(); i++)
                regsCache[spiBuffer[i]] = reg_val[i];
        }
        for (unsigned i = 0; i < cnt; i++)
            data[i] = regsCache[addrs[i]];
        return 0;
    }
    for (unsigned i = 0; i < cnt; i++)
        spiBuffer.push_back(addrs[i]);
    std::vector<uint32_t> reg_val(spiBuffer.size());
    fpgaPort->SPI(spiBuffer.data(), reg_val.data(), spiBuffer.size());
    for (unsigned i = 0; i < cnt; i++)
        data[i] = reg_val[i] & 0xFFFF;
    return 0;
}

int FPGA::StartStreaming()
{
    verbose_printf("%s\n", __func__);
    int interface_ctrl_000A = ReadRegister(0x000A);
    if (interface_ctrl_000A < 0)
        return -1;
    ASSERT_WARNING((interface_ctrl_000A & RX_EN) == 0, "FPGA stream is already started");
    return WriteRegister(0x000A, interface_ctrl_000A | RX_EN);
}

int FPGA::StopStreaming()
{
    verbose_printf("%s\n", __func__);
    int interface_ctrl_000A = ReadRegister(0x000A);
    if (interface_ctrl_000A < 0)
        return -1;
    const uint16_t flags = ~(RX_EN | TX_EN);
    return WriteRegister(0x000A, interface_ctrl_000A & flags);
}

int FPGA::ResetTimestamp()
{
    verbose_printf("%s\n", __func__);
#ifndef NDEBUG
    int interface_ctrl_000A = ReadRegister(0x000A);
    if (interface_ctrl_000A < 0)
        return 0;

    if (interface_ctrl_000A & RX_EN)
        return ReportError(EPERM, "FPGA samples streaming must be stopped to reset timestamp");
#endif // NDEBUG
    //reset hardware timestamp to 0
    int interface_ctrl_0009 = ReadRegister(0x0009);
    if (interface_ctrl_0009 < 0)
        return 0;
    const uint32_t flags = (TXPCT_LOSS_CLR | SMPL_NR_CLR);
    uint32_t addrs[] = { 0x0009, 0x0009, 0x0009 };
    uint32_t values[] = { interface_ctrl_0009 & (~flags), interface_ctrl_0009 | flags, interface_ctrl_0009 & (~flags) };
    return WriteRegisters(addrs, values, 3);
}

int FPGA::WaitTillDone(uint16_t pollAddr, uint16_t doneMask, uint16_t errorMask, const std::string& title)
{
    const auto timeout = chrono::seconds(3);
    auto t1 = chrono::high_resolution_clock::now();
    bool done = false;
    uint16_t error = 0;
    if (!title.empty())
    {
        verbose_printf("%s\n", title.c_str());
    }
    do
    {
        const uint16_t state = ReadRegister(pollAddr);
        done = state & doneMask;
        error = state & errorMask;
        if (error != 0)
        {
            lime::warning("%s error, reg:0x%04X=0x%04X, errorBits:0x%04X", title.c_str(), pollAddr, state, error);
            return EBUSY;
        }

        if (!done)
        {
            if ((chrono::high_resolution_clock::now() - t1) > timeout)
            {
                lime::warning("%s timeout", title.c_str());
                return ETIME;
            }
            else
                std::this_thread::sleep_for(busyPollPeriod);
        }
    } while (!done);
    if (!title.empty())
    {
        verbose_printf("%s done\n", title.c_str());
    }
    return 0;
}

int FPGA::SetPllClock(uint clockIndex, int nSteps, bool waitLock, bool doPhaseSearch, uint16_t& reg23val)
{
    WriteRegistersBatch batch(this);

    batch.WriteRegister(0x0023, reg23val & ~(PLLCFG_START | PHCFG_START));

    int cnt_ind = (clockIndex + 2) & 0x1F; //C0 index 2, C1 index 3...
    reg23val &= ~(0xF << 8);
    reg23val |= cnt_ind << 8;

    if (doPhaseSearch)
    {
        reg23val |= PHCFG_UPDN;
        reg23val |= PHCFG_MODE;
    }
    else
    {
        reg23val &= ~(PHCFG_MODE);
        if (nSteps >= 0)
            reg23val |= PHCFG_UPDN;
        else
            reg23val &= ~PHCFG_UPDN;
    }

    batch.WriteRegister(0x0023, reg23val); //PHCFG_UpDn, CNT_IND
    batch.WriteRegister(0x0024, abs(nSteps)); //CNT_PHASE
    batch.Flush();
    // TODO: could possibly write this in the same batch?
    if (WriteRegister(0x0023, reg23val | PHCFG_START) != 0)
        lime::error("FPGA SetPllFrequency: find phase, failed to write registers");

    const uint16_t doneMask = doPhaseSearch ? 0x4 : 0x1;
    const uint16_t errorMask = doPhaseSearch ? 0x8 : (0xFF << 7);

    if (waitLock)
    {
        const std::string title = "PLL Clock[" + std::to_string(clockIndex) + "] PHCFG_START";
        int status = WaitTillDone(busyAddr, doneMask, errorMask, title);
        if (status != 0)
            return status;
    }
    else
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (WriteRegister(0x0023, reg23val & ~PHCFG_START) != 0) // redundant clear
        ReportError(EIO, "FPGA SetPllClock: failed to write registers");
    return 0;
}

/** @brief Configures board FPGA clocks
    @param pllIndex index of FPGA pll
    @param inputFreq input frequency
    @param clocks list of clocks to configure
    @param clockCount number of clocks to configure
    @return 0-success, other-failure
*/
int FPGA::SetPllFrequency(const uint8_t pllIndex, const double inputFreq, FPGA_PLL_clock* clocks, const uint8_t clockCount)
{
    verbose_printf("FPGA SetPllFrequency: PLL[%i] input:%.3f MHz clockCount:%i\n", pllIndex, inputFreq / 1e6, clockCount);
    WriteRegistersBatch batch(this);
    const auto timeout = chrono::seconds(3);
    if (not fpgaPort)
        return ReportError(ENODEV, "ConfigureFPGA_PLL: connection port is NULL");

    const bool waitForDone = HasWaitForDone(ReadRegister(0)); // read targetDevice
    bool willDoPhaseSearch = false;

    if (pllIndex > 15)
        ReportError(ERANGE, "FPGA SetPllFrequency: PLL index(%i) out of range [0-15]", pllIndex);

    //check if all clocks are above 5MHz
    const double PLLlowerLimit = 5e6;
    if (inputFreq < PLLlowerLimit)
        return ReportError(ERANGE, "FPGA SetPllFrequency: PLL[%i] input frequency must be >=%g MHz", pllIndex, PLLlowerLimit / 1e6);
    for (int i = 0; i < clockCount; ++i)
    {
        verbose_printf("CLK[%i] Fout:%.3f MHz bypass:%i phase:%g findPhase: %i\n",
            clocks[i].index,
            clocks[i].outFrequency / 1e6,
            clocks[i].bypass,
            clocks[i].phaseShift_deg,
            clocks[i].findPhase);
        willDoPhaseSearch |= clocks[i].findPhase;
        if (clocks[i].outFrequency < PLLlowerLimit && not clocks[i].bypass)
            return ReportError(
                ERANGE, "FPGA SetPllFrequency: PLL[%i], clock[%i] must be >=%g MHz", pllIndex, i, PLLlowerLimit / 1e6);
    }

    uint16_t drct_clk_ctrl_0005 = ReadRegister(0x0005);
    uint16_t reg23val = ReadRegister(0x0023);
    uint16_t reg25 = ReadRegister(0x0025);

    //disable direct clock source
    batch.WriteRegister(0x0005, drct_clk_ctrl_0005 & ~(1 << pllIndex));
    reg23val &= ~(PLLCFG_START | PHCFG_START | PLLRST_START | PHCFG_UPDN); //clear controls
    reg23val &= ~(0x1F << 3); //clear PLL index
    reg23val |= pllIndex << 3;

    batch.WriteRegister(0x0025, reg25 | 0x80); // TODO: what's 0x80?
    batch.WriteRegister(0x0023, reg23val); //PLL_IND
    batch.Flush();

    if (!willDoPhaseSearch)
    {
        WriteRegister(0x0023, reg23val | PLLRST_START);
        if (waitForDone)
        {
            const std::string title = "FPGA PLL[" + std::to_string(pllIndex) + "] PLLRST_START";

            int status = WaitTillDone(busyAddr, 0x0001, 0xFF << 7, title);
            if (status != 0)
                return status;
        }
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        batch.WriteRegister(0x0023, reg23val & ~PLLRST_START); // redundant clear
    }

    //configure FPGA PLLs
    const double vcoLimits_Hz[2] = { 600e6, 1280e6 };

    // Collect all desired VCO frequencies
    map<uint64_t, uint8_t> desiredVCO; // <VCOfreq, demandByClocks>
    for (int i = 0; i < clockCount; ++i)
    {
        if (clocks[i].outFrequency == 0 || clocks[i].bypass)
            continue;

        unsigned long freq = clocks[i].outFrequency * ceil(vcoLimits_Hz[0] / clocks[i].outFrequency);
        // Fill VCO frequencies that have integer dividers of clock outputs
        while (freq >= vcoLimits_Hz[0] && freq <= vcoLimits_Hz[1])
        {
            auto it = desiredVCO.find(freq);
            if (it != desiredVCO.end())
                it->second++; // increase demand
            else // add new frequency demand
                desiredVCO.insert(pair<uint64_t, uint8_t>(freq, 1));
            freq += clocks[i].outFrequency;
        }
    }
    if (desiredVCO.size() == 0)
        return ReportError(EINVAL, "FPGA SetPllFrequency: no suitable VCO frequencies found for requested clocks\n");

    // Find VCO that satisfies most outputs with integer dividers
    uint64_t bestFreqVCO = std::max_element(
        desiredVCO.begin(), desiredVCO.end(), [](const pair<uint64_t, uint8_t>& p1, const pair<uint64_t, uint8_t>& p2) {
            if (p1.second == p2.second)
                return p1.first < p2.first; // sort by VCO frequency
            return p1.second < p2.second;
        })->first;

    // Calculate coefficients to multiply input frequency*(M/N) up to VCO frequency
    const int N = 1;
    const int M = ceil(bestFreqVCO / inputFreq);

    // TODO: if multiple VCO frequencies would have the same demand, choose one with less deviation
    //const double deviation = fabs(bestFreqVCO - inputFreq * M / N);

    int mlow = M / 2;
    int mhigh = mlow + M % 2;
    const double Fvco = inputFreq * M / N; //actual VCO freq
    lime::debug("FPGA PLL[%i] M=%i, N=%i, Fvco=%.3f MHz (Requested %.3f MHz)", pllIndex, M, N, Fvco / 1e6, bestFreqVCO / 1e6);
    if (Fvco < vcoLimits_Hz[0] || Fvco > vcoLimits_Hz[1])
        return ReportError(ERANGE,
            "FPGA SetPllFrequency: PLL[%i], VCO(%g MHz) out of range [%g:%g] MHz",
            pllIndex,
            Fvco / 1e6,
            vcoLimits_Hz[0] / 1e6,
            vcoLimits_Hz[1] / 1e6);

    uint16_t M_N_odd_byp = (M % 2 << 3) | (N % 2 << 1);
    if (M == 1)
        M_N_odd_byp |= 1 << 2; //bypass M
    if (N == 1)
        M_N_odd_byp |= 1; //bypass N
    batch.WriteRegister(0x0026, M_N_odd_byp);
    int nlow = N / 2;
    int nhigh = nlow + N % 2;
    batch.WriteRegister(0x002A, nhigh << 8 | nlow); //N_high_cnt, N_low_cnt
    batch.WriteRegister(0x002B, mhigh << 8 | mlow);

    uint16_t c7_c0_odds_byps = 0x5555; //bypass all C
    uint16_t c15_c8_odds_byps = 0x5555; //bypass all C

    // Set clock outputs
    for (int i = 0; i < clockCount; ++i)
    {
        const int C = ceil(Fvco / clocks[i].outFrequency);
        if (i < 8)
        {
            if (not clocks[i].bypass && C != 1)
                c7_c0_odds_byps &= ~(1 << (i * 2)); //enable output
            c7_c0_odds_byps |= (C % 2) << (i * 2 + 1); //odd bit
        }
        else
        {
            if (not clocks[i].bypass && C != 1)
                c15_c8_odds_byps &= ~(1 << ((i - 8) * 2)); //enable output
            c15_c8_odds_byps |= (C % 2) << ((i - 8) * 2 + 1); //odd bit
        }
        const int clow = C / 2;
        const int chigh = clow + C % 2;
        batch.WriteRegister(0x002E + i, chigh << 8 | clow);
        clocks[i].rd_actualFrequency = (inputFreq * M / N) / (chigh + clow);
    }
    batch.WriteRegister(0x0027, c7_c0_odds_byps);
    batch.WriteRegister(0x0028, c15_c8_odds_byps);
    batch.Flush();

    if (clockCount != 4 || clocks->index == 3) // TODO: this seems to be LimeSDR-Mini specific
        WriteRegister(0x0023, reg23val | PLLCFG_START);
    if (waitForDone) //wait for config to activate
    {
        const std::string title = "FPGA PLL[" + std::to_string(pllIndex) + "] PLLCFG_START";

        int status = WaitTillDone(busyAddr, 0x0001, 0xFF << 7, title);
        if (status != 0)
            return status;
    }
    else
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    WriteRegister(0x0023, reg23val & ~PLLCFG_START); // redundant clear

    for (int i = 0; i < clockCount; ++i)
    {
        int C = ceil(Fvco / clocks[i].outFrequency);
        float fOut_MHz = inputFreq / 1e6;
        float Fstep_us = 1 / (8 * fOut_MHz * C);
        float Fstep_deg = (360 * Fstep_us) / (1 / fOut_MHz);
        int status = 0;
        int nSteps = 0;
        if (clocks[i].findPhase == false)
            nSteps = 0.49 + clocks[i].phaseShift_deg / Fstep_deg;
        else
            nSteps = (360.0 / Fstep_deg) - 0.5;
        status = SetPllClock(clocks[i].index, nSteps, waitForDone, clocks[i].findPhase, reg23val);
        if (status != 0)
            return status;
    }
    return 0;
}

int FPGA::SetDirectClocking(int clockIndex)
{
    if (not fpgaPort)
        return ReportError(ENODEV, "SetDirectClocking: connection port is NULL");

    uint16_t drct_clk_ctrl_0005 = ReadRegister(0x0005);
    //enable direct clocking
    if (WriteRegister(0x0005, drct_clk_ctrl_0005 | (1 << clockIndex)) != 0)
        return ReportError(EIO, "SetDirectClocking: failed to write registers");
    return 0;
}

/** @brief Parses FPGA packet payload into samples
*/
int FPGA::FPGAPacketPayload2Samples(const uint8_t* buffer, int bufLen, bool mimo, bool compressed, complex16_t** samples)
{
    if (compressed) //compressed samples
    {
        int16_t sample;
        int collected = 0;
        for (int b = 0; b < bufLen; collected++)
        {
            //I sample
            sample = buffer[b++];
            sample |= (buffer[b] << 8);
            sample <<= 4;
            samples[0][collected].i = sample >> 4;
            //Q sample
            sample = buffer[b++];
            sample |= buffer[b++] << 8;
            samples[0][collected].q = sample >> 4;
            if (mimo)
            {
                //I sample
                sample = buffer[b++];
                sample |= (buffer[b] << 8);
                sample <<= 4;
                samples[1][collected].i = sample >> 4;
                //Q sample
                sample = buffer[b++];
                sample |= buffer[b++] << 8;
                samples[1][collected].q = sample >> 4;
            }
        }
        return collected;
    }

    if (mimo) //uncompressed samples
    {
        complex16_t* ptr = (complex16_t*)buffer;
        const int collected = bufLen / sizeof(complex16_t) / 2;
        for (int i = 0; i < collected; i++)
        {
            samples[0][i] = *ptr++;
            samples[1][i] = *ptr++;
        }
        return collected;
    }

    memcpy(samples[0], buffer, bufLen);
    return bufLen / sizeof(complex16_t);
}

/** @brief Parses FPGA packet payload into samples
*/
int FPGA::FPGAPacketPayload2SamplesFloat(const uint8_t* buffer, int bufLen, bool mimo, bool compressed, complex32f_t** samples)
{
    const float normalizationAmplitude = compressed ? 2048 : 32768;
    if (compressed) //compressed samples
    {
        int16_t sample;
        int collected = 0;
        for (int b = 0; b < bufLen; collected++)
        {
            //I sample
            sample = buffer[b++];
            sample |= (buffer[b] << 8);
            sample <<= 4;
            samples[0][collected].i = (sample >> 4) / normalizationAmplitude;
            //Q sample
            sample = buffer[b++];
            sample |= buffer[b++] << 8;
            samples[0][collected].q = (sample >> 4) / normalizationAmplitude;
            if (mimo)
            {
                //I sample
                sample = buffer[b++];
                sample |= (buffer[b] << 8);
                sample <<= 4;
                samples[1][collected].i = (sample >> 4) / normalizationAmplitude;
                //Q sample
                sample = buffer[b++];
                sample |= buffer[b++] << 8;
                samples[1][collected].q = (sample >> 4) / normalizationAmplitude;
            }
        }
        return collected;
    }

    complex16_t* src = (complex16_t*)buffer;
    if (mimo) //uncompressed samples
    {
        const int collected = bufLen / sizeof(complex16_t) / 2;
        for (int i = 0; i < collected; i++)
        {
            samples[0][i].i = (*src).i / normalizationAmplitude;
            samples[0][i].q = (*src).q / normalizationAmplitude;
            ++src;
            samples[1][i].i = (*src).i / normalizationAmplitude;
            samples[1][i].q = (*src).q / normalizationAmplitude;
            ++src;
        }
        return collected;
    }
    else
    {
        const int collected = bufLen / sizeof(complex16_t);
        for (int i = 0; i < collected; i++)
        {
            samples[0][i].i = (*src).i / normalizationAmplitude;
            samples[0][i].q = (*src).q / normalizationAmplitude;
            ++src;
        }
        return collected;
    }
}

int FPGA::Samples2FPGAPacketPayloadFloat(
    const complex32f_t* const* samples, int samplesCount, bool mimo, bool compressed, uint8_t* buffer)
{
    const float amplitude = compressed ? 2047 : 32767;
    if (compressed)
    {
        int b = 0;
        for (int src = 0; src < samplesCount; ++src)
        {
            int16_t i = samples[0][src].i * amplitude;
            int16_t q = samples[0][src].q * amplitude;
            buffer[b++] = i;
            buffer[b++] = ((i >> 8) & 0x0F) | (q << 4);
            buffer[b++] = q >> 4;
            if (mimo)
            {
                int16_t i = samples[1][src].i * amplitude;
                int16_t q = samples[1][src].q * amplitude;
                buffer[b++] = i;
                buffer[b++] = ((i >> 8) & 0x0F) | (q << 4);
                buffer[b++] = q >> 4;
            }
        }
        return b;
    }

    complex16_t* dest = (complex16_t*)buffer;
    if (mimo)
    {
        for (int src = 0; src < samplesCount; ++src)
        {
            (*dest).i = samples[0][src].i * amplitude;
            (*dest).q = samples[0][src].q * amplitude;
            ++dest;
            (*dest).i = samples[1][src].i * amplitude;
            (*dest).q = samples[1][src].q * amplitude;
            ++dest;
        }
        return samplesCount * sizeof(complex16_t) * 2;
    }
    else
    {
        for (int src = 0; src < samplesCount; ++src)
        {
            (*dest).i = samples[0][src].i * amplitude;
            (*dest).q = samples[0][src].q * amplitude;
            ++dest;
        }
        return samplesCount * sizeof(complex16_t);
    }
}

int FPGA::Samples2FPGAPacketPayload(
    const complex16_t* const* samples, int samplesCount, bool mimo, bool compressed, uint8_t* buffer)
{
    if (compressed)
    {
        int b = 0;
        for (int src = 0; src < samplesCount; ++src)
        {
            buffer[b++] = samples[0][src].i;
            buffer[b++] = ((samples[0][src].i >> 8) & 0x0F) | (samples[0][src].q << 4);
            buffer[b++] = samples[0][src].q >> 4;
            if (mimo)
            {
                buffer[b++] = samples[1][src].i;
                buffer[b++] = ((samples[1][src].i >> 8) & 0x0F) | (samples[1][src].q << 4);
                buffer[b++] = samples[1][src].q >> 4;
            }
        }
        return b;
    }

    if (mimo)
    {
        complex16_t* ptr = (complex16_t*)buffer;
        for (int src = 0; src < samplesCount; ++src)
        {
            *ptr++ = samples[0][src];
            *ptr++ = samples[1][src];
        }
        return samplesCount * 2 * sizeof(complex16_t);
    }
    memcpy(buffer, samples[0], samplesCount * sizeof(complex16_t));
    return samplesCount * sizeof(complex16_t);
}
/*
int FPGA::UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex)
{
    bool comp = (epIndex==2 && format!=StreamConfig::FMT_INT12) ? false : true;

    const int samplesInPkt = comp ? samples12InPkt : samples16InPkt;
    WriteRegister(0xFFFF, 1 << epIndex);
    WriteRegister(0x000C, chCount == 2 ? 0x3 : 0x1); //channels 0,1
    WriteRegister(0x000E, comp ? 0x2 : 0x0); //16bit samples

    uint16_t regValue = ReadRegister(0x000D);
    regValue |= 0x4;
    WriteRegister(0x000D, regValue);

    lime::FPGA_DataPacket pkt;
    size_t samplesUsed = 0;
    int cnt = sample_count;

    const complex16_t* const* src = (const complex16_t* const*)samples;
    const lime::complex16_t** batch = new const lime::complex16_t*[chCount];
    lime::complex16_t** samplesShort = new lime::complex16_t*[chCount];
    for(unsigned i=0; i<chCount; ++i)
        samplesShort[i] = nullptr;

    if (format == StreamConfig::FMT_INT16 && comp == true)
    {
        for(unsigned i=0; i<chCount; ++i)
            samplesShort[i] = new lime::complex16_t[sample_count];
        for (int ch = 0; ch < chCount; ch++)
            for(size_t i=0; i < sample_count; ++i)
            {
                samplesShort[ch][i].i = src[ch][i].i >> 4;
                samplesShort[ch][i].q = src[ch][i].q >> 4;
            }
        src = samplesShort;
    }
    else if(format == StreamConfig::FMT_FLOAT32)
    {
        const float mult = comp ? 2047.0f : 32767.0f;
        for(unsigned i=0; i<chCount; ++i)
            samplesShort[i] = new lime::complex16_t[sample_count];

        const float* const* samplesFloat = (const float* const*)samples;
        for (int ch = 0; ch < chCount; ch++)
            for(size_t i=0; i < sample_count; ++i)
            {
                samplesShort[ch][i].i = samplesFloat[ch][2*i]*mult;
                samplesShort[ch][i].q = samplesFloat[ch][2*i+1]*mult;
            }
        src = samplesShort;
    }

    while(cnt > 0)
    {
        pkt.counter = 0;
        pkt.reserved[0] = 0;
        int samplesToSend = cnt > samplesInPkt/chCount ? samplesInPkt/chCount : cnt;

        for(unsigned i=0; i<chCount; ++i)
            batch[i] = &src[i][samplesUsed];
        samplesUsed += samplesToSend;

        int bufPos = Samples2FPGAPacketPayload(batch, samplesToSend, chCount==2, comp, pkt.data);
        int payloadSize = (bufPos / 4) * 4;
        if(bufPos % 4 != 0)
            lime::warning("Packet samples count not multiple of 4");
        pkt.reserved[2] = (payloadSize >> 8) & 0xFF; //WFM loading
        pkt.reserved[1] = payloadSize & 0xFF; //WFM loading
        pkt.reserved[0] = 0x1 << 5; //WFM loading

        long bToSend = 16+payloadSize;
        if (connection->SendData((const char*)&pkt,bToSend,epIndex,500)!=bToSend)
            break;
        cnt -= samplesToSend;
    }
    delete[] batch;
    for(unsigned i=0; i<chCount; ++i)
        if (samplesShort[i])
            delete [] samplesShort[i];
    delete[] samplesShort;

    // Give some time to load samples to FPGA
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    WriteRegister(0x000D, regValue & (~4));
    connection->AbortSending(epIndex);
    if(cnt == 0)
        return 0;
    else
        return ReportError(-1, "Failed to upload waveform");
}
*/

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int FPGA::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, double txPhase, double rxPhase, int channel)
{
    verbose_printf("FPGA::SetInterfaceFreq tx:%.3f MHz rx:%.3f MHz txPhase:%g rxPhase:%g ch:%i\n",
        txRate_Hz / 1e6,
        rxRate_Hz / 1e6,
        txPhase,
        rxPhase,
        channel);
    lime::FPGA::FPGA_PLL_clock clocks[2];
    int status = 0;

    const uint32_t addr = 0x002A;
    uint32_t val;
    val = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
    WriteLMS7002MSPI(&val, 1);
    ReadLMS7002MSPI(&addr, &val, 1);
    bool bypassTx = (val & 0xF0) == 0x00;
    bool bypassRx = (val & 0x0F) == 0x0D;

    if (rxRate_Hz >= 5e6)
    {
        clocks[0].index = 0;
        clocks[0].outFrequency = bypassRx ? 2 * rxRate_Hz : rxRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = clocks[0].outFrequency;
        clocks[1].phaseShift_deg = rxPhase;
        status = SetPllFrequency(1, rxRate_Hz, clocks, 2);
    }
    else
        status = SetDirectClocking(1);

    if (txRate_Hz >= 5e6)
    {
        clocks[0].index = 0;
        clocks[0].outFrequency = bypassTx ? 2 * txRate_Hz : txRate_Hz;
        clocks[1].index = 1;
        clocks[1].outFrequency = clocks[0].outFrequency;
        clocks[1].phaseShift_deg = txPhase;
        status |= SetPllFrequency(0, txRate_Hz, clocks, 2);
    }
    else
        status |= SetDirectClocking(0);
    return status;
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int FPGA::SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, int chipIndex)
{
    verbose_printf("FPGA::SetInterfaceFreq tx:%.3f MHz rx:%.3f MHz chipIndex:%i\n", txRate_Hz / 1e6, rxRate_Hz / 1e6, chipIndex);
    //PrintStackTrace();
    const int pll_ind = (chipIndex == 1) ? 2 : 0;
    int status = 0;
    uint32_t reg20;
    bool bypassTx = false;
    bool bypassRx = false;
    bool phaseSearch = false;

    // TODO: magic numbers, are they device specific?
    const double rxPhC1 = 89.46;
    const double rxPhC2 = 1.24e-6;
    const double txPhC1 = 89.61;
    const double txPhC2 = 2.71e-7;

    const std::vector<uint32_t> spiAddr = {
        0x021, 0x022, 0x023, 0x024, 0x027, 0x02A, 0x82, 0x400, 0x40C, 0x40B, 0x400, 0x40B, 0x400
    };
    const int bakRegCnt = spiAddr.size() - 4;

    if (rxRate_Hz >= 5e6 && txRate_Hz >= 5e6)
    {
        uint32_t addr[3] = { 0, 1, 2 }; // TargetDevice, version, revision
        uint32_t vals[3];
        ReadRegisters(addr, vals, 3);
        phaseSearch = HasFPGAClockPhaseSearch(vals[0], vals[1], vals[2]);
    }

    if (!phaseSearch)
        return SetInterfaceFreq(txRate_Hz, rxRate_Hz, txPhC1 + txPhC2 * txRate_Hz, rxPhC1 + rxPhC2 * rxRate_Hz, chipIndex);

    std::vector<uint32_t> dataRdA;
    std::vector<uint32_t> dataRdB;
    std::vector<uint32_t> dataWr;

    dataWr.resize(spiAddr.size());
    dataRdA.resize(bakRegCnt);
    dataRdB.clear();
    //backup registers
    dataWr[0] = 0x0020;
    ReadLMS7002MSPI(dataWr.data(), &reg20, 1);

    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
    WriteLMS7002MSPI(dataWr.data(), 1);

    ReadLMS7002MSPI(spiAddr.data(), dataRdA.data(), bakRegCnt);

    {
        const uint32_t addr = 0x002A;
        uint32_t val;
        ReadLMS7002MSPI(&addr, &val, 1);
        bypassTx = (val & 0xF0) == 0x00;
        bypassRx = (val & 0x0F) == 0x0D;
    }

    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFE; //msbit 1=SPI write
    WriteLMS7002MSPI(dataWr.data(), 1);

    for (int i = 0; i < bakRegCnt; ++i)
        if (spiAddr[i] >= 0x100)
            dataRdB.push_back(spiAddr[i]);
    ReadLMS7002MSPI(dataRdB.data(), dataRdB.data(), dataRdB.size());

    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFF; //msbit 1=SPI write
    WriteLMS7002MSPI(dataWr.data(), 1);

    {
        std::vector<uint32_t> spiData = {
            0x0E9F, 0x0FFF, 0x5550, 0xE4E4, 0xE4E4, 0x0086, 0x8001, 0x028D, 0x00FF, 0x5555, 0x02CD, 0xAAAA, 0x02ED
        };
        if (bypassRx)
            spiData[5] = 0xD;
        //Load test config
        const int setRegCnt = spiData.size();
        for (int i = 0; i < setRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
        WriteLMS7002MSPI(dataWr.data(), setRegCnt);
    }

    bool phaseSearchSuccess = false;
    lime::FPGA::FPGA_PLL_clock clocks[2];
    clocks[0].index = 0;
    clocks[0].outFrequency = bypassRx ? 2 * rxRate_Hz : rxRate_Hz;
    clocks[0].phaseShift_deg = rxPhC1 + rxPhC2 * rxRate_Hz;
    clocks[0].findPhase = false;
    clocks[1] = clocks[0];
    clocks[1].index = 1;
    clocks[1].findPhase = true;

    for (int i = 0; i < 10; i++) //attempt phase search 10 times
    {
        if (SetPllFrequency(pll_ind + 1, rxRate_Hz, clocks, 2) == 0)
        {
            phaseSearchSuccess = true;
            break;
        }
        else
        {
            verbose_printf("Retry%i: SetPllFrequency\n", i);
            std::this_thread::sleep_for(busyPollPeriod);
        }
    }

    if (!phaseSearchSuccess)
    {
        lime::error("LML RX phase search FAIL");
        status = -1;
        clocks[0].index = 0;
        clocks[0].phaseShift_deg = 0;
        clocks[0].findPhase = false;
        clocks[1].findPhase = false;
        SetPllFrequency(pll_ind + 1, rxRate_Hz, clocks, 2);
    }

    WriteRegister(0xFFFF, 1 << chipIndex);
    uint16_t reg_000A = ReadRegister(0x000A);
    WriteRegister(0x000A, reg_000A & ~(RX_EN | TX_EN | TX_PTRN_EN | RX_PTRN_EN)); // clear test patterns
    {
        std::vector<uint32_t> spiData = { 0x0E9F, 0x0FFF, 0x5550, 0xE4E4, 0xE4E4, 0x0484, 0x8001 };
        if (bypassTx)
            spiData[5] ^= 0x80;
        if (bypassRx)
            spiData[5] ^= 0x9;
        //Load test config
        const int setRegCnt = spiData.size();
        for (int i = 0; i < setRegCnt; ++i)
            dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write
        WriteLMS7002MSPI(dataWr.data(), setRegCnt);
    }

    phaseSearchSuccess = false;
    clocks[0].index = 0;
    clocks[0].outFrequency = bypassTx ? 2 * txRate_Hz : txRate_Hz;
    clocks[0].phaseShift_deg = txPhC1 + txPhC2 * txRate_Hz;
    clocks[0].findPhase = false;
    clocks[1] = clocks[0];
    clocks[1].index = 1;
    clocks[1].findPhase = true;
    WriteRegister(0x000A, reg_000A | TX_PTRN_EN);
    for (int i = 0; i < 10; i++) //attempt phase search 10 times
    {
        if (SetPllFrequency(pll_ind, txRate_Hz, clocks, 2) == 0)
        {
            phaseSearchSuccess = true;
            break;
        }
        else
        {
            verbose_printf("Retry%i: SetPllFrequency\n", i);
            std::this_thread::sleep_for(busyPollPeriod);
        }
    }

    if (!phaseSearchSuccess)
    {
        lime::error("LML TX phase search FAIL");
        status = -1;
        clocks[0].index = 0;
        clocks[0].phaseShift_deg = 0;
        clocks[0].findPhase = false;
        clocks[1].findPhase = false;
        SetPllFrequency(pll_ind, txRate_Hz, clocks, 2);
    }

    //Restore registers
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD; //msbit 1=SPI write
    WriteLMS7002MSPI(dataWr.data(), 1);
    for (int i = 0; i < bakRegCnt; ++i)
        dataWr[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRdA[i]; //msbit 1=SPI write
    WriteLMS7002MSPI(dataWr.data(), bakRegCnt);
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFE; //msbit 1=SPI write
    WriteLMS7002MSPI(dataWr.data(), 1);

    int k = 0;
    for (int i = 0; i < bakRegCnt; ++i)
        if (spiAddr[i] >= 0x100)
        {
            dataWr[k] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | dataRdB[k]; //msbit 1=SPI write
            k++;
        }
    WriteLMS7002MSPI(dataWr.data(), k);
    dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20; //msbit 1=SPI write
    WriteLMS7002MSPI(dataWr.data(), 1);
    WriteRegister(0x000A, reg_000A);
    return status;
}

int FPGA::ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms)
{
    WriteRegister(0xFFFF, 1 << epIndex);
    StopStreaming();
    // TODO: connection->ResetStreamBuffers();
    WriteRegister(0x0008, 0x0100 | 0x2);
    WriteRegister(0x0007, 1);
    StartStreaming();
    int totalBytesReceived = 0; // TODO: connection->ReceiveData(buffer,length, epIndex, timeout_ms);
    StopStreaming();
    // TODO: connection->AbortReading(epIndex);
    return totalBytesReceived;
}

double FPGA::DetectRefClk(double fx3Clk)
{
    verbose_printf("FPGA::DetectRefClk fx3Clk:%g\n", fx3Clk);
    const double fx3Cnt = 16777210; //fixed fx3 counter in FPGA
    const double clkTbl[] = { 10e6, 30.72e6, 38.4e6, 40e6, 52e6 };
    const uint32_t addr[] = { 0x61, 0x63 };
    const uint32_t vals[] = { 0x0, 0x0 };
    if (WriteRegisters(addr, vals, 2) != 0)
        return -1;

    auto start = std::chrono::steady_clock::now();
    if (WriteRegister(0x61, 0x4) != 0)
        return -1;

    while (1) //wait for test to finish
    {
        int completed = ReadRegister(0x65);
        if (completed < 0)
            return -1;
        if (completed & 0x4)
            break;

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        if (elapsed_seconds.count() > 0.5) //timeout
            return -1;
    }

    const uint32_t addr2[] = { 0x72, 0x73 };
    uint32_t vals2[2];
    if (ReadRegisters(addr2, vals2, 2) != 0)
        return -1;

    double count = (vals2[0] | (vals2[1] << 16)); //cock counter
    count *= fx3Clk / fx3Cnt; //estimate ref clock based on FX3 Clock
    lime::debug("Estimated reference clock %1.4f MHz", count / 1e6);
    unsigned i = 0;
    double delta = 100e6;

    while (i < sizeof(clkTbl) / sizeof(*clkTbl))
        if (delta < fabs(count - clkTbl[i]))
            break;
        else
            delta = fabs(count - clkTbl[i++]);

    if (i == 0)
        return -1;
    lime::info("Reference clock %1.2f MHz", clkTbl[i - 1] / 1e6);
    return clkTbl[i - 1];
}

FPGA::GatewareInfo FPGA::GetGatewareInfo()
{
    GatewareInfo info;
    info.boardID = 0;
    info.version = 0;
    info.revision = 0;

    const uint32_t addrs[4] = { 0x0000, 0x0001, 0x0002, 0x0003 };
    uint32_t data[4];
    if (ReadRegisters(addrs, data, 4) != 0)
        return info;

    info.boardID = data[0];
    info.version = (int16_t)data[1];
    info.revision = data[2];
    info.hardwareVersion = data[3] & 0x7F;
    return info;
}

void FPGA::GatewareToDescriptor(const FPGA::GatewareInfo& gw, SDRDevice::Descriptor& desc)
{
    desc.gatewareTargetBoard = GetDeviceName(eLMS_DEV(gw.boardID));
    desc.gatewareVersion = std::to_string(int(gw.version));
    desc.gatewareRevision = std::to_string(int(gw.revision));
    desc.hardwareVersion = std::to_string(int(gw.hardwareVersion));
}

} //namespace lime
