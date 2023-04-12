#include "LMS7002M_SDRDevice.h"

#include "DeviceExceptions.h"
#include "LMSBoards.h"
#include "TRXLooper.h"
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"

namespace lime {

LMS7002M_SDRDevice::LMS7002M_SDRDevice() : mCallback_logData(nullptr), mCallback_logMessage(nullptr)
{
}

LMS7002M_SDRDevice::~LMS7002M_SDRDevice()
{
}

/*
LMS7002M_SDRDevice::DeviceInfo LMS7002M_SDRDevice::GetDeviceInfo(void)
{
    LMS7002M_SDRDevice::DeviceInfo info;
    //initialize to UNKNOWN board type
    //causes lms7_device::CreateDevice() to use LMS7_Generic
    info.deviceName = GetDeviceName(LMS_DEV_UNKNOWN);
    info.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);
    return info;
}
*/
void LMS7002M_SDRDevice::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    throw(OperationNotSupported("TransactSPI not implemented"));
}

int LMS7002M_SDRDevice::I2CWrite(int address, const uint8_t *data, uint32_t length)
{
    throw(OperationNotSupported("WriteI2C not implemented"));
}

int LMS7002M_SDRDevice::I2CRead(int addr, uint8_t *dest, uint32_t length)
{
    throw(OperationNotSupported("ReadI2C not implemented"));
}

int LMS7002M_SDRDevice::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIOWrite not implemented"));
}

int LMS7002M_SDRDevice::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIORead not implemented"));
}

int LMS7002M_SDRDevice::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirWrite not implemented"));
}

int LMS7002M_SDRDevice::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirRead not implemented"));
}

int LMS7002M_SDRDevice::CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units)
{
    throw(OperationNotSupported("CustomParameterWrite not implemented"));
}
int LMS7002M_SDRDevice::CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units)
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

// int LMS7002M_SDRDevice::ReadLMSReg(uint16_t address, int ind)
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
// void LMS7002M_SDRDevice::WriteLMSReg(uint16_t address, uint16_t val, int ind)
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
// int LMS7002M_SDRDevice::ReadFPGAReg(uint16_t address)
// {
//     uint32_t mosi = address;
//     uint32_t miso = 0;
//     const auto slaves = GetDescriptor().spiSlaveIds;
//     SPI(slaves.at("FPGA"), &mosi, &miso, 1);
//     return miso & 0xFFFF;
// }
// void LMS7002M_SDRDevice::WriteFPGAReg(uint16_t address, uint16_t val)
// {
//     const uint32_t mosi = ((address | 0x8000) << 16) | val;
//     const auto slaves = GetDescriptor().spiSlaveIds;
//     SPI(slaves.at("FPGA"), &mosi, nullptr, 1);
// }

void LMS7002M_SDRDevice::Reset()
{
    for(auto iter : mLMSChips)
        iter->ResetChip();
}

void LMS7002M_SDRDevice::Synchronize(bool toChip)
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

void LMS7002M_SDRDevice::EnableCache(bool enable)
{
    for (auto iter : mLMSChips)
        iter->EnableValuesCache(enable);
    if (mFPGA)
        mFPGA->EnableValuesCache(enable);
}

void *LMS7002M_SDRDevice::GetInternalChip(uint32_t index)
{
    if(index >= mLMSChips.size())
        throw std::logic_error("Invalid chip index");
    return mLMSChips[index];
}

void LMS7002M_SDRDevice::StreamStart(uint8_t moduleIndex)
{
    mStreamers.at(moduleIndex)->Start();
}

void LMS7002M_SDRDevice::StreamStop(uint8_t moduleIndex)
{
    if (mStreamers.at(moduleIndex) == nullptr)
        return;

    mStreamers.at(moduleIndex)->Stop();

    if(mStreamers[moduleIndex])
        delete mStreamers[moduleIndex];
    mStreamers[moduleIndex] = nullptr;
}

int LMS7002M_SDRDevice::StreamRx(uint8_t moduleIndex, void **dest, uint32_t count, StreamMeta *meta)
{
    return mStreamers[moduleIndex]->StreamRx(dest, count, meta);
}

int LMS7002M_SDRDevice::StreamTx(uint8_t moduleIndex, const void **samples, uint32_t count, const StreamMeta *meta)
{
    return mStreamers[moduleIndex]->StreamTx(samples, count, meta);
}

void LMS7002M_SDRDevice::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx)
{
    TRXLooper *trx = mStreamers.at(moduleIndex);
    if (rx)
        *rx = trx->GetStats(false);
    if (tx)
        *tx = trx->GetStats(true);
}

bool LMS7002M_SDRDevice::UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback)
{
    throw(OperationNotSupported("UploadMemory not implemented"));
}

} // namespace lime
