/**
    @file   basicRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal RX example
 */
#include "limesuite/DeviceRegistry.h"
#include "limesuite/LMS7002M.h"
#include "limesuite/SDRDevice.h"
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
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

#undef USE_GNU_PLOT

using namespace lime;

//Device structure, should be initialize to NULL
SDRDevice *device = nullptr;

SDRDevice::StreamConfig stream;
SDRDevice::SDRConfig config;

static const double frequencyLO = 2e9;
float sampleRate = 10e6;
static uint8_t chipIndex = 0; // device might have several RF chips
bool stoping(false);

void intHandler(int dummy) {
    std::cout << "Stoppping\n";
    stoping = true;
}

int error()
{
    if (device != nullptr)
        DeviceRegistry::freeDevice(device);
    exit(-1);
}

int main(int argc, char** argv)
{
    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0) {
        printf("No devices found\n");
        return -1;
    }
    std::cout << "Devices found :" << std::endl;
    for (uint32_t i = 0; i < handles.size(); i++) //print device list
        std::cout << i << ": " << handles[i].serialize() << std::endl;
    std::cout << std::endl;

    //Open the first device
    device = DeviceRegistry::makeDevice(handles.at(0));
    device->Init();

    config.channel[0].tx.enabled = true; //for DAC ch1
    config.channel[0].rx.enabled = true;
    config.channel[0].rx.centerFrequency = frequencyLO;
    config.channel[0].tx.centerFrequency = frequencyLO; //for DAC ch1
    config.channel[0].rx.sampleRate = sampleRate;
    config.channel[0].tx.oversample = 2; // ???
    config.channel[0].rx.oversample = 2; // ???
    config.channel[0].rx.lpf = 0;
    config.channel[0].rx.path = 2; // band1 // TODO: replace with string names
    config.channel[0].rx.calibrate = false;
    config.channel[0].rx.testSignal = false;
    
    stream.rxCount = 1;
    stream.txCount = 0;
    stream.rxChannels[0] = 0;
    stream.format = SDRDevice::StreamConfig::DataFormat::F32;
    stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
    stream.alignPhase = false;

    std::cout << "Configuring device ...\n";
    try {
        auto t1 = std::chrono::high_resolution_clock::now();
        device->Configure(config, chipIndex);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "SDR configured in " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << "ms\n";
    }
    catch ( std::runtime_error &e) {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        error ();
    }
    catch ( std::logic_error &e) {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        error ();
    }
    device->StreamSetup(stream, chipIndex);
    device->StreamStart(chipIndex);
    std::cout << "Stream started ...\n";
#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
#endif
    //Initialize data buffers
    const int channelCount = std::max(stream.rxCount, stream.txCount);
    const int samplesInPkt = 256;//(stream.linkFormat == SDRDevice::StreamConfig::I12 ? 1360 : 1020)/channelCount;

    const float rxBufferTime = 0.002; // max buffer size in time (seconds)
    const uint32_t samplesToBuffer = (int)(rxBufferTime*sampleRate/samplesInPkt)*samplesInPkt;
    assert(samplesToBuffer > 0);

    const float txTimeOffset = 0.005; // tx packets delay in time (seconds), will be rounded to even packets count

    std::vector< std::vector<complex32f_t> > rxSamples(2); // allocate two channels for simplicity
    for(uint i=0; i<rxSamples.size(); ++i)
        rxSamples[i].resize(samplesToBuffer);

    int samplesRead;
    int samplesReceived = 0;

    SDRDevice::StreamMeta rxMeta;
    complex32f_t *dest[2] = {rxSamples[0].data(), rxSamples[1].data()};
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    float approxSampleRate = 0;
    while (std::chrono::high_resolution_clock::now() - t1 < std::chrono::seconds(10)) //run for 10 seconds
    {
        rxMeta.timestamp = 0;
        int samplesRead = device->StreamRx(chipIndex, dest, samplesInPkt*4, &rxMeta);
        samplesReceived += samplesRead;
        approxSampleRate += samplesRead;
        auto duration = std::chrono::high_resolution_clock::now() - t2;
        if (duration > std::chrono::seconds(1)) {
            t2 = std::chrono::high_resolution_clock::now();
            std::cout << "Samples Got: " << samplesReceived << "\n";
            std::cout << "Approx sample rate: " << (approxSampleRate/1e6) << "Msps\n";
            approxSampleRate = 0;
        }
	/*
		INSERT CODE FOR PROCESSING RECEIVED SAMPLES
	*/
#ifdef USE_GNU_PLOT
        //Plot samples
        gp.write("plot '-' with points\n");
        for (int j = 0; j < samplesRead; ++j)
        {
//            printf ("%f %f\n", rxSamples[2 * j], rxSamples[2 * j + 1]);
//            gp.writef("%i %i\n", rxSamples[2 * j], rxSamples[2 * j + 1]);
        }
//        gp.write("e\n");
//        gp.flush();
#endif


    }
    DeviceRegistry::freeDevice(device);
    return 0;    
}    



#ifdef eee

lms_device_t* device = NULL;

int error()
{
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}

int main(int argc, char** argv)
{
    //Find devices
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        error();

    cout << "Devices found: " << n << endl; //print number of devices
    if (n < 1)
        return -1;

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
        error();

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(device) != 0)
        error();

    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0)
        error();

    //Set center frequency to 800 MHz
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, 800e6) != 0)
        error();

    //Set sample rate to 8 MHz, ask to use 2x oversampling in RF
    //This set sampling rate for all channels
    if (LMS_SetSampleRate(device, 8e6, 2) != 0)
        error();

    //Enable test signal generation
    //To receive data from RF, remove this line or change signal to LMS_TESTSIG_NONE
    if (LMS_SetTestSignal(device, LMS_CH_RX, 0, LMS_TESTSIG_NCODIV8, 0, 0) != 0)
        error();

    //Streaming Setup

    //Initialize stream
    lms_stream_t streamId; //stream structure
    streamId.channel = 0; //channel number
    streamId.fifoSize = 1024 * 1024; //fifo size in samples
    streamId.throughputVsLatency = 1.0; //optimize for max throughput
    streamId.isTx = false; //RX channel
    streamId.dataFmt = lms_stream_t::LMS_FMT_I12; //12-bit integers
    if (LMS_SetupStream(device, &streamId) != 0)
        error();

    //Initialize data buffers
    const int sampleCnt = 5000; //complex samples per buffer
    int16_t buffer[sampleCnt * 2]; //buffer to hold complex values (2*samples))

    //Start streaming
    LMS_StartStream(&streamId);

    //Streaming
#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
#endif
    auto t1 = chrono::high_resolution_clock::now();
    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(5)) //run for 5 seconds
    {
        //Receive samples
        int samplesRead = LMS_RecvStream(&streamId, buffer, sampleCnt, NULL, 1000);
	    //I and Q samples are interleaved in buffer: IQIQIQ...
        printf("Received %d samples\n", samplesRead);
	/*
		INSERT CODE FOR PROCESSING RECEIVED SAMPLES
	*/
#ifdef USE_GNU_PLOT
        //Plot samples
        gp.write("plot '-' with points\n");
        for (int j = 0; j < samplesRead; ++j)
//            gp.writef("%i %i\n", buffer[2 * j], buffer[2 * j + 1]);
        gp.write("e\n");
        gp.flush();
#endif
    }
    //Stop streaming
    LMS_StopStream(&streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_DestroyStream(device, &streamId); //stream is deallocated and can no longer be used

    //Close device
    LMS_Close(device);
}
#endif

