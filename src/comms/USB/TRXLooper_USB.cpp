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
    printf("TransmitPacketsLoop\n");
    //at this point FPGA has to be already configured to output samples
    const uint8_t chCount = mConfig.txCount;
    const bool packed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const uint32_t samplesInPacket = (packed ? samples12InPkt : samples16InPkt) / chCount;

    const uint8_t batchCount = 4; // how many async reads to schedule, dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = 4; // dataPort->CheckStreamSize(rxBatchSize);
    const uint32_t bufferSize = packetsToBatch * sizeof(FPGA_DataPacket);
    std::vector<int> handles(batchCount, -1);
    std::vector<uint8_t> buffers(batchCount * bufferSize, 0);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    for (int i = 0; i<batchCount; ++i)
        handles[i] = comms->BeginDataXfer(&buffers[i*bufferSize], bufferSize, rxEndPt);

    int bi = 0;
    uint64_t totalBytesSent = 0; //for data rate calculation

    std::vector<complex16_t> parsedBuffer(samplesInPacket);
    FPGA_DataPacket *pkt = nullptr;
    while (terminateTx.load(std::memory_order_relaxed) == false) {
        int32_t bytesSent = 0;
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
            double dataRate = 1000.0 * totalBytesSent / timePeriod;
            printf("Tx: %.3f MB/s\n", dataRate / 1000000.0);
            totalBytesSent = 0;
            txDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
        }
    }
    // if (pkt)
    //     txFIFO->release(pkt);
    // comms->AbortEndpointXfers(rxEndPt);
    txDataRate_Bps.store(0, std::memory_order_relaxed);
}

/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_USB::ReceivePacketsLoop()
{
    printf("ReceivePacketsLoop\n");
    //at this point FPGA has to be already configured to output samples
    const uint8_t chCount = mConfig.rxCount;
    const bool packed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const uint32_t samplesInPacket = (packed ? samples12InPkt : samples16InPkt) / chCount;

    const uint8_t batchCount = 8; // how many async reads to schedule, dataPort->GetBuffersCount();
    const uint8_t packetsToBatch = 16; // dataPort->CheckStreamSize(rxBatchSize);
    const uint32_t bufferSize = packetsToBatch * sizeof(FPGA_DataPacket);
    std::vector<int> handles(batchCount, -1);
    std::vector<uint8_t> buffers(batchCount * bufferSize, 0);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    for (int i = 0; i<batchCount; ++i)
        handles[i] = comms->BeginDataXfer(&buffers[i*bufferSize], bufferSize, rxEndPt);

    int bi = 0;
    uint64_t totalBytesReceived = 0; //for data rate calculation
    uint64_t lastTS = 0;

    std::vector<complex16_t> parsedBuffer(samplesInPacket);
    while (terminateRx.load(std::memory_order_relaxed) == false) {
        int32_t bytesReceived = 0;
        if(handles[bi] >= 0)
        {
            if (comms->WaitForXfer(handles[bi], 1000) == true)
            {
                bytesReceived = comms->FinishDataXfer(&buffers[bi*bufferSize], bufferSize, handles[bi]);
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

        for (uint8_t j = 0; j < bytesReceived / sizeof(FPGA_DataPacket); ++j) {
            const FPGA_DataPacket* pkt = (FPGA_DataPacket*)&buffers[bi*bufferSize];
            const uint8_t byte0 = pkt[j].header0;
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
            rxLastTimestamp.store(lastTS, std::memory_order_relaxed);

            
            // FPGA_DataPacket *fifoPkt = rxFIFO->acquire();
            // if(fifoPkt)
            // {
            //     memcpy(fifoPkt, pkt, sizeof(FPGA_DataPacket));
            //     rxFIFO->push(fifoPkt);
            // }
            // else
            // {
            //     // TODO: Rx FIFO full, report overrun
            // }
        }
        // Re-submit this request to keep the queue full
        handles[bi] = comms->BeginDataXfer(&buffers[bi*bufferSize], bufferSize, rxEndPt);
        bi = (bi + 1) & (batchCount-1);

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000) {
            t1 = t2;
            double dataRate = 1000.0 * totalBytesReceived / timePeriod;
            printf("Rx: %.3f MB/s\n", dataRate / 1000000.0);
            totalBytesReceived = 0;
            rxDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
        }
    }
    comms->AbortEndpointXfers(rxEndPt);
    rxDataRate_Bps.store(0, std::memory_order_relaxed);
}

} // namespace lime
