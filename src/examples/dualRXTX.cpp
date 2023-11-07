/**
    @file   dualRXTX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal RX loopback to Tx example
 */
#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <signal.h>
#ifdef USE_GNU_PLOT
    #include "gnuPlotPipe.h"
#endif

using namespace lime;

static const double frequencyLO = 1.5e9;
float sampleRate = 10e6;
static uint8_t chipIndex = 0; // device might have several RF chips

bool stopProgram(false);
void intHandler(int dummy)
{
    std::cout << "Stoppping\n";
    stopProgram = true;
}

static SDRDevice::LogLevel logVerbosity = SDRDevice::VERBOSE;
static void LogCallback(SDRDevice::LogLevel lvl, const char* msg)
{
    if (lvl > logVerbosity)
        return;
    printf("%s\n", msg);
}

int main(int argc, char** argv)
{
    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0)
    {
        printf("No devices found\n");
        return -1;
    }
    std::cout << "Devices found :" << std::endl;
    for (size_t i = 0; i < handles.size(); i++)
        std::cout << i << ": " << handles[i].Serialize() << std::endl;
    std::cout << std::endl;

    // Use first available device
    SDRDevice* device = DeviceRegistry::makeDevice(handles.at(0));
    if (!device)
    {
        std::cout << "Failed to connect to device" << std::endl;
        return -1;
    }
    device->SetMessageLogCallback(LogCallback);
    device->Init();

    // RF parameters
    SDRDevice::SDRConfig config;
    for (int c = 0; c < 2; ++c) // MIMO
    {
        config.channel[c].rx.enabled = true;
        config.channel[c].rx.centerFrequency = frequencyLO;
        config.channel[c].rx.sampleRate = sampleRate;
        config.channel[c].rx.oversample = 2;
        config.channel[c].rx.lpf = 0;
        config.channel[c].rx.path = 2; // TODO: replace with string names
        config.channel[c].rx.calibrate = false;

        config.channel[c].tx.enabled = true;
        config.channel[c].tx.sampleRate = sampleRate;
        config.channel[c].tx.oversample = 2;
        config.channel[c].tx.path = 2; // TODO: replace with string names
        config.channel[c].tx.centerFrequency = frequencyLO;
        config.channel[c].tx.calibrate = false;
    }

    // Samples data streaming configuration
    SDRDevice::StreamConfig stream;
    stream.rxCount = 2; // rx channels count
    stream.txCount = 2;
    stream.rxChannels[0] = 0;
    stream.rxChannels[1] = 1;
    stream.txChannels[0] = 0;
    stream.txChannels[1] = 1;
    stream.format = SDRDevice::StreamConfig::DataFormat::F32;
    stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;

    signal(SIGINT, intHandler);

    const int samplesInBuffer = 256 * 4;
    complex32f_t** rxSamples = new complex32f_t*[2]; // allocate two channels for simplicity
    for (int i = 0; i < 2; ++i)
        rxSamples[i] = new complex32f_t[samplesInBuffer];

#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    gp.write("set size square\n set xrange[-1:1]\n set yrange[-1:1]\n");
#endif

    std::cout << "Configuring device ...\n";
    try
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        device->Configure(config, chipIndex);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "SDR configured in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";

        device->StreamSetup(stream, chipIndex);
        device->StreamStart(chipIndex);

    } catch (std::runtime_error& e)
    {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        return -1;
    } catch (std::logic_error& e)
    {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        return -1;
    }
    std::cout << "Stream started ...\n";

    auto startTime = std::chrono::high_resolution_clock::now();
    auto t1 = startTime;
    auto t2 = t1;

    int totalSamplesReceived = 0;
    int totalSamplesSent = 0;
    float maxSignalAmplitude = 0;

    SDRDevice::StreamMeta rxMeta;
    while (std::chrono::high_resolution_clock::now() - startTime < std::chrono::seconds(10) && !stopProgram)
    {
        int samplesRead = device->StreamRx(chipIndex, rxSamples, samplesInBuffer, &rxMeta);
        totalSamplesReceived += samplesRead;

        // process samples
        for (int n = 0; n < samplesRead; ++n)
        {
            float amplitude = pow(rxSamples[0][n].i, 2) + pow(rxSamples[0][n].q, 2);
            if (amplitude > maxSignalAmplitude)
                maxSignalAmplitude = amplitude;
        }

        SDRDevice::StreamMeta txMeta;
        txMeta.timestamp = rxMeta.timestamp + samplesInBuffer * 64;
        txMeta.useTimestamp = true;
        txMeta.flush = false;
        int samplesSent = device->StreamTx(chipIndex, rxSamples, samplesInBuffer, &txMeta);
        if (samplesSent < 0)
        {
            printf("Failure to send\n");
            break;
        }
        totalSamplesSent += samplesSent;

        t2 = std::chrono::high_resolution_clock::now();
        if (t2 - t1 > std::chrono::seconds(1))
        {
            t1 = t2;
            std::cout << "Total samples received: " << totalSamplesReceived << "  signal amplitude: " << sqrt(maxSignalAmplitude)
                      << "  total samples sent: " << totalSamplesSent << std::endl;

#ifdef USE_GNU_PLOT
            gp.write("plot '-' with points title 'ch 0'");
            for (int c = 1; c < stream.rxCount; ++c)
                gp.writef(", '-' with points title 'ch %i'\n", c);
            for (int c = 0; c < stream.rxCount; ++c)
            {
                for (uint32_t n = 0; n < samplesInBuffer; ++n)
                    gp.writef("%f %f\n", rxSamples[c][n].i, rxSamples[c][n].q);
                gp.write("e\n");
                gp.flush();
            }
#endif
            maxSignalAmplitude = 0;
        }
    }
    DeviceRegistry::freeDevice(device);

    for (int i = 0; i < 2; ++i)
        delete[] rxSamples[i];
    delete[] rxSamples;
    return 0;
}
