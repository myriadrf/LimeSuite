#include "LMS7002M_SDRDevice.h"

#include "DeviceExceptions.h"
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"
#include "mcu_program/common_src/lms7002m_calibrations.h"
#include "mcu_program/common_src/lms7002m_filters.h"
#include "lms7002m/MCU_BD.h"
#include "LMSBoards.h"
#include "Logger.h"
#include "TRXLooper.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>

namespace lime {

#ifdef NEW_GAIN_BEHAVIOUR
constexpr static int MAXIMUM_GAIN_VALUE = 62; // Gain table size
// clang-format off
// LNA table
constexpr static std::array<unsigned int, MAXIMUM_GAIN_VALUE> LNATable = {
    0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,
    5,  5,  6,  6,  6,  7,  7,  7,  8,  9,  10, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 12, 13, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14
};
// PGA table
constexpr static std::array<unsigned int, MAXIMUM_GAIN_VALUE> PGATable = {
    0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,
    1,  2,  0,  1,  2,  0,  1,  2,  0,  0,  0,  0,  1,  2,  3,  4,
    5,  6,  7,  8,  9,  10, 11, 12, 12, 12, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
// clang-format on
#else
constexpr static int MAXIMUM_GAIN_VALUE = 74;
// clang-format off
// LNA table
constexpr static std::array<unsigned int, MAXIMUM_GAIN_VALUE> LNATable = {
    0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,
    5,  5,  6,  6,  6,  7,  7,  7,  8,  9,  10, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 12, 13, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14
};
// PGA table
constexpr static std::array<unsigned int, MAXIMUM_GAIN_VALUE> PGATable = {
    0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,
    1,  2,  0,  1,  2,  0,  1,  2,  0,  0,  0,  0,  1,  2,  3,  4,
    5,  6,  7,  8,  9,  10, 11, 12, 12, 12, 12, 4,  5,  6,  7,  8,
    9,  10, 11, 12, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
// clang-format on
#endif

LMS7002M_SDRDevice::LMS7002M_SDRDevice()
    : mCallback_logData(nullptr)
    , mCallback_logMessage(nullptr)
{
}

LMS7002M_SDRDevice::~LMS7002M_SDRDevice()
{
    for (LMS7002M* soc : mLMSChips)
    {
        if (soc != nullptr)
        {
            delete soc;
        }
    }

    for (auto& streamer : mStreamers)
    {
        if (streamer != nullptr)
        {
            if (streamer->IsStreamRunning())
            {
                streamer->Stop();
            }

            delete streamer;
            streamer = nullptr;
        }
    }

    if (mFPGA != nullptr)
    {
        delete mFPGA;
    }
}

OpStatus LMS7002M_SDRDevice::EnableChannel(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool enable)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);
    return lms->EnableChannel(trx, channel % 2, enable);
}

void LMS7002M_SDRDevice::SetDataLogCallback(DataCallbackType callback)
{
    mCallback_logData = callback;
}

void LMS7002M_SDRDevice::SetMessageLogCallback(LogCallbackType callback)
{
    mCallback_logMessage = callback;
}

const SDRDevice::Descriptor& LMS7002M_SDRDevice::GetDescriptor() const
{
    return mDeviceDescriptor;
}

OpStatus LMS7002M_SDRDevice::Reset()
{
    OpStatus status;
    for (auto iter : mLMSChips)
    {
        status = iter->ResetChip();
        if (status != OpStatus::SUCCESS)
            return status;
    }
    return OpStatus::SUCCESS;
}

OpStatus LMS7002M_SDRDevice::GetGPSLock(GPS_Lock* status)
{
    uint16_t regValue = mFPGA->ReadRegister(0x114);
    status->glonass = static_cast<GPS_Lock::LockStatus>((regValue >> 0) & 0x3);
    status->gps = static_cast<GPS_Lock::LockStatus>((regValue >> 4) & 0x3);
    status->beidou = static_cast<GPS_Lock::LockStatus>((regValue >> 8) & 0x3);
    status->galileo = static_cast<GPS_Lock::LockStatus>((regValue >> 12) & 0x3);
    // TODO: not all boards have GPS
    return OpStatus::SUCCESS;
}

double LMS7002M_SDRDevice::GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (moduleIndex >= mLMSChips.size())
    {
        ReportError(OpStatus::OUT_OF_RANGE, "GetSample rate invalid module index (%i)", moduleIndex);
        return 0;
    }
    return mLMSChips[moduleIndex]->GetSampleRate(trx, LMS7002M::Channel::ChA);
}

double LMS7002M_SDRDevice::GetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    // TODO:
    // double offset = GetNCOOffset(moduleIndex, trx, channel);

    if (trx == TRXDir::Rx)
    {
        lms->Modify_SPI_Reg_bits(LMS7_MAC, 1); // Sets the current channel to channel A
        if (lms->Get_SPI_Reg_bits(LMS7_PD_VCO) == 1)
        {
            trx = TRXDir::Tx; // Assume that Tx PLL used for TX and RX
        }
    }
    return lms->GetFrequencySX(trx); // - offset;
}

OpStatus LMS7002M_SDRDevice::SetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double frequency)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    int chA = channel & (~1);
    int chB = channel | 1;

    auto channelAFrequency = GetFrequency(moduleIndex, trx, chA);
    auto channelBFrequency = GetFrequency(moduleIndex, trx, chB);

    auto channelANCOFrequency = GetNCOFrequency(moduleIndex, trx, chA, 0);
    auto channelBNCOFrequency = GetNCOFrequency(moduleIndex, trx, chB, 0);

    auto channelANCOOffset = channelAFrequency - channelANCOFrequency;
    auto channelBNCOOffset = channelBFrequency - channelBNCOFrequency;

    auto channelOffset = channel == chA ? channelANCOOffset : channelBNCOOffset;

    auto setTDD = [&](double center) {
        TRXDir otherDir = trx == TRXDir::Rx ? TRXDir::Tx : TRXDir::Rx;
        auto otherFrequency = GetFrequency(moduleIndex, otherDir, chA);
        auto otherOffset = GetNCOOffset(moduleIndex, otherDir, chA);

        bool tdd = std::fabs(otherFrequency + otherOffset - center) > 0.1 ? false : true;
        lms->EnableSXTDD(tdd);

        if (trx == TRXDir::Tx || (!tdd))
        {
            if (lms->SetFrequencySX(trx, center) != OpStatus::SUCCESS)
            {
                throw std::runtime_error("Setting TDD failed (failed SetFrequencySX)");
            }
        }

        return;
    };

    if (channel == chA)
    {
        channelAFrequency = frequency;
    }
    else
    {
        channelBFrequency = frequency;
    }

    if (channelAFrequency > 0 && channelBFrequency > 0)
    {
        double delta = std::fabs(channelAFrequency - channelBFrequency);
        if (delta > 0.1)
        {
            double rate = GetSampleRate(moduleIndex, trx, channel);
            if ((delta <= rate * 31) && (delta + rate <= 160e6))
            {
                double center = (channelAFrequency + channelBFrequency) / 2;
                if (center < 30e6)
                {
                    center = 30e6;
                }
                channelANCOOffset = center - channelAFrequency;
                channelBNCOOffset = center - channelBFrequency;

                setTDD(center);

                return SetSampleRate(moduleIndex, trx, channel, rate, 2);
            }
        }
    }

    if (frequency < 30e6)
    {
        setTDD(30e6);

        channelOffset = 30e6 - frequency;
        double rate = GetSampleRate(moduleIndex, trx, channel);
        if (channelOffset + rate / 2.0 >= rate / 2.0)
        {
            return SetSampleRate(moduleIndex, trx, channel, rate, 2);
        }
        else
        {
            SetNCOFrequency(moduleIndex, trx, channel, 0, channelOffset * (trx == TRXDir::Tx ? -1.0 : 1.0));
        }
    }

    if (channelOffset != 0)
    {
        SetNCOFrequency(moduleIndex, trx, channel, -1, 0.0);
    }

    setTDD(frequency);
    return OpStatus::SUCCESS;
}

double LMS7002M_SDRDevice::GetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    return GetFrequency(moduleIndex, trx, channel) - GetNCOFrequency(moduleIndex, trx, channel, 0);
}

double LMS7002M_SDRDevice::GetNCOFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    lms->SetActiveChannel(channel == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);
    double freq = lms->GetNCOFrequency(trx, index, true);

    bool down = lms->Get_SPI_Reg_bits(trx == TRXDir::Tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP);
    if (!(trx == TRXDir::Tx) && (lms->Get_SPI_Reg_bits(LMS7_MASK) == 0))
    {
        down = !down;
    }
    return down ? -freq : freq;
}

OpStatus LMS7002M_SDRDevice::SetNCOFrequency(
    uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index, double frequency, double phaseOffset)
{
    if (index > 15)
        return ReportError(OpStatus::OUT_OF_RANGE, "%s NCO%i index invalid", ToCString(trx), index);

    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    lms->SetActiveChannel(channel == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);

    bool enable = frequency != 0;
    bool tx = trx == TRXDir::Tx;

    if ((lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, !enable) != OpStatus::SUCCESS) ||
        (lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP, enable) != OpStatus::SUCCESS))
    {
        return ReportError(OpStatus::ERROR, "Failed to set %s NCO%i frequency", ToCString(trx), index);
    }

    OpStatus status = lms->SetNCOFrequency(trx, index, std::fabs(frequency));
    if (status != OpStatus::SUCCESS)
        return status;

    if (enable)
    {
        bool down = frequency < 0;
        if ((!tx) && (lms->Get_SPI_Reg_bits(LMS7_MASK) == 0))
        {
            down = !down;
        }

        if ((lms->Modify_SPI_Reg_bits(tx ? LMS7_SEL_TX : LMS7_SEL_RX, index) != OpStatus::SUCCESS) ||
            (lms->Modify_SPI_Reg_bits(tx ? LMS7_MODE_TX : LMS7_MODE_RX, 0) != OpStatus::SUCCESS) ||
            (lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP, down) != OpStatus::SUCCESS))
        {
            return ReportError(OpStatus::ERROR, "Failed to set %s NCO%i frequency", ToCString(trx), index);
        }
    }

    if (phaseOffset != -1.0)
        return lms->SetNCOPhaseOffsetForMode0(trx, phaseOffset);
    return OpStatus::SUCCESS;
}

double LMS7002M_SDRDevice::GetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    return lowPassFilterCache[trx][channel]; // Default initializes to 0
}

OpStatus LMS7002M_SDRDevice::SetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double lpf)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    LMS7002M::Channel ch = channel == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB;

    lms->SetActiveChannel(ch);

    const auto& bw_range = mDeviceDescriptor.rfSOC.at(moduleIndex).lowPassFilterRange.at(trx);
    bool tx = trx == TRXDir::Tx;

    if (lpf < 0)
    {
        lpf = lowPassFilterCache[trx][channel]; // Default initializes to 0
    }

    double newLPF = std::clamp(lpf, bw_range.min, bw_range.max);

    if (newLPF != lpf)
    {
        lime::warning("%cXLPF set to %.3f MHz (requested %0.3f MHz [out of range])", tx ? 'T' : 'R', newLPF / 1e6, lpf / 1e6);
    }

    lpf = newLPF;
    lowPassFilterCache[trx][channel] = lpf;

    OpStatus status = OpStatus::SUCCESS;
    if (tx)
    {
        int gain = lms->GetTBBIAMP_dB(ch);
        status = lms->TuneTxFilter(lpf);
        lms->SetTBBIAMP_dB(gain, ch);
    }
    else
    {
        status = lms->TuneRxFilter(lpf);
    }

    if (status != OpStatus::SUCCESS)
        return status;

    lime::info("%s LPF configured", ToCString(trx));
    return OpStatus::SUCCESS;
}

uint8_t LMS7002M_SDRDevice::GetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    auto lms = mLMSChips.at(moduleIndex);

    if (trx == TRXDir::Tx)
    {
        return lms->GetBandTRF();
    }
    else
    {
        return static_cast<uint8_t>(lms->GetPathRFE());
    }
}

OpStatus LMS7002M_SDRDevice::SetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t path)
{
    if (path >= mDeviceDescriptor.rfSOC.at(0).pathNames.size())
    {
        path = trx == TRXDir::Tx ? 1 : 2; // Default settings: Rx: LNAL, Tx: Band1
    }

    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    return lms->SetPath(trx, channel % 2, path);
}

OpStatus LMS7002M_SDRDevice::Calibrate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double bandwidth)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);
    lms->SetActiveChannel(static_cast<LMS7002M::Channel>((channel % 2) + 1));
    OpStatus ret;
    auto reg20 = lms->SPI_read(0x20);
    lms->SPI_write(0x20, reg20 | (20 << (channel % 2)));
    if (trx == TRXDir::Tx)
    {
        ret = lms->CalibrateTx(bandwidth, false);
    }
    else
    {
        ret = lms->CalibrateRx(bandwidth, false);
    }
    lms->SPI_write(0x20, reg20);
    return ret;
}

OpStatus LMS7002M_SDRDevice::ConfigureGFIR(
    uint8_t moduleIndex, TRXDir trx, uint8_t channel, ChannelConfig::Direction::GFIRFilter settings)
{
    LMS7002M* lms = mLMSChips.at(moduleIndex);
    return lms->SetGFIRFilter(trx, channel, settings.enabled, settings.bandwidth);
}

OpStatus LMS7002M_SDRDevice::SetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double value)
{
    auto device = mLMSChips.at(moduleIndex);
    LMS7002M::Channel enumChannel = channel > 0 ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA;

    switch (gain)
    {
    case eGainTypes::LNA:
        return device->SetRFELNA_dB(value, enumChannel);
    case eGainTypes::LoopbackLNA:
        return device->SetRFELoopbackLNA_dB(value, enumChannel);
    case eGainTypes::PGA:
        return device->SetRBBPGA_dB(value, enumChannel);
    case eGainTypes::TIA:
        return device->SetRFETIA_dB(value, enumChannel);
    case eGainTypes::PAD:
        return device->SetTRFPAD_dB(value, enumChannel);
    case eGainTypes::IAMP:
        return device->SetTBBIAMP_dB(value, enumChannel);
    case eGainTypes::LoopbackPAD:
        return device->SetTRFLoopbackPAD_dB(value, enumChannel);
    case eGainTypes::PA:
        // TODO: implement
        return OpStatus::ERROR;
    case eGainTypes::UNKNOWN:
    default:
        if (TRXDir::Tx == direction)
        {
            return SetGenericTxGain(device, enumChannel, value);
        }

        return SetGenericRxGain(device, enumChannel, value);
    }
}

OpStatus LMS7002M_SDRDevice::SetGenericTxGain(lime::LMS7002M* device, LMS7002M::Channel channel, double value)
{
    if (device->SetTRFPAD_dB(value, channel) != OpStatus::SUCCESS)
        return OpStatus::ERROR;

#ifdef NEW_GAIN_BEHAVIOUR
    if (value <= 0)
    {
        return device->Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    }

    if (device->GetTBBIAMP_dB(channel) < 0.0)
    {
        return device->CalibrateTxGain(0, nullptr);
    }
#else
    value -= device->GetTRFPAD_dB(channel);
    if (device->SetTBBIAMP_dB(value, channel) != OpStatus::SUCCESS)
    {
        return OpStatus::ERROR;
    }
#endif
    return OpStatus::SUCCESS;
}

OpStatus LMS7002M_SDRDevice::SetGenericRxGain(lime::LMS7002M* device, LMS7002M::Channel channel, double value)
{
    value = std::clamp(static_cast<int>(value + 12), 0, MAXIMUM_GAIN_VALUE - 1);

    unsigned int lna = LNATable.at(std::lround(value));
    unsigned int pga = PGATable.at(std::lround(value));

    unsigned int tia = 0;
#ifdef NEW_GAIN_BEHAVIOUR
    if (value > 0)
    {
        tia = 1;
    }
#else
    // TIA table
    if (value > 51)
    {
        tia = 2;
    }
    else if (value > 42)
    {
        tia = 1;
    }
#endif
    int rcc_ctl_pga_rbb = (430 * (pow(0.65, pga / 10.0)) - 110.35) / 20.4516 + 16; // From datasheet

    if ((device->Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), lna + 1) != OpStatus::SUCCESS) ||
        (device->Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), tia + 1) != OpStatus::SUCCESS) ||
        (device->Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), pga) != OpStatus::SUCCESS) ||
        (device->Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB), rcc_ctl_pga_rbb) != OpStatus::SUCCESS))
    {
        return OpStatus::IO_FAILURE;
    }

    return OpStatus::SUCCESS;
}

OpStatus LMS7002M_SDRDevice::GetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double& value)
{
    auto device = mLMSChips.at(moduleIndex);
    LMS7002M::Channel enumChannel = channel > 0 ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA;

    switch (gain)
    {
    case eGainTypes::LNA:
        value = device->GetRFELNA_dB(enumChannel);
        return OpStatus::SUCCESS;
    case eGainTypes::LoopbackLNA:
        value = device->GetRFELoopbackLNA_dB(enumChannel);
        return OpStatus::SUCCESS;
    case eGainTypes::PGA:
        value = device->GetRBBPGA_dB(enumChannel);
        return OpStatus::SUCCESS;
    case eGainTypes::TIA:
        value = device->GetRFETIA_dB(enumChannel);
        return OpStatus::SUCCESS;
    case eGainTypes::PAD:
        value = device->GetTRFPAD_dB(enumChannel);
        return OpStatus::SUCCESS;
    case eGainTypes::IAMP:
        value = device->GetTBBIAMP_dB(enumChannel);
        return OpStatus::SUCCESS;
    case eGainTypes::LoopbackPAD:
        value = device->GetTRFLoopbackPAD_dB(enumChannel);
        return OpStatus::SUCCESS;
    case eGainTypes::PA:
        // TODO: implement
        return OpStatus::ERROR;
    case eGainTypes::UNKNOWN:
    default:
#ifdef NEW_GAIN_BEHAVIOUR
        if (TRXDir::Tx == direction)
        {
            value = device->GetTRFPAD_dB(enumChannel);
        }
        else
        {
            value = device->GetRFELNA_dB(enumChannel) + device->GetRBBPGA_dB(enumChannel);
        }
#else
        if (TRXDir::Tx == direction)
        {
            value = device->GetTRFPAD_dB(enumChannel) + device->GetTBBIAMP_dB(enumChannel);
        }
        else
        {
            value = device->GetRFELNA_dB(enumChannel) + device->GetRFETIA_dB(enumChannel) + device->GetRBBPGA_dB(enumChannel);
        }
#endif
        return OpStatus::SUCCESS;
    }
}

bool LMS7002M_SDRDevice::GetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (trx == TRXDir::Rx)
    {
        auto lms = mLMSChips.at(moduleIndex);
        return lms->Get_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), channel) == 0;
    }

    return false;
}

OpStatus LMS7002M_SDRDevice::SetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool isAutomatic)
{
    if (trx == TRXDir::Tx)
        return OpStatus::NOT_SUPPORTED;

    auto lms = mLMSChips.at(moduleIndex);
    return lms->Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), isAutomatic == 0, channel);
}

complex64f_t LMS7002M_SDRDevice::GetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    double I = 0.0;
    double Q = 0.0;

    auto lms = mLMSChips.at(moduleIndex);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), (channel % 2) + 1);
    lms->GetDCOffset(trx, I, Q);
    return { I, Q };
}

OpStatus LMS7002M_SDRDevice::SetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& offset)
{
    auto lms = mLMSChips.at(moduleIndex);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), (channel % 2) + 1);
    return lms->SetDCOffset(trx, offset.i, offset.q);
}

complex64f_t LMS7002M_SDRDevice::GetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    auto lms = mLMSChips.at(moduleIndex);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), (channel % 2) + 1);

    double phase = 0.0, gainI = 0.0, gainQ = 0.0;
    lms->GetIQBalance(trx, phase, gainI, gainQ);
    auto balance = (gainI / gainQ) * std::polar(1.0, phase);
    return { balance.real(), balance.imag() };
}

OpStatus LMS7002M_SDRDevice::SetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& balance)
{
    std::complex<double> bal{ balance.i, balance.q };
    double gain = std::abs(bal);

    double gainI = 1.0;
    if (gain < 1.0)
    {
        gainI = gain;
    }

    double gainQ = 1.0;
    if (gain > 1.0)
    {
        gainQ = 1.0 / gain;
    }

    auto lms = mLMSChips.at(moduleIndex);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), (channel % 2) + 1);
    return lms->SetIQBalance(trx, std::arg(bal), gainI, gainQ);
}

bool LMS7002M_SDRDevice::GetCGENLocked(uint8_t moduleIndex)
{
    auto lms = mLMSChips.at(moduleIndex);
    return lms->GetCGENLocked();
}

double LMS7002M_SDRDevice::GetTemperature(uint8_t moduleIndex)
{
    auto lms = mLMSChips.at(moduleIndex);
    return lms->GetTemperature();
}

bool LMS7002M_SDRDevice::GetSXLocked(uint8_t moduleIndex, TRXDir trx)
{
    auto lms = mLMSChips.at(moduleIndex);
    return lms->GetSXLocked(trx);
}

unsigned int LMS7002M_SDRDevice::ReadRegister(uint8_t moduleIndex, unsigned int address, bool useFPGA)
{
    if (useFPGA)
    {
        return ReadFPGARegister(address);
    }

    auto lms = mLMSChips.at(moduleIndex);
    return lms->SPI_read(address);
}

OpStatus LMS7002M_SDRDevice::WriteRegister(uint8_t moduleIndex, unsigned int address, unsigned int value, bool useFPGA)
{
    if (useFPGA)
        return WriteFPGARegister(address, value);

    return mLMSChips.at(moduleIndex)->SPI_write(address, value);
}

OpStatus LMS7002M_SDRDevice::LoadConfig(uint8_t moduleIndex, const std::string& filename)
{
    auto lms = mLMSChips.at(moduleIndex);
    return lms->LoadConfig(filename);
}

OpStatus LMS7002M_SDRDevice::SaveConfig(uint8_t moduleIndex, const std::string& filename)
{
    auto lms = mLMSChips.at(moduleIndex);
    return lms->SaveConfig(filename);
}

uint16_t LMS7002M_SDRDevice::GetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey)
{
    auto lms = mLMSChips.at(moduleIndex);
    lms->SetActiveChannel(channel % 2 == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);

    try
    {
        uint16_t val = lms->Get_SPI_Reg_bits(lms->GetParam(parameterKey));
        return val;
    } catch (...)
    {
        throw std::runtime_error("failure getting key: " + parameterKey);
    }
}

OpStatus LMS7002M_SDRDevice::SetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey, uint16_t value)
{
    auto lms = mLMSChips.at(moduleIndex);
    lms->SetActiveChannel(channel % 2 == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);
    return lms->Modify_SPI_Reg_bits(lms->GetParam(parameterKey), value);
}

uint16_t LMS7002M_SDRDevice::GetParameter(uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb)
{
    auto lms = mLMSChips.at(moduleIndex);
    lms->SetActiveChannel(channel % 2 == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);

    try
    {
        uint16_t val = lms->Get_SPI_Reg_bits(address, msb, lsb);
        return val;
    } catch (...)
    {
        // TODO: fix return
        throw std::runtime_error("failure setting parameter: " + address);
    }
}

OpStatus LMS7002M_SDRDevice::SetParameter(
    uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb, uint16_t value)
{
    auto lms = mLMSChips.at(moduleIndex);
    lms->SetActiveChannel(channel % 2 == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);
    return lms->Modify_SPI_Reg_bits(address, msb, lsb, value);
}

OpStatus LMS7002M_SDRDevice::Synchronize(bool toChip)
{
    OpStatus status = OpStatus::SUCCESS;
    for (auto iter : mLMSChips)
    {
        status = toChip ? iter->UploadAll() : iter->DownloadAll();
        if (status != OpStatus::SUCCESS)
            return status;
    }
    return status;
}

void LMS7002M_SDRDevice::EnableCache(bool enable)
{
    for (auto iter : mLMSChips)
        iter->EnableValuesCache(enable);
    if (mFPGA)
        mFPGA->EnableValuesCache(enable);
}

void* LMS7002M_SDRDevice::GetInternalChip(uint32_t index)
{
    if (index >= mLMSChips.size())
        throw std::logic_error("Invalid chip index");
    return mLMSChips[index];
}

uint64_t LMS7002M_SDRDevice::GetHardwareTimestamp(uint8_t moduleIndex)
{
    return mStreamers.at(moduleIndex)->GetHardwareTimestamp();
}

OpStatus LMS7002M_SDRDevice::SetHardwareTimestamp(uint8_t moduleIndex, const uint64_t now)
{
    // TODO: return status
    mStreamers.at(moduleIndex)->SetHardwareTimestamp(now);
    return OpStatus::SUCCESS;
}

OpStatus LMS7002M_SDRDevice::SetTestSignal(uint8_t moduleIndex,
    TRXDir direction,
    uint8_t channel,
    SDRDevice::ChannelConfig::Direction::TestSignal signalConfiguration,
    int16_t dc_i,
    int16_t dc_q)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    switch (direction)
    {
    case TRXDir::Rx:
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), signalConfiguration.enabled, true) != OpStatus::SUCCESS)
            return ReportError(OpStatus::IO_FAILURE, "Failed to set Rx test signal.");

        lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), static_cast<uint8_t>(signalConfiguration.divide));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), static_cast<uint8_t>(signalConfiguration.scale));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP), signalConfiguration.dcMode);
        break;
    case TRXDir::Tx:
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), signalConfiguration.enabled, true) != OpStatus::SUCCESS)
            return ReportError(OpStatus::IO_FAILURE, "Failed to set Tx test signal.");

        lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), static_cast<uint8_t>(signalConfiguration.divide));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), static_cast<uint8_t>(signalConfiguration.scale));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), signalConfiguration.dcMode);
        break;
    }

    if (signalConfiguration.dcMode)
        return lms->LoadDC_REG_IQ(direction, dc_i, dc_q);

    return OpStatus::SUCCESS;
}

SDRDevice::ChannelConfig::Direction::TestSignal LMS7002M_SDRDevice::GetTestSignal(
    uint8_t moduleIndex, TRXDir direction, uint8_t channel)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);
    ChannelConfig::Direction::TestSignal signalConfiguration;

    switch (direction)
    {
    case TRXDir::Tx:
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_TXTSP)) == 0)
        {
            return signalConfiguration;
        }
        signalConfiguration.enabled = true;

        if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP)) != 0)
        {
            signalConfiguration.dcMode = true;
            return signalConfiguration;
        }

        signalConfiguration.divide =
            static_cast<ChannelConfig::Direction::TestSignal::Divide>(lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP)));
        signalConfiguration.scale =
            static_cast<ChannelConfig::Direction::TestSignal::Scale>(lms->Get_SPI_Reg_bits(LMS7param(TSGFC_TXTSP)));

        return signalConfiguration;
    case TRXDir::Rx:
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_RXTSP)) == 0)
        {
            return signalConfiguration;
        }
        signalConfiguration.enabled = true;

        if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP)) != 0)
        {
            signalConfiguration.dcMode = true;
            return signalConfiguration;
        }

        signalConfiguration.divide =
            static_cast<ChannelConfig::Direction::TestSignal::Divide>(lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP)));
        signalConfiguration.scale =
            static_cast<ChannelConfig::Direction::TestSignal::Scale>(lms->Get_SPI_Reg_bits(LMS7param(TSGFC_RXTSP)));

        return signalConfiguration;
    }

    throw std::runtime_error("Failed to get test mode");
}

std::vector<double> LMS7002M_SDRDevice::GetGFIRCoefficients(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    const uint8_t count = gfirID == 2 ? 120 : 40;
    std::vector<double> coefficientBuffer(count);

    lms->GetGFIRCoefficients(trx, gfirID, coefficientBuffer.data(), count);

    return coefficientBuffer;
}

OpStatus LMS7002M_SDRDevice::SetGFIRCoefficients(
    uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, std::vector<double> coefficients)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);
    return lms->SetGFIRCoefficients(trx, gfirID, coefficients.data(), coefficients.size());
}

OpStatus LMS7002M_SDRDevice::SetGFIR(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, bool enabled)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);

    if (gfirID > 2)
        return ReportError(OpStatus::OUT_OF_RANGE, "Failed to set GFIR filter, invalid filter index %i.", gfirID);

    std::vector<std::reference_wrapper<const LMS7Parameter>> txGfirBypasses = {
        LMS7_GFIR1_BYP_TXTSP, LMS7_GFIR2_BYP_TXTSP, LMS7_GFIR3_BYP_TXTSP
    };
    std::vector<std::reference_wrapper<const LMS7Parameter>> rxGfirBypasses = {
        LMS7_GFIR1_BYP_RXTSP, LMS7_GFIR2_BYP_RXTSP, LMS7_GFIR3_BYP_RXTSP
    };

    lms->SetActiveChannel(static_cast<LMS7002M::Channel>((channel % 2) + 1));
    OpStatus status;
    if (trx == TRXDir::Tx)
        status = lms->Modify_SPI_Reg_bits(txGfirBypasses[gfirID], enabled == false);
    else
        status = lms->Modify_SPI_Reg_bits(rxGfirBypasses[gfirID], enabled == false);
    return status;
}

void LMS7002M_SDRDevice::StreamStart(uint8_t moduleIndex)
{
    mStreamers.at(moduleIndex)->Start();
}

void LMS7002M_SDRDevice::StreamStop(uint8_t moduleIndex)
{
    if (mStreamers.at(moduleIndex) == nullptr)
    {
        return;
    }

    mStreamers.at(moduleIndex)->Stop();

    if (mStreamers.at(moduleIndex) != nullptr)
    {
        delete mStreamers[moduleIndex];
    }

    mStreamers[moduleIndex] = nullptr;
}

int LMS7002M_SDRDevice::StreamRx(uint8_t moduleIndex, complex32f_t* const* dest, uint32_t count, StreamMeta* meta)
{
    return mStreamers[moduleIndex]->StreamRx(dest, count, meta);
}

int LMS7002M_SDRDevice::StreamRx(uint8_t moduleIndex, complex16_t* const* dest, uint32_t count, StreamMeta* meta)
{
    return mStreamers[moduleIndex]->StreamRx(dest, count, meta);
}

int LMS7002M_SDRDevice::StreamTx(uint8_t moduleIndex, const complex32f_t* const* samples, uint32_t count, const StreamMeta* meta)
{
    return mStreamers[moduleIndex]->StreamTx(samples, count, meta);
}

int LMS7002M_SDRDevice::StreamTx(uint8_t moduleIndex, const complex16_t* const* samples, uint32_t count, const StreamMeta* meta)
{
    return mStreamers[moduleIndex]->StreamTx(samples, count, meta);
}

void LMS7002M_SDRDevice::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx)
{
    TRXLooper* trx = mStreamers.at(moduleIndex);
    if (rx != nullptr)
    {
        *rx = trx->GetStats(TRXDir::Rx);
    }

    if (tx != nullptr)
    {
        *tx = trx->GetStats(TRXDir::Tx);
    }
}

OpStatus LMS7002M_SDRDevice::UploadMemory(
    eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
{
    return OpStatus::NOT_IMPLEMENTED;
}

OpStatus LMS7002M_SDRDevice::UpdateFPGAInterfaceFrequency(LMS7002M& soc, FPGA& fpga, uint8_t chipIndex)
{
    double fpgaTxPLL = soc.GetReferenceClk_TSP(TRXDir::Tx);
    int interp = soc.Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    if (interp != 7)
    {
        uint8_t siso = soc.Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        fpgaTxPLL /= std::pow(2, interp + siso);
    }
    double fpgaRxPLL = soc.GetReferenceClk_TSP(TRXDir::Rx);
    int dec = soc.Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    if (dec != 7)
    {
        uint8_t siso = soc.Get_SPI_Reg_bits(LMS7_LML2_SISODDR);
        fpgaRxPLL /= std::pow(2, dec + siso);
    }

    OpStatus status = fpga.SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, chipIndex);
    if (status != OpStatus::SUCCESS)
        return status;
    soc.ResetLogicregisters();
    return OpStatus::SUCCESS;
}

int LMS7002M_SDRDevice::ReadFPGARegister(uint32_t address)
{
    return mFPGA->ReadRegister(address);
}

OpStatus LMS7002M_SDRDevice::WriteFPGARegister(uint32_t address, uint32_t value)
{
    return mFPGA->WriteRegister(address, value);
}

void LMS7002M_SDRDevice::SetGainInformationInDescriptor(RFSOCDescriptor& descriptor)
{
    descriptor.gainValues[TRXDir::Rx][eGainTypes::LNA] = { { 1, 0 },
        { 2, 3 },
        { 3, 6 },
        { 4, 9 },
        { 5, 12 },
        { 6, 15 },
        { 7, 18 },
        { 8, 21 },
        { 9, 24 },
        { 10, 25 },
        { 11, 26 },
        { 12, 27 },
        { 13, 28 },
        { 14, 29 },
        { 15, 30 } };
    descriptor.gainValues[TRXDir::Rx][eGainTypes::TIA] = { { 1, 0 }, { 2, 9 }, { 3, 12 } };

    std::vector<GainValue> PGAParameter(32);
    for (uint8_t i = 0; i < PGAParameter.size(); ++i)
    {
        PGAParameter[i] = { i, static_cast<float>(i - 12) };
    }
    descriptor.gainValues[TRXDir::Rx][eGainTypes::PGA] = PGAParameter;

    std::vector<GainValue> IAMPParameter(63);
    for (uint8_t i = 1; i <= IAMPParameter.size(); ++i)
    {
        IAMPParameter[i - 1] = { i, static_cast<float>(i) };
    }
    descriptor.gainValues[TRXDir::Tx][eGainTypes::IAMP] = IAMPParameter;

    std::vector<GainValue> PADParameter(31);
    for (uint8_t i = 0; i < PADParameter.size(); ++i)
    {
        PADParameter[i] = { i, static_cast<float>(i) };
    }
    descriptor.gainValues[TRXDir::Tx][eGainTypes::PAD] = PADParameter;

    descriptor.gains[TRXDir::Rx] = {
        eGainTypes::LNA,
        eGainTypes::PGA,
        eGainTypes::TIA,
    };

    descriptor.gains[TRXDir::Tx] = {
        eGainTypes::PAD,
        eGainTypes::IAMP,
    };

    descriptor.gainRange[TRXDir::Rx][eGainTypes::LNA] = Range(0, 30);
    descriptor.gainRange[TRXDir::Rx][eGainTypes::LoopbackLNA] = Range(0, 40);
    descriptor.gainRange[TRXDir::Rx][eGainTypes::TIA] = Range(0, 12);
    descriptor.gainRange[TRXDir::Rx][eGainTypes::PGA] = Range(-12, 19);
    descriptor.gainRange[TRXDir::Tx][eGainTypes::PAD] = Range(0, 52);
    descriptor.gainRange[TRXDir::Tx][eGainTypes::LoopbackPAD] = Range(-4.3, 0);
    descriptor.gainRange[TRXDir::Tx][eGainTypes::IAMP] = Range(-12, 12);

#ifdef NEW_GAIN_BEHAVIOUR
    descriptor.gainRange[TRXDir::Rx][eGainTypes::UNKNOWN] = Range(-12, 49);
    descriptor.gainRange[TRXDir::Tx][eGainTypes::UNKNOWN] = Range(0, 52);
#else
    descriptor.gainRange[TRXDir::Rx][eGainTypes::UNKNOWN] = Range(-12, 61);
    descriptor.gainRange[TRXDir::Tx][eGainTypes::UNKNOWN] = Range(-12, 64);
#endif
}

OpStatus LMS7002M_SDRDevice::LMS7002LOConfigure(LMS7002M* chip, const SDRDevice::SDRConfig& cfg)
{
    bool rxUsed = false;
    bool txUsed = false;
    for (int i = 0; i < 2; ++i)
    {
        const SDRDevice::ChannelConfig& ch = cfg.channel[i];
        rxUsed |= ch.rx.enabled;
        txUsed |= ch.tx.enabled;
    }

    OpStatus status = OpStatus::SUCCESS;
    if (cfg.referenceClockFreq != 0)
    {
        status = chip->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, cfg.referenceClockFreq, 0);
        if (status != OpStatus::SUCCESS)
            return status;
    }

    const bool tddMode = cfg.channel[0].rx.centerFrequency == cfg.channel[0].tx.centerFrequency;
    chip->EnableSXTDD(tddMode);
    // Rx PLL is not used in TDD mode
    if (!tddMode && rxUsed && cfg.channel[0].rx.centerFrequency > 0)
    {
        status = chip->SetFrequencySX(TRXDir::Rx, cfg.channel[0].rx.centerFrequency);
        if (status != OpStatus::SUCCESS)
            return status;
    }
    if (txUsed && cfg.channel[0].tx.centerFrequency > 0)
    {
        status = chip->SetFrequencySX(TRXDir::Tx, cfg.channel[0].tx.centerFrequency);
        if (status != OpStatus::SUCCESS)
            return status;
    }
    return status;
}

OpStatus LMS7002M_SDRDevice::LMS7002ChannelConfigure(LMS7002M* chip, const SDRDevice::ChannelConfig& config, uint8_t channelIndex)
{
    const SDRDevice::ChannelConfig& ch = config;
    chip->SetActiveChannel((channelIndex & 1) ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA);

    chip->EnableChannel(TRXDir::Rx, channelIndex, ch.rx.enabled);
    chip->SetPathRFE(static_cast<LMS7002M::PathRFE>(ch.rx.path));
    if (static_cast<LMS7002M::PathRFE>(ch.rx.path) == LMS7002M::PathRFE::LB1 ||
        static_cast<LMS7002M::PathRFE>(ch.rx.path) == LMS7002M::PathRFE::LB2)
    {
        // TODO: confirm which should be used for loopback
        if (ch.rx.lpf > 0)
            chip->Modify_SPI_Reg_bits(LMS7_INPUT_CTL_PGA_RBB, 3); // baseband loopback
        else
            chip->Modify_SPI_Reg_bits(LMS7_INPUT_CTL_PGA_RBB, 2); // LPF bypass
    }

    chip->EnableChannel(TRXDir::Tx, channelIndex, ch.tx.enabled);
    chip->SetBandTRF(ch.tx.path);

    for (const auto& gain : ch.rx.gain)
    {
        SetGain(0, TRXDir::Rx, channelIndex, gain.first, gain.second);
    }

    for (const auto& gain : ch.tx.gain)
    {
        SetGain(0, TRXDir::Tx, channelIndex, gain.first, gain.second);
    }
    // TODO: set GFIR filters...
    return OpStatus::SUCCESS;
}

OpStatus LMS7002M_SDRDevice::LMS7002ChannelCalibration(LMS7002M* chip, const SDRDevice::ChannelConfig& config, uint8_t channelIndex)
{
    int i = channelIndex;
    chip->SetActiveChannel(i == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);
    const SDRDevice::ChannelConfig& ch = config;

    // TODO: Don't configure GFIR when external ADC/DAC is used
    if (ch.rx.enabled && chip->SetGFIRFilter(TRXDir::Rx, i, ch.rx.gfir.enabled, ch.rx.gfir.bandwidth) != OpStatus::SUCCESS)
        return lime::ReportError(OpStatus::ERROR, "Rx ch%i GFIR config failed", i);
    if (ch.tx.enabled && chip->SetGFIRFilter(TRXDir::Tx, i, ch.tx.gfir.enabled, ch.tx.gfir.bandwidth) != OpStatus::SUCCESS)
        return lime::ReportError(OpStatus::ERROR, "Tx ch%i GFIR config failed", i);

    if (ch.rx.calibrate && ch.rx.enabled)
    {
        SetupCalibrations(chip, ch.rx.sampleRate);
        int status = CalibrateRx(false, false);
        if (status != MCU_BD::MCU_NO_ERROR)
            return lime::ReportError(OpStatus::ERROR, "Rx ch%i DC/IQ calibration failed: %s", i, MCU_BD::MCUStatusMessage(status));
    }
    if (ch.tx.calibrate && ch.tx.enabled)
    {
        SetupCalibrations(chip, ch.tx.sampleRate);
        int status = CalibrateTx(false);
        if (status != MCU_BD::MCU_NO_ERROR)
            return lime::ReportError(OpStatus::ERROR, "Rx ch%i DC/IQ calibration failed: %s", i, MCU_BD::MCUStatusMessage(status));
    }
    if (ch.rx.lpf > 0 && ch.rx.enabled)
    {
        SetupCalibrations(chip, ch.rx.sampleRate);
        int status = TuneRxFilter(ch.rx.lpf);
        if (status != MCU_BD::MCU_NO_ERROR)
            return lime::ReportError(OpStatus::ERROR, "Rx ch%i filter calibration failed: %s", i, MCU_BD::MCUStatusMessage(status));
    }
    if (ch.tx.lpf > 0 && ch.tx.enabled)
    {
        SetupCalibrations(chip, ch.tx.sampleRate);
        int status = TuneTxFilter(ch.tx.lpf);
        if (status != MCU_BD::MCU_NO_ERROR)
            return lime::ReportError(OpStatus::ERROR, "Tx ch%i filter calibration failed: %s", i, MCU_BD::MCUStatusMessage(status));
    }
    return OpStatus::SUCCESS;
}

OpStatus LMS7002M_SDRDevice::LMS7002TestSignalConfigure(
    LMS7002M* chip, const SDRDevice::ChannelConfig& config, uint8_t channelIndex)
{
    const SDRDevice::ChannelConfig& ch = config;
    chip->Modify_SPI_Reg_bits(LMS7_INSEL_RXTSP, ch.rx.testSignal.enabled ? 1 : 0);
    if (ch.rx.testSignal.enabled)
    {
        const SDRDevice::ChannelConfig::Direction::TestSignal& signal = ch.rx.testSignal;
        bool fullscale = signal.scale == SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Full;
        bool div4 = signal.divide == SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div4;
        chip->Modify_SPI_Reg_bits(LMS7_TSGFC_RXTSP, fullscale ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7_TSGFCW_RXTSP, div4 ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7_TSGMODE_RXTSP, signal.dcMode ? 1 : 0);
        chip->SPI_write(0x040C, 0x01FF); // DC.. bypasss
        // LMS7_TSGMODE_RXTSP change resets DC values
        return chip->LoadDC_REG_IQ(TRXDir::Rx, signal.dcValue.real(), signal.dcValue.imag());
    }

    chip->Modify_SPI_Reg_bits(LMS7_INSEL_TXTSP, ch.tx.testSignal.enabled ? 1 : 0);
    if (ch.tx.testSignal.enabled)
    {
        const SDRDevice::ChannelConfig::Direction::TestSignal& signal = ch.tx.testSignal;
        bool fullscale = signal.scale == SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Full;
        bool div4 = signal.divide == SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div4;
        chip->Modify_SPI_Reg_bits(LMS7_TSGFC_TXTSP, fullscale ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7_TSGFCW_TXTSP, div4 ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7_TSGMODE_TXTSP, signal.dcMode ? 1 : 0);
        chip->SPI_write(0x040C, 0x01FF); // DC.. bypasss
        // LMS7_TSGMODE_TXTSP change resets DC values
        return chip->LoadDC_REG_IQ(TRXDir::Tx, signal.dcValue.real(), signal.dcValue.imag());
    }
    return OpStatus::SUCCESS;
}

} // namespace lime
