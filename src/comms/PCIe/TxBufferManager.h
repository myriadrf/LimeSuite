#ifndef LIME_TXBUFFERMANAGER_H
#define LIME_TXBUFFERMANAGER_H

#include <cstdint>
#include <cstring>

#include "BufferInterleaving.h"
#include "limesuite/SDRDevice.h"
#include "protocols/DataPacket.h"

namespace lime {

/** 
  @brief A class for managing the transmission buffer for the PCIe transfer.
  @tparam T The samples packet input type.
 */
template<class T> class TxBufferManager
{
  public:
    TxBufferManager(bool mimo,
        bool compressed,
        uint32_t maxSamplesInPkt,
        uint32_t maxPacketsInBatch,
        SDRDevice::StreamConfig::DataFormat inputFormat)
        : header(nullptr)
        , payloadPtr(nullptr)
        , mData(nullptr)
        , bytesUsed(0)
        , mCapacity(0)
        , maxPacketsInBatch(maxPacketsInBatch)
        , maxSamplesInPkt(maxSamplesInPkt)
        , packetsCreated(0)
        , payloadSize(0)
    {
        bytesForFrame = (compressed ? 3 : 4) * (mimo ? 2 : 1);
        conversion.srcFormat = inputFormat; //SDRDevice::StreamConfig::DataFormat::F32;
        conversion.destFormat = compressed ? SDRDevice::StreamConfig::DataFormat::I12 : SDRDevice::StreamConfig::DataFormat::I16;
        conversion.channelCount = mimo ? 2 : 1;
        maxPayloadSize = std::min(4080u, bytesForFrame * maxSamplesInPkt);
    }

    void Reset(uint8_t* memPtr, uint32_t capacity)
    {
        packetsCreated = 0;
        bytesUsed = 0;
        mData = memPtr;
        mCapacity = capacity;
        std::memset(mData, 0, capacity);
        header = reinterpret_cast<StreamHeader*>(mData);
        header->Clear();
        payloadSize = 0;
        payloadPtr = reinterpret_cast<uint8_t*>(header) + sizeof(StreamHeader);
    }

    inline bool hasSpace() const
    {
        const bool packetNotFull = payloadSize < maxPayloadSize;
        const bool spaceAvailable = mCapacity - bytesUsed > sizeof(StreamHeader);
        return packetNotFull && spaceAvailable;
    }

    inline bool consume(T* src)
    {
        bool sendBuffer = false;
        while (!src->empty())
        {
            if (payloadSize >= maxPayloadSize || payloadSize == maxSamplesInPkt * bytesForFrame)
            {
                header = reinterpret_cast<StreamHeader*>(mData + bytesUsed);
                header->Clear();
                payloadPtr = reinterpret_cast<uint8_t*>(header) + sizeof(StreamHeader);
                payloadSize = 0;
            }

            header->ignoreTimestamp(!src->metadata.waitForTimestamp);
            if (payloadSize == 0)
            {
                ++packetsCreated;
                header->counter = src->metadata.timestamp;
                bytesUsed += sizeof(StreamHeader);
            }
            const uint32_t freeSpace = std::min(maxPayloadSize - payloadSize, mCapacity - bytesUsed - 16);
            uint32_t transferCount = std::min(freeSpace / bytesForFrame, src->size());
            transferCount = std::min(transferCount, maxSamplesInPkt);
            if (transferCount > 0)
            {
                int samplesDataSize = Interleave(src, transferCount, conversion, payloadPtr);
                payloadPtr = payloadPtr + samplesDataSize;
                payloadSize += samplesDataSize;
                bytesUsed += samplesDataSize;
                header->SetPayloadSize(payloadSize);
                assert(payloadSize > 0);
                assert(payloadSize <= maxPayloadSize);
            }
            else
                sendBuffer = true;

            if (packetsCreated >= maxPacketsInBatch && !hasSpace())
                sendBuffer = true;

            if (bytesUsed >= mCapacity - sizeof(StreamHeader))
                sendBuffer = true; // not enough space for more packets, need to flush
            if ((uint64_t)payloadPtr & 0xF)
                sendBuffer = true; // next packets payload memory is not suitably aligned for vectorized filling

            if (sendBuffer)
            {
                const int busWidthBytes = 16;
                int extraBytes = bytesUsed % busWidthBytes;
                if (extraBytes != 0)
                {
                    //printf("Patch buffer, bytes %i, extra: %i, last payload: %i\n", bytesUsed, extraBytes, payloadSize);
                    // patch last packet so that whole buffer size would be multiple of bus width
                    int padding = busWidthBytes - extraBytes;
                    std::memset(payloadPtr, 0, padding);
                    payloadSize += padding;
                    bytesUsed += padding;
                    header->SetPayloadSize(payloadSize);
                    //printf("Patch buffer, bytes %i, last payload: %i\n", bytesUsed, payloadSize);
                }
                break;
            }
        }
        if (!hasSpace())
            return true;
        return src->metadata.flushPartialPacket || sendBuffer;
    }

    inline int size() const { return bytesUsed; };
    inline uint8_t* data() const { return mData; };
    inline int packetCount() const { return packetsCreated; };

  private:
    DataConversion conversion;
    StreamHeader* header;
    uint8_t* payloadPtr;
    uint8_t* mData;
    uint32_t bytesUsed;
    uint32_t mCapacity;
    uint32_t maxPacketsInBatch;
    uint32_t maxSamplesInPkt;
    uint32_t maxPayloadSize;
    uint16_t packetsCreated;
    uint32_t payloadSize;
    uint8_t bytesForFrame;
};

} // namespace lime

#endif // LIME_TXBUFFERMANAGER_H