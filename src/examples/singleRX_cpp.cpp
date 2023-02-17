/**
    @file   singleRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  RX example
 */
#include "limesuite/SDRDevice.h"
#include "dataTypes.h"
#include "limesuite/LMS7002M.h"
#include "DeviceRegistry.h"
#include <iostream>
#include <chrono>

//#undef USE_GNU_PLOT

#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

using namespace lime;
using namespace std;

SDRDevice *device = nullptr;

static const uint8_t mimo = false;
static const double sampleRate = 10e6;
static const double frequencyLO = 1e9;
static const uint8_t chipIndex = 0; // device might have several RF chips

int error()
{
    if (device != NULL)
        DeviceRegistry::freeDevice(device);
    printf("Error\n");
    exit(-1);
}

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

    // Initialize device with default configuration
    // Do not use if you want to keep existing configuration
    // Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    device->Init();
    //device->Synchronize(false);

    SDRDevice::SDRConfig config;

    const uint8_t channelCount = mimo ? 2 : 1;
    for (int i = 0; i < channelCount; ++i) {
        config.channel[i].rxEnabled = true;
        config.channel[i].rxCenterFrequency = frequencyLO;
        config.channel[i].rxSampleRate = sampleRate;
        config.channel[i].rxPath = 1;
        config.channel[i].rxCalibrate = false;
        config.channel[i].rxTestSignal = true;
    }

    try {
        auto t1 = std::chrono::high_resolution_clock::now();
        device->Configure(config, chipIndex);
        auto t2 = std::chrono::high_resolution_clock::now();
        cout << "SDR configured in " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
            << "ms\n";
    }
    catch ( std::runtime_error &e) {
        printf("Failed to configure settings: %s\n", e.what());
        return -1;
    }
    catch ( std::logic_error &e) {
        printf("Failed to configure settings: %s\n", e.what());
        return -1;
    }

    SDRDevice::StreamConfig stream;
    stream.rxCount = channelCount;
    stream.rxChannels[0] = 0;
    stream.rxChannels[1] = 1;
    stream.txCount = 0;
    stream.format = SDRDevice::StreamConfig::DataFormat::I16;
    stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
    stream.alignPhase = false;

    //Initialize stream
    device->StreamSetup(stream, chipIndex);
    device->StreamStart(chipIndex);

    //Data buffers
    const int samplesInPkt = 700;//1020/stream.rxCount;
    const int bufersize = samplesInPkt * 16; //complex samples per buffer
    lime::complex16_t **buffers = new complex16_t *[stream.rxCount];
    for (int i = 0; i < stream.rxCount; ++i)
        buffers[i] = new lime::complex16_t[bufersize];

#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    const float range = stream.format == SDRDevice::StreamConfig::DataFormat::I16 ? 32768 : 2048;
    gp.writef("set size square\n set xrange[-%f:%f]\n set yrange[-%f:%f]\n", range, range, range, range);

    GNUPlotPipe gpTime;
    gpTime.writef("set yrange[-%f:%f]\n", range, range);
#endif
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    int samplesReceived = 0;

    int samplesRead = 0;
    SDRDevice::StreamMeta meta;

    float approxSampleRate = 0;
    //while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(4)) //run for 10 seconds
    while(1)
    {

        //Receive samples
        samplesRead = device->StreamRx(chipIndex, (void **)buffers, 2304, &meta);
        if(samplesRead != bufersize)
            printf("Expected %i, got %i\n", bufersize, samplesRead);
        samplesReceived += samplesRead;
        approxSampleRate += samplesRead;

        //Print stats (once per second)
        auto duration = chrono::high_resolution_clock::now() - t2;
        if (duration > chrono::seconds(1)) {
            t2 = chrono::high_resolution_clock::now();
            cout << "Samples Got: " << samplesReceived << "\n";
            cout << "Approx sample rate: " << (approxSampleRate/1e6) << "Msps\n";
            approxSampleRate = 0;
        }
    }

    device->StreamStop(chipIndex);

    printf("FS: %f\n", samplesReceived/4e6);
    //Plot samples
#ifdef USE_GNU_PLOT
    if(samplesRead > 0)
    {
        if (mimo)
            gp.write("plot '-' with points, '-' with points\n");
        else
            gp.write("plot '-' with points\n");
        for(int i=0; i < (mimo ? 2 : 1); ++i)
        {
            if (stream.format ==  SDRDevice::StreamConfig::DataFormat::I16
                ||  SDRDevice::StreamConfig::DataFormat::I12)
                for (int j = 0; j < samplesRead; ++j)
                    gp.writef("%i %i\n", buffers[i][j].i, buffers[i][j].q);
            else
                for (int j = 0; j < samplesRead; ++j)
                    gp.writef("%f %f\n", buffers[i][j].i, buffers[i][j].q);
            gp.write("e\n");
        }
        gp.flush();

        if (mimo)
            gpTime.write("plot '-' with lines, '-' with lines, '-' with lines, '-' with lines\n");
        else
            gpTime.write("plot '-' with lines, '-' with lines\n");
        for(int i=0; i < (mimo ? 2 : 1); ++i)
        {
            if (stream.format ==  SDRDevice::StreamConfig::DataFormat::I16
                ||  SDRDevice::StreamConfig::DataFormat::I12)
            {
                for (int j = 0; j < samplesRead; ++j)
                    gpTime.writef("%i %i\n", j, buffers[i][j].i);
                gpTime.write("e\n");
                for (int j = 0; j < samplesRead; ++j)
                    gpTime.writef("%i %i\n", j, buffers[i][j].q);
                gpTime.write("e\n");
            }
        }
        gpTime.flush();
    }
#endif

    for (int i = 0; i < stream.rxCount; ++i)
        delete[] buffers[i];
    delete[] buffers;

    //Close device
    DeviceRegistry::freeDevice(device);

    return 0;
}
