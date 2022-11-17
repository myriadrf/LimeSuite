#include "SDRDevice.h"

#include "DeviceExceptions.h"
#include "LMSBoards.h"
#include "TRXLooper.h"
#include "FPGA_common.h"
#include "LMS7002M.h"
#include "Profiler.h"

namespace lime {

SDRDevice::SDRDevice() : mCallback_logData(nullptr), mCallback_logMessage(nullptr)
{
}

SDRDevice::~SDRDevice()
{
}

const DeviceHandle& SDRDevice::GetHandle(void) const
{
    return _handle;
}
/*
SDRDevice::DeviceInfo SDRDevice::GetDeviceInfo(void)
{
    SDRDevice::DeviceInfo info;
    //initialize to UNKNOWN board type
    //causes lms7_device::CreateDevice() to use LMS7_Generic
    info.deviceName = GetDeviceName(LMS_DEV_UNKNOWN);
    info.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);
    return info;
}
*/
void SDRDevice::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    throw(OperationNotSupported("TransactSPI not implemented"));
}

int SDRDevice::I2CWrite(int address, const uint8_t *data, uint32_t length)
{
    throw(OperationNotSupported("WriteI2C not implemented"));
}

int SDRDevice::I2CRead(int addr, uint8_t *dest, uint32_t length)
{
    throw(OperationNotSupported("ReadI2C not implemented"));
}

int SDRDevice::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIOWrite not implemented"));
}

int SDRDevice::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIORead not implemented"));
}

int SDRDevice::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirWrite not implemented"));
}

int SDRDevice::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirRead not implemented"));
}

int SDRDevice::CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units)
{
    throw(OperationNotSupported("CustomParameterWrite not implemented"));
}
int SDRDevice::CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units)
{
    throw(OperationNotSupported("CustomParameterRead not implemented"));
}

void SDRDevice::SetDataLogCallback(DataCallbackType callback)
{
    mCallback_logData = callback;
}

void SDRDevice::SetMessageLogCallback(LogCallbackType callback)
{
    mCallback_logMessage = callback;
}

// int SDRDevice::ReadLMSReg(uint16_t address, int ind)
// {
//     uint32_t mosi = address;
//     uint32_t miso = 0;
//     const auto slaves = GetDescriptor().spiSlaveIds;

//     uint32_t slaveAddr = 0;
//     auto iter = slaves.find("LMS7002M");
//     if (iter != slaves.end())
//         slaveAddr = iter->second;
//     else
//     {
//         char ctemp[128];
//         sprintf(ctemp, "LMS7002M_%i", ind+1);
//         iter = slaves.find(ctemp);
//         if (iter != slaves.end())
//             slaveAddr = iter->second;
//         else
//             throw std::runtime_error("LMS7002M spi slave address not found");
//     }

//     SPI(slaveAddr, &mosi, &miso, 1);
//     return miso & 0xFFFF;
// }
// void SDRDevice::WriteLMSReg(uint16_t address, uint16_t val, int ind)
// {
//     const uint32_t mosi = ((address | 0x8000) << 16) | val;
//     const auto slaves = GetDescriptor().spiSlaveIds;
//     uint32_t slaveAddr = 0;
//     auto iter = slaves.find("LMS7002M");
//     if (iter != slaves.end())
//         slaveAddr = iter->second;
//     else
//     {
//         char ctemp[128];
//         sprintf(ctemp, "LMS7002M_%i", ind+1);
//         iter = slaves.find(ctemp);
//         if (iter != slaves.end())
//             slaveAddr = iter->second;
//         else
//             throw std::runtime_error("LMS7002M spi slave address not found");
//     }
//     SPI(slaveAddr, &mosi, nullptr, 1);
// }
// int SDRDevice::ReadFPGAReg(uint16_t address)
// {
//     uint32_t mosi = address;
//     uint32_t miso = 0;
//     const auto slaves = GetDescriptor().spiSlaveIds;
//     SPI(slaves.at("FPGA"), &mosi, &miso, 1);
//     return miso & 0xFFFF;
// }
// void SDRDevice::WriteFPGAReg(uint16_t address, uint16_t val)
// {
//     const uint32_t mosi = ((address | 0x8000) << 16) | val;
//     const auto slaves = GetDescriptor().spiSlaveIds;
//     SPI(slaves.at("FPGA"), &mosi, nullptr, 1);
// }

void SDRDevice::Reset()
{
    for(auto iter : mLMSChips)
        iter->ResetChip();
}

void SDRDevice::Synchronize(bool toChip)
{
    for (auto iter : mLMSChips)
    {
        if (toChip) {
            if (iter->UploadAll() == 0)
                iter->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true);
        }
        else
            iter->DownloadAll();
    }
}

void SDRDevice::EnableCache(bool enable)
{
    for (auto iter : mLMSChips)
        iter->EnableValuesCache(enable);
    if (mFPGA)
        mFPGA->EnableValuesCache(enable);
}

void *SDRDevice::GetInternalChip(uint32_t index)
{
    if(index >= mLMSChips.size())
        throw std::logic_error("Invalid chip index");
    return mLMSChips[index];
}

void SDRDevice::StreamStart(uint8_t moduleIndex)
{
    mStreamers.at(moduleIndex)->Start();
}

void SDRDevice::StreamStop(uint8_t moduleIndex)
{
    if (mStreamers.at(moduleIndex) == nullptr)
        return;

    mStreamers.at(moduleIndex)->Stop();

    if(mStreamers[moduleIndex])
        delete mStreamers[moduleIndex];
    mStreamers[moduleIndex] = nullptr;
}

int SDRDevice::StreamRx(uint8_t moduleIndex, void **dest, uint32_t count, StreamMeta *meta)
{
    return mStreamers[moduleIndex]->StreamRx(dest, count, meta);
}

int SDRDevice::StreamTx(uint8_t moduleIndex, const void **samples, uint32_t count, const StreamMeta *meta)
{
    return mStreamers[moduleIndex]->StreamTx(samples, count, meta);
}

} // namespace lime
