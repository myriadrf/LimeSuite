#include "limesuite/SDRDevice.h"

#include "Logger.h"

using namespace lime;

SDRDevice::StreamConfig::Extras::Extras()
    : usePoll{ true }
    , negateQ{ false }
    , waitPPS{ false }
{
}

SDRDevice::StreamConfig::Extras::PacketTransmission::PacketTransmission()
    : samplesInPacket{ 0 }
    , packetsInBatch{ 0 }
{
}

SDRDevice::StreamConfig::StreamConfig()
    : format{ DataFormat::I16 }
    , linkFormat{ DataFormat::I16 }
    , bufferSize{ 0 }
    , hintSampleRate{ 0 }
    , alignPhase{ false }
    , statusCallback{ nullptr }
    , userData{ nullptr }
    , extraConfig{}
{
    channels[TRXDir::Rx] = {};
    channels[TRXDir::Tx] = {};
}

const char SDRDevice::Descriptor::DEVICE_NUMBER_SEPARATOR_SYMBOL = '@';
const char SDRDevice::Descriptor::PATH_SEPARATOR_SYMBOL = '/';

OpStatus SDRDevice::SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "TransactSPI not implemented");
}

OpStatus SDRDevice::I2CWrite(int address, const uint8_t* data, uint32_t length)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "WriteI2C not implemented");
}

OpStatus SDRDevice::I2CRead(int addr, uint8_t* dest, uint32_t length)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "ReadI2C not implemented");
}

OpStatus SDRDevice::GPIOWrite(const uint8_t* buffer, const size_t bufLength)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "GPIOWrite not implemented");
}

OpStatus SDRDevice::GPIORead(uint8_t* buffer, const size_t bufLength)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "GPIORead not implemented");
}

OpStatus SDRDevice::GPIODirWrite(const uint8_t* buffer, const size_t bufLength)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "GPIODirWrite not implemented");
}

OpStatus SDRDevice::GPIODirRead(uint8_t* buffer, const size_t bufLength)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "GPIODirRead not implemented");
}

OpStatus SDRDevice::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "CustomParameterWrite not implemented");
}

OpStatus SDRDevice::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return ReportError(OpStatus::NOT_IMPLEMENTED, "CustomParameterRead not implemented");
}

OpStatus SDRDevice::UploadMemory(
    eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
{
    return OpStatus::NOT_IMPLEMENTED;
}

OpStatus SDRDevice::MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data)
{
    return OpStatus::NOT_IMPLEMENTED;
}

OpStatus SDRDevice::MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data)
{
    return OpStatus::NOT_IMPLEMENTED;
}
