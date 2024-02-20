#include "Equalizer.h"

#include "Register.h"
#include "limesuite/IComms.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <memory>

using namespace lime;

static constexpr Register EN_RXTSP(0x00A0, 0, 0, 0, 0);
static constexpr Register EN_TXTSP(0x0080, 0, 0, 0, 0);

static constexpr Register RX_DCCORR_BYP(0x00AC, 2, 2, 0, 0);
static constexpr Register RX_PHCORR_BYP(0x00AC, 0, 0, 0, 0);
static constexpr Register RX_GCORR_BYP(0x00AC, 1, 1, 0, 0);

static constexpr Register RX_EQU_BYP(0x00AC, 5, 5, 0, 0);

static constexpr Register RX_DCLOOP_BYP(0x00AC, 8, 8, 0, 0);
static constexpr Register RX_DCLOOP_AVG(0x00A4, 2, 0, 0, 0);

static constexpr Register TX_HB_BYP(0x0088, 0, 0, 1, 0);
static constexpr Register TX_HB_DEL(0x0088, 1, 1, 0, 0);
static constexpr Register SLEEP_CFR(0x0088, 2, 2, 1, 0);
static constexpr Register BYPASS_CFR(0x0088, 3, 3, 1, 0);
static constexpr Register ODD_CFR(0x0088, 4, 4, 1, 0);
static constexpr Register BYPASSGAIN_CFR(0x0088, 5, 5, 1, 0);
static constexpr Register SLEEP_FIR(0x0088, 6, 6, 1, 0);
static constexpr Register BYPASS_FIR(0x0088, 7, 7, 1, 0);

static constexpr Register ODD_FIR(0x0088, 8, 8, 0, 0);
static constexpr Register TX_PHCORR_BYP(0x0088, 9, 9, 0, 0);
static constexpr Register TX_GCORR_BYP(0x0088, 10, 10, 0, 0);
static constexpr Register TX_DCCORR_BYP(0x0088, 11, 11, 0, 0);
static constexpr Register TX_ISINC_BYP(0x0088, 12, 12, 1, 0);
static constexpr Register TX_EQU_BYP(0x0088, 13, 13, 1, 0);
static constexpr Register TX_INVERTQ(0x0088, 15, 15, 0, 0);

static constexpr Register TX_GCORRQ(0x0081, 11, 0, 2047, 0);
static constexpr Register TX_GCORRI(0x0082, 11, 0, 2047, 0);
static constexpr Register TX_PHCORR(0x0083, 11, 0, 0, 1);
static constexpr Register TX_DCCORRI(0x0084, 15, 0, 0, 0);
static constexpr Register TX_DCCORRQ(0x0085, 15, 0, 0, 0);
static constexpr Register thresholdSpin(0x0086, 15, 0, 0, 0);
static constexpr Register thresholdGain(0x0087, 15, 0, 0, 0);
static constexpr Register CFR_ORDER(0x008C, 7, 0, 0, 0); // dummy register

static constexpr Register RX_GCORRQ(0x00A1, 11, 0, 2047, 0);
static constexpr Register RX_GCORRI(0x00A2, 11, 0, 2047, 0);
static constexpr Register RX_PHCORR(0x00A3, 11, 0, 0, 1);

static constexpr Register cmbInsel(0x0080, 2, 2, 0, 0);

static constexpr Register MAC(0xFFFF, 1, 0, 0, 0);

Equalizer::Equalizer(std::shared_ptr<ISPI> comms, uint32_t spiBusAddr)
    : m_Comms(comms)
    , mSPIbusAddr(spiBusAddr)
{
}

Equalizer::~Equalizer()
{
}

void Equalizer::WriteRegister(const Register& reg, uint16_t value)
{
    uint32_t mosi = reg.address;
    uint32_t miso = 0;
    m_Comms->SPI(mSPIbusAddr, &mosi, &miso, 1);
    const uint16_t regMask = bitMask(reg.msb, reg.lsb);

    uint32_t regValue = (miso & ~regMask);
    regValue |= ((value << reg.lsb) & regMask);
    mosi = (1 << 31) | reg.address << 16 | regValue;
    m_Comms->SPI(mSPIbusAddr, &mosi, nullptr, 1);
}

uint16_t Equalizer::ReadRegister(const Register& reg)
{
    uint32_t mosi = reg.address;
    uint32_t miso = 0;
    m_Comms->SPI(mSPIbusAddr, &mosi, &miso, 1);
    const uint16_t regMask = bitMask(reg.msb, reg.lsb);
    return (miso & regMask) >> reg.lsb;
}

void Equalizer::Configure(const Equalizer::Config& state)
{
    // TODO: batch writes
    for (uint8_t ch = 0; ch < 2; ++ch)
    {
        WriteRegister(MAC, ch + 1);
        WriteRegister(RX_EQU_BYP, state.bypassRxEqualizer[ch]);
        WriteRegister(TX_EQU_BYP, state.bypassTxEqualizer[ch]);

        const Config::CFR& cfr = state.cfr[ch];
        const Config::FIR& fir = state.fir[ch];

        const bool useOversample = std::min(cfr.interpolation, uint8_t(2)) != 1;

        WriteRegister(TX_HB_BYP, !useOversample);
        WriteRegister(TX_HB_DEL, useOversample);
        WriteRegister(SLEEP_CFR, cfr.sleep);
        WriteRegister(BYPASS_CFR, cfr.bypass);
        WriteRegister(ODD_CFR, fir.coefficientsCount % 2);
        WriteRegister(BYPASSGAIN_CFR, cfr.bypassGain);

        assert(fir.coefficientsCount <= 32);
        SetFIRCoefficients(fir.coefficients, fir.coefficientsCount);
        // WriteRegister(ODD_FIR, fir.coefficientsCount % 2); // set in SetFIRCoefficients
        WriteRegister(SLEEP_FIR, fir.sleep);
        WriteRegister(BYPASS_FIR, fir.bypass);

        if (cfr.order >= 2 && cfr.order <= 32)
        {
            WriteRegister(CFR_ORDER, cfr.order);
            UpdateHannCoeff(cfr.order);
        }
        WriteRegister(thresholdSpin, cfr.threshold);
        WriteRegister(thresholdGain, cfr.thresholdGain);
    }
}

// Generates coefficients based on CFR order
void Equalizer::UpdateHannCoeff(uint16_t Filt_N)
{
    uint16_t msb, lsb = 0;
    uint16_t data = 0;
    uint16_t addr = 0;
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t offset = 0;
    uint16_t w[40];

    // CFR registers
    uint16_t maddressf0 = 0x000C; // 12*64=768
    uint16_t maddressf1 = 0x000D; // 13*64=832
    uint16_t NN = 15;
    const uint16_t MaxFilt_N = 32;

    Filt_N = std::min(Filt_N, MaxFilt_N);

    for (i = 0; i < Filt_N; ++i)
        w[i] = (uint16_t)(32768.0 * 0.25 * (1.0 - cos(2.0 * M_PI * i / (Filt_N - 1))));

    WriteRegister(SLEEP_CFR, 1);
    msb = lsb = 0;
    data = 0;
    i = 0;

    //WriteRegister(RESET_N, 0);
    //WriteRegister(RESET_N, 1);

    // TODO: figure out this address mess. For now just replaced SPI write functions
    std::vector<uint32_t> mosi;
    while (i < MaxFilt_N)
    {
        addr = (2 << 15) + (maddressf0 << 6) + (msb << 4) + lsb;
        mosi.push_back((1 << 31) | addr << 16 | data);
        addr = (2 << 15) + (maddressf1 << 6) + (msb << 4) + lsb;
        mosi.push_back((1 << 31) | addr << 16 | data);
        if (lsb >= NN) // 15
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;
        i++;
    }
    m_Comms->SPI(mSPIbusAddr, mosi.data(), nullptr, mosi.size());
    mosi.clear();

    msb = lsb = 0;
    i = j = 0;
    offset = 0;
    while (i <= (uint16_t)((Filt_N / 2) - 1))
    {
        addr = (2 << 15) + (maddressf1 << 6) + (msb << 4) + lsb;
        if (j >= offset)
            data = w[(uint16_t)((Filt_N + 1) / 2 + i)];
        else
            data = 0;
        mosi.push_back((1 << 31) | addr << 16 | data);
        if (lsb >= NN) // 15
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;

        if (j >= offset)
            i++;
        j++;
    }
    m_Comms->SPI(mSPIbusAddr, mosi.data(), nullptr, mosi.size());
    mosi.clear();

    msb = lsb = 0;
    i = j = 0;
    offset = (MaxFilt_N / 2) - ((uint16_t)((Filt_N + 1) / 2));
    while (i < Filt_N)
    {
        addr = (2 << 15) + (maddressf0 << 6) + (msb << 4) + lsb;

        if (j >= offset)
            data = w[i];
        else
            data = 0;

        mosi.push_back((1 << 31) | addr << 16 | data);
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;

        if (j >= offset)
            i++;
        j++;
    }
    m_Comms->SPI(mSPIbusAddr, mosi.data(), nullptr, mosi.size());
    mosi.clear();

    WriteRegister(ODD_CFR, Filt_N % 2);
    WriteRegister(SLEEP_CFR, 0); // RELEASE SLEEP_CFR

    // software reset
    //WriteRegister(RESET_N, 0);
    //WriteRegister(RESET_N, 1);
}

void Equalizer::SetFIRCoefficients(const int16_t* coefficients, uint16_t count)
{
    const int maxCoefCount = 32;
    assert(count <= 32);

    // FIR registers
    uint16_t maddressf0 = 0x000E; // 14x64 = 896;
    uint16_t maddressf1 = 0x000F; // 15x64 = 960;

    const uint16_t Filt_N = 32;
    uint16_t NN = 0;
    uint16_t addr = 0;
    uint16_t data = 0;
    uint16_t msb = 0;
    uint16_t lsb = 0;
    uint16_t i = 0;

    NN = 15;

    // std::vector<double> coefficients;
    // coefficients.resize(Filt_N, 0);

    // for (i = 0; i < Filt_N; i++) // maxCoefCount
    //     coefficients[i] = 0.0;

    // read coeffs
    // msb = lsb = i = 0;
    // while (i <= (uint16_t)((Filt_N)-1))
    // {
    //     addr = (maddressf0 << 6) + (msb << 4) + lsb;
    //     uint32_t mosi = addr;
    //     uint32_t miso = 0;
    //     m_Comms->SPI(mSPIbusAddr, &mosi, &miso, 1);
    //     coefficients[i] = (double)(miso & 0xFFFF);
    //     if (lsb >= NN) // 15
    //     {
    //         lsb = 0;
    //         msb++;
    //     }
    //     else
    //         lsb++;
    //     i++;
    // }

    WriteRegister(SLEEP_FIR, 1);

    std::vector<uint32_t> mosi;
    for (i = 0; i < maxCoefCount; i++)
    {
        addr = (maddressf0 << 6) + i;
        mosi.push_back((1 << 31) | addr << 16 | 0);
        addr = (maddressf1 << 6) + i;
        mosi.push_back((1 << 31) | addr << 16 | 0);
    }
    m_Comms->SPI(mSPIbusAddr, mosi.data(), nullptr, mosi.size());
    mosi.clear();

    msb = lsb = i = 0;
    while (i <= (uint16_t)((Filt_N)-1))
    {
        addr = (maddressf0 << 6) + (msb << 4) + lsb;
        data = (uint16_t)coefficients[i];
        mosi.push_back((1 << 31) | addr << 16 | data);

        addr = (maddressf1 << 6) + (msb << 4) + lsb;
        data = (uint16_t)coefficients[i];
        mosi.push_back((1 << 31) | addr << 16 | data);
        if (lsb >= NN) // 15
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;
        i++;
    }
    m_Comms->SPI(mSPIbusAddr, mosi.data(), nullptr, mosi.size());
    mosi.clear();

    WriteRegister(ODD_FIR, Filt_N % 2);
    WriteRegister(SLEEP_FIR, 0);
}

void Equalizer::SetOversample(uint8_t oversample)
{
    // treat oversample 0 as "auto", maximum available oversample
    const uint8_t maxOversample = 2;
    oversample = std::min(oversample, maxOversample);

    const bool useOversample = oversample != 1;
    // TODO: batch writes
    for (uint8_t ch = 0; ch < 2; ++ch)
    {
        WriteRegister(MAC, ch + 1);
        WriteRegister(TX_HB_BYP, !useOversample);
        WriteRegister(TX_HB_DEL, useOversample);
    }
}

uint8_t Equalizer::GetOversample()
{
    const int ch = 0;
    WriteRegister(MAC, ch + 1);
    int bypass = ReadRegister(TX_HB_BYP);
    int delay = ReadRegister(TX_HB_DEL);
    // TODO: Warn if bypass and delay are incompatible
    return (delay && !bypass) ? 2 : 1;
}
