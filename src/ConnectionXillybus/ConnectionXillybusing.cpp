/**
    @file ConnectionXillybusing.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection (streaming API)
*/

#include "ConnectionXillybus.h"
#include "fifo.h"
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>
#include <ciso646>
#include <FPGA_common.h>
#include <ErrorReporting.h>

using namespace std;
using namespace lime;

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
int ConnectionXillybus::UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz)
{
    std::cout << "ConnectionXillybus::ConfigureFPGA_PLL(tx=" << txRate_Hz/1e6 << "MHz, rx=" << rxRate_Hz/1e6 << "MHz)" << std::endl;
    const float txInterfaceClk = 2 * txRate_Hz;
    const float rxInterfaceClk = 2 * rxRate_Hz;
    mExpectedSampleRate = rxRate_Hz;
    int status = 0;
    if(txInterfaceClk >= 5e6)
    {
        lime::fpga::FPGA_PLL_clock clocks[2];
        clocks[0].bypass = false;
        clocks[0].index = 0;
        clocks[0].outFrequency = txInterfaceClk;
        clocks[0].phaseShift_deg = 0;
        clocks[1].bypass = false;
        clocks[1].index = 1;
        clocks[1].outFrequency = txInterfaceClk;
        clocks[1].phaseShift_deg = 90;
        status = lime::fpga::SetPllFrequency(this, 0, txInterfaceClk, clocks, 2);
    }
    else
        status = lime::fpga::SetDirectClocking(this, 0, txInterfaceClk, 90);
    if(status != 0)
        return status;

    if(rxInterfaceClk >= 5e6)
    {
        lime::fpga::FPGA_PLL_clock clocks[2];
        clocks[0].bypass = false;
        clocks[0].index = 0;
        clocks[0].outFrequency = rxInterfaceClk;
        clocks[0].phaseShift_deg = 0;
        clocks[1].bypass = false;
        clocks[1].index = 1;
        clocks[1].outFrequency = rxInterfaceClk;
        clocks[1].phaseShift_deg = 90;
        status = lime::fpga::SetPllFrequency(this, 1, rxInterfaceClk, clocks, 2);
    }
    else
        status = lime::fpga::SetDirectClocking(this, 1, rxInterfaceClk, 90);
    return status;
}

/** @brief Function dedicated for receiving data samples from board
    @param rxFIFO FIFO to store received data
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void ConnectionXillybus::ReceivePacketsLoop(const ThreadData args)
{
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;
    auto generateData = args.generateData;
    auto safeToConfigInterface = args.safeToConfigInterface;

    //at this point FPGA has to be already configured to output samples
    const uint8_t chCount = args.channels.size();
    const auto link = args.channels[0]->config.linkFormat;
    const uint32_t samplesInPacket = (link == StreamConfig::STREAM_12_BIT_COMPRESSED ? 1360 : 1020)/chCount;

    double latency=0;
    for (int i = 0; i < chCount; i++)
    {
        latency += args.channels[i]->config.performanceLatency/chCount;
    }
    const unsigned tmp_cnt = (latency * 6)+0.5;

    const uint8_t packetsToBatch = (1<<tmp_cnt);
    const uint32_t bufferSize = packetsToBatch*sizeof(FPGA_DataPacket);
    vector<char>buffers(bufferSize, 0);
    vector<StreamChannel::Frame> chFrames;
    try
    {
        chFrames.resize(chCount);
    }
    catch (const std::bad_alloc &ex)
    {
        ReportError("Error allocating Rx buffers, not enough memory");
        return;
    }

    unsigned long totalBytesReceived = 0; //for data rate calculation
    int m_bufferFailures = 0;
    int32_t droppedSamples = 0;
    int32_t packetLoss = 0;

    vector<uint32_t> samplesReceived(chCount, 0);

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    std::mutex txFlagsLock;
    condition_variable resetTxFlags;
    //worker thread for reseting late Tx packet flags
    std::thread txReset([](ILimeSDRStreaming* port,
                        atomic<bool> *terminate,
                        mutex *spiLock,
                        condition_variable *doWork)
    {
        uint32_t reg9;
        port->ReadRegister(0x0009, reg9);
        const uint32_t addr[] = {0x0009, 0x0009};
        const uint32_t data[] = {reg9 | (1 << 1), reg9 & ~(1 << 1)};
        while (not terminate->load())
        {
            std::unique_lock<std::mutex> lck(*spiLock);
            doWork->wait(lck);
            port->WriteRegisters(addr, data, 2);
        }
    }, this, terminate, &txFlagsLock, &resetTxFlags);

    int resetFlagsDelay = 128;
    uint64_t prevTs = 0;
    while (terminate->load() == false)
    {
        if(generateData->load())
        {
            fpga::StopStreaming(this);
            safeToConfigInterface->notify_all(); //notify that it's safe to change chip config
            const int batchSize = (this->mExpectedSampleRate/chFrames[0].samplesCount)/10;
            IStreamChannel::Metadata meta;
            for(int i=0; i<batchSize; ++i)
            {
                for(int ch=0; ch<chCount; ++ch)
                {
                    meta.timestamp = chFrames[ch].timestamp;
                    for(int j=0; j<chFrames[ch].samplesCount; ++j)
                    {
                        chFrames[ch].samples[j].i = 0;
                        chFrames[ch].samples[j].q = 0;
                    }
                    uint32_t samplesPushed = args.channels[ch]->Write((const void*)chFrames[ch].samples, chFrames[ch].samplesCount, &meta);
                    samplesReceived[ch] += chFrames[ch].samplesCount;
                    if(samplesPushed != chFrames[ch].samplesCount)
                        printf("Rx samples pushed %i/%i\n", samplesPushed, chFrames[ch].samplesCount);
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        int32_t bytesReceived = 0;

        bytesReceived = this->ReceiveData(&buffers[0], bufferSize,200);
        totalBytesReceived += bytesReceived;
        if (bytesReceived != int32_t(bufferSize)) //data should come in full sized packets
            ++m_bufferFailures;

        bool txLate=false;
        for (uint8_t pktIndex = 0; pktIndex < bytesReceived / sizeof(FPGA_DataPacket); ++pktIndex)
        {
            const FPGA_DataPacket* pkt = (FPGA_DataPacket*)&buffers[0];
            const uint8_t byte0 = pkt[pktIndex].reserved[0];
            if ((byte0 & (1 << 3)) != 0 && !txLate) //report only once per batch
            {
                txLate = true;
                if(resetFlagsDelay > 0)
                    --resetFlagsDelay;
                else
                {
                    printf("L");
                    resetTxFlags.notify_one();
                    resetFlagsDelay = packetsToBatch*2;
                }
            }
            uint8_t* pktStart = (uint8_t*)pkt[pktIndex].data;
            if(pkt[pktIndex].counter - prevTs != samplesInPacket && pkt[pktIndex].counter != prevTs)
            {
#ifndef NDEBUG
                printf("\tRx pktLoss@%i - ts diff: %li  pktLoss: %.1f\n", pktIndex, pkt[pktIndex].counter - prevTs, float(pkt[pktIndex].counter - prevTs)/samplesInPacket);
#endif
                packetLoss += (pkt[pktIndex].counter - prevTs)/samplesInPacket;
            }
            prevTs = pkt[pktIndex].counter;
            if(args.lastTimestamp)
                args.lastTimestamp->store(pkt[pktIndex].counter);
            //parse samples
            vector<complex16_t*> dest(chCount);
            for(uint8_t c=0; c<chCount; ++c)
                dest[c] = (chFrames[c].samples);
            size_t samplesCount = 0;
            fpga::FPGAPacketPayload2Samples(pktStart, 4080, chCount, link, dest.data(), &samplesCount);

            for(int ch=0; ch<chCount; ++ch)
            {
                IStreamChannel::Metadata meta;
                meta.timestamp = pkt[pktIndex].counter;
                meta.flags = RingFIFO::OVERWRITE_OLD;
                uint32_t samplesPushed = args.channels[ch]->Write((const void*)chFrames[ch].samples, samplesCount, &meta, 100);
                if(samplesPushed != samplesCount)
                    droppedSamples += samplesCount-samplesPushed;
            }
        }
        // Re-submit this request to keep the queue full
        if(not generateData->load())
        {
            fpga::StartStreaming(this);
        }
        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            t1 = t2;
            //total number of bytes sent per second
            double dataRate = 1000.0*totalBytesReceived / timePeriod;
#ifndef NDEBUG
            //each channel sample rate
            float samplingRate = 1000.0*samplesReceived[0] / timePeriod;
            printf("Rx: %.3f MB/s, Fs: %.3f MHz, overrun: %i, loss: %i \n", dataRate / 1000000.0, samplingRate / 1000000.0, droppedSamples, packetLoss);
#endif
            samplesReceived[0] = 0;
            totalBytesReceived = 0;
            m_bufferFailures = 0;
            droppedSamples = 0;
            packetLoss = 0;

            if (dataRate_Bps)
                dataRate_Bps->store((uint32_t)dataRate);
        }
    }
    this->AbortReading();
    resetTxFlags.notify_one();
    txReset.join();
    if (dataRate_Bps)
        dataRate_Bps->store(0);
}

/** @brief Functions dedicated for transmitting packets to board
    @param txFIFO data source FIFO
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void ConnectionXillybus::TransmitPacketsLoop(const ThreadData args)
{
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;

    //at this point FPGA has to be already configured to output samples
    const uint8_t maxChannelCount = 2;
    const uint8_t chCount = args.channels.size();
    const auto link = args.channels[0]->config.linkFormat;

    double latency=0;
    for (int i = 0; i < chCount; i++)
    {
        latency += args.channels[i]->config.performanceLatency/chCount;
    }
    const unsigned tmp_cnt = (latency * 6)+0.5;
    const uint8_t packetsToBatch = (1<<tmp_cnt); //packets in single USB transfer
    const uint32_t bufferSize = packetsToBatch*4096;
    const uint32_t popTimeout_ms = 100;

    const int maxSamplesBatch = (link==StreamConfig::STREAM_12_BIT_COMPRESSED?1360:1020)/chCount;
    vector<complex16_t> samples[maxChannelCount];
    vector<char> buffers;
    try
    {
        for(int i=0; i<chCount; ++i)
            samples[i].resize(maxSamplesBatch);
        buffers.resize(bufferSize, 0);
    }
    catch (const std::bad_alloc& ex) //not enough memory for buffers
    {
        printf("Error allocating Tx buffers, not enough memory\n");
        return;
    }

    int m_bufferFailures = 0;
    long totalBytesSent = 0;

    uint32_t samplesSent = 0;

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    uint64_t timestamp = 0;
    while (terminate->load() != true)
    {
        int i=0;

        while(i<packetsToBatch)
        {
            IStreamChannel::Metadata meta;
            FPGA_DataPacket* pkt = reinterpret_cast<FPGA_DataPacket*>(&buffers[0]);
            for(int ch=0; ch<chCount; ++ch)
            {
                int samplesPopped = args.channels[ch]->Read(samples[ch].data(), maxSamplesBatch, &meta, popTimeout_ms);
                if (samplesPopped != maxSamplesBatch)
                {
                #ifndef NDEBUG
                    printf("Warning popping from TX, samples popped %i/%i\n", samplesPopped, maxSamplesBatch);
                #endif
                }
            }
            if(terminate->load() == true) //early termination
                break;
            pkt[i].counter = meta.timestamp;
            pkt[i].reserved[0] = 0;
            //by default ignore timestamps
            const int ignoreTimestamp = !(meta.flags & IStreamChannel::Metadata::SYNC_TIMESTAMP);
            pkt[i].reserved[0] |= ((int)ignoreTimestamp << 4); //ignore timestamp

            vector<complex16_t*> src(chCount);
            for(uint8_t c=0; c<chCount; ++c)
                src[c] = (samples[c].data());
            uint8_t* const dataStart = (uint8_t*)pkt[i].data;
            fpga::Samples2FPGAPacketPayload(src.data(), maxSamplesBatch, chCount, link, dataStart, nullptr);
            samplesSent += maxSamplesBatch;
            ++i;
        }

        uint32_t bytesSent = this->SendData(&buffers[0], bufferSize,200);
                totalBytesSent += bytesSent;
        if (bytesSent != bufferSize)
            ++m_bufferFailures;

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            float dataRate = 1000.0*totalBytesSent / timePeriod;
            if(dataRate_Bps)
                dataRate_Bps->store(dataRate);
            m_bufferFailures = 0;
            samplesSent = 0;
            totalBytesSent = 0;
            t1 = t2;
#ifndef NDEBUG
            //total number of samples from all channels per second
            float sampleRate = 1000.0*samplesSent / timePeriod;
            printf("Tx: %.3f MB/s, Fs: %.3f MHz, failures: %i, ts:%li\n", dataRate / 1000000.0, sampleRate / 1000000.0, m_bufferFailures, timestamp);
#endif
        }
    }

    // Wait for all the queued requests to be cancelled
    this->AbortSending();
    if (dataRate_Bps)
        dataRate_Bps->store(0);
}
