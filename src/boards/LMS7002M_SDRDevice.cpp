#include "LMS7002M_SDRDevice.h"

#include "DeviceExceptions.h"
#include "LMSBoards.h"
#include "TRXLooper.h"
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"

#include <cmath>

namespace lime {

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

const SDRDevice::Descriptor& LMS7002M_SDRDevice::GetDescriptor()
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

double LMS7002M_SDRDevice::GetSampleRate(uint8_t moduleIndex, TRXDir trx)
{
    if (moduleIndex >= mLMSChips.size())
        throw std::logic_error("Invalid module index");
    return mLMSChips[moduleIndex]->GetSampleRate(trx, LMS7002M::Channel::ChA);
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

int LMS7002M_SDRDevice::StreamRx(uint8_t moduleIndex, complex32f_t** dest, uint32_t count, StreamMeta* meta)
{
    return mStreamers[moduleIndex]->StreamRx(dest, count, meta);
}

int LMS7002M_SDRDevice::StreamRx(uint8_t moduleIndex, complex16_t** dest, uint32_t count, StreamMeta* meta)
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

} // namespace lime
