    /**
    @file ConnectionSTREAMing.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection (streaming API)
*/

#include "ConnectionSTREAM.h"
#include "fifo.h" //from StreamerLTE
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>
#include <ciso646>
#include <FPGA_common.h>
#include "ErrorReporting.h"

using namespace lime;
using namespace std;

#define STREAM_MTU (PacketFrame::maxSamplesInPacket/(2))

/***********************************************************************
 * Streaming API implementation
 **********************************************************************/

int ConnectionSTREAM::SetupStream(size_t &streamID, const StreamConfig &config)
{
    if(rxRunning.load() == true || txRunning.load() == true)
        return ReportError(EPERM, "All streams must be stopped before doing setups");
    streamID = ~0;
    StreamChannel* stream = new StreamChannel(this);
    stream->config = config;
    //TODO check for duplicate streams
    if(config.isTx)
        mTxStreams.push_back(stream);
    else
        mRxStreams.push_back(stream);
    streamID = size_t(stream);
    return 0; //success
}

int ConnectionSTREAM::CloseStream(const size_t streamID)
{
    if(rxRunning.load() == true || txRunning.load() == true)
        return ReportError(EPERM, "All streams must be stopped before closing");
    StreamChannel *stream = (StreamChannel*)streamID;
    for(auto i=mRxStreams.begin(); i!=mRxStreams.end(); ++i)
    {
        if(*i==stream)
        {
            mRxStreams.erase(i);
            break;
        }
    }
    for(auto i=mTxStreams.begin(); i!=mTxStreams.end(); ++i)
    {
        if(*i==stream)
        {
            mTxStreams.erase(i);
            break;
        }
    }
    return 0;
}

size_t ConnectionSTREAM::GetStreamSize(const size_t streamID)
{
    return 680;
}

int ConnectionSTREAM::ControlStream(const size_t streamID, const bool enable)
{
    auto *stream = (StreamChannel* )streamID;
    assert(stream != nullptr);

    if(enable)
        return stream->Start();
    else
        return stream->Stop();
}

int ConnectionSTREAM::ReadStream(const size_t streamID, void* buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata)
{
    assert(streamID != 0);
    lime::IStreamChannel* channel = (lime::IStreamChannel*)streamID;
    lime::IStreamChannel::Metadata meta;
    meta.flags = 0;
    meta.flags |= metadata.hasTimestamp ? lime::IStreamChannel::Metadata::SYNC_TIMESTAMP : 0;
    meta.timestamp = metadata.timestamp;
    int status = channel->Read(buffs, length, &meta, timeout_ms);
    metadata.timestamp = meta.timestamp;
    return status;
}

int ConnectionSTREAM::WriteStream(const size_t streamID, const void* buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata)
{
    assert(streamID != 0);
    lime::IStreamChannel* channel = (lime::IStreamChannel*)streamID;
    lime::IStreamChannel::Metadata meta;
    meta.flags = 0;
    meta.flags |= metadata.hasTimestamp ? lime::IStreamChannel::Metadata::SYNC_TIMESTAMP : 0;
    meta.timestamp = metadata.timestamp;
    int status = channel->Write(buffs, length, &meta, timeout_ms);
    return status;
}

int ConnectionSTREAM::ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata &metadata)
{
    assert(streamID != 0);
    lime::IStreamChannel* channel = (lime::IStreamChannel*)streamID;
    StreamChannel::Info info = channel->GetInfo();
    metadata.hasTimestamp = true;
    metadata.timestamp = info.timestamp;
    metadata.lateTimestamp = info.underrun > 0;
    metadata.packetDropped = info.droppedPackets > 0;
    return 0;
}

/** @brief Configures FPGA PLLs to LimeLight interface frequency
*/
void ConnectionSTREAM::UpdateExternalDataRate(const size_t channel, const double txRate_Hz, const double rxRate_Hz)
{
    std::cout << "ConnectionSTREAM::ConfigureFPGA_PLL(tx=" << txRate_Hz/1e6 << "MHz, rx=" << rxRate_Hz/1e6 << "MHz)" << std::endl;
    const float txInterfaceClk = 2 * txRate_Hz;
    const float rxInterfaceClk = 2 * rxRate_Hz;
    mExpectedSampleRate = rxRate_Hz;
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
        lime::fpga::SetPllFrequency(this, 0, txInterfaceClk, clocks, 2);
    }
    else
        lime::fpga::SetDirectClocking(this, 0, txInterfaceClk, 90);

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
        lime::fpga::SetPllFrequency(this, 1, rxInterfaceClk, clocks, 2);
    }
    else
        lime::fpga::SetDirectClocking(this, 1, rxInterfaceClk, 90);
}

void ConnectionSTREAM::EnterSelfCalibration(const size_t channel)
{
    if(rxRunning)
    {
        generateData.store(true);
        std::unique_lock<std::mutex> lck(streamStateLock);
        //wait untill all existing USB transfers complete
        safeToConfigInterface.wait_for(lck, std::chrono::milliseconds(250));
    }
}

void ConnectionSTREAM::ExitSelfCalibration(const size_t channel)
{
    generateData.store(false);
}

uint64_t ConnectionSTREAM::GetHardwareTimestamp(void)
{
    //return mStreamService->mLastRxTimestamp + mStreamService->mTimestampOffset;
    return 0;
}

void ConnectionSTREAM::SetHardwareTimestamp(const uint64_t now)
{
    //if (not mStreamService) mStreamService.reset(new USBStreamService(this));
    //mStreamService->mTimestampOffset = int64_t(now)-int64_t(mStreamService->mLastRxTimestamp);
}

double ConnectionSTREAM::GetHardwareTimestampRate(void)
{
    return mExpectedSampleRate;
}

/** @brief Function dedicated for receiving data samples from board
    @param rxFIFO FIFO to store received data
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void ConnectionSTREAM::ReceivePacketsLoop(const ConnectionSTREAM::ThreadData args)
{
    ConnectionSTREAM* pthis = args.dataPort;
    auto dataPort = args.dataPort;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;
    auto generateData = args.generateData;
    auto safeToConfigInterface = args.safeToConfigInterface;

    //at this point FPGA has to be already configured to output samples
    const uint8_t chCount = args.channels.size();
    const uint8_t packetsToBatch = 64;
    const uint32_t bufferSize = packetsToBatch*sizeof(PacketLTE);
    const uint8_t buffersCount = 16; // must be power of 2
    vector<int> handles(buffersCount, 0);
    vector<char>buffers(buffersCount*bufferSize, 0);
    vector<ConnectionSTREAM::StreamChannel::Frame> chFrames;
    try
    {
        chFrames.resize(chCount);
    }
    catch (const std::bad_alloc &ex)
    {
        ReportError("Error allocating Rx buffers, not enough memory");
        return;
    }

    uint8_t activeTransfers = 0;
    for (int i = 0; i<buffersCount; ++i)
    {
        handles[i] = dataPort->BeginDataReading(&buffers[i*bufferSize], bufferSize);
        ++activeTransfers;
    }

    int bi = 0;
    unsigned long totalBytesReceived = 0; //for data rate calculation
    int m_bufferFailures = 0;
    int32_t droppedSamples = 0;
    int32_t packetLoss = 0;

    vector<uint32_t> samplesCollected(chCount, 0);
    vector<uint32_t> samplesReceived(chCount, 0);

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    std::mutex txFlagsLock;
    condition_variable resetTxFlags;
    //worker thread for reseting late Tx packet flags
    std::thread txReset([](ConnectionSTREAM* port,
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
    }, pthis, terminate, &txFlagsLock, &resetTxFlags);

    int resetFlagsDelay = 128;
    uint64_t prevTs = 0;
    while (terminate->load() == false)
    {
        if(generateData->load())
        {
            if(activeTransfers == 0) //stop FPGA when last transfer completes
                fpga::StopStreaming(pthis);
            safeToConfigInterface->notify_all(); //notify that it's safe to change chip config
            const int batchSize = (pthis->mExpectedSampleRate/chFrames[0].samplesCount)/10;
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
        if(handles[bi] >= 0)
        {
            if (dataPort->WaitForReading(handles[bi], 1000) == false)
                ++m_bufferFailures;
            bytesReceived = dataPort->FinishDataReading(&buffers[bi*bufferSize], bufferSize, handles[bi]);
            --activeTransfers;
            totalBytesReceived += bytesReceived;
            if (bytesReceived != bufferSize) //data should come in full sized packets
                ++m_bufferFailures;
        }
        bool txLate=false;
        for (uint8_t pktIndex = 0; pktIndex < bytesReceived / sizeof(PacketLTE); ++pktIndex)
        {
            const PacketLTE* pkt = (PacketLTE*)&buffers[bi*bufferSize];
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
                    resetFlagsDelay = packetsToBatch*buffersCount;
                }
            }
            uint8_t* pktStart = (uint8_t*)pkt[pktIndex].data;
            if(pkt[pktIndex].counter - prevTs != 1360/chCount)
            {
#ifndef NDEBUG
                printf("\tRx pktLoss@%i - ts diff: %li  pktLoss: %.1f\n", pktIndex, pkt[pktIndex].counter - prevTs, (pkt[pktIndex].counter - prevTs)/(1360.0/chCount));
#endif
                packetLoss += (pkt[pktIndex].counter - prevTs)/(1360.0/chCount);
            }
            prevTs = pkt[pktIndex].counter;
            for(int ch=0; ch<chCount; ++ch)
            {
                chFrames[ch].timestamp = pkt[pktIndex].counter;
                uint32_t &collected = samplesCollected[ch];
                const uint8_t stepSize = chCount * 3;
                for (uint16_t b = 0; b < sizeof(pkt->data); b += stepSize)
                {
                    int16_t sample;
                    //I sample
                    sample = (pktStart[b + 1 + 3 * ch] & 0x0F) << 8;
                    sample |= (pktStart[b + 3 * ch] & 0xFF);
                    sample = sample << 4;
                    sample = sample >> 4;
                    chFrames[ch].samples[collected].i = sample;

                    //Q sample
                    sample = pktStart[b + 2 + 3 * ch] << 4;
                    sample |= (pktStart[b + 1 + 3 * ch] >> 4) & 0x0F;
                    sample = sample << 4;
                    sample = sample >> 4;
                    chFrames[ch].samples[collected].q = sample;
                    ++collected;
                    ++samplesReceived[ch];
                }
                if(collected == chFrames[ch].samplesCount)
                {
                    IStreamChannel::Metadata meta;
                    meta.timestamp = chFrames[ch].timestamp;
                    meta.flags = 0;
                    uint32_t samplesPushed = args.channels[ch]->Write((const void*)chFrames[ch].samples, collected, &meta, 100);
                    if(samplesPushed != collected)
                        droppedSamples += collected-samplesPushed;
                    collected = 0;
                }
            }
        }
        // Re-submit this request to keep the queue full
        if(not generateData->load())
        {
            if(activeTransfers == 0) //reactivate FPGA and USB transfers
                fpga::StartStreaming(pthis);
            for(int i=0; i<buffersCount-activeTransfers; ++i)
            {
                handles[bi] = dataPort->BeginDataReading(&buffers[bi*bufferSize], bufferSize);
                bi = (bi + 1) & (buffersCount-1);
                ++activeTransfers;
            }
        }
        else
        {
            handles[bi] = -1;
            bi = (bi + 1) & (buffersCount-1);
        }
        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            t1 = t2;
            //total number of bytes sent per second
            double dataRate = 1000.0*totalBytesReceived / timePeriod;
            //each channel sample rate
            float samplingRate = 1000.0*samplesReceived[0] / timePeriod;
//#ifndef NDEBUG
            printf("Rx: %.3f MB/s, Fs: %.3f MHz, overrun: %i, loss: %i \n", dataRate / 1000000.0, samplingRate / 1000000.0, droppedSamples, packetLoss);
//#endif
            samplesReceived[0] = 0;
            totalBytesReceived = 0;
            m_bufferFailures = 0;
            droppedSamples = 0;
            packetLoss = 0;

            if (dataRate_Bps)
                dataRate_Bps->store((uint32_t)dataRate);
        }
    }
    dataPort->AbortReading();
    for (int j = 0; j<buffersCount; j++)
    {
        if(handles[bi] >= 0)
        {
            dataPort->WaitForReading(handles[bi], 1000);
            dataPort->FinishDataReading(&buffers[bi*bufferSize], bufferSize, handles[bi]);
        }
        bi = (bi + 1) & (buffersCount-1);
    }
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
void ConnectionSTREAM::TransmitPacketsLoop(const ConnectionSTREAM::ThreadData args)
{
    auto dataPort = args.dataPort;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;

    //at this point FPGA has to be already configured to output samples
    const uint8_t maxChannelCount = 2;
    const uint8_t chCount = args.channels.size();

    const uint8_t buffersCount = 16; // must be power of 2
    assert(buffersCount % 2 == 0);
    const uint8_t packetsToBatch = 64; //packets in single USB transfer
    const uint32_t bufferSize = packetsToBatch*4096;
    const uint32_t popTimeout_ms = 100;

    const int maxSamplesBatch = 1360/chCount;
    vector<int> handles(buffersCount, 0);
    vector<bool> bufferUsed(buffersCount, 0);
    vector<uint32_t> bytesToSend(buffersCount, 0);
    vector<complex16_t> samples[maxChannelCount];
    vector<char> buffers;
    try
    {
        for(int i=0; i<chCount; ++i)
            samples[i].resize(maxSamplesBatch);
        buffers.resize(buffersCount*bufferSize, 0);
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
    uint8_t bi = 0; //buffer index
    while (terminate->load() != true)
    {
        if (bufferUsed[bi])
        {
            if (dataPort->WaitForSending(handles[bi], 1000) == false)
                ++m_bufferFailures;
            uint32_t bytesSent = dataPort->FinishDataSending(&buffers[bi*bufferSize], bytesToSend[bi], handles[bi]);
            totalBytesSent += bytesSent;
            if (bytesSent != bytesToSend[bi])
                ++m_bufferFailures;
            bufferUsed[bi] = false;
        }
        int i=0;

        while(i<packetsToBatch)
        {
            IStreamChannel::Metadata meta;
            PacketLTE* pkt = reinterpret_cast<PacketLTE*>(&buffers[bi*bufferSize]);
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

            uint8_t* const dataStart = (uint8_t*)pkt[i].data;
            const uint8_t stepSize = chCount * 3;
            for (uint16_t b=0, s=0; b < sizeof(pkt->data) && s<maxSamplesBatch; b += stepSize)
            {
                for(int ch=0; ch<chCount; ++ch)
                {
                    //I sample
                    dataStart[b + 3 * ch] = samples[ch][s].i & 0xFF;
                    dataStart[b + 1 + 3 * ch] = (samples[ch][s].i >> 8) & 0x0F;

                    //Q sample
                    dataStart[b + 1 + 3 * ch] |= (samples[ch][s].q << 4) & 0xF0;
                    dataStart[b + 2 + 3 * ch] = (samples[ch][s].q >> 4) & 0xFF;
                }
                ++s;
                ++samplesSent;
            }
            ++i;
        }

        //bytesToSend[bi] = sizeof(PacketLTE)*i;
        bytesToSend[bi] = bufferSize;
        handles[bi] = dataPort->BeginDataSending(&buffers[bi*bufferSize], bytesToSend[bi]);
        bufferUsed[bi] = true;

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            float dataRate = 1000.0*totalBytesSent / timePeriod;
            //total number of samples from all channels per second
            float sampleRate = 1000.0*samplesSent / timePeriod;
            if(dataRate_Bps)
                dataRate_Bps->store(dataRate);
            m_bufferFailures = 0;
            samplesSent = 0;
            totalBytesSent = 0;
            t1 = t2;
//#ifndef NDEBUG
            printf("Tx: %.3f MB/s, Fs: %.3f MHz, failures: %i, ts:%li\n", dataRate / 1000000.0, sampleRate / 1000000.0, m_bufferFailures, timestamp);
//#endif
        }
        bi = (bi + 1) & (buffersCount-1);
    }

    // Wait for all the queued requests to be cancelled
    dataPort->AbortSending();
    for (int j = 0; j<buffersCount; j++)
    {
        if (bufferUsed[bi])
        {
            dataPort->WaitForSending(handles[bi], 1000);
            dataPort->FinishDataSending(&buffers[bi*bufferSize], bufferSize, handles[bi]);
        }
        bi = (bi + 1) & (buffersCount-1);
    }
    if (dataRate_Bps)
        dataRate_Bps->store(0);
}
