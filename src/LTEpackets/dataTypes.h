#ifndef LMS_DATA_TYPES_H
#define LMS_DATA_TYPES_H

typedef struct
{
    uint8_t reserved[8];
    uint64_t counter;
    uint8_t data[4080];
} PacketLTE;

typedef struct
{
    int16_t i;
    int16_t q;
} complex16_t;

class PacketFrame
{
public:
    static const int16_t maxSamplesInPacket = 1360; //total number of samples in all channels combined
    uint8_t channelsCount; //number of channels in packet
    uint16_t samplesCount; //maximum number of samples in each channel
    uint64_t timestamp; //timestamp of the packet
    uint16_t first; //index of first unused sample in samples
    uint16_t last; //end index of samples
    complex16_t** samples;
    uint32_t flags;

    PacketFrame() 
    {   
        channelsCount = 0;
        samplesCount = 0;
        timestamp = 0;
        first = 0;
        last = 0;
        flags = 0;
        samples = nullptr;
        Initialize(1);
    }

    PacketFrame(const uint8_t channelsCount) : channelsCount(channelsCount), samplesCount(maxSamplesInPacket / channelsCount)
    {   
        timestamp = 0;
        first = 0;
        last = 0;
        flags = 0;
        samples = nullptr;
    }
    ~PacketFrame()
    {
        for (int i = 0; i < channelsCount; ++i)
            if (samples[i] != nullptr)
            {
                delete[] samples[i];
                samples[i] = nullptr;
            }
        if (samples)
            delete[]samples;
    }

    void Initialize(const uint8_t channelsCount)
    {        
        if (samples)
        {
            for (int i = 0; i < this->channelsCount; ++i)
                if (samples[i] != nullptr)
                {
                    delete[] samples[i];
                    samples[i] = nullptr;
                }
            delete[]samples;
        }

        this->channelsCount = channelsCount;
        this->samplesCount = (maxSamplesInPacket / channelsCount);
        timestamp = 0;
        first = 0;
        last = 0;
        flags = 0;
        samples = new complex16_t*[channelsCount];
        for (int i = 0; i < channelsCount; ++i)
        {
            samples[i] = nullptr;
            samples[i] = new complex16_t[samplesCount];
        }
    }

    PacketFrame& PacketFrame::operator= (const PacketFrame &pkt)
    {
        if (this->channelsCount != pkt.channelsCount)
            this->Initialize(pkt.channelsCount);
        this->timestamp = pkt.timestamp;
        this->first = pkt.first;
        this->last = pkt.last;
        this->flags = pkt.flags;
        for (int i = 0; i < channelsCount; ++i)
        {
            memcpy(samples[i], pkt.samples[i], sizeof(complex16_t)*samplesCount);
        }
    }
};

#endif