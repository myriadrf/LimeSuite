#ifndef LIME_DATAPACKET_H
#define LIME_DATAPACKET_H

#include "BufferInterleaving.h"

namespace lime {

struct PCIE_DataPacket {
    PCIE_DataPacket() { memset(this, 0, sizeof(PCIE_DataPacket)); }

    inline bool txWasDropped() const { return header0 & (1 << 3); }

    inline void ClearHeader()
    {
        assert(sizeof(PCIE_DataPacket) - sizeof(data) == 16);
        memset(this, 0, 16);
    }
    inline float RxFIFOFill() { return (header0 & 0x7) * 0.125; }
    inline void SetPayloadSize(uint16_t size)
    {
        payloadSizeLSB = size & 0xFF;
        payloadSizeMSB = (size >> 8) & 0xFF;
    }
    inline uint16_t GetPayloadSize() const { return (payloadSizeMSB << 8) | payloadSizeLSB; }
    // order matters
    uint8_t header0;
    // payload size specifies how many bytes are valid samples data, 0-full packet is valid
    uint8_t payloadSizeLSB;
    uint8_t payloadSizeMSB;
    uint8_t reserved[5];
    int64_t
        counter; // should be unsigned, but that's prone to underflow during arithmetic and would choke FPGA, packets would not be sent
    uint8_t data[4096];
};

struct FPGA_DataPacket {
    FPGA_DataPacket() { memset(this, 0, sizeof(FPGA_DataPacket)); }

    inline bool txWasDropped() const { return header0 & (1 << 3); }
    inline void ignoreTimestamp(bool enabled)
    {
        constexpr uint8_t mask = 1 << 4;
        header0 &= ~mask; //clear ignore timestamp
        header0 |= enabled ? mask : 0; //ignore timestamp
    }
    inline bool getIgnoreTimestamp()
    {
        constexpr uint8_t mask = 1 << 4;
        return header0 & mask; //ignore timestamp
    }
    inline void ClearHeader()
    {
        assert(sizeof(FPGA_DataPacket) - sizeof(data) == 16);
        memset(this, 0, 16);
    }
    inline float RxFIFOFill() { return (header0 & 0x7) * 0.125; }
    inline void SetPayloadSize(uint16_t size)
    {
        payloadSizeLSB = size & 0xFF;
        payloadSizeMSB = (size >> 8) & 0xFF;
    }
    inline uint16_t GetPayloadSize() const { return (payloadSizeMSB << 8) | payloadSizeLSB; }
    // order matters
    uint8_t header0;
    // payload size specifies how many bytes are valid samples data, 0-full packet is valid
    uint8_t payloadSizeLSB;
    uint8_t payloadSizeMSB;
    uint8_t reserved[5];
    int64_t
        counter; // should be unsigned, but that's prone to underflow during arithmetic and would choke FPGA, packets would not be sent
    uint8_t data[4080];
};

/** @brief Data structure used for interacting with the header of sample stream packets. */
struct StreamHeader {
    StreamHeader() { Clear(); }

    inline void ignoreTimestamp(bool enabled)
    {
        constexpr uint8_t mask = 1 << 4;
        header0 &= ~mask; //clear ignore timestamp
        header0 |= enabled ? mask : 0; //ignore timestamp
    }
    inline bool getIgnoreTimestamp()
    {
        constexpr uint8_t mask = 1 << 4;
        return header0 & mask; //ignore timestamp
    }
    inline void Clear()
    {
        assert(sizeof(StreamHeader) == 16);
        memset(this, 0, sizeof(StreamHeader));
    }

    inline void SetPayloadSize(uint16_t size)
    {
        payloadSizeLSB = size & 0xFF;
        payloadSizeMSB = (size >> 8) & 0xFF;
    }
    inline uint16_t GetPayloadSize() const { return (payloadSizeMSB << 8) | payloadSizeLSB; }
    // order matters
    uint8_t header0;
    // payload size specifies how many bytes are valid samples data, 0-full packet is valid (4080 bytes payload)
    uint8_t payloadSizeLSB;
    uint8_t payloadSizeMSB;
    uint8_t reserved[5];
    int64_t
        counter; // should be unsigned, but that's prone to underflow during arithmetic and would choke FPGA, packets would not be sent
};

} // namespace lime

#endif // LIME_DATAPACKET_H
