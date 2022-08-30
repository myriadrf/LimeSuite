#include "SDRDevice.h"
#include "dataTypes.h"
#include "LMS7002M.h"
#include "DeviceRegistry.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <assert.h>
#include "Profiler.h"
#undef USE_GNU_PLOT

#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

using namespace lime;
using namespace std;

SDRDevice *device = nullptr;

static const double frequencyLO = 2e9;
static const uint8_t chipIndex = 1; // device might have several RF chips

typedef std::pair<SDRDevice::SDRConfig, SDRDevice::StreamConfig> TestConfigType;

TestConfigType generateTestConfig(bool mimo, float sampleRate)
{
    SDRDevice::SDRConfig config;
    config.skipDefaults = true; // defaults are already initialized once at the startup
    const uint8_t channelCount = mimo ? 2 : 1;
    for (int i = 0; i < channelCount; ++i) {
        config.channel[i].rxEnabled = true;
        config.channel[i].txEnabled = true;
        config.channel[i].rxCenterFrequency = frequencyLO;
        config.channel[i].txCenterFrequency = frequencyLO;
        config.channel[i].rxSampleRate = sampleRate;
        config.channel[i].txSampleRate = sampleRate;
        config.channel[i].rxOversample = 2;
        config.channel[i].txOversample = 2;
        config.channel[i].rxPath = 2; // Loopback_1 // TODO: replace with string names
        config.channel[i].txPath = 2; // band1 // TODO: replace with string names
        config.channel[i].rxCalibrate = false;
        config.channel[i].rxTestSignal = false;
        config.channel[i].txTestSignal = false;
    }

    SDRDevice::StreamConfig stream;
    stream.rxCount = channelCount;
    stream.rxChannels[0] = 0;
    stream.rxChannels[1] = 1;
    stream.txCount = channelCount;
    stream.txChannels[0] = 0;
    stream.txChannels[1] = 1;
    stream.format = SDRDevice::StreamConfig::DataFormat::F32;
    stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
    stream.alignPhase = false;
    //stream.statusCallback = OnStreamStatusChange; // each test override for it's own purpose
    return TestConfigType(config, stream);
}

bool OnStreamStatusChange(const SDRDevice::StreamStats *s, void* userData)
{
    // TODO: quite spammy if every packet has problems
    // TODO: report first problem instantly and accumulate following ones, report periodically
    // printf("Stream (%s @ %li): overrun:%i, loss:%i, late:%i\n",
    //  (s->isTx ? "Tx" : "Rx"),
    //  s->timestamp,
    //  s->overrun,
    //  s->loss,
    //  s->late);

    // s->isTx, don't care now if it's comming from Rx or Tx
    bool streamIssues = s->late | s->loss | s->overrun;
    if(userData)
        *(bool*)userData = streamIssues; // report that there were issues with stream
    return false;
}

int TrySDRConfigure(SDRDevice::SDRConfig &config)
{
    try {
        auto t1 = std::chrono::high_resolution_clock::now();
        device->Init();
        device->Configure(config, chipIndex);
        auto t2 = std::chrono::high_resolution_clock::now();
        cout << "SDR configured in " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
            << "ms\n";
        return 0;
    }
    catch ( std::runtime_error &e) {
        printf("Failed to configure settings: %s\n", e.what());
        return -1;
    }
    catch ( std::logic_error &e) {
        printf("Failed to configure settings: %s\n", e.what());
        return -1;
    }
}

bool FullStreamTxRx(SDRDevice &dev, bool MIMO)
{
    const float sampleRate = 40e6;
    printf("----------TEST FullStreamTxRx, sampleRate: %g MHz, MIMO:%s\n", sampleRate/1e6, MIMO ? "yes" : "no");
    auto configPair = generateTestConfig(MIMO, sampleRate);
    SDRDevice::StreamConfig &stream = configPair.second;

    if (TrySDRConfigure(configPair.first) != 0)
        return false;

    const int samplesInPkt = (stream.linkFormat == SDRDevice::StreamConfig::I12 ? 1360 : 1020)/stream.rxCount;

    const float rxBufferTime = 0.002; // max buffer size in time (seconds)
    const uint32_t samplesToBuffer = (int)(rxBufferTime*sampleRate/samplesInPkt)*samplesInPkt;
    assert(samplesToBuffer > 0);

    const float txTimeOffset = 0.001; // tx packets delay in time (seconds), will be rounded to even packets count
    const int32_t txDeltaTS = (int)(txTimeOffset*sampleRate/samplesInPkt)*samplesInPkt;

    std::vector< std::vector<complex32f_t> > rxSamples(2); // allocate two channels for simplicity
    for(uint i=0; i<rxSamples.size(); ++i)
        rxSamples[i].resize(samplesToBuffer);

    // precomputing tx samples here, the result might not be continous
    // each packet with different amplitude to distinguish them in time
    std::vector< std::vector<complex32f_t> > txPattern(stream.txCount);
    const int txPacketCount = 1;
    for(uint i=0; i<txPattern.size(); ++i)
    {
        txPattern[i].resize(txPacketCount*samplesInPkt); // 4 packets should be enough
        for(int j=0; j<txPacketCount; ++j)
        {
            int16_t src[4] = {1, 0, -1, 0};
            float ampl = (j+1)*(1.0/(txPacketCount+1));
            for(int k=0; k<samplesInPkt; ++k)
            {
                txPattern[i][j*samplesInPkt+k].i = src[k & 3];
                txPattern[i][j*samplesInPkt+k].q = src[(k+1) & 3];
            }
        }
    }

    // skip some packets at the start in case of leftover data garbage
    int64_t ignoreSamplesAtStart = 0;//samplesInPkt*32;

    //Initialize stream
    bool streamHadIssues = false;
    stream.statusCallback = OnStreamStatusChange;
    stream.userData = &streamHadIssues; // gets set to true if problems occour
    device->StreamSetup(stream, chipIndex);

    // simple pointers for stream functions, can't just pass vector of vectors
    lime::complex32f_t *dest[2] = {rxSamples[0].data(), rxSamples[1].data()};
    lime::complex32f_t *src[2] = {txPattern[0].data(), txPattern[1].data()};

    dev.StreamStart(chipIndex);

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    //while(1)
    while (chrono::high_resolution_clock::now() - t1 < chrono::milliseconds(2100))
    {
        //Receive samples
        SDRDevice::StreamMeta rxMeta;
        int samplesRead = dev.StreamRx(chipIndex, (void **)dest, samplesInPkt, &rxMeta);
        if(samplesRead == 0)
        {
            printf("Failed to StreamRx\n");
            streamHadIssues = true;
            break;
        }
        // if(rxcnt < 8)
        //     printf("StreamRx %li, %f\n", rxMeta.timestamp, lime::GetTimeSinceStartup());
        //if(rxMeta.timestamp < 2e6)
        //printf("StreamRX @ %li\n", rxMeta.timestamp);

        // TODO: verify if Rx RF is what has been transmitted, need to setup digital
        // loopback or verify gain values to know what power change to expect

        if(rxMeta.timestamp >= ignoreSamplesAtStart)
        {
            SDRDevice::StreamMeta txMeta;
            int64_t rxNow = rxMeta.timestamp + samplesInPkt;
            txMeta.timestamp = rxNow + txDeltaTS;
            txMeta.useTimestamp = true;
            txMeta.flushPartialPacket = true; // not really matters since only full packets are sent here
            // if(rxMeta.timestamp < 2e6)
            //     printf("StreamTX @ %li\n", txMeta.timestamp);
            int samplesSent = dev.StreamTx(chipIndex, (const void **)src, txPacketCount*samplesInPkt, &txMeta);
            if(samplesSent <= 0)
            {
                if(samplesSent < 0)
                    printf("Tx timestamp is already late by %i\n", samplesSent);
                else
                {
                    printf("Failure to send\n");
                    return false;
                }
            }
            // if(txcnt < 8)
            //     printf("StreamTx %li, %f\n", txMeta.timestamp, lime::GetTimeSinceStartup());
            // if(streamIssues)
            //     return false;
        }
    }
    
    device->StreamStop(chipIndex);
    return !streamHadIssues;
}


bool TxTiming(SDRDevice &dev, bool MIMO)
{
    const float sampleRate = 10e6;
    printf("----------TEST TxTiming, sampleRate: %g MHz, MIMO:%s\n", sampleRate/1e6, MIMO ? "yes" : "no");
    auto configPair = generateTestConfig(MIMO, sampleRate);
    SDRDevice::StreamConfig &stream = configPair.second;

    if (TrySDRConfigure(configPair.first) != 0)
        return false;

    const int samplesInPkt = (stream.linkFormat == SDRDevice::StreamConfig::I12 ? 1360 : 1020)/stream.rxCount;

    const float rxBufferTime = 0.002; // max buffer size in time (seconds)
    const uint32_t samplesToBuffer = (int)(rxBufferTime*sampleRate/samplesInPkt)*samplesInPkt;
    assert(samplesToBuffer > 0);

    const float txTimeOffset = 0.001; // tx packets delay in time (seconds), will be rounded to even packets count
    const int32_t txDeltaTS = (int)(txTimeOffset*sampleRate/samplesInPkt)*samplesInPkt;

    std::vector< std::vector<complex32f_t> > rxSamples(2); // allocate two channels for simplicity
    for(uint i=0; i<rxSamples.size(); ++i)
        rxSamples[i].resize(samplesToBuffer);

    // precomputing tx samples here, the result might not be continous
    // each packet with different amplitude to distinguish them in time
    std::vector< std::vector<complex32f_t> > txPattern(stream.txCount);
    const int txPacketCount = 1;
    for(uint i=0; i<txPattern.size(); ++i)
    {
        txPattern[i].resize(txPacketCount*samplesInPkt); // 4 packets should be enough
        for(int j=0; j<txPacketCount; ++j)
        {
            int16_t src[4] = {1, 0, -1, 0};
            float ampl = (j+1)*(1.0/(txPacketCount));
            for(int k=0; k<samplesInPkt; ++k)
            {
                txPattern[i][j*samplesInPkt+k].i = src[k & 3];
                txPattern[i][j*samplesInPkt+k].q = src[(k+1) & 3];
            }
        }
    }

    // skip some packets at the start in case of leftover data garbage
    int64_t ignoreSamplesAtStart = 0;//samplesInPkt*32;

    //Initialize stream
    bool streamHadIssues = false;
    stream.statusCallback = OnStreamStatusChange;
    stream.userData = &streamHadIssues; // gets set to true if problems occour
    device->StreamSetup(stream, chipIndex);

    // simple pointers for stream functions, can't just pass vector of vectors
    lime::complex32f_t *dest[2] = {rxSamples[0].data(), rxSamples[1].data()};
    lime::complex32f_t *src[2] = {txPattern[0].data(), txPattern[1].data()};

    dev.StreamStart(chipIndex);

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    bool txPending = false;
    SDRDevice::StreamMeta txMeta;
    float RxAmpl = 0;

    bool done = false;
    while (chrono::high_resolution_clock::now() - t1 < chrono::milliseconds(5100) && !done)
    {
        //Receive samples
        SDRDevice::StreamMeta rxMeta;
        int samplesRead = dev.StreamRx(chipIndex, (void **)dest, samplesInPkt, &rxMeta);
        if(samplesRead == 0)
        {
            printf("Failed to StreamRx\n");
            streamHadIssues = true;
            break;
        }

        if (rxMeta.timestamp < ignoreSamplesAtStart)
            continue;

        if(!txPending)
        {
            int64_t rxNow = rxMeta.timestamp + samplesInPkt;
            txMeta.timestamp = rxNow + txDeltaTS;
            txMeta.useTimestamp = true;
            txMeta.flushPartialPacket = true; // not really matters since only full packets are sent here
            int samplesSent = dev.StreamTx(chipIndex, (const void **)src, txPacketCount*samplesInPkt, &txMeta);
            if(samplesSent <= 0)
            {
                if(samplesSent < 0)
                    printf("Tx timestamp is already late by %i\n", samplesSent);
                else
                {
                    printf("Failure to send\n");
                    return false;
                }
            }
            float i = src[0][0].i;
            float q = src[0][0].q;
            float ampl = sqrt(pow(i, 2) + pow(q, 2));
            printf("Tx sent @ %li, ampl: %g\n", txMeta.timestamp, ampl);
            txPending = true;
        }
        else // wait and check for tx packet reception
        {
            for(int j=0; j<samplesRead; ++j)
            {
                float i = dest[0][j].i;
                float q = dest[0][j].q;
                float ampl = sqrt(pow(i, 2) + pow(q, 2));
                RxAmpl = ampl;
                if (ampl > 0.3)
                {
                    printf("Reception @ %li, ampl: %g,  delta: %li, j: %i\n", rxMeta.timestamp, ampl, rxMeta.timestamp + j - txMeta.timestamp, j);
                    txPending = false;
                    done = true;
                    break;
                }
            }
        }
    }
    printf("Rx ampl: %g\n", RxAmpl);
    device->StreamStop(chipIndex);
    return !streamHadIssues;
}

/*
bool TestSynclessTx(SDRDevice &dev, SDRDevice::StreamConfig &stream)
{
    //Data buffers
    const int samplesInPkt = 1020/stream.rxCount;
    const int bufersize = samplesInPkt * 16; //complex samples per buffer
    lime::complex16_t **buffers = new complex16_t *[stream.rxCount];
    for (int i = 0; i < stream.rxCount; ++i)
        buffers[i] = new lime::complex16_t[bufersize];

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    int samplesReceived = 0;

    int samplesRead = 0;
    SDRDevice::StreamMeta meta;

    const float bufferTime = 0.05;
    const uint32_t samplesToBuffer = (int)(bufferTime*sampleRate/samplesInPkt)*samplesInPkt;
    assert(samplesToBuffer > 0);

    const float txTimeOffset = 0.0005;
    const int32_t deltaTS = samplesInPkt * (int)(txTimeOffset*sampleRate/samplesInPkt);

    std::vector< std::vector<complex16_t> > rxSamples(stream.rxCount);
    for(uint i=0; i<rxSamples.size(); ++i)
        rxSamples[i].resize(samplesToBuffer);

    lime::complex16_t *dest[2];
    dest[0] = static_cast<lime::complex16_t *>(rxSamples[0].data());
    if (mimo)
        dest[1] = static_cast<lime::complex16_t *>(rxSamples[1].data());


    std::vector< std::vector<complex16_t> > txPattern(stream.txCount);
    const int txPacketCount = 1;
    for(uint i=0; i<txPattern.size(); ++i)
    {
        txPattern[i].resize(txPacketCount*samplesInPkt); // 4 packets should be enough
        for(int j=0; j<txPacketCount; ++j)
        {
            int16_t src[4] = {1, 0, -1, 0};
            int16_t ampl = (j+1)*(32768/(txPacketCount+1));
            for(int k=0; k<samplesInPkt; ++k)
            {
                txPattern[i][j*samplesInPkt+k].i = src[k & 3] * ampl;
                txPattern[i][j*samplesInPkt+k].q = src[(k+1) & 3] * ampl;
            }
        }
    }

    lime::complex16_t *src[2];
    src[0] = static_cast<lime::complex16_t *>(txPattern[0].data());
    if (mimo)
        src[1] = static_cast<lime::complex16_t *>(txPattern[1].data());

    bool txSent = false;
    SDRDevice::StreamMeta txMeta;
    int64_t currentTimestamp = 0;
    int64_t startOffset = samplesInPkt*1024*20;

    struct TestData
    {
        int64_t sentAt;
        int64_t recvAt;
    };

    TestData testNoTS;
    testNoTS.sentAt = 0;
    testNoTS.recvAt = 0;

    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(3)) //run for 10 seconds
    {
        //Receive samples
        samplesRead = dev.StreamRx(chipIndex, (void **)dest, samplesInPkt, &meta);
        if(samplesRead == 0)
        {
            printf("Failed to StreamRx\n");
            return false;
        }
        currentTimestamp = meta.timestamp;

        float ampl = sqrt(pow(rxSamples[0][0].i, 2) + pow(rxSamples[0][0].q, 2));
        bool gotSignal = ampl > 2000;
        if(gotSignal)
        {
            printf("Got something @ %li, ampl: %g\n", currentTimestamp, ampl);
        }

        if(currentTimestamp >= startOffset)
        {
            if(testNoTS.sentAt == 0)
            {
                testNoTS.sentAt = currentTimestamp + samplesInPkt;  // it's already in the past, need to add samples count
                txMeta.useTimestamp = false;
                txMeta.flushPartialPacket = true;
                uint samplesSent = dev.StreamTx(chipIndex, (const void **)src, txPacketCount*samplesInPkt, &txMeta);
                if(samplesSent == 0)
                {
                    printf("Failure to send\n");
                    return false;
                }
                else
                    printf("Syncless: Tx sent @ %li\n", testNoTS.sentAt);
            }
            else if (gotSignal)
            {
                testNoTS.recvAt = currentTimestamp;
                float timeDiff = float(testNoTS.recvAt-testNoTS.sentAt)/sampleRate;
                printf("Syncless: Rx got @ %li, RF frontend overhead: %li (%.3f ms)\n", testNoTS.recvAt, testNoTS.recvAt-testNoTS.sentAt, timeDiff*1e3);
                return true;
            }
        }
    }
    return false;
}
*/
int main(int argc, char **argv)
{
    //Find devices
    auto handles = DeviceRegistry::enumerate();
    for (uint32_t i = 0; i < handles.size(); i++) //print device list
        cout << i << ": " << handles[i].serialize() << endl;
    cout << endl;

    if (handles.size() == 0) {
        printf("No devices found\n");
        return -1;
    }

    //Open the first device
    device = DeviceRegistry::makeDevice(handles[0]);
    device->Init();

    // Run tests
    return TxTiming(*device, true);

    bool okStreamSISO = FullStreamTxRx(*device, false);

    // !!!need to close device entirely to clear PCIE data buffers between runs, otherwise FPGA gets stuck with packets from previous run and waits for their timestamps
    // might be LitePCIE Kernel driver issue, because closing individual write/read endpoints does not clear buffers
    DeviceRegistry::freeDevice(device);
    device = DeviceRegistry::makeDevice(handles[0]);

    bool okStreamMIMO = FullStreamTxRx(*device, true);

    printf("StreamSISO: %i\nStreamMIMO: %i\n", okStreamSISO, okStreamMIMO);

    //Close device
    DeviceRegistry::freeDevice(device);

    return 0;
}
