/**
    @file   basicTX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal TX example
 */

#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <signal.h>

using namespace lime;

static const double frequencyLO = 2e9;
float sampleRate = 10e6;
static uint8_t chipIndex = 0; // device might have several RF chips

bool stopProgram(false);
void intHandler(int dummy)
{
    std::cout << "Stoppping\n";
    stopProgram = true;
}

static SDRDevice::LogLevel logVerbosity = SDRDevice::LogLevel::VERBOSE;
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
    config.channel[0].tx.enabled = true;
    config.channel[0].rx.enabled = false;
    config.channel[0].rx.centerFrequency = frequencyLO;
    config.channel[0].tx.centerFrequency = frequencyLO;
    config.channel[0].rx.sampleRate = sampleRate;
    config.channel[0].rx.oversample = 2;
    config.channel[0].tx.sampleRate = sampleRate;
    config.channel[0].tx.oversample = 2;
    config.channel[0].tx.lpf = 0;
    config.channel[0].tx.path = 2; // TODO: replace with string names
    config.channel[0].tx.calibrate = true;
    config.channel[0].tx.testSignal.enabled = false;

    std::cout << "Configuring device ...\n";
    try
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        device->Configure(config, chipIndex);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "SDR configured in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";

        // Samples data streaming configuration
        SDRDevice::StreamConfig stream;

        stream.channels[TRXDir::Tx] = { 0 };

        stream.format = SDRDevice::StreamConfig::DataFormat::F32;
        stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;

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
    signal(SIGINT, intHandler);

    std::vector<std::vector<complex32f_t>> txPattern(2);
    const int txPacketCount = 4;
    const int samplesInPkt = 1024;
    for (uint i = 0; i < txPattern.size(); ++i)
    {
        txPattern[i].resize(txPacketCount * samplesInPkt);
        for (int j = 0; j < txPacketCount; ++j)
        {
            float src[4] = { 1.0, 0.0, -1.0, 0.0 };
            float ampl = 0.8;
            for (int k = 0; k < samplesInPkt; ++k)
            {
                txPattern[i][j * samplesInPkt + k].q = src[k & 3] * ampl;
                txPattern[i][j * samplesInPkt + k].i = src[(k + 1) & 3] * ampl;
            }
        }
    }

    std::cout << "Stream started ...\n";
    const lime::complex32f_t* src[2] = { txPattern[0].data(), txPattern[1].data() };
    auto startTime = std::chrono::high_resolution_clock::now();
    auto t1 = startTime;
    auto t2 = t1;

    SDRDevice::StreamMeta txMeta;
    txMeta.timestamp = 0;
    txMeta.waitForTimestamp = true;
    txMeta.flushPartialPacket = true;

    uint32_t totalSamplesSent = 0;

    while (std::chrono::high_resolution_clock::now() - startTime < std::chrono::seconds(10) && !stopProgram) //run for 10 seconds
    {
        uint32_t samplesToSend = samplesInPkt * txPacketCount;
        uint32_t samplesSent = device->StreamTx(chipIndex, src, samplesToSend, &txMeta);
        if (samplesSent < 0)
        {
            printf("Failure to send\n");
            break;
        }
        if (samplesSent > 0)
        {
            txMeta.timestamp += samplesSent;
            totalSamplesSent += samplesSent;
        }
        //Print data rate (once per second)
        t2 = std::chrono::high_resolution_clock::now();
        if (t2 - t1 > std::chrono::seconds(1))
        {
            t1 = std::chrono::high_resolution_clock::now();
            std::cout << "TX total samples sent: " << totalSamplesSent << std::endl;
        }
    }

    DeviceRegistry::freeDevice(device);

    return 0;
}
