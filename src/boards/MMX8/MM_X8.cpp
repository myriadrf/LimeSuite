#include "MM_X8.h"

#include <fcntl.h>
#include <sstream>

#include "Logger.h"
#include "LitePCIe.h"
#include "limesuite/LMS7002M.h"
#include "lms7002m/LMS7002M_validation.h"
#include "FPGA_common.h"
#include "DSP/Equalizer.h"
#include "protocols/ADCUnits.h"

#include "boards/LimeSDR_XTRX/LimeSDR_XTRX.h"

#include "mcu_program/common_src/lms7002m_calibrations.h"
#include "mcu_program/common_src/lms7002m_filters.h"
#include "MCU_BD.h"

#include "math.h"

namespace lime
{

// X3 board specific subdevice ids
static constexpr uint8_t spi_LMS7002M_1 = 0;
static constexpr uint8_t spi_LMS7002M_2 = 1;
static constexpr uint8_t spi_LMS7002M_3 = 2;
static constexpr uint8_t spi_FPGA = 3;

static inline void ValidateChannel(uint8_t channel)
{
    if (channel > 2)
        throw std::logic_error("invalid channel index");
}

// Do not perform any unnecessary configuring to device in constructor, so you
// could read back it's state for debugging purposes
LimeSDR_MMX8::LimeSDR_MMX8(std::vector<lime::IComms*> &spiLMS7002M, std::vector<lime::IComms*> &spiFPGA, std::vector<lime::LitePCIe*> trxStreams)
    : mTRXStreamPorts(trxStreams)
{
    mMainFPGAcomms =  spiFPGA[8];
    SDRDevice::Descriptor &desc = mDeviceDescriptor;
    desc.name = GetDeviceName(LMS_DEV_LIMESDR_MMX8);

    // LMS64CProtocol::FirmwareInfo fw;
    // LMS64CProtocol::GetFirmwareInfo(controlPipe, fw);
    // LMS64CProtocol::FirmwareToDescriptor(fw, desc);

    // mFPGA = new lime::FPGA_X3(spiFPGA, spi_LMS7002M_1);
    // mFPGA->SetConnection(&mFPGAcomms);
    // FPGA::GatewareInfo gw = mFPGA->GetGatewareInfo();
    // FPGA::GatewareToDescriptor(gw, desc);

    mSubDevices.resize(8);
    desc.spiSlaveIds["FPGA"] = 0;
    desc.memoryDevices.push_back({"FPGA FLASH", (uint32_t)eMemoryDevice::FPGA_FLASH});
    for (size_t i=0; i<mSubDevices.size(); ++i)
    {
        mSubDevices[i] = new LimeSDR_XTRX(spiLMS7002M[i], spiFPGA[i], trxStreams[i]);
        const SDRDevice::Descriptor &d = mSubDevices[i]->GetDescriptor();

        for (const auto &s : d.spiSlaveIds)
        {
            char ctemp[512];
            sprintf(ctemp, "%s@%li", s.first.c_str(), i+1);
            desc.spiSlaveIds[ctemp] = (i+1) << 8 | s.second;
            chipSelectToDevice[desc.spiSlaveIds[ctemp]] = mSubDevices[i];
        }

        for (const auto &s : d.memoryDevices)
        {
            char ctemp[512];
            sprintf(ctemp, "%s@%li", s.name.c_str(), i+1);
            desc.memoryDevices.push_back({ctemp, (i+1) << 8 | s.id});
            memorySelectToDevice[(i+1) << 8 | s.id] = mSubDevices[i];
        }

        for (const auto &s : d.customParameters)
        {
            SDRDevice::CustomParameter p = s;
            p.id |= (i+1) << 8;
            char ctemp[512];
            sprintf(ctemp, "%s@%li", s.name.c_str(), i+1);
            p.name = ctemp;
            desc.customParameters.push_back(p);
            customParameterToDevice[p.id] = mSubDevices[i];
        }
    }
}

LimeSDR_MMX8::~LimeSDR_MMX8()
{
    for (size_t i=0; i<mSubDevices.size(); ++i)
        delete mSubDevices[i];
}

const SDRDevice::Descriptor& LimeSDR_MMX8::GetDescriptor()
{
    return mDeviceDescriptor;
}

void LimeSDR_MMX8::Configure(const SDRConfig& cfg, uint8_t socIndex)
{
    try
    {
        mSubDevices[socIndex]->Configure(cfg, 0);
    } //try
    catch (std::logic_error &e) {
        printf("LimeSDR_MMX8 config: %s\n", e.what());
        throw;
    }
    catch (std::runtime_error &e) {
        throw;
    }
}

int LimeSDR_MMX8::Init()
{
    for (size_t i=0; i<mSubDevices.size(); ++i)
        mSubDevices[i]->Init();
    return 0;
}

void LimeSDR_MMX8::Reset()
{
    for(uint32_t i=0; i<mSubDevices.size(); ++i)
        mSubDevices[i]->Reset();
}

double LimeSDR_MMX8::GetSampleRate(uint8_t moduleIndex, TRXDir trx)
{
    return mSubDevices[moduleIndex]->GetSampleRate(0, trx);
}

double LimeSDR_MMX8::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    return mSubDevices[channel/2]->GetClockFreq(clk_id, channel & 0x1);
}

void LimeSDR_MMX8::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    mSubDevices[channel/2]->SetClockFreq(clk_id, freq, channel & 1);
}

void LimeSDR_MMX8::Synchronize(bool toChip)
{
    for (auto &d : mSubDevices)
        d->Synchronize(toChip);
}

void LimeSDR_MMX8::EnableCache(bool enable)
{
    for (auto &d : mSubDevices)
        d->EnableCache(enable);
}

int LimeSDR_MMX8::StreamSetup(const StreamConfig &config, uint8_t moduleIndex)
{
    return mSubDevices[moduleIndex]->StreamSetup(config, 0);
}

void LimeSDR_MMX8::StreamStart(uint8_t moduleIndex)
{
    mSubDevices[moduleIndex]->StreamStart(0);
}

void LimeSDR_MMX8::StreamStop(uint8_t moduleIndex)
{
    mSubDevices[moduleIndex]->StreamStop(0);
}

int LimeSDR_MMX8::StreamRx(uint8_t moduleIndex, lime::complex32f_t** dest, uint32_t count, StreamMeta* meta)
{
    return mSubDevices[moduleIndex]->StreamRx(0, dest, count, meta);
}

int LimeSDR_MMX8::StreamRx(uint8_t moduleIndex, lime::complex16_t** dest, uint32_t count, StreamMeta* meta)
{
    return mSubDevices[moduleIndex]->StreamRx(0, dest, count, meta);
}

int LimeSDR_MMX8::StreamTx(uint8_t moduleIndex, const lime::complex32f_t* const* samples, uint32_t count, const StreamMeta* meta)
{
    return mSubDevices[moduleIndex]->StreamTx(0, samples, count, meta);
}

int LimeSDR_MMX8::StreamTx(uint8_t moduleIndex, const lime::complex16_t* const* samples, uint32_t count, const StreamMeta* meta)
{
    return mSubDevices[moduleIndex]->StreamTx(0, samples, count, meta);
}

void LimeSDR_MMX8::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx)
{
    mSubDevices[moduleIndex]->StreamStatus(0, rx, tx);
}

void LimeSDR_MMX8::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    if (chipSelect == 0)
    {
        mMainFPGAcomms->SPI(MOSI, MISO, count);
        return;
    }

    SDRDevice* dev = chipSelectToDevice.at(chipSelect);
    if (!dev)
    {
        throw std::logic_error("invalid SPI chip select");
        return;
    }

    uint32_t subSelect = chipSelect & 0xFF;
    dev->SPI(subSelect, MOSI, MISO, count);
}

int LimeSDR_MMX8::I2CWrite(int address, const uint8_t *data, uint32_t length)
{
    return -1;
}

int LimeSDR_MMX8::I2CRead(int addres, uint8_t *dest, uint32_t length)
{
    return -1;
}

void LimeSDR_MMX8::SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase, double rxPhase)
{
    for (size_t i=0; i<mSubDevices.size(); ++i)
        mSubDevices[i]->SetFPGAInterfaceFreq(interp, dec, txPhase, rxPhase);
}

int LimeSDR_MMX8::CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units)
{
    // PCIE_CSR_Pipe pipe(*mControlPort);
    // return LMS64CProtocol::CustomParameterWrite(pipe, ids, values, count, units);
    return -1;
}

int LimeSDR_MMX8::CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units)
{
    // PCIE_CSR_Pipe pipe(*mControlPort);
    // return LMS64CProtocol::CustomParameterRead(pipe, ids, values, count, units);
    return -1;
}

bool LimeSDR_MMX8::UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback)
{
    if (id == 0)
    {
        int progMode;
        LMS64CProtocol::ProgramWriteTarget target;
        target = LMS64CProtocol::ProgramWriteTarget::FPGA;
        progMode = 1;
        return mMainFPGAcomms->ProgramWrite(data, length, progMode, target, callback);
    }

    SDRDevice* dev = memorySelectToDevice.at(id);
    if (!dev)
    {
        throw std::logic_error("invalid id select");
        return false;
    }

    uint32_t subSelect = id & 0xFF;
    return dev->UploadMemory(subSelect, data, length, callback);
}

int LimeSDR_MMX8::UploadTxWaveform(const StreamConfig &config, uint8_t moduleIndex, const void** samples, uint32_t count)
{
    return mSubDevices[moduleIndex]->UploadTxWaveform(config, 0, samples, count);
}

} //namespace lime
