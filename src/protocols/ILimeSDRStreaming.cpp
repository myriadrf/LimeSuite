#include "ILimeSDRStreaming.h"
#include "ErrorReporting.h"
#include <assert.h>
#include "FPGA_common.h"
#include "Logger.h"

using namespace lime;

static const int MAX_CHANNEL_COUNT = 6;

ILimeSDRStreaming::ILimeSDRStreaming()
{
    for (int i = 0; i < MAX_CHANNEL_COUNT/2; i++)
    	mStreamers.push_back(new Streamer(this));
}
ILimeSDRStreaming::~ILimeSDRStreaming()
{
    for (unsigned i = 0; i < mStreamers.size() ; i++)
        delete mStreamers[i];
}

StreamChannel* ILimeSDRStreaming::SetupStream(const StreamConfig& config)
{
    if ( config.channelID >= MAX_CHANNEL_COUNT)
        return nullptr;
    unsigned index = config.channelID/2;
    return mStreamers[index]->SetupStream(config);
}

uint64_t ILimeSDRStreaming::GetHardwareTimestamp(void)
{
    return mStreamers[0]->GetHardwareTimestamp();
}

void ILimeSDRStreaming::SetHardwareTimestamp(const uint64_t now)
{
    mStreamers[0]->SetHardwareTimestamp(now);
}

int ILimeSDRStreaming::ReceiveData(char* buffer, int length, int epIndex, int timeout)
{
    return ReportError("Function not supported");
}

int ILimeSDRStreaming::SendData(const char* buffer, int length, int epIndex, int timeout)
{
    return ReportError("Function not supported");
}

int ILimeSDRStreaming::UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex)
{
    bool comp = true;

    switch (GetInfo().device)
    {
        case LMS_DEV_STREAM:
        case LMS_DEV_LIMESDR:
        case LMS_DEV_LIMESDR_PCIE:
        case LMS_DEV_LMS7002M_ULTIMATE_EVB:
            break;
        case LMS_DEV_LIMESDR_QPCIE:
            if (epIndex == 2)
                comp = (format == StreamConfig::FMT_INT12);
            break;
        default: return ReportError("UploadWFM not supported");
    }

    const int samplesInPkt = comp ? samples12InPkt : samples16InPkt;
    WriteRegister(0xFFFF, 1 << epIndex);
    WriteRegister(0x000C, chCount == 2 ? 0x3 : 0x1); //channels 0,1
    WriteRegister(0x000E, comp ? 0x2 : 0x0); //16bit samples

    uint16_t regValue = 0;
    ReadRegister(0x000D,regValue);
    regValue |= 0x4;
    WriteRegister(0x000D, regValue);

    lime::FPGA_DataPacket pkt;
    size_t samplesUsed = 0;
    int cnt = sample_count;

    const complex16_t* const* src = (const complex16_t* const*)samples;
    const lime::complex16_t** batch = new const lime::complex16_t*[chCount];
    lime::complex16_t** samplesShort = new lime::complex16_t*[chCount];
    for(unsigned i=0; i<chCount; ++i)
        samplesShort[i] = nullptr;


    if (format == StreamConfig::FMT_INT16 && comp == true)
    {
        for(unsigned i=0; i<chCount; ++i)
            samplesShort[i] = new lime::complex16_t[sample_count];
        for (int ch = 0; ch < chCount; ch++)
            for(size_t i=0; i < sample_count; ++i)
            {
                samplesShort[ch][i].i = src[ch][i].i >> 4;
                samplesShort[ch][i].q = src[ch][i].q >> 4;
            }
        src = samplesShort;
    }
    else if(format == StreamConfig::FMT_FLOAT32)
    {
        const float mult = comp ? 2047.0f : 32767.0f;
        for(unsigned i=0; i<chCount; ++i)
            samplesShort[i] = new lime::complex16_t[sample_count];

        const float* const* samplesFloat = (const float* const*)samples;
        for (int ch = 0; ch < chCount; ch++)
            for(size_t i=0; i < sample_count; ++i)
            {
                samplesShort[ch][i].i = samplesFloat[ch][2*i]*mult;
                samplesShort[ch][i].q = samplesFloat[ch][2*i+1]*mult;
            }
        src = samplesShort;
    }

    while(cnt > 0)
    {
        pkt.counter = 0;
        pkt.reserved[0] = 0;
        int samplesToSend = cnt > samplesInPkt/chCount ? samplesInPkt/chCount : cnt;

        for(unsigned i=0; i<chCount; ++i)
            batch[i] = &src[i][samplesUsed];
        samplesUsed += samplesToSend;

        int bufPos = lime::fpga::Samples2FPGAPacketPayload(batch, samplesToSend, chCount==2, comp, pkt.data);
        int payloadSize = (bufPos / 4) * 4;
        if(bufPos % 4 != 0)
            lime::error("Packet samples count not multiple of 4");
        pkt.reserved[2] = (payloadSize >> 8) & 0xFF; //WFM loading
        pkt.reserved[1] = payloadSize & 0xFF; //WFM loading
        pkt.reserved[0] = 0x1 << 5; //WFM loading

        long bToSend = 16+payloadSize;
        if (SendData((const char*)&pkt,bToSend,epIndex,500)!=bToSend)
            break;
        cnt -= samplesToSend;
    }
    delete[] batch;
    for(unsigned i=0; i<chCount; ++i)
        if (samplesShort[i])
            delete [] samplesShort[i];
    delete[] samplesShort;

    /*Give FX3 some time to load samples to FPGA*/
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if(cnt == 0)
        return 0;
    else
        return ReportError(-1, "Failed to upload waveform");
}
