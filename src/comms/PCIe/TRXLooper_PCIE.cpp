#include <assert.h>
#include "FPGA_common.h"
#include "LMS7002M.h"
#include <ciso646>
#include "Logger.h"
#include <complex>
#include <errno.h>
#include <string.h>

#include "TRXLooper_PCIE.h"
#include "LitePCIe.h"
#include "Profiler.h"

#include "ConstellationDisplay.h"

static bool showStats = false;

namespace lime {

TRXLooper_PCIE::TRXLooper_PCIE(LitePCIe *rxPort, LitePCIe *txPort, FPGA *f, LMS7002M *chip, uint8_t moduleIndex)
    : TRXLooper(f, chip, moduleIndex), rxPort(rxPort), txPort(txPort)
{
}

TRXLooper_PCIE::~TRXLooper_PCIE() {}

void TRXLooper_PCIE::TransmitPacketsLoop()
{
    //txProfiler = new Profiler();
    //at this point FPGA has to be already configured to output samples
    const uint8_t maxPacketsToBatch = 1; // dataPort->CheckStreamSize(rxBatchSize);
    const uint16_t packetSize = sizeof(FPGA_DataPacket);
    uint8_t buffers[maxPacketsToBatch * packetSize];

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;

    int64_t totalBytesSent = 0; //for data rate calculation
    int retryWrites = 0;
    int writeTransactions = 0;
    int packetsSent = 0;

    uint64_t transferCounter = 0;
    float timer = 0;
    while (terminateTx.load(std::memory_order_relaxed) == false) {
        int packetsAcquired = 0;
        {
            //ProfilerScope aaa(txProfiler, "TxPop", 2);
            while (packetsAcquired < maxPacketsToBatch)
            {
                FPGA_DataPacket* pkt = txFIFO->pop();
                if(pkt == nullptr)
                    break; // fifo empty, don't wait for full batch and just continue to send to RF what's already been acquired
                timer = GetTimeSinceStartup();
                // if(packetsSent < 8)
                //     printf("Tx packet %li, %f\n", pkt->counter, GetTimeSinceStartup());
                memcpy(&buffers[packetsAcquired*packetSize], pkt, packetSize);
                txFIFO->release(pkt);
                ++packetsAcquired;
            }
        }

        if (packetsAcquired > 0)
        {
            //ProfilerScope p(txProfiler, "TxPackets", 2);
            const int bytesToWrite = packetsAcquired*packetSize;
            FPGA_DataPacket* pkt = reinterpret_cast<FPGA_DataPacket*>(buffers);
            int64_t rxAt = rxLastTimestamp.load(std::memory_order_relaxed);
            // if(pkt->counter < rxAt+510)
            //     printf("Tx will be late, RxTS@%li, packetTxTS@%li, diff:%li, txFIFO: %.0f, rxFIFO: %.0f %f\n", rxAt, pkt->counter, pkt->counter-rxAt, txFIFO->Usage(), rxFIFO->Usage(), timer);
            auto tr0 = std::chrono::high_resolution_clock::now();
            int32_t bytesSent = 0;
            {
                //ProfilerScope a(txProfiler, "WriteRaw", 2);
                bytesSent = txPort->WriteRaw(buffers, bytesToWrite, 1000);
            }

            auto tr1 = std::chrono::high_resolution_clock::now();
            auto transferDuration = std::chrono::duration_cast<std::chrono::microseconds>(tr1 - tr0).count();
            // if (transferCounter < 8) //transferDuration >= 10)
            //     printf("Rx@ %li Tx TS: %li. cnt: %li  duration: %lius\n", rxAt, pkt->counter, transferCounter, transferDuration);

            if (bytesSent <= 0) {
                if(bytesSent < 0)
                    printf("Tx write failed - error(%s)\n", strerror(errno));
                ++retryWrites;
                // bytesSent == 0 should generally not happen, unless FPGA
                // is already full and cannot accept more data.
                // if(totalBytesSent > 0) // FPGA did accept data before
                //     printf("FPGA does not accept more data, might be stuck waiting with timestamp packets in far future\n");
                //std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            else {
                ++writeTransactions;
                if(bytesSent != bytesToWrite)
                {
                    printf("Tx sent %i, actual %i, remaining: %i\n", bytesToWrite, bytesSent, bytesToWrite-bytesSent);
                    // TODO: handle partial writes when more than 1 packet in batch
                }
                totalBytesSent += bytesSent;
                packetsSent += bytesSent/packetSize;//packetsAcquired;
                //printf("Tx sent for @%li , Rx@ %li, diff: %li\n", pkt->counter, rxAt, rxAt-pkt->counter);
            }
            ++transferCounter;
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000) {
            t1 = t2;
            double dataRate = 1000.0 * totalBytesSent / timePeriod;
            float avgBatchSize = float(totalBytesSent/packetSize)/writeTransactions;
            if(showStats)
                printf("%s Tx: %.3f MB/s | avgPacketsInBatch:%g retries:%i, packetsSent: %i, txFIFO: %.0f\n",
                    txPort->GetPathName().c_str(),
                    dataRate / 1000000.0,
                    avgBatchSize,
                    retryWrites,
                    packetsSent,
                    txFIFO->Usage()
                );
            //packetsSent = 0;
            writeTransactions = 0;
            totalBytesSent = 0;
            txDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
            retryWrites = 0;
        }
    }
    txDataRate_Bps.store(0, std::memory_order_relaxed);
    //txProfiler->Done();
    //delete txProfiler;
    //txProfiler = nullptr;
}

/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_PCIE::ReceivePacketsLoop()
{
    //rxProfiler = new Profiler();
    //ProfilerScope p(rxProfiler, "ReceivePacketsLoop");
    rxLastTimestamp.store(0, std::memory_order_relaxed);
    //at this point FPGA has to be already configured to output samples
    const int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360);
    const int64_t expectedTSdiff = samplesInPkt / std::max(mConfig.rxCount, mConfig.txCount);
    const int packetSize = sizeof(FPGA_DataPacket);
    const uint8_t maxPacketsToBatch = 1; // dataPort->CheckStreamSize(rxBatchSize);
    std::vector<uint8_t> buffers(maxPacketsToBatch * packetSize, 0);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;

    int64_t totalBytesReceived = 0; //for data rate calculation
    int64_t lastTS = 0;

    SDRDevice::StreamStats stats;
    int txDrops = 0;
    int rxDiscontinuity = 0;
    int rxOverrun = 0; // fifo is full, data is arriving faster than PC can process

    uint8_t probeBuffer[8];

    int packetsRecv = 0;

    uint64_t transferCounter = 0;
    while (terminateRx.load(std::memory_order_relaxed) == false) {
        int bytesReceived = 0;
        {
            //ProfilerScope p(rxProfiler, "ReadRaw", 0);
            bytesReceived = rxPort->ReadRaw(buffers.data(), maxPacketsToBatch * packetSize, 100);
        }

        if (bytesReceived <= 0) {
            if(bytesReceived < 0)
                printf("Rx read failed - error(%s)\n", strerror(errno));
            // else
            // {
            //     // we're reading data faster than it's produced
            //     // sleep for a bit to not waste CPU cycles
            //     //std::this_thread::sleep_for(std::chrono::microseconds(100));
            //     bytesReceived = 0;
            // }
        }
        else {
            //ProfilerScope p(rxProfiler, "RxPackets", 1);
            ++transferCounter;
            totalBytesReceived += bytesReceived;
            // TODO: handle partial reads, even though they should never happen
            if(bytesReceived % packetSize != 0)
                printf("Rx partial read of packet\n");

            for (uint8_t j = 0; j < bytesReceived / packetSize; ++j) {
                FPGA_DataPacket *pkt = rxFIFO->acquire();
                if(pkt == nullptr)
                {
                    ++rxOverrun;
                    ++stats.overrun;
                    if(mConfig.statusCallback)
                        mConfig.statusCallback(&stats, mConfig.userData);
                    continue;
                }
                memcpy(pkt, &buffers[j*packetSize], packetSize);
                memcpy(probeBuffer, pkt->data, sizeof(probeBuffer));
                ++packetsRecv;
                // if(packetsRecv < 8)
                //     printf("RxTS: %li, cnt: %i, %f\n", pkt->counter, packetsRecv, GetTimeSinceStartup());

                const int64_t samplesLost = pkt->counter - lastTS;
                if (samplesLost != expectedTSdiff && lastTS > 0) {
                    printf("Rx discontinuity: %li -> %li (diff: %li)\n", lastTS, pkt->counter, samplesLost);
                    ++rxDiscontinuity;
                    ++stats.loss;
                    stats.timestamp = pkt->counter;
                    if(mConfig.statusCallback)
                        mConfig.statusCallback(&stats, mConfig.userData);
                }
                lastTS = pkt->counter;
                rxLastTimestamp.store(lastTS+expectedTSdiff,  std::memory_order_relaxed);

                if (pkt->txWasDropped()) {
                    //printf("Rx got %i, @ %li, txDrop: %i\n", transferCounter, pkt->counter, txDropped);
                    ++txDrops;
                    ++stats.late;
                    stats.timestamp = lastTS;
                    if(mConfig.statusCallback)
                        mConfig.statusCallback(&stats, mConfig.userData);
                }

                {
                    //ProfilerScope a(rxProfiler, "RxPush", 1);
                    rxFIFO->push(pkt);
                }
                // float usage = rxFIFO->Usage();
                // if (usage > 16)
                //     printf("Rx usage: %f\n", usage);
            }
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000) {
            t1 = t2;
            double dataRateBps = 1000.0 * totalBytesReceived / timePeriod;
            if (showStats)
            {
                char ctemp[64];
                sprintf(ctemp, "%02X %02X %02X %02X %02X %02X %02X %02X", probeBuffer[0],probeBuffer[1],probeBuffer[2],probeBuffer[3],probeBuffer[4],probeBuffer[5], probeBuffer[6],probeBuffer[7]);
                printf("%s Rx: %.3f MB/s | Overrun: %i, rxDiscontinuity: %i, rxPacketsRecv:%i, txPacketsDropped: %i, rxFIFO: %.0f, %s\n",
                    rxPort->GetPathName().c_str(),
                    dataRateBps / 1000000.0, rxOverrun, rxDiscontinuity, packetsRecv, txDrops, rxFIFO->Usage(), ctemp);
            }
            //packetsRecv = 0;
            //stats.late = 0;
            totalBytesReceived = 0;
            rxDiscontinuity = 0;
            txDrops = 0;
            rxOverrun = 0;
            rxDataRate_Bps.store((uint32_t)dataRateBps, std::memory_order_relaxed);
        }
    }
    rxDataRate_Bps.store(0, std::memory_order_relaxed);
    //rxProfiler->Done();
    // delete rxProfiler;
    // rxProfiler = nullptr;
}

} // namespace lime
