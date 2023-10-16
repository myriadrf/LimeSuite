#include <assert.h>
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"
#include <ciso646>
#include "Logger.h"
#include <complex>

#include "TRXLooper_USB.h"
#include "USBGeneric.h"

namespace lime {

TRXLooper_USB::TRXLooper_USB(USBGeneric *comms, FPGA *f, LMS7002M *chip, uint8_t rxEndPt,
                             uint8_t txEndPt)
    : TRXLooper(f, chip, 0), comms(comms), rxEndPt(rxEndPt), txEndPt(txEndPt)
{
}

TRXLooper_USB::~TRXLooper_USB() {}

void TRXLooper_USB::TransmitPacketsLoop()
{
    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lk(streamMutex);
        while (!mStreamEnabled && !mRx.terminate.load(std::memory_order_relaxed))
            streamActive.wait_for(lk, std::chrono::milliseconds(100));
        lk.unlock();
    }

    printf("TransmitPacketsLoop\n");
    //at this point FPGA has to be already configured to output samples
    // const uint8_t chCount = mConfig.txCount;
    // const bool packed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    // const uint32_t samplesInPacket = (packed ? samples12InPkt : samples16InPkt) / chCount;

    const uint8_t batchCount = 1; // how many async reads to schedule, dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = 1; // dataPort->CheckStreamSize(rxBatchSize);
    const uint32_t bufferSize = packetsToBatch * sizeof(FPGA_DataPacket);
    std::vector<int> handles(batchCount, -1);
    std::vector<uint8_t> buffers(batchCount * bufferSize, 0);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    for (int i = 0; i<batchCount; ++i)
        handles[i] = comms->BeginDataXfer(&buffers[i*bufferSize], bufferSize, rxEndPt);

    int bi = 0;
    uint64_t totalBytesSent = 0; //for data rate calculation

    // std::vector<complex16_t> parsedBuffer(samplesInPacket);
    // FPGA_DataPacket *pkt = nullptr;
    while (mTx.terminate.load(std::memory_order_relaxed) == false) {
        if(handles[bi] >= 0)
        {
            if (comms->WaitForXfer(handles[bi], 1000) == true)
            {
                int bytesReceived = comms->FinishDataXfer(&buffers[bi*bufferSize], bufferSize, handles[bi]);
                totalBytesSent += bytesReceived;
                //printf("Recv %i\n", bytesReceived);
            }
            else
            {
                // TODO: callback for Rx timeout
                printf("Tx WaitForXfer timeout\n");
                continue;
            }
        }
        // if (!pkt)
        //     pkt = txFIFO->pop();

        // int32_t bytesSent = 0;
        // if (pkt)
        // {
        //     bytesSent = comms->BulkTransfer(txEndPt, reinterpret_cast<uint8_t *>(pkt),
        //                                         sizeof(FPGA_DataPacket), 500);
        //     if (bytesSent == 0) {
        //         printf("Tx failed transfer\n");
        //         continue;
        //     }
        //     else {
        //         //printf("Rx recv %i\n", bytesReceived);
        //         totalBytesSent += bytesSent;
        //     }
        //     txFIFO->release(pkt);
        //     pkt = nullptr;
        // }
        // else
        //     printf("Tx underrun\n");

        // Re-submit this request to keep the queue full
        handles[bi] = comms->BeginDataXfer(&buffers[bi*bufferSize], bufferSize, rxEndPt);
        bi = (bi + 1) & (batchCount-1);

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000) {
            t1 = t2;
            float dataRate = 1000.0 * totalBytesSent / timePeriod;
            printf("Tx: %.3f MB/s\n", dataRate / 1000000.0);
            totalBytesSent = 0;
            mTx.stats.dataRate_Bps = dataRate;
        }
    }
    // if (pkt)
    //     txFIFO->release(pkt);
    comms->AbortEndpointXfers(txEndPt);
    mTx.stats.dataRate_Bps = 0;
}

/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_USB::ReceivePacketsLoop()
{
    //at this point FPGA has to be already configured to output samples
    // const uint8_t chCount = mConfig.rxCount;
    // const bool packed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    // const uint32_t samplesInPacket = (packed ? samples12InPkt : samples16InPkt) / chCount;

    DataConversion conversion;
    conversion.srcFormat = mConfig.linkFormat;
    conversion.destFormat = mConfig.format;
    conversion.channelCount = std::max(mConfig.txCount, mConfig.rxCount);

    typedef FPGA_DataPacket ReceivePacket;

    // D.P.: can't really be set to a higher value without packet drops/duplicates from the FPGA?
    const uint8_t batchCount = 1; // how many async reads to schedule, dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = 1; // mRx.packetsToBatch; // dataPort->CheckStreamSize(rxBatchSize);
    const uint32_t bufferSize = packetsToBatch * sizeof(ReceivePacket);
    std::vector<int> handles(batchCount, -1);
    std::vector<uint8_t> buffers(batchCount * bufferSize, 0);
    int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / conversion.channelCount;//mRx.samplesInPkt;

    const int outputSampleSize = mConfig.format == SDRDevice::StreamConfig::F32 ? sizeof(complex32f_t) : sizeof(complex16_t);
    const int32_t outputPktSize = SamplesPacketType::headerSize
        + packetsToBatch * samplesInPkt
        * outputSampleSize;

    char name[64];
    sprintf(name, "Rx%i_memPool", chipId);
    const int chCount = std::max(mConfig.rxCount, mConfig.txCount);
    const int upperAllocationLimit = sizeof(complex32f_t) * packetsToBatch * samplesInPkt * chCount + SamplesPacketType::headerSize;
    mRx.memPool = new MemoryPool(1024, upperAllocationLimit, 4096, name);

    SDRDevice::StreamStats &stats = mRx.stats;

    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lk(streamMutex);
        while (!mStreamEnabled && !mRx.terminate.load(std::memory_order_relaxed))
            streamActive.wait_for(lk, std::chrono::milliseconds(100));
        lk.unlock();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    for (int i = 0; i<batchCount; ++i)
        handles[i] = comms->BeginDataXfer(&buffers[i*bufferSize], bufferSize, rxEndPt);

    int bi = 0;
    uint64_t totalBytesReceived = 0; //for data rate calculation
    // uint64_t lastTS = 0;

    // std::vector<complex16_t> parsedBuffer(samplesInPacket);
    SamplesPacketType* outputPkt = nullptr;
    int64_t expectedTS = 0;

    while (mRx.terminate.load(std::memory_order_relaxed) == false) {
        uint32_t bytesReceived = 0;
        if(handles[bi] >= 0)
        {
            if (comms->WaitForXfer(handles[bi], 1000) == true)
            {
                bytesReceived = comms->FinishDataXfer(&buffers[bi*bufferSize], bufferSize, handles[bi]);
                stats.packets++;
                totalBytesReceived += bytesReceived;
                if(bytesReceived != bufferSize)
                    printf("Recv %i, expected : %i\n", bytesReceived, bufferSize);
            }
            else
            {
                // TODO: callback for Rx timeout
                printf("Rx WaitForXfer timeout\n");
                continue;
            }
        }

        for (uint8_t j = 0; j < bytesReceived / sizeof(ReceivePacket); ++j) {
            if (!outputPkt)
                outputPkt = SamplesPacketType::ConstructSamplesPacket(mRx.memPool->Allocate(outputPktSize), samplesInPkt, outputSampleSize);

            const ReceivePacket* pkt = reinterpret_cast<ReceivePacket*>(&buffers[bi*sizeof(ReceivePacket)*j]);
            if (pkt->counter - expectedTS != 0)
            {
                printf("Loss: transfer:%li packet:%i, exp: %li, got: %li, diff: %li\n", stats.packets, j, expectedTS, pkt->counter, pkt->counter-expectedTS);
                ++stats.loss;
            }
            if (pkt->txWasDropped())
                ++mTx.stats.loss;

            int payloadSize = pkt->GetPayloadSize();

            if (payloadSize == 256 || payloadSize == 0)
            {
                payloadSize = 4080;
            }

            const int samplesProduced = Deinterleave(conversion, pkt->data, payloadSize, outputPkt);
            expectedTS = pkt->counter + samplesProduced;
            mRx.lastTimestamp.store(expectedTS, std::memory_order_relaxed);
            
            if (mConfig.extraConfig != nullptr && mConfig.extraConfig->negateQ)
            {
                switch (mConfig.format)
                {
                case SDRDevice::StreamConfig::DataFormat::I16:
                    outputPkt->Scale<complex16_t>(1, -1, mConfig.rxCount);
                    break;
                case SDRDevice::StreamConfig::DataFormat::F32:
                    outputPkt->Scale<complex32f_t>(1, -1, mConfig.rxCount);
                    break;
                }
            }

            if (mRx.fifo->push(outputPkt, false))
            {
                // maxFIFOlevel = std::max(maxFIFOlevel, (int)mRx.fifo.size());
                outputPkt = nullptr;
            }
            else
            {
                ++stats.overrun;
                if(outputPkt)
                    outputPkt->Reset();
            }
        }
        // Re-submit this request to keep the queue full
        handles[bi] = comms->BeginDataXfer(&buffers[bi*bufferSize], bufferSize, rxEndPt);
        bi = (bi + 1) & (batchCount-1);

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000) {
            t1 = t2;
            float dataRate = 1000.0 * totalBytesReceived / timePeriod;
            printf("Rx: %.3f MB/s\n", dataRate / 1000000.0);
            totalBytesReceived = 0;

            mRx.stats.dataRate_Bps = dataRate;
        }
    }
    comms->AbortEndpointXfers(rxEndPt);
    mRx.stats.dataRate_Bps = 0;
}

} // namespace lime
