#include <assert.h>
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"
#include <ciso646>
#include "Logger.h"
#include <complex>

#include "TRXLooper_USB.h"
#include "USBGeneric.h"

using namespace std::literals::string_literals;

namespace lime {

TRXLooper_USB::TRXLooper_USB(std::shared_ptr<USBGeneric> comms, FPGA* f, LMS7002M* chip, uint8_t rxEndPt, uint8_t txEndPt)
    : TRXLooper(f, chip, 0)
    , comms(comms)
    , rxEndPt(rxEndPt)
    , txEndPt(txEndPt)
{
}

TRXLooper_USB::~TRXLooper_USB()
{
    Stop();
    mRx.terminate.store(true, std::memory_order_relaxed);
    mTx.terminate.store(true, std::memory_order_relaxed);

    // Thread joining code has to be as high as possible,
    // so that every variable and virtual function is still properly accessible when the thread is still executing.
    // Otherwise, it can cause crashes when the destructor is being called before the thread is fully stopped and
    // it is still trying to access variables, or, even worse, virtual functions of the class.
    if (mTx.thread.joinable())
    {
        mTx.thread.join();
    }

    if (mRx.thread.joinable())
    {
        mRx.thread.join();
    }
}

OpStatus TRXLooper_USB::Setup(const lime::SDRDevice::StreamConfig& config)
{
    mConfig = config;

    if (config.channels.at(TRXDir::Rx).size() > 0)
    {
        RxSetup();
    }

    if (config.channels.at(TRXDir::Tx).size() > 0)
    {
        TxSetup();
    }

    return TRXLooper::Setup(config);
}

int TRXLooper_USB::TxSetup()
{
    const std::string name = "MemPool_Tx" + std::to_string(chipId);

    const int channelCount = std::max(mConfig.channels.at(TRXDir::Tx).size(), mConfig.channels.at(TRXDir::Rx).size());
    const int upperAllocationLimit =
        sizeof(complex32f_t) * mTx.packetsToBatch * mTx.samplesInPkt * channelCount + SamplesPacketType::headerSize;

    const int memPoolBlockCount = 1024;
    const int memPoolAlignment = 4096;
    mTx.memPool = new MemoryPool(memPoolBlockCount, upperAllocationLimit, memPoolAlignment, name);

    return 0;
}

bool TRXLooper_USB::GetSamplesPacket(SamplesPacketType** srcPkt)
{
    if ((*srcPkt) != nullptr && !(*srcPkt)->empty())
    {
        return true;
    }

    if ((*srcPkt) != nullptr)
    {
        mTx.memPool->Free((*srcPkt));
        (*srcPkt) = nullptr;
    }

    if (!mTx.fifo->pop(srcPkt, true, 100))
    {
        return false;
    }

    NegateQ((*srcPkt), TRXDir::Tx);

    return true;
}

void TRXLooper_USB::TransmitPacketsLoop()
{
    //at this point FPGA has to be already configured to output samples

    DataConversion conversion;
    conversion.destFormat = mConfig.linkFormat;
    conversion.srcFormat = mConfig.format;
    conversion.channelCount = std::max(mConfig.channels.at(lime::TRXDir::Tx).size(), mConfig.channels.at(lime::TRXDir::Rx).size());

    const uint8_t batchCount = 8; // how many async reads to schedule
    const uint8_t packetsToBatch = mTx.packetsToBatch;
    const uint32_t bufferSize = packetsToBatch * sizeof(FPGA_TxDataPacket);

    std::vector<int> handles(batchCount, -1);
    std::vector<uint8_t> buffers(batchCount * bufferSize, 0);
    int bufferIndex = 0;

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    uint64_t totalBytesSent = 0; //for data rate calculation

    SamplesPacketType* srcPkt = nullptr;

    bool isBufferFull = false;
    uint32_t payloadSize = 0;
    uint32_t bytesUsed = 0;
    uint32_t packetsCreated = 0;

    const bool packed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    uint32_t samplesInPkt = (packed ? 1360 : 1020) / conversion.channelCount;

    const bool mimo = std::max(mConfig.channels.at(lime::TRXDir::Tx).size(), mConfig.channels.at(lime::TRXDir::Rx).size()) > 1;
    const int bytesForFrame = (packed ? 3 : 4) * (mimo ? 2 : 1);
    uint32_t maxPayloadSize = std::min(4080u, bytesForFrame * samplesInPkt);

    const uint8_t safeTxEndPt = txEndPt; // To make sure no undefined behaviour happens when killing the thread

    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lock(streamMutex);
        while (!mStreamEnabled && !mTx.terminate.load(std::memory_order_relaxed))
            streamActive.wait_for(lock, std::chrono::milliseconds(100));
        lock.unlock();
    }

    StreamHeader* header = reinterpret_cast<StreamHeader*>(&buffers[bufferIndex * bufferSize]);
    uint8_t* payloadPtr = reinterpret_cast<uint8_t*>(header) + sizeof(StreamHeader);

    while (!mTx.terminate.load(std::memory_order_relaxed))
    {
        if (handles[bufferIndex] >= 0)
        {
            const int timeToWaitMs = 1000;
            if (comms->WaitForXfer(handles[bufferIndex], timeToWaitMs))
            {
                int bytesSent = comms->FinishDataXfer(&buffers[bufferIndex * bufferSize], bufferSize, handles[bufferIndex]);
                totalBytesSent += bytesSent;
                mTx.stats.bytesTransferred += bytesSent;
                handles[bufferIndex] = -1;
                mTx.stats.packets++;
            }
            else
            {
                // TODO: callback for Rx timeout
                lime::error("Tx WaitForXfer timeout"s);
                continue;
            }
        }

        if (!GetSamplesPacket(&srcPkt))
        {
            std::this_thread::yield();
            continue;
        }

        while (!srcPkt->empty())
        {
            if ((payloadSize >= maxPayloadSize || payloadSize == samplesInPkt * bytesForFrame) &&
                bytesUsed + sizeof(StreamHeader) <= bufferSize)
            {
                header = reinterpret_cast<StreamHeader*>(&buffers[bufferIndex * bufferSize + bytesUsed]);
                payloadPtr = reinterpret_cast<uint8_t*>(header) + sizeof(StreamHeader);
                payloadSize = 0;
            }

            if (payloadSize == 0)
            {
                header->Clear();
                ++packetsCreated;
                header->counter = srcPkt->timestamp;
                bytesUsed += sizeof(StreamHeader);
            }

            header->ignoreTimestamp(!srcPkt->useTimestamp);

            const uint32_t freeSpace = std::min(maxPayloadSize - payloadSize, bufferSize - bytesUsed);
            uint32_t transferCount = std::min(freeSpace / bytesForFrame, static_cast<uint32_t>(srcPkt->size()));
            transferCount = std::min(transferCount, samplesInPkt);

            if (transferCount > 0)
            {
                int samplesDataSize = Interleave(srcPkt, transferCount, conversion, payloadPtr);

                payloadPtr += samplesDataSize;
                payloadSize += samplesDataSize;
                bytesUsed += samplesDataSize;

                header->SetPayloadSize(payloadSize == 4080 ? 0 : payloadSize);

                assert(payloadSize > 0);
                assert(payloadSize <= maxPayloadSize);
            }
            else
            {
                isBufferFull = true;
            }

            if ((packetsCreated >= packetsToBatch && payloadSize >= maxPayloadSize) || (bytesUsed >= bufferSize))
            {
                isBufferFull = true;
            }

            if (isBufferFull)
            {
                handles[bufferIndex] = comms->BeginDataXfer(&buffers[bufferIndex * bufferSize], bufferSize, txEndPt);
                bufferIndex = (bufferIndex + 1) % batchCount;
                isBufferFull = false;
                bytesUsed = 0;
                payloadSize = 0;
                packetsCreated = 0;

                mTx.stats.timestamp = srcPkt->timestamp;

                header = reinterpret_cast<StreamHeader*>(&buffers[bufferIndex * bufferSize]);
                payloadPtr = reinterpret_cast<uint8_t*>(header) + sizeof(StreamHeader);

                break;
            }
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

        if (timePeriod >= std::chrono::milliseconds(1000))
        {
            t1 = t2;

            float dataRate = 1000.0 * totalBytesSent / timePeriod.count();
            lime::info("Tx: %.3f MB/s", dataRate / 1000000.0);
            totalBytesSent = 0;

            mTx.stats.dataRate_Bps = dataRate;
        }
    }

    comms->AbortEndpointXfers(safeTxEndPt);
    mTx.stats.dataRate_Bps = 0;
}

int TRXLooper_USB::RxSetup()
{
    const std::string name = "MemPool_Rx" + std::to_string(chipId);

    const int channelCount = std::max(mConfig.channels.at(lime::TRXDir::Tx).size(), mConfig.channels.at(lime::TRXDir::Rx).size());
    const int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / channelCount;
    const uint8_t packetsToBatch = mRx.packetsToBatch;

    const int upperAllocationLimit =
        sizeof(complex32f_t) * packetsToBatch * samplesInPkt * channelCount + SamplesPacketType::headerSize;

    const int memPoolBlockCount = 1024;
    const int memPoolAlignment = 4096;
    mRx.memPool = new MemoryPool(memPoolBlockCount, upperAllocationLimit, memPoolAlignment, name);

    return 0;
}

/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_USB::ReceivePacketsLoop()
{
    const uint8_t safeRxEndPt = rxEndPt; // To make sure no undefined behaviour happens when killing the thread
    //at this point FPGA has to be already configured to output samples

    DataConversion conversion;
    conversion.srcFormat = mConfig.linkFormat;
    conversion.destFormat = mConfig.format;
    conversion.channelCount = std::max(mConfig.channels.at(lime::TRXDir::Tx).size(), mConfig.channels.at(lime::TRXDir::Rx).size());

    const uint8_t batchCount = 8; // how many async reads to schedule
    const uint8_t packetsToBatch = mRx.packetsToBatch;
    const uint32_t bufferSize = packetsToBatch * sizeof(FPGA_RxDataPacket);

    std::vector<int> handles(batchCount, -1);
    std::vector<uint8_t> buffers(batchCount * bufferSize, 0);
    int bufferIndex = 0;

    const int samplesInPkt =
        (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / conversion.channelCount;
    const int outputSampleSize =
        mConfig.format == SDRDevice::StreamConfig::DataFormat::F32 ? sizeof(complex32f_t) : sizeof(complex16_t);
    const int32_t outputPktSize = SamplesPacketType::headerSize + packetsToBatch * samplesInPkt * outputSampleSize;

    SamplesPacketType* outputPkt = nullptr;
    int64_t expectedTS = 0;

    SDRDevice::StreamStats& stats = mRx.stats;

    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lock(streamMutex);
        while (!mStreamEnabled && !mRx.terminate.load(std::memory_order_relaxed))
            streamActive.wait_for(lock, std::chrono::milliseconds(100));
        lock.unlock();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    uint64_t totalBytesReceived = 0; //for data rate calculation

    for (int i = 0; i < batchCount; ++i)
    {
        handles[i] = comms->BeginDataXfer(&buffers[i * bufferSize], bufferSize, safeRxEndPt);
    }

    while (!mRx.terminate.load(std::memory_order_relaxed))
    {
        uint32_t bytesReceived = 0;

        if (handles[bufferIndex] >= 0)
        {
            const int timeToWaitMs = 1000;
            if (comms->WaitForXfer(handles[bufferIndex], timeToWaitMs))
            {
                bytesReceived = comms->FinishDataXfer(&buffers[bufferIndex * bufferSize], bufferSize, handles[bufferIndex]);
                stats.packets++;
                totalBytesReceived += bytesReceived;
                stats.bytesTransferred += bytesReceived;

                if (bytesReceived != bufferSize)
                {
                    lime::error("Recv %i, expected : %i", bytesReceived, bufferSize);
                }
            }
            else
            {
                // TODO: callback for Rx timeout
                lime::error("Rx WaitForXfer timeout"s);
                continue;
            }
        }

        const uint8_t fullPacketsReceived = bytesReceived / sizeof(FPGA_RxDataPacket);
        for (uint8_t j = 0; j < fullPacketsReceived; ++j)
        {
            if (outputPkt == nullptr)
            {
                outputPkt =
                    SamplesPacketType::ConstructSamplesPacket(mRx.memPool->Allocate(outputPktSize), samplesInPkt, outputSampleSize);
            }

            const FPGA_RxDataPacket* pkt =
                reinterpret_cast<FPGA_RxDataPacket*>(&buffers[bufferIndex * bufferSize + sizeof(FPGA_RxDataPacket) * j]);

            if (pkt->counter - expectedTS != 0)
            {
                lime::warning("Loss: transfer:%li packet:%i, exp: %li, got: %li, diff: %li, handle: %i",
                    stats.packets,
                    j,
                    expectedTS,
                    pkt->counter,
                    pkt->counter - expectedTS,
                    handles[bufferIndex]);
                ++stats.loss;
            }

            if (pkt->txWasDropped())
            {
                ++mTx.stats.loss;
            }

            int payloadSize = pkt->GetPayloadSize();

            if (payloadSize == 256 || payloadSize == 0)
            {
                payloadSize = 4080;
            }

            const int samplesProduced = Deinterleave(conversion, pkt->data, payloadSize, outputPkt);
            expectedTS = pkt->counter + samplesProduced;
            mRx.lastTimestamp.store(expectedTS, std::memory_order_relaxed);
            stats.timestamp = expectedTS;

            NegateQ(outputPkt, TRXDir::Rx);

            if (mRx.fifo->push(outputPkt, false))
            {
                // maxFIFOlevel = std::max(maxFIFOlevel, (int)mRx.fifo.size());
                outputPkt = nullptr;
            }
            else
            {
                ++stats.overrun;
                if (outputPkt)
                {
                    outputPkt->Reset();
                }
            }
        }
        // Re-submit this request to keep the queue full
        handles[bufferIndex] = comms->BeginDataXfer(&buffers[bufferIndex * bufferSize], bufferSize, rxEndPt);
        bufferIndex = (bufferIndex + 1) % batchCount;

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

        if (timePeriod >= std::chrono::milliseconds(1000))
        {
            t1 = t2;

            float dataRate = 1000.0 * totalBytesReceived / timePeriod.count();
            lime::info("Rx: %.3f MB/s", dataRate / 1000000.0);
            mRx.stats.dataRate_Bps = dataRate;

            totalBytesReceived = 0;
        }
    }

    comms->AbortEndpointXfers(safeRxEndPt);
    mRx.stats.dataRate_Bps = 0;
}

void TRXLooper_USB::NegateQ(SamplesPacketType* packet, TRXDir direction)
{
    if (!mConfig.extraConfig.negateQ)
    {
        return;
    }

    const int channelCount = mConfig.channels.at(direction).size();

    switch (mConfig.format)
    {
    case SDRDevice::StreamConfig::DataFormat::I16:
        packet->Scale<complex16_t>(1, -1, channelCount);
        break;
    case SDRDevice::StreamConfig::DataFormat::F32:
        packet->Scale<complex32f_t>(1, -1, channelCount);
        break;
    default:
        break;
    }
}

} // namespace lime
