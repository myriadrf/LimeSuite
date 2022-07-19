#include <assert.h>
#include "FPGA_common.h"
#include "LMS7002M.h"
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

void TRXLooper_USB::AssignFIFO(PacketsFIFO<FPGA_DataPacket> *rx, PacketsFIFO<FPGA_DataPacket> *tx)
{
    rxFIFO = rx;
    txFIFO = tx;
}

void TRXLooper_USB::TransmitPacketsLoop()
{ /*
    //at this point FPGA has to be already configured to output samples
    const uint8_t maxChannelCount = 2;
    const uint8_t chCount = streamSize;
    const bool packed = dataLinkFormat == StreamConfig::FMT_INT12;
    const int epIndex = chipId;
    const uint8_t buffersCount = dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = dataPort->CheckStreamSize(txBatchSize);
    const uint32_t bufferSize = packetsToBatch*sizeof(FPGA_DataPacket);

    const int maxSamplesBatch = (packed ? samples12InPkt:samples16InPkt)/chCount;
    std::vector<int> handles(buffersCount, 0);
    std::vector<bool> bufferUsed(buffersCount, 0);
    std::vector<uint32_t> bytesToSend(buffersCount, 0);
    std::vector<char> buffers;
    buffers.resize(buffersCount*bufferSize, 0);
    std::vector<SamplesPacket> packets;
    for (int i = 0; i<maxChannelCount; ++i)
        packets.emplace_back(maxSamplesBatch);

    uint64_t totalBytesSent = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    bool end_burst = false;
    uint8_t bi = 0; //buffer index
    while (terminateTx.load(std::memory_order_relaxed) != true)
    {
        if (bufferUsed[bi])
        {
            if (dataPort->WaitForSending(handles[bi], 1000) == true)
            {
                unsigned bytesSent = dataPort->FinishDataSending(&buffers[bi*bufferSize], bytesToSend[bi], handles[bi]);
                totalBytesSent += bytesSent;
                bufferUsed[bi] = false;
            }
            else
            {
                txDataRate_Bps.store(totalBytesSent, std::memory_order_relaxed);
                totalBytesSent = 0;
                continue;
            }
        }
        bytesToSend[bi] = 0;
        FPGA_DataPacket* pkt = reinterpret_cast<FPGA_DataPacket*>(&buffers[bi*bufferSize]);
        int i=0;
        do {
            bool has_samples = false;
            int payloadSize = sizeof(FPGA_DataPacket::data);
            for(int ch=0; ch<maxChannelCount; ++ch)
            {
                if (!mTxStreams[ch].used)
                    continue;
                const int ind = chCount == maxChannelCount ? ch : 0;
                if (mTxStreams[ch].mActive==false)
                {
                    memset(packets[ind].samples,0,maxSamplesBatch*sizeof(complex16_t));
                    continue;
                }
                mTxStreams[ch].fifo->pop_packet(packets[ind]);
                int samplesPopped = packets[ind].last;
                if (samplesPopped != maxSamplesBatch)
                {
                    if (!(packets[ind].flags & RingFIFO::END_BURST))
                        continue;
                    payloadSize = samplesPopped * sizeof(FPGA_DataPacket::data) / maxSamplesBatch;
                    int q = packed ? 48 : 16;
                    payloadSize = (1 + (payloadSize - 1) / q) * q;
                    memset(&packets[ind].samples[samplesPopped], 0, (maxSamplesBatch - samplesPopped)*sizeof(complex16_t));
                }
                has_samples = true;
            }

            if (!has_samples)
                break;

            end_burst = (packets[0].flags & RingFIFO::END_BURST);
            pkt[i].counter = packets[0].timestamp;
            pkt[i].reserved[0] = 0;
            //by default ignore timestamps
            const int ignoreTimestamp = !(packets[0].flags & RingFIFO::SYNC_TIMESTAMP);
            pkt[i].reserved[0] |= ((int)ignoreTimestamp << 4); //ignore timestamp
            pkt[i].reserved[1] = payloadSize & 0xFF;
            pkt[i].reserved[2] = (payloadSize >> 8) & 0xFF;
            std::vector<complex16_t*> src(chCount);
            for(uint8_t c=0; c<chCount; ++c)
                src[c] = (packets[c].samples);
            uint8_t* const dataStart = (uint8_t*)pkt[i].data;
            FPGA::Samples2FPGAPacketPayload(src.data(), maxSamplesBatch, chCount==2, packed, dataStart);
            bytesToSend[bi] += 16+payloadSize;
        }while(++i<packetsToBatch && end_burst == false);

        if(terminateTx.load(std::memory_order_relaxed) == true) //early termination
            break;

        if (i)
        {
            handles[bi] = dataPort->BeginDataSending(&buffers[bi*bufferSize], bytesToSend[bi], epIndex);
            txLastTimestamp.store(pkt[i-1].counter+maxSamplesBatch-1, std::memory_order_relaxed); //timestamp of the last sample that was sent to HW
            bufferUsed[bi] = true;
            bi = (bi + 1) & (buffersCount-1);
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            float dataRate = 1000.0*totalBytesSent / timePeriod;
            txDataRate_Bps.store(dataRate, std::memory_order_relaxed);
            totalBytesSent = 0;
            t1 = t2;
#ifndef NDEBUG
            lime::log(LOG_LEVEL_DEBUG, "Tx: %.3f MB/s\n", dataRate / 1000000.0);
#endif
        }
    }

    // Wait for all the queued requests to be cancelled
    dataPort->AbortSending(epIndex);
    txDataRate_Bps.store(0, std::memory_order_relaxed);*/
}

/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_USB::ReceivePacketsLoop()
{
    printf("ReceivePacketsLoop\n");
    //at this point FPGA has to be already configured to output samples
    const uint8_t maxChannelCount = 2;
    const uint8_t chCount = mConfig.rxCount;
    const bool packed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::FMT_INT12;
    const uint32_t samplesInPacket = (packed ? samples12InPkt : samples16InPkt) / chCount;

    const uint8_t batchCount = 4; // how many async reads to schedule, dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = 4; // dataPort->CheckStreamSize(rxBatchSize);
    const uint32_t bufferSize = packetsToBatch * sizeof(FPGA_DataPacket);
    std::vector<int> handles(batchCount, -1);
    std::vector<uint8_t> buffers(batchCount * bufferSize, 0);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    // for (int i = 0; i<batchCount; ++i)
    //     handles[i] = comms->BeginDataXfer(&buffers[i*bufferSize], bufferSize, rxEndPt);

    int bi = 0;
    uint64_t totalBytesReceived = 0; //for data rate calculation
    uint64_t lastTS = 0;

    std::vector<complex16_t> parsedBuffer(samplesInPacket);
    FPGA_DataPacket *pkt = rxFIFO->acquire();
    while (terminateRx.load(std::memory_order_relaxed) == false) {
        int32_t bytesReceived = 0;
        // if(handles[bi] >= 0)
        // {
        //     if (comms->WaitForXfer(handles[bi], 1000) == true)
        //     {
        //         bytesReceived = comms->FinishDataXfer(&buffers[bi*bufferSize], bufferSize, handles[bi]);
        //         totalBytesReceived += bytesReceived;
        //         printf("Recv %i\n", bytesReceived);
        //     }
        //     else
        //     {
        //         // TODO: callback for Rx timeout
        //         printf("Rx timeout\n");
        //         continue;
        //     }
        // }
        if (!pkt)
            pkt = rxFIFO->acquire();

        if (pkt)
            //bytesReceived = comms->BulkTransfer(rxEndPt, &buffers[bi*bufferSize], bufferSize, 500);
            bytesReceived = comms->BulkTransfer(rxEndPt, reinterpret_cast<uint8_t *>(pkt),
                                                sizeof(FPGA_DataPacket), 500);
        else
            printf("Rx failed to acquire packet\n");
        if (bytesReceived == 0) {
            printf("Rx timeout\n");
            continue;
        }
        else {
            //printf("Rx recv %i\n", bytesReceived);
            totalBytesReceived += bytesReceived;
        }
        for (uint8_t j = 0; j < bytesReceived / sizeof(FPGA_DataPacket); ++j) {
            //const FPGA_DataPacket* pkt = (FPGA_DataPacket*)&buffers[bi*bufferSize];
            const uint8_t byte0 = pkt[j].reserved[0];
            lastTS = pkt[j].counter;
            if ((byte0 & (1 << 3)) != 0) {
                // TODO: callback for Tx drops
                printf("Tx drop: %li\n", lastTS);
            }

            const uint64_t samplesLost = pkt[j].counter - lastTS;
            if (samplesLost != 0) {
                // TODO: callback for Rx drops
                printf("Rx drop: %li\n", lastTS);
            }

            //printf("Rx: %li\n", lastTS);
            rxLastTimestamp.store(lastTS, std::memory_order_relaxed);

            // parse samples
            //int samplesCount = FPGA::FPGAPacketPayload2Samples(pkt[j].data, sizeof(FPGA_DataPacket::data), chCount==2, packed, parsedBuffer.data());

            // store samples to FIFO
            /*for(int ch=0; ch<maxChannelCount; ++ch)
            {
                mRxStreams[ch].fifo->push_packet(chFrames[ind]);
            }*/
            rxFIFO->push(pkt);
            pkt = nullptr;
        }
        // Re-submit this request to keep the queue full
        // handles[bi] = comms->BeginDataXfer(&buffers[bi*bufferSize], bufferSize, rxEndPt);
        // bi = (bi + 1) & (batchCount-1);

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000) {
            t1 = t2;
            //total number of bytes sent per second
            double dataRate = 1000.0 * totalBytesReceived / timePeriod;

            printf("Rx: %.3f MB/s\n", dataRate / 1000000.0);

            totalBytesReceived = 0;
            rxDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
        }
    }
    if (pkt)
        rxFIFO->release(pkt);
    // comms->AbortEndpointXfers(rxEndPt);
    rxDataRate_Bps.store(0, std::memory_order_relaxed);
}

} // namespace lime
