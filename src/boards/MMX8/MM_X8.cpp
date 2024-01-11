#include "MM_X8.h"

#include <fcntl.h>
#include <sstream>

#include "Logger.h"
#include "LitePCIe.h"
#include "limesuite/LMS7002M.h"
#include "lms7002m/LMS7002M_validation.h"
#include "FPGA_common.h"
#include "DSP/Equalizer.h"

#include "boards/LimeSDR_XTRX/LimeSDR_XTRX.h"
#include "limesuite/DeviceNode.h"
#include "limesuite/MemoryRegions.h"

#include "math.h"

namespace lime {

static SDRDevice::CustomParameter cp_vctcxo_dac = { "VCTCXO DAC (volatile)", 0, 0, 65535, false };
static double X8ReferenceClock = 30.72e6;

// Do not perform any unnecessary configuring to device in constructor, so you
// could read back it's state for debugging purposes
LimeSDR_MMX8::LimeSDR_MMX8(std::vector<std::shared_ptr<IComms>>& spiLMS7002M,
    std::vector<std::shared_ptr<IComms>>& spiFPGA,
    std::vector<std::shared_ptr<LitePCIe>> trxStreams,
    ISPI* adfComms)
    : mTRXStreamPorts(trxStreams)
{
    mMainFPGAcomms = spiFPGA[8];
    SDRDevice::Descriptor& desc = mDeviceDescriptor;
    desc.name = GetDeviceName(LMS_DEV_LIMESDR_MMX8);

    // LMS64CProtocol::FirmwareInfo fw;
    // LMS64CProtocol::GetFirmwareInfo(controlPipe, fw);
    // LMS64CProtocol::FirmwareToDescriptor(fw, desc);

    // mFPGA = new lime::FPGA_X3(spiFPGA, SPI_LMS7002M_1);
    // mFPGA->SetConnection(&mFPGAcomms);
    // FPGA::GatewareInfo gw = mFPGA->GetGatewareInfo();
    // FPGA::GatewareToDescriptor(gw, desc);

    desc.socTree = std::make_shared<DeviceNode>("X8", eDeviceNodeClass::SDRDevice, this);

    mADF = new ADF4002();
    // TODO: readback board's reference clock
    mADF->Initialize(adfComms, 30.72e6);
    desc.socTree->children.push_back(std::make_shared<DeviceNode>("ADF4002", eDeviceNodeClass::ADF4002, mADF));

    mSubDevices.resize(8);
    desc.spiSlaveIds["FPGA"] = 0;

    const std::unordered_map<eMemoryRegion, Region> eepromMap = { { eMemoryRegion::VCTCXO_DAC, { 16, 2 } } };

    desc.memoryDevices[MEMORY_DEVICES_TEXT.at(eMemoryDevice::FPGA_FLASH)] =
        std::make_shared<DataStorage>(this, eMemoryDevice::FPGA_FLASH);
    desc.memoryDevices[MEMORY_DEVICES_TEXT.at(eMemoryDevice::EEPROM)] =
        std::make_shared<DataStorage>(this, eMemoryDevice::EEPROM, eepromMap);

    desc.customParameters.push_back(cp_vctcxo_dac);
    for (size_t i = 0; i < mSubDevices.size(); ++i)
    {
        mSubDevices[i] = new LimeSDR_XTRX(spiLMS7002M[i], spiFPGA[i], trxStreams[i], X8ReferenceClock);
        const SDRDevice::Descriptor& subdeviceDescriptor = mSubDevices[i]->GetDescriptor();

        for (const auto& soc : subdeviceDescriptor.rfSOC)
        {
            RFSOCDescriptor temp = soc;
            temp.name = soc.name + Descriptor::DEVICE_NUMBER_SEPARATOR_SYMBOL + std::to_string(i + 1);
            desc.rfSOC.push_back(temp);
        }

        for (const auto& slaveId : subdeviceDescriptor.spiSlaveIds)
        {
            const std::string slaveName = slaveId.first + Descriptor::DEVICE_NUMBER_SEPARATOR_SYMBOL + std::to_string(i + 1);
            desc.spiSlaveIds[slaveName] = (i + 1) << 8 | slaveId.second;
            chipSelectToDevice[desc.spiSlaveIds[slaveName]] = mSubDevices[i];
        }

        for (const auto& memoryDevice : subdeviceDescriptor.memoryDevices)
        {
            std::string indexName = subdeviceDescriptor.name + Descriptor::DEVICE_NUMBER_SEPARATOR_SYMBOL + std::to_string(i + 1) +
                                    Descriptor::PATH_SEPARATOR_SYMBOL + memoryDevice.first;

            desc.memoryDevices[indexName] = memoryDevice.second;
        }

        for (const auto& customParameter : subdeviceDescriptor.customParameters)
        {
            SDRDevice::CustomParameter parameter = customParameter;
            parameter.id |= (i + 1) << 8;
            parameter.name = customParameter.name + Descriptor::DEVICE_NUMBER_SEPARATOR_SYMBOL + std::to_string(i + 1);
            desc.customParameters.push_back(parameter);
            customParameterToDevice[parameter.id] = mSubDevices[i];
        }

        const std::string treeName = subdeviceDescriptor.socTree->name + "#" + std::to_string(i + 1);
        subdeviceDescriptor.socTree->name = treeName;
        desc.socTree->children.push_back(subdeviceDescriptor.socTree);
    }
}

LimeSDR_MMX8::~LimeSDR_MMX8()
{
    for (size_t i = 0; i < mSubDevices.size(); ++i)
        delete mSubDevices[i];
}

const SDRDevice::Descriptor& LimeSDR_MMX8::GetDescriptor() const
{
    return mDeviceDescriptor;
}

void LimeSDR_MMX8::Configure(const SDRConfig& cfg, uint8_t socIndex)
{
    try
    {
        mSubDevices[socIndex]->Init();
        mSubDevices[socIndex]->Configure(cfg, 0);
    } //try
    catch (std::logic_error& e)
    {
        printf("LimeSDR_MMX8 config: %s\n", e.what());
        throw;
    } catch (std::runtime_error& e)
    {
        throw;
    }
}

int LimeSDR_MMX8::Init()
{
    for (size_t i = 0; i < mSubDevices.size(); ++i)
        mSubDevices[i]->Init();
    return 0;
}

void LimeSDR_MMX8::Reset()
{
    for (uint32_t i = 0; i < mSubDevices.size(); ++i)
        mSubDevices[i]->Reset();
}

void LimeSDR_MMX8::GetGPSLock(GPS_Lock* status)
{
    // TODO: implement
}

double LimeSDR_MMX8::GetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->GetFrequency(0, trx, channel);
}

void LimeSDR_MMX8::SetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double frequency)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->SetFrequency(0, trx, channel, frequency);
}

double LimeSDR_MMX8::GetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->GetNCOOffset(0, trx, channel);
}

void LimeSDR_MMX8::SetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double offset)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->SetNCOOffset(0, trx, channel, offset);
}

double LimeSDR_MMX8::GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->GetSampleRate(0, trx, channel);
}

void LimeSDR_MMX8::SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    mSubDevices[moduleIndex]->SetSampleRate(0, trx, channel, sampleRate, oversample);
}

double LimeSDR_MMX8::GetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->GetLowPassFilter(0, trx, channel);
}

void LimeSDR_MMX8::SetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double lpf)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->SetLowPassFilter(0, trx, channel, lpf);
}

uint8_t LimeSDR_MMX8::GetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->GetAntenna(0, trx, channel);
}

void LimeSDR_MMX8::SetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t path)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->SetAntenna(0, trx, channel, path);
}

double LimeSDR_MMX8::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    return mSubDevices[channel / 2]->GetClockFreq(clk_id, channel & 0x1);
}

void LimeSDR_MMX8::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    mSubDevices[channel / 2]->SetClockFreq(clk_id, freq, channel & 1);
}

int LimeSDR_MMX8::SetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double value)
{
    auto device = mSubDevices.at(moduleIndex);
    return device->SetGain(0, direction, channel, gain, value);
}

int LimeSDR_MMX8::GetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double& value)
{
    auto device = mSubDevices.at(moduleIndex);
    return device->GetGain(0, direction, channel, gain, value);
}

void LimeSDR_MMX8::Synchronize(bool toChip)
{
    for (auto& d : mSubDevices)
        d->Synchronize(toChip);
}

void LimeSDR_MMX8::EnableCache(bool enable)
{
    for (auto& d : mSubDevices)
        d->EnableCache(enable);
}

void LimeSDR_MMX8::Calibrate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double bandwidth)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->Calibrate(0, trx, channel, bandwidth);
}

void LimeSDR_MMX8::ConfigureGFIR(uint8_t moduleIndex, TRXDir trx, uint8_t channel, ChannelConfig::Direction::GFIRFilter settings)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->ConfigureGFIR(0, trx, channel, settings);
}

uint64_t LimeSDR_MMX8::GetHardwareTimestamp(uint8_t moduleIndex)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->GetHardwareTimestamp(0);
}

void LimeSDR_MMX8::SetHardwareTimestamp(uint8_t moduleIndex, const uint64_t now)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->SetHardwareTimestamp(0, now);
}

void LimeSDR_MMX8::SetTestSignal(uint8_t moduleIndex,
    TRXDir direction,
    uint8_t channel,
    SDRDevice::ChannelConfig::Direction::TestSignal signalConfiguration,
    int16_t dc_i,
    int16_t dc_q)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->SetTestSignal(0, direction, channel, signalConfiguration, dc_i, dc_q);
}

SDRDevice::ChannelConfig::Direction::TestSignal LimeSDR_MMX8::GetTestSignal(uint8_t moduleIndex, TRXDir direction, uint8_t channel)
{
    if (moduleIndex >= 8)
    {
        moduleIndex = 0;
    }

    return mSubDevices[moduleIndex]->GetTestSignal(0, direction, channel);
}

int LimeSDR_MMX8::StreamSetup(const StreamConfig& config, uint8_t moduleIndex)
{
    return mSubDevices[moduleIndex]->StreamSetup(config, 0);
}

void LimeSDR_MMX8::StreamStart(uint8_t moduleIndex)
{
    mSubDevices[moduleIndex]->StreamStart(0);

    FPGA tempFPGA(mMainFPGAcomms, nullptr);
    tempFPGA.StartStreaming();
}

void LimeSDR_MMX8::StreamStop(uint8_t moduleIndex)
{
    mSubDevices[moduleIndex]->StreamStop(0);

    FPGA tempFPGA(mMainFPGAcomms, nullptr);
    tempFPGA.StopStreaming();
}

int LimeSDR_MMX8::StreamRx(uint8_t moduleIndex, lime::complex32f_t* const* dest, uint32_t count, StreamMeta* meta)
{
    return mSubDevices[moduleIndex]->StreamRx(0, dest, count, meta);
}

int LimeSDR_MMX8::StreamRx(uint8_t moduleIndex, lime::complex16_t* const* dest, uint32_t count, StreamMeta* meta)
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

int LimeSDR_MMX8::SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    if (chipSelect == 0)
    {
        return mMainFPGAcomms->SPI(MOSI, MISO, count);
    }

    SDRDevice* dev = chipSelectToDevice.at(chipSelect);
    if (!dev)
    {
        throw std::logic_error("invalid SPI chip select");
    }

    uint32_t subSelect = chipSelect & 0xFF;
    return dev->SPI(subSelect, MOSI, MISO, count);
}

int LimeSDR_MMX8::I2CWrite(int address, const uint8_t* data, uint32_t length)
{
    return -1;
}

int LimeSDR_MMX8::I2CRead(int addres, uint8_t* dest, uint32_t length)
{
    return -1;
}

void LimeSDR_MMX8::SetMessageLogCallback(LogCallbackType callback)
{
    for (size_t i = 0; i < mSubDevices.size(); ++i)
        mSubDevices[i]->SetMessageLogCallback(callback);
}

void* LimeSDR_MMX8::GetInternalChip(uint32_t index)
{
    return mSubDevices[index % mSubDevices.size()]->GetInternalChip(0);
}

int LimeSDR_MMX8::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    int ret = 0;

    for (const CustomParameterIO& param : parameters)
    {
        int subModuleIndex = (param.id >> 8) - 1;
        int id = param.id & 0xFF;

        std::vector<CustomParameterIO> parameter{ { id, param.value, param.units } };

        if (subModuleIndex >= 0)
            ret |= mSubDevices[subModuleIndex]->CustomParameterWrite(parameter);
        else
            ret |= mMainFPGAcomms->CustomParameterWrite(parameter);
    }

    return ret;
}

int LimeSDR_MMX8::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    int ret = 0;

    for (CustomParameterIO& param : parameters)
    {
        int subModuleIndex = (param.id >> 8) - 1;
        int id = param.id & 0xFF;

        std::vector<CustomParameterIO> parameter{ { id, param.value, param.units } };

        if (subModuleIndex >= 0)
            ret |= mSubDevices[subModuleIndex]->CustomParameterRead(parameter);
        else
            ret |= mMainFPGAcomms->CustomParameterRead(parameter);

        param.value = parameter[0].value;
        param.units = parameter[0].units;
    }

    return ret;
}

bool LimeSDR_MMX8::UploadMemory(
    eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
{
    if (device == eMemoryDevice::FPGA_FLASH && moduleIndex == 0)
    {
        int progMode = 1;
        LMS64CProtocol::ProgramWriteTarget target;
        target = LMS64CProtocol::ProgramWriteTarget::FPGA;
        return mMainFPGAcomms->ProgramWrite(data, length, progMode, target, callback);
    }

    SDRDevice* dev = mSubDevices.at(moduleIndex);
    if (!dev)
    {
        throw std::logic_error("Invalid id select");
        return false;
    }

    return dev->UploadMemory(device, 0, data, length, callback);
}

int LimeSDR_MMX8::MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data)
{
    if (storage == nullptr)
    {
        return -1;
    }

    if (storage->ownerDevice == this)
    {
        return mMainFPGAcomms->MemoryWrite(region.address, data, region.size);
    }

    SDRDevice* dev = storage->ownerDevice;
    if (dev == nullptr)
    {
        throw std::logic_error("Device is null");
        return -1;
    }

    return dev->MemoryWrite(storage, region, data);
}

int LimeSDR_MMX8::MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data)
{
    if (storage == nullptr)
    {
        return -1;
    }

    if (storage->ownerDevice == this)
    {
        return mMainFPGAcomms->MemoryRead(region.address, data, region.size);
    }

    SDRDevice* dev = storage->ownerDevice;
    if (dev == nullptr)
    {
        throw std::logic_error("Device is null");
        return -1;
    }

    return dev->MemoryRead(storage, region, data);
}

int LimeSDR_MMX8::UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count)
{
    return mSubDevices[moduleIndex]->UploadTxWaveform(config, 0, samples, count);
}

} //namespace lime
