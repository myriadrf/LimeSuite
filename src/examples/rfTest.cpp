#include "limesuite/SDRDevice.h"
//#include "limesuite/LMS7002M.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/complex.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <assert.h>
#include "Profiler.h"
#include <thread>
#include <signal.h>
#include <atomic>
#undef USE_GNU_PLOT

#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

using namespace lime;
using namespace std;

SDRDevice *device = nullptr;

static const double frequencyLO = 2e9;
static uint8_t chipIndex = 0; // device might have several RF chips
char* iniArg = nullptr;

std::atomic<bool> runForever;
void intHandler(int dummy) {
    printf("Stoppping\n");
    runForever.store(false);
}

static lime::SDRDevice::LogLevel logVerbosity = lime::SDRDevice::DEBUG;
static void LogCallback(SDRDevice::LogLevel lvl, const char* msg)
{
    if (lvl > logVerbosity)
        return;
    printf("%s\n", msg);
}

typedef std::pair<SDRDevice::SDRConfig, SDRDevice::StreamConfig> TestConfigType;

TestConfigType generateTestConfig(bool mimo, float sampleRate)
{
    SDRDevice::SDRConfig config;
    config.skipDefaults = true; // defaults are already initialized once at the startup
    const uint8_t channelCount = mimo ? 2 : 1;
    for (int i = 0; i < channelCount; ++i) {
        config.channel[i].rx.enabled = true;
        config.channel[i].tx.enabled = true;
        config.channel[i].rx.centerFrequency = frequencyLO;
        config.channel[i].tx.centerFrequency = frequencyLO;
        config.channel[i].rx.sampleRate = sampleRate;
        config.channel[i].tx.sampleRate = sampleRate;
        config.channel[i].rx.oversample = 2;
        config.channel[i].tx.oversample = 2;
        config.channel[i].rx.lpf = 0;//5e6;
        config.channel[i].tx.lpf = 0;//5e6;
        config.channel[i].rx.gfir.enabled = false;
        config.channel[i].rx.gfir.bandwidth = config.channel[i].rx.lpf;
        config.channel[i].rx.path = 2; // Loopback_1 // TODO: replace with string names
        config.channel[i].tx.path = 2; // band1 // TODO: replace with string names
        config.channel[i].rx.calibrate = false;
        config.channel[i].tx.calibrate = false;
        config.channel[i].rx.testSignal = false;
        config.channel[i].tx.testSignal = false;
    }

    SDRDevice::StreamConfig stream;
    stream.rxCount = 1;//channelCount;
    stream.rxChannels[0] = 0;
    stream.rxChannels[1] = 1;
    stream.txCount = 1;//channelCount;
    stream.txChannels[0] = 0;
    stream.txChannels[1] = 1;
    stream.format = SDRDevice::StreamConfig::DataFormat::F32;
    stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
    stream.alignPhase = false;
    //stream.statusCallback = OnStreamStatusChange; // each test override for it's own purpose
    return TestConfigType(config, stream);
}

bool OnStreamStatusChange(bool isTx, const SDRDevice::StreamStats *s, void* userData)
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
        //device->Init();
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
    printf("Press CTRL+C to stop\n\n");
    float sampleRate = 10e6;//30.72e6/4;
    chipIndex = 0;
    printf("----------TEST FullStreamTxRx, sampleRate: %g MHz, MIMO:%s\n", sampleRate/1e6, MIMO ? "yes" : "no");
    auto configPair = generateTestConfig(MIMO, sampleRate);
    SDRDevice::StreamConfig &stream = configPair.second;

    // if(iniArg)
    // {
    //     LMS7002M* chip = static_cast<LMS7002M*>(dev.GetInternalChip(chipIndex));
    //     const char* filename = iniArg;//"LMS2lb.ini";
    //     printf("loading ini: %s\n", filename);
    //     if (chip->LoadConfig(filename) != 0)
    //     {
    //         fprintf(stderr, "Error loading file: %s\n", filename);
    //         return -1;
    //     }
    // }

    if (TrySDRConfigure(configPair.first) != 0)
        return false;

    const int channelCount = std::max(stream.rxCount, stream.txCount);
    const int samplesInPkt = 256;//(stream.linkFormat == SDRDevice::StreamConfig::I12 ? 1360 : 1020)/channelCount;

    const float rxBufferTime = 0.002; // max buffer size in time (seconds)
    const uint32_t samplesToBuffer = (int)(rxBufferTime*sampleRate/samplesInPkt)*samplesInPkt;
    assert(samplesToBuffer > 0);

    const float txTimeOffset = 0.005; // tx packets delay in time (seconds), will be rounded to even packets count
    const int64_t txDeltaTS = (int)(txTimeOffset*sampleRate/samplesInPkt)*samplesInPkt;
    printf("TxDeltaTS +%li, (+%.3fms) %li packets\n", txDeltaTS, 1000.0*txDeltaTS/sampleRate, txDeltaTS/samplesInPkt);

    // const int alignment = 4096;
    // complex32f_t rxSamples[2];
    // rxSamples[0] = aligned_alloc(alignment, samplesToBuffer);
    std::vector< std::vector<complex32f_t> > rxSamples(2); // allocate two channels for simplicity
    for(uint i=0; i<rxSamples.size(); ++i)
        rxSamples[i].resize(samplesToBuffer);

    // precomputing tx samples here, the result might not be continous
    // each packet with different amplitude to distinguish them in time
    std::vector< std::vector<complex32f_t> > txPattern(2);
    const int txPacketCount = 4;
    for(uint i=0; i<txPattern.size(); ++i)
    {
        txPattern[i].resize(txPacketCount*samplesInPkt);
        for(int j=0; j<txPacketCount; ++j)
        {
            float src[4] = {1.0, 0.0, -1.0, 0.0};
            float ampl = 1.0;//(j+1)*(1.0/(txPacketCount+1));
            for(int k=0; k<samplesInPkt; ++k)
            {
                txPattern[i][j*samplesInPkt+k].i = src[k & 3] * ampl;
                txPattern[i][j*samplesInPkt+k].q = src[(k+1) & 3] * ampl;
            }
        }
    }

    // skip some packets at the start in case of leftover data garbage
    int64_t ignoreSamplesAtStart = 0;

    //Initialize stream
    bool streamHadIssues = false;
    stream.statusCallback = OnStreamStatusChange;
    stream.userData = &streamHadIssues; // gets set to true if problems occour
    device->StreamSetup(stream, chipIndex);

    //device->StreamSetup(stream2, 0);

    // simple pointers for stream functions, can't just pass vector of vectors
    lime::complex32f_t *dest[2] = {rxSamples[0].data(), rxSamples[1].data()};
    const lime::complex32f_t *src[2] = {txPattern[0].data(), txPattern[1].data()};

    int testStreamIndex = chipIndex;

    dev.StreamStart(testStreamIndex);
    //dev.StreamStart(0);

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;
    auto start = t1;

    int64_t totalSent = 0;
    int64_t brecv = 0;
    int64_t bsent = 0;

    bool show = false;
    int fired = 0;
    int64_t lastRxTS = 0;

    int badSignal = 0;
    while(runForever.load())
    //while (chrono::high_resolution_clock::now() - start < chrono::milliseconds(10100) && runForever.load())
    {

        //Receive samples
        SDRDevice::StreamMeta rxMeta;
        rxMeta.timestamp = 0;
        auto tt1 = std::chrono::high_resolution_clock::now();
        int samplesRead = dev.StreamRx(testStreamIndex, dest, samplesInPkt*txPacketCount, &rxMeta);
        auto tt2 = std::chrono::high_resolution_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
        if(show)
            printf("rxStream %ius\n", duration);
        //int samplesRead = dev.StreamRx(0, (void **)dest, samplesInPkt*txPacketCount, &rxMeta);
        if(samplesRead == 0)
        {
            printf("StreamRx 0\n");
            continue;
        }

        if(samplesRead < 0)
        {
            printf("Failed to StreamRx %i\n", samplesRead);
            streamHadIssues = true;
            break;
        }

        if(rxMeta.timestamp < lastRxTS)
            printf("non monotonous RXTS:%li, last:%li, diff:%li\n", rxMeta.timestamp, lastRxTS, lastRxTS - rxMeta.timestamp);
        lastRxTS = rxMeta.timestamp;
        brecv += txPacketCount;

        // TODO: verify if Rx RF is what has been transmitted, need to setup digital
        // loopback or verify gain values to know what power change to expect

        SDRDevice::StreamMeta txMeta;
        if(rxMeta.timestamp >= ignoreSamplesAtStart && stream.txCount > 0)
        {
            ++fired;
            int64_t rxNow = rxMeta.timestamp + samplesInPkt;
            txMeta.timestamp = rxNow + txDeltaTS;
            txMeta.useTimestamp = true;
            txMeta.flush = false; // not really matters because of continuous trasmitting

            auto tt1 = std::chrono::high_resolution_clock::now();
            int samplesSent = dev.StreamTx(testStreamIndex, src, samplesInPkt*txPacketCount, &txMeta);
            bsent += txPacketCount;
            //int samplesSent2 = dev.StreamTx(0, (const void **)src, samplesInPkt*txPacketCount/4, &txMeta);
            auto tt2 = std::chrono::high_resolution_clock::now();
            int duration = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
            if(show)
                printf("txStream %ius\n", duration);
            if(samplesSent <= 0)
            {
                if(samplesSent < 0) {
                    //printf("Tx timestamp is already late by %i\n", samplesSent);
                }
                else
                {
                    //printf("Send 0\n");
                    //std::this_thread::sleep_for(std::chrono::microseconds(120));
                    //return false;
                }
            }
            else
                totalSent += samplesSent;
            // if(streamIssues)
            //     return false;
        }
        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000) {
            t1 = t2;
            float sumi = 0;
            float sumq = 0;
            int cnt = 100;
            for(int j=0; j<cnt; ++j)
            {
                float i = dest[0][j*20].i;
                sumi += i*i;
                float q = dest[0][j*20].q;
                sumq += q*q;
                float ampl = sqrt(pow(i, 2) + pow(q, 2));
            }
            float rmsI = sqrt(sumi/cnt);
            float rmsQ = sqrt(sumq/cnt);
            printf("Main Rx: %li  ampl: %f, rmsI:%f, rmsQ:%f badSignal: %i\n", rxMeta.timestamp, 0.0, rmsI, rmsQ, badSignal);
        }

    }

    device->StreamStop(testStreamIndex);
    //device->StreamStop(0);
    return !streamHadIssues;
}


bool TxTiming(SDRDevice &dev, bool MIMO, float tsDelay_ms)
{
    chipIndex = 0;
    const float sampleRate = 122.88e6;
    printf("----------TEST TxTiming, sampleRate: %g MHz, MIMO:%s\n", sampleRate/1e6, MIMO ? "yes" : "no");
    auto configPair = generateTestConfig(true, sampleRate);
    SDRDevice::StreamConfig &stream = configPair.second;

    // if(chipIndex == 1)
    // {
    //     LMS7002M* chip = static_cast<LMS7002M*>(dev.GetInternalChip(chipIndex));
    //     const char* filename = "LMS2_5G_MIMO_TX1_RXH_EXT_100M.ini";
    //     if (chip->LoadConfig(filename) != 0)
    //     {
    //         fprintf(stderr, "Error loading file: %s\n", filename);
    //         return -1;
    //     }
    // }

    if (TrySDRConfigure(configPair.first) != 0)
        return false;

    const int samplesInPkt = (stream.linkFormat == SDRDevice::StreamConfig::I12 ? 1360 : 1020)/stream.rxCount;

    const float rxBufferTime = 0.005; // max buffer size in time (seconds)
    const uint32_t samplesToBuffer = (int)(rxBufferTime*sampleRate/samplesInPkt)*samplesInPkt;
    assert(samplesToBuffer > 0);

    const float txTimeOffset = 0.001*tsDelay_ms; // tx packets delay in time (seconds)
    const int64_t txDeltaTS = (int)(txTimeOffset*sampleRate);
    printf("\nusing TxOffsetTS +%li (%+.3fms) (%+.3f packets)\n", txDeltaTS, 1000.0*txDeltaTS/sampleRate, (float)txDeltaTS/samplesInPkt);

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
            //float ampl = (j+1)*(1.0/(txPacketCount));
            for(int k=0; k<samplesInPkt; ++k)
            {
                txPattern[i][j*samplesInPkt+k].i = src[k & 3];
                txPattern[i][j*samplesInPkt+k].q = src[(k+1) & 3];
            }
        }
    }

    // skip some packets at the start in case of leftover data garbage
    int64_t ignoreSamplesAtStart = 0;//samplesInPkt*1024;
    //printf("Skipping %i rx samples at the beginning", ignoreSamplesAtStart);

    //Initialize stream
    bool streamHadIssues = false;
    stream.statusCallback = OnStreamStatusChange;
    stream.userData = &streamHadIssues; // gets set to true if problems occour
    device->StreamSetup(stream, chipIndex);

    // simple pointers for stream functions, can't just pass vector of vectors
    lime::complex32f_t *dest[2] = {rxSamples[0].data(), rxSamples[1].data()};
    const lime::complex32f_t *src[2] = {txPattern[0].data(), txPattern[1].data()};

    dev.StreamStart(chipIndex);

    auto t1 = chrono::high_resolution_clock::now();

    bool txPending = false;
    SDRDevice::StreamMeta txMeta;

    bool done = false;
    while (chrono::high_resolution_clock::now() - t1 < chrono::milliseconds(3100) && !done)
    //while(!done)
    {
        //Receive samples
        SDRDevice::StreamMeta rxMeta;
        int samplesRead = dev.StreamRx(chipIndex, dest, samplesInPkt*txPacketCount, &rxMeta);
        if(samplesRead < 0)
        {
            printf("Failed to StreamRx\n");
            streamHadIssues = true;
            break;
        }
        if(samplesRead == 0)
            continue;


        if (rxMeta.timestamp < ignoreSamplesAtStart)
            continue;

        int64_t rxNow = rxMeta.timestamp + samplesRead;
        if(!txPending)
        {
            txMeta.timestamp = rxNow + txDeltaTS;
            txMeta.useTimestamp = true;
            txMeta.flush = true;
            int samplesSent = dev.StreamTx(chipIndex, src, samplesInPkt, &txMeta);
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
            float i = dest[0][0].i;
            float q = dest[0][0].q;
            float rxAmpl = sqrt(pow(i, 2) + pow(q, 2));

            i = src[0][0].i;
            q = src[0][0].q;
            float txAmpl = sqrt(pow(i, 2) + pow(q, 2));
            printf("meta: %li, RxTS now:%li background amplitude: ~%g, %i Tx packets sent with target TS: %li, amplitude: %g\n\n", rxMeta.timestamp, rxNow, rxAmpl, txPacketCount, txMeta.timestamp, txAmpl);
            txPending = true;
        }
        else // wait and check for tx packet reception
        {
            for(int j=0; j<samplesRead; ++j)
            {
                float i = dest[0][j].i;
                float q = dest[0][j].q;
                float ampl = sqrt(pow(i, 2) + pow(q, 2));
                if (ampl > 0.2)
                {
                    int64_t diff = rxMeta.timestamp + j - txMeta.timestamp;
                    printf("Rx received signal amplitude %g, RxTS:%li - TxTS:%li = %li (%+.3fms) (%+g packets)\n\n",
                        ampl,
                        rxMeta.timestamp + j,
                        txMeta.timestamp,
                        diff,
                        1000.0*diff/sampleRate,
                        float(diff)/samplesInPkt);
                    txPending = false;
                    done = true;
                    break;
                }
            }
        }
    }
    if(!done)
    {
        printf("\n\tDid not receive expected signal!\n\n");
    }
    device->StreamStop(chipIndex);
    return !streamHadIssues;
}

int main(int argc, char **argv)
{
#ifdef NDEBUG
    printf("Release build\n");
#else
    printf("Debug build\n");
#endif
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
    device = DeviceRegistry::makeDevice(handles.at(0));
    device->SetMessageLogCallback(LogCallback);
    device->Init();

    runForever.store(true);
    signal(SIGINT, intHandler);

    // Run tests
    float millis = 10;
    if(argc > 1)
        //sscanf(argv[1], "%f", &millis);
        iniArg = argv[1];
    //return TxTiming(*device, true, millis);

    bool okStreamSISO = FullStreamTxRx(*device, true);
    return 0;
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
