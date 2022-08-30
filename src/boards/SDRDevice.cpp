#include "SDRDevice.h"

#include "DeviceExceptions.h"
#include "LMSBoards.h"
#include "TRXLooper.h"
#include "FPGA_common.h"
#include "LMS7002M.h"
#include "Profiler.h"

namespace lime {

SDRDevice::SDRDevice()
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
void SDRDevice::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count)
{
    throw(OperationNotSupported("TransactSPI not implemented"));
}

int SDRDevice::I2CWrite(int address, const uint8_t *data, size_t length)
{
    throw(OperationNotSupported("WriteI2C not implemented"));
}

int SDRDevice::I2CRead(int addr, uint8_t *dest, size_t length)
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
void SDRDevice::SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback)
{
    throw(OperationNotSupported("SetDataLogCallback not implemented"));
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

//static Profiler prof;

void SDRDevice::StreamStop(uint8_t moduleIndex)
{
    if (mStreamers.at(moduleIndex) == nullptr)
        return;

    mStreamers.at(moduleIndex)->Stop();
    if(txFIFOs[moduleIndex])
        delete txFIFOs[moduleIndex];
    txFIFOs[moduleIndex] = nullptr;
    if(rxFIFOs[moduleIndex])
        delete rxFIFOs[moduleIndex];
    rxFIFOs[moduleIndex] = nullptr;
    if(mStreamers[moduleIndex])
        delete mStreamers[moduleIndex];
    mStreamers[moduleIndex] = nullptr;
    //prof.Done();
}

int SDRDevice::StreamRx(uint8_t moduleIndex, void **samples, uint32_t count, StreamMeta *meta)
{
    if (!rxFIFOs[moduleIndex])
        return 0;

    //ProfilerScope s(&prof, "StreamRx", 1);

    bool timestampSet = false;
    uint32_t samplesProduced = 0;
    const SDRDevice::StreamConfig &config = mStreamers[moduleIndex]->GetConfig();
    const bool mimo = std::max(config.txCount, config.rxCount) > 1;
    const bool compressed =
        config.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;

    PartialPacket &crumbs = rxCrumbs[moduleIndex];

    lime::complex16_t *dest[2];
    dest[0] = &static_cast<lime::complex16_t *>(samples[0])[samplesProduced];
    if (mimo)
        dest[1] = &static_cast<lime::complex16_t *>(samples[1])[samplesProduced];
    lime::complex32f_t *floatDest[2];
    floatDest[0] = &static_cast<lime::complex32f_t *>(samples[0])[samplesProduced];
    if (mimo)
        floatDest[1] = &static_cast<lime::complex32f_t *>(samples[1])[samplesProduced];
    const int samplesInPkt = (compressed ? 1360 : 1020) / (mimo ? 2 : 1);

    if(!timestampSet && crumbs.end != 0)
    {
        if(meta)
            meta->timestamp = crumbs.timestamp+crumbs.start;
        timestampSet = true;
    }

    const float normalizationAmplitude = compressed ? 2048 : 32768;

    auto start = std::chrono::high_resolution_clock::now();
    while (samplesProduced < count) {

        if(crumbs.end == samplesInPkt)
        {
            if(config.format == SDRDevice::StreamConfig::F32)
            {
                for(int i=crumbs.start; i<samplesInPkt && samplesProduced < count; ++i)
                {
                    floatDest[0][samplesProduced].i = crumbs.chA[i].i/normalizationAmplitude;
                    floatDest[0][samplesProduced].q = crumbs.chA[i].q/normalizationAmplitude;
                    if(mimo)
                    {
                        floatDest[1][samplesProduced].i = crumbs.chB[i].i/normalizationAmplitude;
                        floatDest[1][samplesProduced].q = crumbs.chB[i].q/normalizationAmplitude;
                    }
                    ++samplesProduced;
                    ++crumbs.start;
                }
            }
            else
            {
                for(int i=crumbs.start; i<samplesInPkt && samplesProduced < count; ++i)
                {
                    dest[0][samplesProduced] = crumbs.chA[i];
                    if(mimo)
                        dest[1][samplesProduced] = crumbs.chB[i];
                    ++samplesProduced;
                    ++crumbs.start;
                }
            }
        }

        if(samplesProduced == count)
        {
            //printf("TS: %li, rx: %i, Crumbs left: %i\n", meta->timestamp, samplesProduced, crumbs.end-crumbs.start);
            return samplesProduced;
        }

        FPGA_DataPacket *pkt = rxFIFOs[moduleIndex]->pop();
        if (!pkt)
        {
            int duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
            if(duration > 100) // TODO: timeout duration in meta
                return samplesProduced;
            else
                continue;
        }
        if(!timestampSet)
        {
            if(meta)
                meta->timestamp = pkt->counter;
            timestampSet = true;
            //printf("RxPop %f\n", GetTimeSinceStartup());
        }

        int recv;
        {
            lime::complex16_t *dest2[2];
            dest2[0] = static_cast<lime::complex16_t *>(crumbs.chA);
            if (mimo)
                dest2[1] = static_cast<lime::complex16_t *>(crumbs.chB);
            recv = FPGA::FPGAPacketPayload2Samples(pkt->data, sizeof(FPGA_DataPacket::data), mimo,
                                                   compressed, (lime::complex16_t **)dest2);

            crumbs.timestamp = pkt->counter;
            rxFIFOs[moduleIndex]->release(pkt);
            crumbs.start = 0;
            crumbs.end = recv;

        }
    }
    return samplesProduced;
}

int SDRDevice::StreamTx(uint8_t moduleIndex, const void **samples, uint32_t count, const StreamMeta *meta)
{
    if(txFIFOs.at(moduleIndex) == nullptr)
        return 0;

    //ProfilerScope s(&prof, "StreamTx", 2);

    PartialPacket &crumbs = txCrumbs[moduleIndex];

    uint32_t samplesConsumed = 0;
    const SDRDevice::StreamConfig &config = mStreamers[moduleIndex]->GetConfig();
    const bool mimo = std::max(config.txCount, config.rxCount) > 1;
    const bool compressed = config.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const int samplesInPkt = (compressed ? 1360 : 1020) / (mimo ? 2 : 1);

    const lime::complex16_t *src[2];
        src[0] = &(static_cast<const lime::complex16_t *>(samples[0]))[samplesConsumed];
        if (mimo)
            src[1] = &(static_cast<const lime::complex16_t *>(samples[1]))[samplesConsumed];

    const lime::complex32f_t *floatSrc[2];
        floatSrc[0] = &(static_cast<const lime::complex32f_t *>(samples[0]))[samplesConsumed];
        if (mimo)
            floatSrc[1] = &(static_cast<const lime::complex32f_t *>(samples[1]))[samplesConsumed];

    const float normalizationAmplitude = compressed ? 2047 : 32767;

    while (samplesConsumed < count) {
        // fill crumbs
        if(crumbs.end == 0)
            crumbs.timestamp = meta ? meta->timestamp+samplesConsumed : 0;

        if(config.format == StreamConfig::F32)
        {
            for(int i=crumbs.end; i<samplesInPkt && samplesConsumed < count; ++i)
            {
                crumbs.chA[i].i = floatSrc[0][samplesConsumed].i*normalizationAmplitude;
                crumbs.chA[i].q = floatSrc[0][samplesConsumed].q*normalizationAmplitude;
                if(mimo)
                {
                    crumbs.chB[i].i = floatSrc[1][samplesConsumed].i*normalizationAmplitude;
                    crumbs.chB[i].q = floatSrc[1][samplesConsumed].q*normalizationAmplitude;
                }
                ++crumbs.end;
                ++samplesConsumed;
            }
        }
        else
        {
            // TODO: optimize to memcpy
            for(int i=crumbs.end; i<samplesInPkt && samplesConsumed < count; ++i)
            {
                crumbs.chA[i] = src[0][samplesConsumed];
                if(mimo)
                    crumbs.chB[i] = src[1][samplesConsumed];
                ++crumbs.end;
                ++samplesConsumed;
            }
        }

        if(crumbs.end < samplesInPkt && meta->flushPartialPacket==false)
            return samplesConsumed;

        FPGA_DataPacket *pkt = txFIFOs[moduleIndex]->acquire();
        if (!pkt)
        {
            printf("StreamTx failed acquire fifo packet, queue already full\n");
            return samplesConsumed;
        }

        const bool useTimestamp = meta ? meta->useTimestamp : true;
        pkt->ClearHeader();
        pkt->counter = crumbs.timestamp;
        pkt->ignoreTimestamp(!useTimestamp);
        const int payloadSize = sizeof(FPGA_DataPacket::data);
        pkt->payloadSizeLSB = payloadSize & 0xFF;
        pkt->payloadSizeMSB = (payloadSize >> 8) & 0xFF;

        const lime::complex16_t *src[2];
        // src[0] = &(static_cast<const lime::complex16_t *>(samples[0]))[samplesConsumed];
        // if (mimo)
        //     src[1] = &(static_cast<const lime::complex16_t *>(samples[1]))[samplesConsumed];
        src[0] = crumbs.chA;
        if (mimo)
            src[1] = crumbs.chB;

        //memset(pkt->data, 0, 4080);
        int bytesUsed = FPGA::Samples2FPGAPacketPayload(src, crumbs.end, mimo, compressed, pkt->data);
        crumbs.end = 0;
        crumbs.start = 0;
        memset(crumbs.chA, 0, sizeof(crumbs.chA));
        memset(crumbs.chB, 0, sizeof(crumbs.chB));
        // if(meta->flushPartialPacket)
        //     printf("TxTS: %li  , p:%i\n", pkt->counter, meta->flushPartialPacket);
        const int64_t rxTimestampNow = mStreamers[moduleIndex]->GetHardwareTimestamp();
        if (pkt->counter < rxTimestampNow && useTimestamp)
        {
            txFIFOs[moduleIndex]->release(pkt); // packet is going to be late, so don't even add it to FIFO
            return -(rxTimestampNow - pkt->counter); // TODO: error code for late packet
        }
        else
            txFIFOs[moduleIndex]->push(pkt);

    }
    return samplesConsumed;
}

} // namespace lime
