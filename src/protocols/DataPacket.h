#ifndef LIME_DATAPACKET_H
#define LIME_DATAPACKET_H

#include "dataTypes.h"
#include "BufferInterleaving.h"

namespace lime {

struct RxDataPacket
{
    RxDataPacket() {
        memset(this, 0, sizeof(RxDataPacket));
    }

    inline bool txWasDropped() const {
        return header0 & (1 << 3);
    }

    inline void ClearHeader() {
        memset(this, 0, 16);
    }
    inline float RxFIFOFill() {
        return (header0 & 0x7) * 0.125;
    }
    inline void SetPayloadSize(uint16_t size) {
        payloadSizeLSB = size & 0xFF;
        payloadSizeMSB = (size >> 8) & 0xFF;
    }
    inline uint16_t GetPayloadSize() const {
        return (payloadSizeMSB << 8) | payloadSizeLSB;
    }
    // order matters
    uint8_t header0;
    // payload size specifies how many bytes are valid samples data, 0-full packet is valid
    uint8_t payloadSizeLSB;
    uint8_t payloadSizeMSB;
    uint8_t reserved[5];
    int64_t counter; // should be unsigned, but that's prone to underflow during arithmetic and would choke FPGA, packets would not be sent
    uint8_t data[4096];
};

struct TxDataPacket
{
    TxDataPacket() {
        memset(this, 0, sizeof(TxDataPacket));
    }

    inline void ignoreTimestamp(bool enabled) {
        constexpr uint8_t mask = 1 << 4;
        header0 &= ~mask; //clear ignore timestamp
        header0 |= enabled ? mask : 0; //ignore timestamp
    }
    inline bool getIgnoreTimestamp() {
        constexpr uint8_t mask = 1 << 4;
        return header0 & mask; //ignore timestamp
    }
    inline void ClearHeader() {
        memset(this, 0, 16);
    }

    inline void SetPayloadSize(uint16_t size) {
        payloadSizeLSB = size & 0xFF;
        payloadSizeMSB = (size >> 8) & 0xFF;
    }
    inline uint16_t GetPayloadSize() const {
        return (payloadSizeMSB << 8) | payloadSizeLSB;
    }
    // order matters
    uint8_t header0;
    // payload size specifies how many bytes are valid samples data, 0-full packet is valid
    uint8_t payloadSizeLSB;
    uint8_t payloadSizeMSB;
    uint8_t reserved[5];
    int64_t counter; // should be unsigned, but that's prone to underflow during arithmetic and would choke FPGA, packets would not be sent
    uint8_t data[4080];
};

template<class In>
class TxBufferManager
{
public:
    TxBufferManager(bool mimo, bool compressed, uint32_t maxSamplesInPkt) : currentPacket(nullptr), mData(nullptr),
        bytesUsed(0), mCapacity(0), mimo(mimo), compressed(compressed), maxSamplesInPkt(maxSamplesInPkt)
    {
        bytesForFrame = (compressed ? 3 : 4) * (mimo ? 2: 1);
    }

    void Reset(uint8_t* memPtr, uint32_t capacity)
    {
        packetsCreated = 0;
        bytesUsed = 0;
        mData = memPtr;
        mCapacity = capacity;
        memset(mData, 0, capacity);
        currentPacket = reinterpret_cast<TxDataPacket*>(mData);
    }

    inline bool hasSpace() const {
        const bool packetNotFull = currentPacket->GetPayloadSize() < sizeof(TxDataPacket::data);
        const int spaceAvailable = mCapacity-bytesUsed > 16;
        return packetNotFull && spaceAvailable;
    }

    inline bool consume(In* src)
    {
        while(!src->empty())
        {
            ++packetsCreated;
            currentPacket->ignoreTimestamp(!src->useTimestamp);
            currentPacket->counter = src->timestamp;
            //printf("Add tx pkt, TS:%li, sync:%i flush:%i\n", currentPacket->counter, !currentPacket->getIgnoreTimestamp(), src->flush);
            const int pktSpaceFree = sizeof(TxDataPacket::data)-currentPacket->GetPayloadSize();
            int transferCount = std::min(pktSpaceFree/bytesForFrame, src->size());
            transferCount = std::min(transferCount, maxSamplesInPkt);
            DataConversion conversion;
            conversion.srcFormat = SDRDevice::StreamConfig::DataFormat::F32;
            conversion.destFormat = compressed ? SDRDevice::StreamConfig::DataFormat::I12 : SDRDevice::StreamConfig::DataFormat::I16;
            conversion.channelCount = mimo ? 2 : 1;

            //const int payloadSize = FPGA::Samples2FPGAPacketPayloadFloat(src->front(), transferCount, mimo, compressed, currentPacket->data);
            const int payloadSize = Interleave(src, transferCount, conversion, currentPacket->data);
            bytesUsed += 16 + payloadSize;
            //src->pop(transferCount);
            currentPacket->SetPayloadSize(payloadSize);
            assert(payloadSize > 0);
            assert(payloadSize <= sizeof(TxDataPacket::data));
            if(bytesUsed >= mCapacity-16)
                return true; // not enough space for more packets, need to flush
            else
                currentPacket = reinterpret_cast<TxDataPacket*>(mData+bytesUsed);
        }
        return src->flush;
    }

    inline int size() const { return bytesUsed; };
    inline uint8_t* data() const { return mData; };
    inline int packetCount() const { return packetsCreated; };
private:
    TxDataPacket* currentPacket;
    uint8_t* mData;
    uint32_t bytesUsed;
    uint32_t mCapacity;
    uint16_t packetsCreated;
    uint8_t bytesForFrame;
    int maxSamplesInPkt;
    bool mimo;
    bool compressed;
};

}

#endif // LIME_DATAPACKET_H
