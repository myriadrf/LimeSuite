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
    gp.write("set size square\n set xrange[-1:1]\n set yrange[-1:1]\n");
#endif
    //Initialize data buffers
    const int samplesInPkt = 256;//(stream.linkFormat == SDRDevice::StreamConfig::I12 ? 1360 : 1020)/channelCount;
    const float rxBufferTime = 0.002; // max buffer size in time (seconds)
    const uint32_t samplesToBuffer = (int)(rxBufferTime*sampleRate/samplesInPkt)*samplesInPkt;
    assert(samplesToBuffer > 0);
    std::vector< std::vector<complex32f_t> > rxSamples(2); // allocate two channels for simplicity
    for(uint i=0; i<rxSamples.size(); ++i)
        rxSamples[i].resize(samplesToBuffer);
    auto t1 = std::chrono::high_resolution_clock::now();
#ifndef USE_GNU_PLOT
    int samplesReceived = 0;
    auto t2 = t1;
    float approxSampleRate = 0;
#endif
    SDRDevice::StreamMeta rxMeta;
    complex32f_t *dest[2] = {rxSamples[0].data(), rxSamples[1].data()};
    int samplesRead = 0;
    signal(SIGINT, intHandler);
    config.channel[0].rx.centerFrequency = 145.5e6;
    while (std::chrono::high_resolution_clock::now() - t1 < std::chrono::seconds(10) && !stoping) //run for 10 seconds
    {
        rxMeta.timestamp = 0;
        samplesRead = device->StreamRx(chipIndex, dest, samplesInPkt*4, &rxMeta);
#ifndef USE_GNU_PLOT
        samplesReceived += samplesRead;
        approxSampleRate += samplesRead;
        auto duration = std::chrono::high_resolution_clock::now() - t2;
        if (duration > std::chrono::seconds(1)) {
            t2 = std::chrono::high_resolution_clock::now();
            std::cout << "Samples Got: " << samplesReceived << "\n";
            std::cout << "Approx sample rate: " << (approxSampleRate/1e6) << "Msps\n";
            approxSampleRate = 0;
        }
#endif
	/*
		INSERT CODE FOR PROCESSING RECEIVED SAMPLES
	*/
#ifdef USE_GNU_PLOT
        //Plot samples
        gp.write("plot '-' with points\n");
#endif
        for (int j = 0; j < samplesRead; ++j)
        {
#ifdef USE_GNU_PLOT
            gp.writef("%f %f\n",dest[0][j].i, dest[0][j].q);
#endif
#ifndef NDEBUG
            printf ("%f %f\n",dest[0][j].i, dest[0][j].q);
#endif
        }
#ifdef USE_GNU_PLOT
        gp.write("e\n");
        gp.flush();
#endif
    }
    DeviceRegistry::freeDevice(device);
    return 0;    
}    

