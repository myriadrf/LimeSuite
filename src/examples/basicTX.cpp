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

//    config.channel[0].rx.enabled = true;
    config.channel[0].tx.enabled = true;
    //config.channel[0].rx.centerFrequency = frequencyLO;
    config.channel[0].tx.centerFrequency = frequencyLO;
//    config.channel[0].rx.sampleRate = sampleRate;
    config.channel[0].tx.sampleRate = sampleRate;
    config.channel[0].rx.oversample = 2; // ???
    config.channel[0].tx.oversample = 2;
//    config.channel[0].rx.lpf = 0;//5e6;
    config.channel[0].tx.lpf = 0;//5e6;
//    config.channel[0].rx.gfir.enabled = false;
    //config.channel[0].rx.gfir.bandwidth = config.channel[0].rx.lpf;
//    config.channel[0].rx.path = 2; // Loopback_1 // TODO: replace with string names
    config.channel[0].tx.path = 2; // band1 // TODO: replace with string names
//    config.channel[0].rx.calibrate = false;
    config.channel[0].tx.calibrate = false;
//    config.channel[0].rx.testSignal = false;
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
//    stream.statusCallback = OnStreamStatusChange;
    stream.userData = &streamHadIssues; // gets set to true if problems occour
    device->StreamSetup(stream, chipIndex);
    std::cout << "After StreamSetup " << std::endl;
    int testStreamIndex = chipIndex;
    device->StreamStart(testStreamIndex);
    const lime::complex32f_t *src[2] = {txPattern[0].data(), txPattern[1].data()};
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
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
 //std::cout << "streaming..." << std::endl;

        //Print data rate (once per second)
        if (std::chrono::high_resolution_clock::now() - t2 > std::chrono::seconds(1))
        {
            t2 = std::chrono::high_resolution_clock::now();
//            lms_stream_status_t status;
//            LMS_GetStreamStatus(&tx_stream, &status);  //Get stream status
  //          cout << "TX data rate: " << status.linkRate / 1e6 << " MB/s\n"; //link data rate
            std::cout << "streaming..." << std::endl;
        }
    }
    
    
    
#ifdef eee
    //Enable TX channel,Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, true)!=0)
        error();

    //Set sample rate
    if (LMS_SetSampleRate(device, sample_rate, 0)!=0)
        error();
    cout << "Sample rate: " << sample_rate/1e6 << " MHz" << endl;

    //Set center frequency
    if (LMS_SetLOFrequency(device,LMS_CH_TX, 0, frequency)!=0)
        error();
    cout << "Center frequency: " << frequency/1e6 << " MHz" << endl;

    //select TX1_1 antenna
    if (LMS_SetAntenna(device, LMS_CH_TX, 0, LMS_PATH_TX1)!=0)
        error();

    //set TX gain
    if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 0.7) != 0)
        error();

    //calibrate Tx, continue on failure
    LMS_Calibrate(device, LMS_CH_TX, 0, sample_rate, 0);
    
    //Streaming Setup
    
    lms_stream_t tx_stream;                 //stream structure
    tx_stream.channel = 0;                  //channel number
    tx_stream.fifoSize = 256*1024;          //fifo size in samples
    tx_stream.throughputVsLatency = 0.5;    //0 min latency, 1 max throughput
    tx_stream.dataFmt = lms_stream_t::LMS_FMT_F32; //floating point samples
    tx_stream.isTx = true;                  //TX channel
    LMS_SetupStream(device, &tx_stream);

    //Initialize data buffers
    const int buffer_size = 1024*8;
    float tx_buffer[2*buffer_size];     //buffer to hold complex values (2*samples))
    for (int i = 0; i <buffer_size; i++) {      //generate TX tone
        const double pi = acos(-1);
        double w = 2*pi*i*f_ratio;
        tx_buffer[2*i] = cos(w);
        tx_buffer[2*i+1] = sin(w);
    }   
    cout << "Tx tone frequency: " << tone_freq/1e6 << " MHz" << endl;

    const int send_cnt = int(buffer_size*f_ratio) / f_ratio; 
    cout << "sample count per send call: " << send_cnt << std::endl;

    LMS_StartStream(&tx_stream);         //Start streaming
    //Streaming
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;
    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(10)) //run for 10 seconds
    {
        //Transmit samples
        int ret = LMS_SendStream(&tx_stream, tx_buffer, send_cnt, nullptr, 1000);
        if (ret != send_cnt)
            cout << "error: samples sent: " << ret << "/" << send_cnt << endl;
        //Print data rate (once per second)
        if (chrono::high_resolution_clock::now() - t2 > chrono::seconds(1))
        {
            t2 = chrono::high_resolution_clock::now();
            lms_stream_status_t status;
            LMS_GetStreamStatus(&tx_stream, &status);  //Get stream status
            cout << "TX data rate: " << status.linkRate / 1e6 << " MB/s\n"; //link data rate
        }
    }
    //Stop streaming
    LMS_StopStream(&tx_stream);
    LMS_DestroyStream(device, &tx_stream);

    //Disable TX channel
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, false)!=0)
        error();

    //Close device
    if (LMS_Close(device)==0)
        cout << "Closed" << endl;
#endif
    return 0;
}
