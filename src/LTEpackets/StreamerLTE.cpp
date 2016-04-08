#include "StreamerLTE.h"
#include "IConnection.h"
#include "LMS64CProtocol.h" //TODO remove when reset usb is abstracted
#include <fstream>
#include <iostream>
#include <ciso646>
#include "fifo.h"

#include "kiss_fft.h"

using namespace std;
using namespace lime;

IConnection* gDataPort;

StreamerLTE::StreamerLTE(IConnection* dataPort)
{
    mDataPort = dataPort;
    mRxFIFO = new LMS_SamplesFIFO(1, 1);
    mTxFIFO = new LMS_SamplesFIFO(1, 1);
    mStreamRunning.store(false);
}

StreamerLTE::~StreamerLTE()
{
    StopStreaming();
    delete mRxFIFO;
    delete mTxFIFO;
}

/** @brief Crates thread for packets processing
*/
StreamerLTE::STATUS StreamerLTE::StartStreaming(const int fftSize, const int channelsCount, const StreamDataFormat format)
{
    if (mStreamRunning.load() == true)
        return FAILURE;
    if (mDataPort->IsOpen() == false)
        return FAILURE;

    stopProcessing.store(false);
    threadProcessing = std::thread(ProcessPackets, this, fftSize, channelsCount, format);
    mStreamRunning.store(true);
    return SUCCESS;
}

/** @brief Stops receiving, processing and transmitting threads
*/
StreamerLTE::STATUS StreamerLTE::StopStreaming()
{
    if (mStreamRunning.load() == false)
        return FAILURE;
    stopProcessing.store(true);
    threadProcessing.join();
    mStreamRunning.store(false);
    return SUCCESS;
}

/** @brief Function dedicated for receiving data samples from board
    @param rxFIFO FIFO to store received data
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void StreamerLTE::ReceivePackets(const StreamerLTE_ThreadData &args)
{
    auto dataPort = args.dataPort;
    auto rxFIFO = args.FIFO;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;
    auto report = args.report;
    auto getCmd = args.getCmd;

    //at this point Rx must be enabled in FPGA
    int rxDroppedSamples = 0;
    const int channelsCount = rxFIFO->GetChannelsCount();
    uint32_t samplesCollected = 0;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    const int bufferSize = 65536;
    const int buffersCount = 16; // must be power of 2
    const int buffersCountMask = buffersCount - 1;
    int handles[buffersCount];
    memset(handles, 0, sizeof(int)*buffersCount);
    vector<char>buffers;
    try
    {
        buffers.resize(buffersCount*bufferSize, 0);
    }
    catch (const std::bad_alloc &ex)
    {
        printf("Error allocating Rx buffers, not enough memory\n");
        return;
    }

    //temporary buffer to store samples for batch insertion to FIFO
    PacketFrame tempPacket;
    tempPacket.Initialize(channelsCount);

    for (int i = 0; i<buffersCount; ++i)
        handles[i] = dataPort->BeginDataReading(&buffers[i*bufferSize], bufferSize);

    int bi = 0;
    unsigned long totalBytesReceived = 0; //for data rate calculation
    int m_bufferFailures = 0;
    int16_t sample;

    uint32_t samplesReceived = 0;

    bool currentRxCmdValid = false;
    RxCommand currentRxCmd;
    size_t ignoreTxLateCount = 0;

    while (terminate->load() == false)
    {
        if (ignoreTxLateCount != 0) ignoreTxLateCount--;
        if (dataPort->WaitForReading(handles[bi], 1000) == false)
            ++m_bufferFailures;

        long bytesToRead = bufferSize;
        long bytesReceived = dataPort->FinishDataReading(&buffers[bi*bufferSize], bytesToRead, handles[bi]);
        if (bytesReceived > 0)
        {
            if (bytesReceived != bufferSize) //data should come in full sized packets
                ++m_bufferFailures;

            totalBytesReceived += bytesReceived;
            for (int pktIndex = 0; pktIndex < bytesReceived / sizeof(PacketLTE); ++pktIndex)
            {
                PacketLTE* pkt = (PacketLTE*)&buffers[bi*bufferSize];
                tempPacket.first = 0;
                tempPacket.timestamp = pkt[pktIndex].counter;

                int statusFlags = 0;
                uint8_t* pktStart = (uint8_t*)pkt[pktIndex].data;
                const int stepSize = channelsCount * 3;
                size_t numPktBytes = sizeof(pkt->data);

                auto byte0 = pkt[pktIndex].reserved[0];
                if ((byte0 & (1 << 3)) != 0 and ignoreTxLateCount == 0)
                {
                    auto reg7 = Reg_read(dataPort, 0x0007);
                    Reg_write(dataPort, 0x0007, reg7 | (1 << 15));
                    Reg_write(dataPort, 0x0007, reg7 & ~(1 << 15));
                    if (report) report(STATUS_FLAG_TX_LATE, pkt[pktIndex].counter);
                    ignoreTxLateCount = 16;
                }

                if (report) report(STATUS_FLAG_TIME_UP, pkt[pktIndex].counter);

                if (getCmd)
                {
                    auto numPacketSamps = numPktBytes/stepSize;

                    //no valid command, try to pop a new command
                    if (not currentRxCmdValid) currentRxCmdValid = getCmd(currentRxCmd);

                    //command is valid, and this is an infinite read
                    //so we always replace the command if available
                    //the currentRxCmdValid variable remains true
                    else if (!currentRxCmd.finiteRead) getCmd(currentRxCmd);

                    //no valid command, just continue to the next pkt
                    if (not currentRxCmdValid) continue;

                    //handle timestamp logic
                    if (currentRxCmd.waitForTimestamp)
                    {
                        //the specified timestamp is late
                        //TODO report late condition to the rx fifo
                        //we are done with this current command
                        if (currentRxCmd.timestamp < tempPacket.timestamp)
                        {
                            std::cout << "L" << std::flush;
                            //until we can report late, treat it as asap:
                            currentRxCmd.waitForTimestamp = false;
                            //and put this one back in....
                            //currentRxCmdValid = false;
                            if (report) report(STATUS_FLAG_RX_LATE, currentRxCmd.timestamp);
                            continue;
                        }

                        //the specified timestamp is in a future packet
                        //continue onto the next packet
                        if (currentRxCmd.timestamp >= tempPacket.timestamp+numPacketSamps)
                        {
                            continue;
                        }

                        //the specified timestamp is within this packet
                        //adjust pktStart and numPktBytes for the offset
                        currentRxCmd.waitForTimestamp = false;
                        auto offsetSamps = currentRxCmd.timestamp-tempPacket.timestamp;
                        pktStart += offsetSamps*stepSize;
                        numPktBytes -= offsetSamps*stepSize;
                        numPacketSamps -= offsetSamps;
                        tempPacket.timestamp += offsetSamps;
                    }

                    //total adjustments for finite read
                    if (currentRxCmd.finiteRead)
                    {
                        //the current command has been depleted
                        //adjust numPktBytes to the requested end
                        if (currentRxCmd.numSamps <= numPacketSamps)
                        {
                            auto extraSamps = numPacketSamps-currentRxCmd.numSamps;
                            numPktBytes -= extraSamps*stepSize;
                            currentRxCmdValid = false;
                            statusFlags |= STATUS_FLAG_RX_END;
                        }
                        else currentRxCmd.numSamps -= numPacketSamps;
                    }
                }

                for (uint16_t b = 0; b < numPktBytes; b += stepSize)
                {
                    for (int ch = 0; ch < channelsCount; ++ch)
                    {
                        //I sample
                        sample = (pktStart[b + 1 + 3 * ch] & 0x0F) << 8;
                        sample |= (pktStart[b + 3 * ch] & 0xFF);
                        sample = sample << 4;
                        sample = sample >> 4;
                        tempPacket.samples[ch][samplesCollected].i = sample;

                        //Q sample
                        sample = pktStart[b + 2 + 3 * ch] << 4;
                        sample |= (pktStart[b + 1 + 3 * ch] >> 4) & 0x0F;
                        sample = sample << 4;
                        sample = sample >> 4;
                        tempPacket.samples[ch][samplesCollected].q = sample;
                    }
                    ++samplesCollected;
                    ++samplesReceived;
                }
                tempPacket.last = samplesCollected;

                uint32_t samplesPushed = rxFIFO->push_samples((const complex16_t**)tempPacket.samples, samplesCollected, channelsCount, tempPacket.timestamp, 10, statusFlags);
                if (samplesPushed != samplesCollected)
                {
                    rxDroppedSamples += samplesCollected - samplesPushed;
                    if (report) report(STATUS_FLAG_RX_DROP, pkt[pktIndex].counter);
                }
                samplesCollected = 0;
            }
        }
        else
        {
            ++m_bufferFailures;
        }

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            double dataRate = 1000.0*totalBytesReceived / timePeriod;
            //total number of samples from all channels per second
            float samplingRate = 1000.0*samplesReceived / timePeriod;
            samplesReceived = 0;
            t1 = t2;
            totalBytesReceived = 0;
            if (dataRate_Bps)
                dataRate_Bps->store((long)dataRate);
            m_bufferFailures = 0;

#ifndef NDEBUG
            printf("Rx rate: %.3f MB/s Fs: %.3f MHz | dropped samples: %lu | TS: %li\n", dataRate / 1000000.0, samplingRate / 1000000.0, rxDroppedSamples, tempPacket.timestamp);
#endif
            rxDroppedSamples = 0;
        }

        // Re-submit this request to keep the queue full
        memset(&buffers[bi*bufferSize], 0, bufferSize);
        handles[bi] = dataPort->BeginDataReading(&buffers[bi*bufferSize], bufferSize);
        bi = (bi + 1) & buffersCountMask;
    }
    dataPort->AbortReading();
    for (int j = 0; j<buffersCount; j++)
    {
        long bytesToRead = bufferSize;
        dataPort->WaitForReading(handles[j], 1000);
        dataPort->FinishDataReading(&buffers[j*bufferSize], bytesToRead, handles[j]);
    }
}

/** @brief Function dedicated for receiving data samples from board
    @param rxFIFO FIFO to store received data
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void StreamerLTE::ReceivePacketsUncompressed(const StreamerLTE_ThreadData &args)
{
    auto dataPort = args.dataPort;
    auto rxFIFO = args.FIFO;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;

    //at this point Rx must be enabled in FPGA
    int rxDroppedSamples = 0;
    const int channelsCount = 1;
    uint32_t samplesCollected = 0;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    const int bufferSize = 65536;
    const int buffersCount = 16; // must be power of 2
    const int buffersCountMask = buffersCount - 1;
    int handles[buffersCount];
    memset(handles, 0, sizeof(int)*buffersCount);
    vector<char>buffers;
    try
    {
        buffers.resize(buffersCount*bufferSize, 0);
    }
    catch (const std::bad_alloc &ex)
    {
        printf("Error allocating Rx buffers, not enough memory\n");
        return;
    }

    //temporary buffer to store samples for batch insertion to FIFO
    PacketFrame tempPacket;
    tempPacket.Initialize(channelsCount);

    unsigned short regVal = Reg_read(gDataPort, 0x0005);

    printf("Begin Data Reading\n");

    for (int i = 0; i<buffersCount; ++i)
        handles[i] = dataPort->BeginDataReading(&buffers[i*bufferSize], bufferSize);

    bool async = false;
    Reg_write(gDataPort, 0x0005, (regVal & ~0x20) | 0x6 | (async << 4));

    int bi = 0;
    unsigned long totalBytesReceived = 0; //for data rate calculation
    int m_bufferFailures = 0;
    int16_t sample;

    uint32_t samplesReceived = 0;

    while (terminate->load() == false)
    {
        if (dataPort->WaitForReading(handles[bi], 1000) == false)
            ++m_bufferFailures;

        long bytesToRead = bufferSize;
        long bytesReceived = dataPort->FinishDataReading(&buffers[bi*bufferSize], bytesToRead, handles[bi]);
        if (bytesReceived > 0)
        {
            if (bytesReceived != bufferSize) //data should come in full sized packets
                ++m_bufferFailures;

            totalBytesReceived += bytesReceived;
            for (int pktIndex = 0; pktIndex < bytesReceived / sizeof(PacketLTE); ++pktIndex)
            {
                PacketLTE* pkt = (PacketLTE*)&buffers[bi*bufferSize];
                tempPacket.first = 0;
                tempPacket.timestamp = pkt[pktIndex].counter;

                uint8_t* pktStart = (uint8_t*)pkt[pktIndex].data;
                const int stepSize = channelsCount * 4;
                for (uint16_t b = 0; b < sizeof(pkt->data); b += stepSize)
                {
                    for (int ch = 0; ch < channelsCount; ++ch)
                    {
                        //I sample
                        sample = (pktStart[b + 1 + 4 * ch] & 0x0F) << 8;
                        sample |= (pktStart[b + 4 * ch] & 0xFF);
                        sample = sample << 4;
                        sample = sample >> 4;
                        tempPacket.samples[ch][samplesCollected].i = sample;

                        //Q sample
                        sample = (pktStart[b + 3 + 4 * ch] & 0x0F) << 8;
                        sample |= pktStart[b + 2 + 4 * ch] & 0xFF;
                        sample = sample << 4;
                        sample = sample >> 4;
                        tempPacket.samples[ch][samplesCollected].q = sample;
                    }
                    ++samplesCollected;
                    ++samplesReceived;
                }
                tempPacket.last = samplesCollected;

                uint32_t samplesPushed = rxFIFO->push_samples((const complex16_t**)tempPacket.samples, samplesCollected, channelsCount, tempPacket.timestamp, 10);
                if (samplesPushed != samplesCollected)
                    rxDroppedSamples += samplesCollected - samplesPushed;
                samplesCollected = 0;
            }
        }
        else
        {
            ++m_bufferFailures;
        }

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            double dataRate = 1000.0*totalBytesReceived / timePeriod;
            //total number of samples from all channels per second
            float samplingRate = 1000.0*samplesReceived / timePeriod;
            samplesReceived = 0;
            t1 = t2;
            totalBytesReceived = 0;
            if (dataRate_Bps)
                dataRate_Bps->store((long)dataRate);
            m_bufferFailures = 0;
#ifndef NDEBUG
            printf("Rx rate: %.3f MB/s Fs: %.3f MHz | dropped samples: %lu\n", dataRate / 1000000.0, samplingRate / 1000000.0, rxDroppedSamples);
#endif
            rxDroppedSamples = 0;
        }

        // Re-submit this request to keep the queue full
        memset(&buffers[bi*bufferSize], 0, bufferSize);
        handles[bi] = dataPort->BeginDataReading(&buffers[bi*bufferSize], bufferSize);
        bi = (bi + 1) & buffersCountMask;
    }
    dataPort->AbortReading();
    for (int j = 0; j<buffersCount; j++)
    {
        long bytesToRead = bufferSize;
        dataPort->WaitForReading(handles[j], 1000);
        dataPort->FinishDataReading(&buffers[j*bufferSize], bytesToRead, handles[j]);
    }
}

void StreamerLTE::ResetUSBFIFO(LMS64CProtocol* port)
{
// TODO : USB FIFO reset command for IConnection
    if (port == nullptr) return;
    LMS64CProtocol::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_USB_FIFO_RST;
    ctrPkt.outBuffer.push_back(0x01);
    port->TransferPacket(ctrPkt);
    ctrPkt.outBuffer[0] = 0x00;
    port->TransferPacket(ctrPkt);
}

/** @brief Function dedicated for processing incomming data and generating outputs for transmitting
*/
void StreamerLTE::ProcessPackets(StreamerLTE* pthis, const unsigned int fftSize, const int channelsCount, const StreamDataFormat format)
{
    if(pthis->mDataPort == nullptr)
    {
    #ifndef NDEBUG
        printf("ProcessPackets: port not connected");
    #endif
        return;
    }
    pthis->mRxFIFO->Reset(2*4096, channelsCount);
    pthis->mTxFIFO->Reset(2*4096, channelsCount);

    DataToGUI localDataResults;
    localDataResults.nyquist_MHz = 7.68*1000000;
    localDataResults.samplesI[0].resize(fftSize, 0);
    localDataResults.samplesI[1].resize(fftSize, 0);
    localDataResults.samplesQ[0].resize(fftSize, 0);
    localDataResults.samplesQ[1].resize(fftSize, 0);
    localDataResults.fftBins_dbFS[0].resize(fftSize, 0);
    localDataResults.fftBins_dbFS[1].resize(fftSize, 0);
    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];

    const int samplesToRead = fftSize;
    int16_t** buffers;
    buffers = new int16_t*[channelsCount];
    for (int i = 0; i < channelsCount; ++i)
        buffers[i] = new int16_t[samplesToRead * 2];
    uint64_t timestamp = 0;
    int timeout_ms = 1000;

    //switch off Rx/Tx
    uint16_t interface_ctrl_000A = Reg_read(pthis->mDataPort, 0x000A);
    Reg_write(pthis->mDataPort, 0x000A, interface_ctrl_000A & ~0x3);

    //reset hardware timestamp to 0
    uint16_t interface_ctrl_0009 = Reg_read(pthis->mDataPort, 0x0009);
    Reg_write(pthis->mDataPort, 0x0009, interface_ctrl_0009 & ~0x3);
    Reg_write(pthis->mDataPort, 0x0009, interface_ctrl_0009 | 0x3);
    Reg_write(pthis->mDataPort, 0x0009, interface_ctrl_0009 & ~0x3);

    //enable MIMO mode, 12 bit compressed values
    uint16_t smpl_width = 0x2; // 0-16 bit, 1-14 bit, 2-12 bit
    if (channelsCount == 2)
    {
        Reg_write(pthis->mDataPort, 0x0007, 0x0003); //channel enables
        Reg_write(pthis->mDataPort, 0x0008, 0x0100 | smpl_width);
    }
    else
    {
        Reg_write(pthis->mDataPort, 0x0007, 0x0001); //channel enables
        Reg_write(pthis->mDataPort, 0x0008, 0x0000 | smpl_width);
    }

    //USB FIFO reset
    ResetUSBFIFO(dynamic_cast<LMS64CProtocol *>(pthis->mDataPort));

     //switch on Rx
    interface_ctrl_000A = Reg_read(pthis->mDataPort, 0x000A);
    Reg_write(pthis->mDataPort, 0x000A, interface_ctrl_000A | 0x1);
    gDataPort = pthis->mDataPort;

    atomic<bool> stopRx(0);
    atomic<bool> stopTx(0);
    atomic<uint32_t> rxRate_Bps(0);
    atomic<uint32_t> txRate_Bps(0);
    std::thread threadRx;
    std::thread threadTx;

    StreamerLTE_ThreadData threadRxArgs;
    threadRxArgs.dataPort = pthis->mDataPort;
    threadRxArgs.FIFO = pthis->mRxFIFO;
    threadRxArgs.terminate = &stopRx;
    threadRxArgs.dataRate_Bps = &rxRate_Bps;

    StreamerLTE_ThreadData threadTxArgs;
    threadTxArgs.dataPort = pthis->mDataPort;
    threadTxArgs.FIFO = pthis->mTxFIFO;
    threadTxArgs.terminate = &stopTx;
    threadTxArgs.dataRate_Bps = &txRate_Bps;

    if (format == STREAM_12_BIT_COMPRESSED)
    {
        threadRx = std::thread(ReceivePackets, threadRxArgs);
        threadTx = std::thread(TransmitPackets, threadTxArgs);
    }
    else
    {
        threadRx = std::thread(ReceivePacketsUncompressed, threadRxArgs);
        threadTx = std::thread(TransmitPacketsUncompressed, threadTxArgs);
    }

    int updateCounter = 0;

    while (pthis->stopProcessing.load() == false)
    {
        uint32_t samplesPopped = pthis->mRxFIFO->pop_samples((complex16_t**)buffers, samplesToRead, channelsCount, &timestamp, timeout_ms);
        ++updateCounter;
        //Transmit earlier received packets with a counter delay
        uint32_t samplesPushed = pthis->mTxFIFO->push_samples((const complex16_t**)buffers, samplesPopped, channelsCount, timestamp + 1024 * 1024, timeout_ms);

        if (updateCounter & 0x40)
        {

            for (int ch = 0; ch < channelsCount; ++ch)
            {
                for (int i = 0; i < fftSize; ++i)
                {
                    localDataResults.samplesI[ch][i] = m_fftCalcIn[i].r = buffers[ch][2 * i];
                    localDataResults.samplesQ[ch][i] = m_fftCalcIn[i].i = buffers[ch][2 * i + 1];
                }
                kiss_fft(m_fftCalcPlan, m_fftCalcIn, m_fftCalcOut);
                for (int i = 0; i < fftSize; ++i)
                {
                    // normalize FFT results
                    m_fftCalcOut[i].r /= fftSize;
                    m_fftCalcOut[i].i /= fftSize;
                }

                int output_index = 0;
                for (int i = fftSize / 2 + 1; i < fftSize; ++i)
                    localDataResults.fftBins_dbFS[ch][output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
                for (int i = 0; i < fftSize / 2 + 1; ++i)
                    localDataResults.fftBins_dbFS[ch][output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
                for (int s = 0; s < fftSize; ++s)
                    localDataResults.fftBins_dbFS[ch][s] = (localDataResults.fftBins_dbFS[ch][s] != 0 ? (20 * log10(localDataResults.fftBins_dbFS[ch][s])) - 69.2369 : -300);
            }
            {
                localDataResults.rxDataRate_Bps = rxRate_Bps.load();
                localDataResults.txDataRate_Bps = txRate_Bps.load();
                std::unique_lock<std::mutex> lck(pthis->mLockIncomingPacket);
                pthis->mIncomingPacket = localDataResults;
            }
            updateCounter = 0;
        }
    }
    stopTx.store(true);
    stopRx.store(true);

    threadTx.join();
    threadRx.join();

    //stop Tx Rx if they were active
    interface_ctrl_000A = Reg_read(pthis->mDataPort, 0x000A);
    Reg_write(pthis->mDataPort, 0x000A, interface_ctrl_000A & ~0x3);

    kiss_fft_free(m_fftCalcPlan);

    for (int i = 0; i < channelsCount; ++i)
        delete[] buffers[i];
    delete []buffers;
#ifndef NDEBUG
    printf("Processing finished\n");
#endif
}

/** @brief Functions dedicated for transmitting packets to board
    @param txFIFO data source FIFO
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void StreamerLTE::TransmitPackets(const StreamerLTE_ThreadData &args)
{
    auto dataPort = args.dataPort;
    auto txFIFO = args.FIFO;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;

    const int channelsCount = txFIFO->GetChannelsCount();
    const int packetsToBatch = 16;
    const int bufferSize = 65536;
    const int buffersCount = 16; // must be power of 2
    const int buffersCountMask = buffersCount - 1;
    int handles[buffersCount];
    memset(handles, 0, sizeof(int)*buffersCount);
    vector<char> buffers;
    try
    {
        buffers.resize(buffersCount*bufferSize, 0);
    }
    catch (const std::bad_alloc& ex) //not enough memory for buffers
    {
        printf("Error allocating Tx buffers, not enough memory\n");
        return;
    }
    memset(&buffers[0], 0, buffersCount*bufferSize);
    bool bufferUsed[buffersCount];
    memset(bufferUsed, 0, sizeof(bool)*buffersCount);
    uint32_t bytesToSend[buffersCount];
    memset(bytesToSend, 0, sizeof(uint32_t)*buffersCount);

    int bi = 0; //buffer index
    complex16_t **outSamples = new complex16_t*[channelsCount];
    const int samplesInPacket = PacketFrame::maxSamplesInPacket / channelsCount;
    for (int i = 0; i < channelsCount; ++i)
    {
        outSamples[i] = new complex16_t[samplesInPacket];
    }

    int m_bufferFailures = 0;
    long bytesSent = 0;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    long totalBytesSent = 0;

    unsigned long outputCounter = 0;
    uint32_t samplesSent = 0;

    uint32_t samplesPopped = 0;
    uint64_t timestamp = 0;
    uint64_t batchSamplesFilled = 0;

    while (terminate->load() != true)
    {
        if (bufferUsed[bi])
        {
            if (dataPort->WaitForSending(handles[bi], 1000) == false)
            {
                ++m_bufferFailures;
            }
            long tempToSend = bytesToSend[bi];
            bytesSent = dataPort->FinishDataSending(&buffers[bi*bufferSize], tempToSend, handles[bi]);
            if (bytesSent == tempToSend)
                totalBytesSent += bytesSent;
            else
                ++m_bufferFailures;
            bufferUsed[bi] = false;
        }
        int i = 0;
        while (i < packetsToBatch)
        {
            uint32_t statusFlags = 0;
            int samplesPopped = txFIFO->pop_samples(outSamples, samplesInPacket, channelsCount, &timestamp, 1000, &statusFlags);
            if (samplesPopped == 0 || samplesPopped != samplesInPacket)
            {
                if (samplesPopped != 0)
                    printf("Error popping from TX, samples popped %i/%i\n", samplesPopped, samplesInPacket);
                if (terminate->load() == true)
                    break;
                continue;
            }
            PacketLTE* pkt = (PacketLTE*)&buffers[bi*bufferSize];
            pkt[i].counter = timestamp;
            if(statusFlags & STATUS_FLAG_TX_TIME)
                pkt[i].reserved[0] &= ~(1 << 4); //synchronize to timestamp
            else
                pkt[i].reserved[0] |= (1 << 4); //ignore timestamp
            uint8_t* dataStart = (uint8_t*)pkt[i].data;
            const int stepSize = channelsCount * 3;
            int samplesCollected = 0;
            for (uint16_t b = 0; b < sizeof(pkt->data); b += stepSize)
            {
                for (int ch = 0; ch < channelsCount; ++ch)
                {
                    //I sample
                    dataStart[b + 3 * ch] = outSamples[ch][samplesCollected].i & 0xFF;
                    dataStart[b + 1 + 3 * ch] = (outSamples[ch][samplesCollected].i >> 8) & 0x0F;

                    //Q sample
                    dataStart[b + 1 + 3 * ch] |= (outSamples[ch][samplesCollected].q << 4) & 0xF0;
                    dataStart[b + 2 + 3 * ch] = (outSamples[ch][samplesCollected].q >> 4) & 0xFF;
                }
                ++samplesCollected;
                ++samplesSent;
            }
            ++i;
            if ((statusFlags & STATUS_FLAG_TX_END) != 0) break;
        }

        bytesToSend[bi] = sizeof(PacketLTE)*i;
        handles[bi] = dataPort->BeginDataSending(&buffers[bi*bufferSize], bytesToSend[bi]);
        bufferUsed[bi] = true;

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            double m_dataRate = 1000.0*totalBytesSent / timePeriod;
            //total number of samples from all channels per second
            float samplingRate = 1000.0*samplesSent / timePeriod;
            if(dataRate_Bps)
                dataRate_Bps->store(m_dataRate);
            samplesSent = 0;
            t1 = t2;
            totalBytesSent = 0;
#ifndef NDEBUG
            printf("Tx rate: %.3f MB/s Fs: %.3f MHz |  failures: %u | TS: %li\n", m_dataRate / 1000000.0, samplingRate / 1000000.0, m_bufferFailures, timestamp);
#endif
            m_bufferFailures = 0;
        }
        bi = (bi + 1) & buffersCountMask;
    }

    // Wait for all the queued requests to be cancelled
    dataPort->AbortSending();
    for (int j = 0; j<buffersCount; j++)
    {
        long bytesToSend = bufferSize;
        if (bufferUsed[j])
        {
            dataPort->WaitForSending(handles[j], 1000);
            dataPort->FinishDataSending(&buffers[j*bufferSize], bytesToSend, handles[j]);
        }
    }
    for (int i = 0; i < channelsCount; ++i)
    {
        delete[]outSamples[i];
    }
    delete[]outSamples;
}

/** @brief Functions dedicated for transmitting packets to board
    @param txFIFO data source FIFO
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void StreamerLTE::TransmitPacketsUncompressed(const StreamerLTE_ThreadData &args)
{
    auto dataPort = args.dataPort;
    auto txFIFO = args.FIFO;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;

    const int channelsCount = 1;
    const int packetsToBatch = 16;
    const int bufferSize = 65536;
    const int buffersCount = 16; // must be power of 2
    const int buffersCountMask = buffersCount - 1;
    int handles[buffersCount];
    memset(handles, 0, sizeof(int)*buffersCount);
    vector<char> buffers;
    try
    {
        buffers.resize(buffersCount*bufferSize, 0);
    }
    catch (const std::bad_alloc& ex) //not enough memory for buffers
    {
        printf("Error allocating Tx buffers, not enough memory\n");
        return;
    }
    memset(&buffers[0], 0, buffersCount*bufferSize);
    bool bufferUsed[buffersCount];
    memset(bufferUsed, 0, sizeof(bool)*buffersCount);
    uint32_t bytesToSend[buffersCount];
    memset(bytesToSend, 0, sizeof(uint32_t)*buffersCount);

    int bi = 0; //buffer index
    complex16_t **outSamples = new complex16_t*[channelsCount];
    const int samplesInPacket = 1020;
    for (int i = 0; i < channelsCount; ++i)
    {
        outSamples[i] = new complex16_t[samplesInPacket];
    }

    int m_bufferFailures = 0;
    long bytesSent = 0;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    long totalBytesSent = 0;

    unsigned long outputCounter = 0;
    uint32_t samplesSent = 0;

    uint32_t samplesPopped = 0;
    uint64_t timestamp = 0;
    uint64_t batchSamplesFilled = 0;

    while (terminate->load() != true)
    {
        if (bufferUsed[bi])
        {
            if (dataPort->WaitForSending(handles[bi], 1000) == false)
            {
                ++m_bufferFailures;
            }
            long tempToSend = bytesToSend[bi];
            bytesSent = dataPort->FinishDataSending(&buffers[bi*bufferSize], tempToSend, handles[bi]);
            if (bytesSent == tempToSend)
                totalBytesSent += bytesSent;
            else
                ++m_bufferFailures;
            bufferUsed[bi] = false;
        }
        int i = 0;
        while (i < packetsToBatch)
        {
            int samplesPopped = txFIFO->pop_samples(outSamples, samplesInPacket, channelsCount, &timestamp, 1000);
            if (samplesPopped == 0 || samplesPopped != samplesInPacket)
            {
                printf("Error popping from TX, samples popped %i/%i\n", samplesPopped, samplesInPacket);
                if (terminate->load() == true)
                    break;
                continue;
            }
            PacketLTE* pkt = (PacketLTE*)&buffers[bi*bufferSize];
            pkt[i].counter = timestamp;
            uint8_t* dataStart = (uint8_t*)pkt[i].data;
            const int stepSize = channelsCount * 4;
            int samplesCollected = 0;
            for (uint16_t b = 0; b < sizeof(pkt->data); b += stepSize)
            {
                for (int ch = 0; ch < channelsCount; ++ch)
                {
                    //I sample
                    dataStart[b + 4 * ch] = outSamples[ch][samplesCollected].i & 0xFF;
                    dataStart[b + 1 + 4 * ch] = (outSamples[ch][samplesCollected].i >> 8) & 0x0F | 0x10;

                    //Q sample
                    dataStart[b + 2 + 4 * ch] |= (outSamples[ch][samplesCollected].q ) & 0xFF;
                    dataStart[b + 3 + 4 * ch] = (outSamples[ch][samplesCollected].q >> 8) & 0x0F;
                }
                ++samplesCollected;
                ++samplesSent;
            }
            ++i;
        }

        bytesToSend[bi] = sizeof(PacketLTE)*packetsToBatch;
        handles[bi] = dataPort->BeginDataSending(&buffers[bi*bufferSize], bytesToSend[bi]);
        bufferUsed[bi] = true;

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            //total number of bytes sent per second
            double m_dataRate = 1000.0*totalBytesSent / timePeriod;
            //total number of samples from all channels per second
            float samplingRate = 1000.0*samplesSent / timePeriod;
            if (dataRate_Bps)
                dataRate_Bps->store(m_dataRate);
            samplesSent = 0;
            t1 = t2;
            totalBytesSent = 0;
#ifndef NDEBUG
            printf("Tx rate: %.3f MB/s Fs: %.3f MHz |  failures: %u\n", m_dataRate / 1000000.0, samplingRate / 1000000.0, m_bufferFailures);
#endif
            m_bufferFailures = 0;
        }
        bi = (bi + 1) & buffersCountMask;
    }

    // Wait for all the queued requests to be cancelled
    dataPort->AbortSending();
    for (int j = 0; j<buffersCount; j++)
    {
        long bytesToSend = bufferSize;
        if (bufferUsed[j])
        {
            dataPort->WaitForSending(handles[j], 1000);
            dataPort->FinishDataSending(&buffers[j*bufferSize], bytesToSend, handles[j]);
        }
    }
    for (int i = 0; i < channelsCount; ++i)
    {
        delete[]outSamples[i];
    }
    delete[]outSamples;
}


/** @brief Returns current data state for user interface
*/
StreamerLTE::DataToGUI StreamerLTE::GetIncomingData()
{
    std::unique_lock<std::mutex> lck(mLockIncomingPacket);
    DataToGUI data = mIncomingPacket;
    return data;
}

StreamerLTE::Stats StreamerLTE::GetStats()
{
    Stats data;
    LMS_SamplesFIFO::BufferInfo rxInfo = mRxFIFO->GetInfo();
    LMS_SamplesFIFO::BufferInfo txInfo = mTxFIFO->GetInfo();
    data.rxBufSize = rxInfo.size;
    data.rxBufFilled = rxInfo.itemsFilled;
    data.txBufSize = txInfo.size;
    data.txBufFilled = txInfo.itemsFilled;
    return data;
}

StreamerLTE::STATUS StreamerLTE::Reg_write(IConnection* dataPort, uint16_t address, uint16_t data)
{
    if(dataPort == nullptr) return FAILURE;
    OperationStatus status;
    status = dataPort->WriteRegister(address, data);
    return status == OperationStatus::SUCCESS ? SUCCESS : FAILURE;
}
uint16_t StreamerLTE::Reg_read(IConnection* dataPort, uint16_t address)
{
    if(dataPort == nullptr) return 0;
    uint32_t dataRd = 0;
    OperationStatus status;
    status = dataPort->ReadRegister(address, dataRd);
    if (status == OperationStatus::SUCCESS)
        return dataRd & 0xFFFF;
    else
        return 0;
}
