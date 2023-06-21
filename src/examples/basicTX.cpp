/**
    @file   basicTX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal TX example
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

    config.channel[0].tx.enabled = true;
    config.channel[0].tx.centerFrequency = frequencyLO;
    config.channel[0].tx.sampleRate = sampleRate;
    config.channel[0].rx.oversample = 2; // ???
    config.channel[0].tx.oversample = 2;
    config.channel[0].tx.lpf = 0;
    config.channel[0].tx.path = 2; // band1 // TODO: replace with string names
    config.channel[0].tx.calibrate = false;
    config.channel[0].tx.testSignal = false;

    stream.rxCount = 0;
    stream.txCount = 1;
    stream.txChannels[0] = 0;
    stream.format = SDRDevice::StreamConfig::DataFormat::F32;
    stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
    stream.alignPhase = false;

    std::cout << "Configuring device ...\n";
    try {
        auto t1 = std::chrono::high_resolution_clock::now();
        //device->Init();
        device->Configure(config, chipIndex);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "SDR configured in " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
            << "ms\n";
    }
    catch ( std::runtime_error &e) {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        error ();
    }
    catch ( std::logic_error &e) {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        error ();
    }
    // precomputing tx samples here, the result might not be continous
    // each packet with different amplitude to distinguish them in time
    std::vector< std::vector<complex32f_t> > txPattern(2);
    const int txPacketCount = 4;
    const int samplesInPkt = 256;
    for(uint i=0; i<txPattern.size(); ++i)
    {
        txPattern[i].resize(txPacketCount*samplesInPkt);
        for(int j=0; j<txPacketCount; ++j)
        {
            float src[4] = {1.0, 0.0, -1.0, 0.0};
            float ampl = 1.0;
            for(int k=0; k<samplesInPkt; ++k)
            {
                txPattern[i][j*samplesInPkt+k].i = src[k & 3] * ampl;
                txPattern[i][j*samplesInPkt+k].q = src[(k+1) & 3] * ampl;
            }
        }
    }
   //Initialize stream
    bool streamHadIssues = false;
    stream.userData = &streamHadIssues; // gets set to true if problems occour
    device->StreamSetup(stream, chipIndex);
    device->StreamStart(chipIndex);
    std::cout << "Stream started ...\n";
    const lime::complex32f_t *src[2] = {txPattern[0].data(), txPattern[1].data()};
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    signal(SIGINT, intHandler);
    while (std::chrono::high_resolution_clock::now() - t1 < std::chrono::seconds(10) && !stoping) //run for 10 seconds
    {
        SDRDevice::StreamMeta txMeta;

        txMeta.timestamp = 0;
        txMeta.useTimestamp = false;
        txMeta.flush = false; // not really matters because of continuous trasmitting
        int samplesSent = device->StreamTx (chipIndex, src, samplesInPkt, &txMeta);
        if (samplesSent < 0)
        {
            printf("Failure to send\n");
            error ();
        }
        //Print data rate (once per second)
        if (std::chrono::high_resolution_clock::now() - t2 > std::chrono::seconds(1))
        {
            t2 = std::chrono::high_resolution_clock::now();
            SDRDevice::StreamStats rxstat,txstat;
            device->StreamStatus (chipIndex, &rxstat, &txstat);
            std::cout << "TX data rate: " << txstat.dataRate_Bps / 1e6 << " MB/s\n"; //link data rate
        }
    }
    return 0;
}
