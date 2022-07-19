/**
    @file   singleRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  RX example
 */
#include "SDRDevice.h"
#include "dataTypes.h"
#include "LMS7002M.h"
#include "DeviceRegistry.h"
#include <iostream>
#include <chrono>
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

using namespace lime;
using namespace std;

//Device structure, should be initialize to NULL
SDRDevice *device = NULL;

int error()
{
    if (device != NULL)
        DeviceRegistry::freeDevice(device);
    printf("Error\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    DeviceHandle hanle;

    //Find devices

    auto handles = DeviceRegistry::findDevices();
    for (uint32_t i = 0; i < handles.size(); i++) //print device list
        cout << i << ": " << handles[i].serialize() << endl;
    cout << endl;

    if (handles.size() == 0) {
        printf("No devices found\n");
        return -1;
    }

    //Open the first device
    device = DeviceRegistry::makeDevice(handles[0]);

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    device->Init();

    SDRDevice::SDRConfig config;

    for (int i = 0; i < 2; ++i) {
        config.channel[i].rxEnabled = true;
        config.channel[i].rxCenterFrequency = 1e9;
        config.channel[i].rxSampleRate = 5e6;
        config.channel[i].rxPath = 1;
        config.channel[i].rxCalibrate = true;
    }

    try {
        device->Configure(config);
    }
    catch (...) {
        printf("Failed to configure settings\n");
        return -1;
    }

    SDRDevice::StreamConfig stream;
    stream.rxCount = 2;
    stream.rxChannels[0] = 0;
    stream.rxChannels[1] = 1;
    stream.txCount = 0;
    stream.format = SDRDevice::StreamConfig::DataFormat::FMT_INT16;
    stream.linkFormat = SDRDevice::StreamConfig::DataFormat::FMT_INT16;
    stream.alignPhase = false;

    //Initialize stream
    device->StreamStart(stream);

    //Data buffers
    const int samplesInPkt = 1020;
    const int bufersize = samplesInPkt * 16; //complex samples per buffer
    lime::complex16_t **buffers = new complex16_t *[stream.rxCount];
    for (int i = 0; i < stream.rxCount; ++i)
        buffers[i] = new lime::complex16_t[bufersize];

#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    gp.write("set size square\n set xrange[-1:1]\n set yrange[-1:1]\n");
#endif
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    int samplesReceived = 0;

    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(2)) //run for 10 seconds
    {
        int samplesRead;
        //Receive samples
        samplesRead = device->StreamRx(0, (void **)buffers, bufersize, nullptr);
        samplesReceived += samplesRead;
        //I and Q samples are interleaved in buffer: IQIQIQ...

        //Plot samples
#ifdef USE_GNU_PLOT
        gp.write("plot '-' with points\n");
        for (int j = 0; j < samplesRead; ++j)
            gp.writef("%f %f\n", buffer[2 * j], buffer[2 * j + 1]);
        gp.write("e\n");
        gp.flush();
#endif
        //Print stats (once per second)
        if (chrono::high_resolution_clock::now() - t2 > chrono::seconds(1)) {
            t2 = chrono::high_resolution_clock::now();
            cout << "Samples Got: " << samplesReceived << "\n";
        }
    }

    device->StreamStop();

    for (int i = 0; i < stream.rxCount; ++i)
        delete[] buffers[i];
    delete[] buffers;

    //Close device
    DeviceRegistry::freeDevice(device);

    return 0;
}
