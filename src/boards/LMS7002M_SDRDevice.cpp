#include "LMS7002M_SDRDevice.h"

#include "DeviceExceptions.h"
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"
#include "LMSBoards.h"
#include "Logger.h"
#include "TRXLooper.h"

#include <array>
#include <cmath>

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

void LMS7002M_SDRDevice::EnableChannel(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool enable)
{
    lime::LMS7002M* lms = mLMSChips.at(channel / 2);
    lms->EnableChannel(trx, channel % 2, enable);
}

int LMS7002M_SDRDevice::SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    throw(OperationNotSupported("TransactSPI not implemented"));
}

int LMS7002M_SDRDevice::I2CWrite(int address, const uint8_t* data, uint32_t length)
{
    throw(OperationNotSupported("WriteI2C not implemented"));
}

int LMS7002M_SDRDevice::I2CRead(int addr, uint8_t* dest, uint32_t length)
{
    throw(OperationNotSupported("ReadI2C not implemented"));
}

int LMS7002M_SDRDevice::GPIOWrite(const uint8_t* buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIOWrite not implemented"));
}

int LMS7002M_SDRDevice::GPIORead(uint8_t* buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIORead not implemented"));
}

int LMS7002M_SDRDevice::GPIODirWrite(const uint8_t* buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirWrite not implemented"));
}

int LMS7002M_SDRDevice::GPIODirRead(uint8_t* buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirRead not implemented"));
}

int LMS7002M_SDRDevice::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    throw(OperationNotSupported("CustomParameterWrite not implemented"));
}

int LMS7002M_SDRDevice::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    throw(OperationNotSupported("CustomParameterRead not implemented"));
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

void LMS7002M_SDRDevice::Reset()
{
    for (auto iter : mLMSChips)
        iter->ResetChip();
}

void LMS7002M_SDRDevice::GetGPSLock(GPS_Lock* status)
{
    uint16_t regValue = mFPGA->ReadRegister(0x114);
    status->glonass = static_cast<GPS_Lock::LockStatus>((regValue >> 0) & 0x3);
    status->gps = static_cast<GPS_Lock::LockStatus>((regValue >> 4) & 0x3);
    status->beidou = static_cast<GPS_Lock::LockStatus>((regValue >> 8) & 0x3);
    status->galileo = static_cast<GPS_Lock::LockStatus>((regValue >> 12) & 0x3);
}

double LMS7002M_SDRDevice::GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (moduleIndex >= mLMSChips.size())
        throw std::logic_error("Invalid module index");
    return mLMSChips[moduleIndex]->GetSampleRate(trx, LMS7002M::Channel::ChA);
}

double LMS7002M_SDRDevice::GetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    lime::LMS7002M* lms = mLMSChips.at(channel / 2);

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

void LMS7002M_SDRDevice::SetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double frequency)
{
    lime::LMS7002M* lms = mLMSChips.at(channel / 2);

    int chA = channel & (~1);
    int chB = channel | 1;

    auto channelAFrequency = GetFrequency(moduleIndex, trx, chA);
    auto channelBFrequency = GetFrequency(moduleIndex, trx, chB);

    auto channelANCOFrequency = GetNCOFrequency(moduleIndex, trx, chA, 0);
    auto channelBNCOFrequency = GetNCOFrequency(moduleIndex, trx, chB, 0);

    auto channelANCOOffset = channelAFrequency - channelANCOFrequency;
    auto channelBNCOOffset = channelBFrequency - channelBNCOFrequency;

    auto channelOffset = channel == chA ? channelANCOOffset : channelBNCOOffset;

    // std::vector<ChannelInfo>& channels = isTx ? tx_channels : rx_channels;

    auto setTDD = [&](double center) {
        // std::vector<ChannelInfo>& other = isTx ? rx_channels : tx_channels;
        TRXDir otherDir = trx == TRXDir::Rx ? TRXDir::Tx : TRXDir::Rx;
        auto otherFrequency = GetFrequency(moduleIndex, otherDir, chA);
        auto otherOffset = GetNCOOffset(moduleIndex, otherDir, chA);

        bool tdd = std::fabs(otherFrequency + otherOffset - center) > 0.1 ? false : true;
        lms->EnableSXTDD(tdd);

        if (trx == TRXDir::Tx || (!tdd))
        {
            if (lms->SetFrequencySX(trx, center) != 0)
            {
                throw std::runtime_error("Setting TDD failed (failed SetFrequencySX)");
            }
        }

        return;
    };

    // channels[chan].freq = frequency;

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

                SetSampleRate(moduleIndex, trx, channel, rate, 2);
                // return -1;
                return;
            }
        }
    }

    if (frequency < 30e6)
    {
        setTDD(30e6);

        channelOffset = 30e6 - frequency;
        // double rf_rate;
        double rate = GetSampleRate(moduleIndex, trx, channel);
        if (channelOffset + rate / 2.0 >= rate / 2.0)
        {
            SetSampleRate(moduleIndex, trx, channel, rate, 2);
            // return -1;
            return;
        }
        else
        {
            auto returnValue = lms->SetNCOFrequency(trx, 0, channelOffset * (trx == TRXDir::Tx ? -1.0 : 1.0));
            if (returnValue != 0)
            {
                throw std::runtime_error("SetNCOFrequency failed");
            }
        }
    }

    if (channelOffset != 0)
    {
        lms->SetNCOFrequency(trx, -1, 0.0);
    }

    setTDD(frequency);
}

double LMS7002M_SDRDevice::GetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    // throw std::logic_error("GetNCOOffset not implemented currently. TODO: implement");
    return GetFrequency(moduleIndex, trx, channel) - GetNCOFrequency(moduleIndex, trx, channel, 0);
}

void LMS7002M_SDRDevice::SetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double offset)
{
    throw std::logic_error("SetNCOOffset not implemented currently. TODO: implement");
}

double LMS7002M_SDRDevice::GetNCOFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index)
{
    lime::LMS7002M* lms = mLMSChips.at(channel / 2);

    lms->SetActiveChannel(channel == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);
    double freq = lms->GetNCOFrequency(trx, index, true);

    bool down = lms->Get_SPI_Reg_bits(trx == TRXDir::Tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP);
    if (!(trx == TRXDir::Tx) && (lms->Get_SPI_Reg_bits(LMS7_MASK) == 0))
    {
        down = !down;
    }
    return down ? -freq : freq;
}

void LMS7002M_SDRDevice::SetNCOFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index, double frequency)
{
    lime::LMS7002M* lms = mLMSChips.at(channel / 2);

    lms->SetActiveChannel(channel == 0 ? LMS7002M::Channel::ChA : LMS7002M::Channel::ChB);

    bool enable = (index >= 0) && (frequency != 0);
    bool tx = trx == TRXDir::Tx;

    if ((lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, !enable) != 0) ||
        (lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP, enable) != 0))
    {
        throw std::runtime_error("Failure in LMS7002M_SDRDevice::SetNCOFrequency");
    }

    if ((index >= 0) && (lms->SetNCOFrequency(trx, index, std::fabs(frequency)) != 0))
    {
        throw std::runtime_error("Failure while in LMS7002M::SetNCOFrequency");
    }

    if (enable)
    {
        bool down = frequency < 0;
        if ((!tx) && (lms->Get_SPI_Reg_bits(LMS7_MASK) == 0))
        {
            down = !down;
        }

        if ((lms->Modify_SPI_Reg_bits(tx ? LMS7_SEL_TX : LMS7_SEL_RX, index) != 0) ||
            (lms->Modify_SPI_Reg_bits(tx ? LMS7_MODE_TX : LMS7_MODE_RX, 0) != 0) ||
            (lms->Modify_SPI_Reg_bits(tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP, down) != 0))
        {
            throw std::runtime_error("Failure in LMS7002M_SDRDevice::SetNCOFrequency");
        }
    }
}

double LMS7002M_SDRDevice::GetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    throw std::logic_error("GetLowPassFilter not implemented currently. TODO: implement");
}

void LMS7002M_SDRDevice::SetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double lpf)
{
    throw std::logic_error("SetLowPassFilter not implemented currently. TODO: implement");
}

uint8_t LMS7002M_SDRDevice::GetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    auto lms = mLMSChips.at(channel / 2);

    if (trx == TRXDir::Tx)
    {
        return static_cast<std::size_t>(lms->GetBandTRF());
    }
    else
    {
        return static_cast<std::size_t>(lms->GetPathRFE());
    }
}

void LMS7002M_SDRDevice::SetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t path)
{
    if (path >= mDeviceDescriptor.rfSOC.at(0).pathNames.size())
    {
        path = trx == TRXDir::Tx ? 1 : 2; // Default settings: Rx: LNAL, Tx: Band1
    }

    lime::LMS7002M* lms = mLMSChips.at(channel / 2);

    int returnValue = lms->SetPath(trx, channel % 2, path);
    if (returnValue != 0)
    {
        throw std::runtime_error("Failed to set path.");
    }
}

void LMS7002M_SDRDevice::Calibrate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double bandwidth)
{
    lime::LMS7002M* lms = mLMSChips.at(moduleIndex);
    lms->SetActiveChannel(static_cast<LMS7002M::Channel>((channel + 1) % 2));
    int ret;
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

    if (ret != 0)
    {
        throw std::runtime_error("Calibration failure");
    }
}

void LMS7002M_SDRDevice::ConfigureGFIR(
    uint8_t moduleIndex, TRXDir trx, uint8_t channel, ChannelConfig::Direction::GFIRFilter settings)
{
    LMS7002M* lms = mLMSChips.at(moduleIndex);
    int returnValue = lms->SetGFIRFilter(trx, channel, settings.enabled, settings.bandwidth);
    if (returnValue != 0)
    {
        throw std::runtime_error("Setting GFIR Filter failed");
    }
}

int LMS7002M_SDRDevice::SetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double value)
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
        return -1;
    case eGainTypes::UNKNOWN:
    default:
        if (TRXDir::Tx == direction)
        {
            return SetGenericTxGain(device, enumChannel, value);
        }

        return SetGenericRxGain(device, enumChannel, value);
    }
}

int LMS7002M_SDRDevice::SetGenericTxGain(lime::LMS7002M* device, LMS7002M::Channel channel, double value)
{
    if (device->SetTRFPAD_dB(value, channel) != 0)
    {
        return -1;
    }
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
    if (device->SetTBBIAMP_dB(value, channel) != 0)
    {
        return -1;
    }
#endif
    return 0;
}

int LMS7002M_SDRDevice::SetGenericRxGain(lime::LMS7002M* device, LMS7002M::Channel channel, double value)
{
    value = std::clamp(static_cast<int>(value + 12), 0, MAXIMUM_GAIN_VALUE - 1);

    unsigned int lna = LNATable[std::lround(value)];
    unsigned int pga = PGATable[std::lround(value)];

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

    if ((device->Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), lna + 1) != 0) ||
        (device->Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), tia + 1) != 0) ||
        (device->Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), pga) != 0) ||
        (device->Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB), rcc_ctl_pga_rbb) != 0))
    {
        return -1;
    }

    return 0;
}

int LMS7002M_SDRDevice::GetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double& value)
{
    auto device = mLMSChips.at(moduleIndex);
    LMS7002M::Channel enumChannel = channel > 0 ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA;

    switch (gain)
    {
    case eGainTypes::LNA:
        value = device->GetRFELNA_dB(enumChannel);
        return 0;
    case eGainTypes::LoopbackLNA:
        value = device->GetRFELoopbackLNA_dB(enumChannel);
        return 0;
    case eGainTypes::PGA:
        value = device->GetRBBPGA_dB(enumChannel);
        return 0;
    case eGainTypes::TIA:
        value = device->GetRFETIA_dB(enumChannel);
        return 0;
    case eGainTypes::PAD:
        value = device->GetTRFPAD_dB(enumChannel);
        return 0;
    case eGainTypes::IAMP:
        value = device->GetTBBIAMP_dB(enumChannel);
        return 0;
    case eGainTypes::LoopbackPAD:
        value = device->GetTRFLoopbackPAD_dB(enumChannel);
        return 0;
    case eGainTypes::PA:
        // TODO: implement
        return -1;
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
        return 0;
    }
}

void LMS7002M_SDRDevice::Synchronize(bool toChip)
{
    for (auto iter : mLMSChips)
    {
        if (toChip)
        {
            if (iter->UploadAll() == 0)
                iter->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true);
        }
        else
            iter->DownloadAll();
    }
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
    return mStreamers.at(0)->GetHardwareTimestamp();
}

void LMS7002M_SDRDevice::SetHardwareTimestamp(uint8_t moduleIndex, const uint64_t now)
{
    mStreamers.at(0)->SetHardwareTimestamp(now);
}

void LMS7002M_SDRDevice::SetTestSignal(uint8_t moduleIndex,
    TRXDir direction,
    uint8_t channel,
    SDRDevice::ChannelConfig::Direction::TestSignal signalConfiguration,
    int16_t dc_i,
    int16_t dc_q)
{
    lime::LMS7002M* lms = mLMSChips.at(channel / 2);

    switch (direction)
    {
    case TRXDir::Rx:
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), signalConfiguration.enabled, true) != 0)
        {
            throw std::runtime_error("Failed to set test mode");
        }

        lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), static_cast<uint8_t>(signalConfiguration.divide));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), static_cast<uint8_t>(signalConfiguration.scale));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP), signalConfiguration.dcMode);
        break;
    case TRXDir::Tx:
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), signalConfiguration.enabled, true) != 0)
        {
            throw std::runtime_error("Failed to set test mode");
        }

        lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), static_cast<uint8_t>(signalConfiguration.divide));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), static_cast<uint8_t>(signalConfiguration.scale));
        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), signalConfiguration.dcMode);
        break;
    }

    if (signalConfiguration.dcMode)
    {
        lms->LoadDC_REG_IQ(direction, dc_i, dc_q);
    }
}

SDRDevice::ChannelConfig::Direction::TestSignal LMS7002M_SDRDevice::GetTestSignal(
    uint8_t moduleIndex, TRXDir direction, uint8_t channel)
{
    lime::LMS7002M* lms = mLMSChips.at(channel / 2);
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

bool LMS7002M_SDRDevice::UploadMemory(
    eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
{
    throw(OperationNotSupported("UploadMemory not implemented"));
}

int LMS7002M_SDRDevice::UpdateFPGAInterfaceFrequency(LMS7002M& soc, FPGA& fpga, uint8_t chipIndex)
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

    if (fpga.SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, chipIndex) != 0)
        return -1;
    soc.ResetLogicregisters();
    return 0;
}

int LMS7002M_SDRDevice::ReadFPGARegister(uint32_t address)
{
    return mFPGA->ReadRegister(address);
}

int LMS7002M_SDRDevice::WriteFPGARegister(uint32_t address, uint32_t value)
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
    soc.gainRange[TRXDir::Rx][eGainTypes::UNKNOWN] = Range(-12, 49);
    soc.gainRange[TRXDir::Tx][eGainTypes::UNKNOWN] = Range(0, 52);
#else
    descriptor.gainRange[TRXDir::Rx][eGainTypes::UNKNOWN] = Range(-12, 61);
    descriptor.gainRange[TRXDir::Tx][eGainTypes::UNKNOWN] = Range(-12, 64);
#endif
}

} // namespace lime
