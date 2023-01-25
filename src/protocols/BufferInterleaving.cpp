#include "BufferInterleaving.h"

#include "FPGA_common.h"

namespace lime 
{

template<class DestType, bool mimo, bool compressed>
int ParseRxPayload(const uint8_t* buffer, int bufLen, DestType** samples)
{
    if(compressed) //compressed samples
    {
        int16_t sample;
        int collected = 0;
        for(int b=0; b<bufLen;collected++)
        {
            //I sample
            sample = buffer[b++];
            sample |= (buffer[b] << 8);
            sample <<= 4;
            samples[0][collected].i = sample >> 4;
            //Q sample
            sample =  buffer[b++];
            sample |= buffer[b++] << 8;
            samples[0][collected].q = sample >> 4;
            if (mimo)
            {
                //I sample
                sample = buffer[b++];
                sample |= (buffer[b] << 8);
                sample <<= 4;
                samples[1][collected].i = sample >> 4;
                //Q sample
                sample =  buffer[b++];
                sample |= buffer[b++] << 8;
                samples[1][collected].q = sample >> 4;
            }
        }
        return collected;
    }

    if (mimo) //uncompressed samples
    {
        complex16_t* ptr = (complex16_t*)buffer;
        const int collected = bufLen/sizeof(complex16_t)/2;
        for(int i=0; i<collected;i++)
        {
            samples[0][i] = *ptr++;
            samples[1][i] = *ptr++;
        }
        return collected;
    }
    else
        memcpy(samples[0], buffer, bufLen);
    return bufLen/sizeof(complex16_t);
}

int Deinterleave(const DataConversion &fmt, const uint8_t* buffer, uint32_t length, TRXLooper::SamplesPacketType* output)
{
    const bool mimo = fmt.channelCount > 1;
    const bool compressed = fmt.srcFormat == SDRDevice::StreamConfig::DataFormat::I12;
    int frameSize;
    int samplesProduced;
    if (fmt.destFormat == SDRDevice::StreamConfig::DataFormat::F32)
    {
        frameSize = sizeof(complex32f_t);
        complex32f_t** dest = reinterpret_cast<complex32f_t**>(output->back());
        samplesProduced = FPGA::FPGAPacketPayload2SamplesFloat(buffer, length, mimo, compressed, dest);
    }
    else
    {
        frameSize = sizeof(complex16_t);
        complex16_t** dest = reinterpret_cast<complex16_t**>(output->back());
        samplesProduced = FPGA::FPGAPacketPayload2Samples(buffer, length, mimo, compressed, dest);
    }
    output->SetSize(output->size()+samplesProduced);
    return samplesProduced;
}

int Interleave(TRXLooper::SamplesPacketType* input, uint32_t count, const DataConversion &fmt, uint8_t* buffer)
{
    const bool mimo = fmt.channelCount > 1;
    const bool compressed = fmt.destFormat == SDRDevice::StreamConfig::DataFormat::I12;
    int frameSize;
    int bytesProduced;
    if (fmt.srcFormat == SDRDevice::StreamConfig::DataFormat::F32)
    {
        frameSize = sizeof(complex32f_t);
        const complex32f_t* const * src = reinterpret_cast<const complex32f_t* const *>(input->front());
        bytesProduced = FPGA::Samples2FPGAPacketPayloadFloat(src, count, mimo, compressed, buffer);
    }
    else
    {
        frameSize = sizeof(complex16_t);
        const complex16_t* const * src = reinterpret_cast<const complex16_t* const *>(input->front());
        bytesProduced = FPGA::Samples2FPGAPacketPayload(src, count, mimo, compressed, buffer);
    }
    input->pop(count);
    return bytesProduced;
}

}
