#include "StreamerLTE.h"
#include "lmsComms.h"
#include <fstream>
#include <iostream>

#include "kiss_fft.h"

StreamerLTE::StreamerLTE(LMScomms* dataPort)
{
    mDataPort = dataPort;
    mRxFIFO = new StreamerFIFO<PacketLTE>(4096*8);
    mTxFIFO = new StreamerFIFO<PacketLTE>(4096*8);
    mStreamRunning.store(false);
    mTransmitDelay = 1048576;
}

StreamerLTE::~StreamerLTE()
{
    StopStreaming();
    delete mRxFIFO;
    delete mTxFIFO;
}

/** @brief Crates threads for packets receiving, processing and transmitting
*/
StreamerLTE::STATUS StreamerLTE::StartStreaming(unsigned int fftSize)
{
    if (mStreamRunning.load() == true)
        return FAILURE;
    if (mDataPort->IsOpen() == false)
        return FAILURE;
    mRxFIFO->reset();
    mTxFIFO->reset();
    stopRx.store(false);
    stopProcessing.store(false);
    stopTx.store(false);
    threadRx = std::thread(ReceivePackets, this);
    threadProcessing = std::thread(ProcessPackets, this, fftSize);
    threadTx = std::thread(TransmitPackets, this);
    mStreamRunning.store(true);
    return SUCCESS;
}

/** @brief Stops receiving, processing and transmitting threads
*/
StreamerLTE::STATUS StreamerLTE::StopStreaming()
{
    if (mStreamRunning.load() == false)
        return FAILURE;
    stopTx.store(true);
    stopProcessing.store(true);
    stopRx.store(true);
    threadTx.join();
    threadProcessing.join();
    threadRx.join();
    mStreamRunning.store(false);
    return SUCCESS;
}

/** @brief Function dedicated for receiving data samples from board
*/
void StreamerLTE::ReceivePackets(StreamerLTE* pthis)
{
    PacketLTE pkt;
    pkt.counter = 0;
    memset(pkt.samples, 0, sizeof(pkt.samples));
    memset(pkt.reserved, 0, sizeof(pkt.reserved));
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    const int buffer_size = 65536;// 4096;
    const int buffers_count = 16; // must be power of 2
    const int buffers_count_mask = buffers_count - 1;
    int handles[buffers_count];
    memset(handles, 0, sizeof(int)*buffers_count);
    char *buffers = NULL;
    buffers = new char[buffers_count*buffer_size];
    if (buffers == 0)
    {
        printf("error allocating buffers\n");
        return;
    }
    memset(buffers, 0, buffers_count*buffer_size);

    //USB FIFO reset
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_USB_FIFO_RST;
    ctrPkt.outBuffer.push_back(0x01);
    pthis->mDataPort->TransferPacket(ctrPkt);
    ctrPkt.outBuffer[0] = 0x00;
    pthis->mDataPort->TransferPacket(ctrPkt);

    uint16_t regVal = pthis->SPI_read(0x0005);
    pthis->SPI_write(0x0005, regVal | 0x4);

    for (int i = 0; i<buffers_count; ++i)
        handles[i] = pthis->mDataPort->BeginDataReading(&buffers[i*buffer_size], buffer_size);

    int bi = 0;
    int packetsReceived = 0;
    unsigned long BytesReceived = 0;
    int m_bufferFailures = 0;
    while (pthis->stopRx.load() == false)
    {   
        if (pthis->mDataPort->WaitForReading(handles[bi], 1000) == false)
        {
            ++m_bufferFailures;
        }
        long bytesToRead = buffer_size;
        if (pthis->mDataPort->FinishDataReading(&buffers[bi*buffer_size], bytesToRead, handles[bi]))
        {
            ++packetsReceived;
            BytesReceived += bytesToRead;
            for (int p = 0; p < buffer_size / sizeof(pkt); ++p)
            {
                memcpy(&pkt, &buffers[bi*buffer_size+p*sizeof(pkt)], sizeof(pkt));
                int16_t sample;
                for (int i = 0; i < sizeof(pkt.samples) / sizeof(int16_t); ++i)
                {
                    sample = (pkt.samples[i] & 0xFFF);
                    sample = sample << 4;
                    sample = sample >> 4;
                    pkt.samples[i] = sample;
                }
                if (pthis->mRxFIFO->push_back(pkt, 0) == false)
                    ++m_bufferFailures;
            }
        }
        else
        {
            ++m_bufferFailures;
        }

        t2 = chrono::high_resolution_clock::now();
        long timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            float m_dataRate = 1000.0*BytesReceived / timePeriod;
            t1 = t2;
            BytesReceived = 0;
            StreamerFIFO<PacketLTE>::Status rxstats = pthis->mRxFIFO->GetStatus();

            pthis->mRxDataRate.store(m_dataRate);
            pthis->mRxFIFOfilled.store(100.0*rxstats.filledElements / rxstats.maxElements);
#ifndef NDEBUG
            printf("Rx: %.1f%% \t packet counter: %i\n", 100.0*rxstats.filledElements / rxstats.maxElements, pkt.counter);
            printf("Rx rate: %.0f kB/s failures:%i\n", m_dataRate / 1000.0, m_bufferFailures);
#endif
            m_bufferFailures = 0;
        }

        // Re-submit this request to keep the queue full
        memset(&buffers[bi*buffer_size], 0, buffer_size);
        handles[bi] = pthis->mDataPort->BeginDataReading(&buffers[bi*buffer_size], buffer_size);
        bi = (bi + 1) & buffers_count_mask;
    }
    pthis->mDataPort->AbortReading();
    for (int j = 0; j<buffers_count; j++)
    {
        long bytesToRead = buffer_size;
        pthis->mDataPort->WaitForReading(handles[j], 1000);
        pthis->mDataPort->FinishDataReading(&buffers[j*buffer_size], bytesToRead, handles[j]);
    }

    regVal = pthis->SPI_read(0x0005);
    pthis->SPI_write(0x0005, regVal & ~0x4);

    delete[] buffers;
#ifndef NDEBUG
    printf("Rx finished\n");
#endif
}

/** @brief Function dedicated for processing incomming data and generating outputs for transmitting
*/
void StreamerLTE::ProcessPackets(StreamerLTE* pthis, unsigned int fftSize)
{
    DataToGUI localDataResults;
    localDataResults.nyquist_MHz = 7.68*1000000;
    localDataResults.samplesI.resize(fftSize, 0);
    localDataResults.samplesQ.resize(fftSize, 0);
    localDataResults.fftBins_dbFS.resize(fftSize, 0);
    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];

    const int samplesInPacket = (4096-16) / 4;
    const int packetsCountForFFT = fftSize / samplesInPacket + ((fftSize % samplesInPacket) != 0);

    PacketLTE* pkt = new PacketLTE[packetsCountForFFT];
    while (pthis->stopProcessing.load() == false)
    {
		//collect enough packets for desired FFT size
        for (int i = 0; i < packetsCountForFFT; ++i)
        {
            if (pthis->mRxFIFO->pop_front(&pkt[i], 1000) == false)
            {
                if (pthis->stopProcessing.load() == true)
                    break;
                printf("\tError: popping from RX\n");
                continue;
            }
        }

        unsigned int samplesUsed = 0;
        for (int i = 0; i < fftSize; ++i)
        {
            localDataResults.samplesI[i] = m_fftCalcIn[i].r = pkt[i / samplesInPacket].samples[(2 * i) % (2*samplesInPacket)];
            localDataResults.samplesQ[i] = m_fftCalcIn[i].i = pkt[i / samplesInPacket].samples[(2 * i + 1) % (2*samplesInPacket)];
        }
        kiss_fft(m_fftCalcPlan, m_fftCalcIn, m_fftCalcOut);

        for (int i = 0; i<fftSize; ++i)
        {
            // normalize FFT results
            m_fftCalcOut[i].r /= fftSize;
            m_fftCalcOut[i].i /= fftSize;
        }
        int output_index = 0;
        for (int i = fftSize / 2 + 1; i <fftSize; ++i)
            localDataResults.fftBins_dbFS[output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
        for (int i = 0; i <fftSize / 2 + 1; ++i)
            localDataResults.fftBins_dbFS[output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
        for (int s = 0; s<fftSize; ++s)
            localDataResults.fftBins_dbFS[s] = (localDataResults.fftBins_dbFS[s] != 0 ? (20 * log10(localDataResults.fftBins_dbFS[s])) - 69.2369 : -300);

        {
            std::unique_lock<std::mutex> lck(pthis->mLockIncomingPacket);
            pthis->mIncomingPacket = localDataResults;
        }
		
		//Transmit earlier received packets with a counter delay
        for (int i = 0; i < packetsCountForFFT; ++i)
        {
            pkt[i].counter += pthis->mTransmitDelay;
            bool iqSelect = true;
            for (int j = 0; j < sizeof(pkt[i].samples) / sizeof(int16_t); ++j)
            {
                if (iqSelect) //output samples need to have iq select bit
                    pkt[i].samples[j] = (pkt[i].samples[j] & 0xFFF) | 0x1000;
                else
                    pkt[i].samples[j] = (pkt[i].samples[j] & 0xFFF);
                iqSelect = !iqSelect;
            }
            if (pthis->mTxFIFO->push_back(pkt[i], 1000) == false)
            {
                printf("\tError: pushing into TX\n");
                if (pthis->stopProcessing.load() == true)
                    break;
                continue;
            }
        }
    }
    delete[] pkt;
#ifndef NDEBUG
    printf("Processing finished\n");
#endif
}

/** @brief Functions dedicated for transmitting packets to board
*/
void StreamerLTE::TransmitPackets(StreamerLTE* pthis)
{
    const int packetsToBatch = 16;
    const int buffer_size = sizeof(PacketLTE)*packetsToBatch;
    const int buffers_count = 16; // must be power of 2
    const int buffers_count_mask = buffers_count - 1;
    int handles[buffers_count];
    memset(handles, 0, sizeof(int)*buffers_count);
    char *buffers = NULL;
    buffers = new char[buffers_count*buffer_size];
    if (buffers == 0)
    {
        printf("error allocating buffers\n");
    }
    memset(buffers, 0, buffers_count*buffer_size);
    bool *bufferUsed = new bool[buffers_count];
    memset(bufferUsed, 0, sizeof(bool)*buffers_count);

    int bi = 0; //buffer index

    PacketLTE* pkt = new PacketLTE[packetsToBatch];
    int m_bufferFailures = 0;
    long bytesSent = 0;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    long totalBytesSent = 0;

    unsigned long outputCounter = 0;

    while (pthis->stopTx.load() == false)
    {
        for (int i = 0; i < packetsToBatch; ++i)
        {
            if (pthis->mTxFIFO->pop_front(&pkt[i], 200) == false)
            {
                printf("Error popping from TX\n");
                if (pthis->stopTx.load() == false)
                    break;
                continue;
            }
        }
        //wait for desired slot buffer to be transferred
        if (bufferUsed[bi])
        {
            if (pthis->mDataPort->WaitForSending(handles[bi], 1000) == false)
            {
                ++m_bufferFailures;
            }
            // Must always call FinishDataXfer to release memory of contexts[i]
            long bytesToSend = buffer_size;
            bytesSent = pthis->mDataPort->FinishDataSending(&buffers[bi*buffer_size], bytesToSend, handles[bi]);
            if (bytesSent > 0)
                totalBytesSent += bytesSent;
            else
                ++m_bufferFailures;
            bufferUsed[bi] = false;
        }

        memcpy(&buffers[bi*buffer_size], &pkt[0], sizeof(PacketLTE)*packetsToBatch);
        handles[bi] = pthis->mDataPort->BeginDataSending(&buffers[bi*buffer_size], buffer_size);
        bufferUsed[bi] = true;

        t2 = chrono::high_resolution_clock::now();
        long timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            float m_dataRate = 1000.0*totalBytesSent / timePeriod;
            t1 = t2;
            totalBytesSent = 0;
#ifndef NDEBUG
            cout << "Tx rate: " << m_dataRate/1000 << " kB/s\t failures: " << m_bufferFailures << endl;
#endif
            StreamerFIFO<PacketLTE>::Status txstats = pthis->mTxFIFO->GetStatus();
            pthis->mTxDataRate.store(m_dataRate);
            pthis->mTxFIFOfilled.store(100.0*txstats.filledElements / txstats.maxElements);
            m_bufferFailures = 0;
        }
        bi = (bi + 1) & buffers_count_mask;
    }

    // Wait for all the queued requests to be cancelled
    pthis->mDataPort->AbortSending();
    for (int j = 0; j<buffers_count; j++)
    {
        long bytesToSend = buffer_size;
        if (bufferUsed[bi])
        {
            pthis->mDataPort->WaitForSending(handles[j], 1000);
            pthis->mDataPort->FinishDataSending(&buffers[j*buffer_size], bytesToSend, handles[j]);
        }
    }

    delete[] buffers;
    delete[] bufferUsed;
    delete[] pkt;
#ifndef NDEBUG
    printf("Tx finished\n");
#endif
}

/** @brief Returns current data state for user interface
*/
StreamerLTE::DataToGUI StreamerLTE::GetIncomingData()
{
    std::unique_lock<std::mutex> lck(mLockIncomingPacket);
    return mIncomingPacket;
}

StreamerLTE::ProgressStats StreamerLTE::GetStats()
{
    ProgressStats stats;
    stats.RxRate_Bps = mRxDataRate.load();
    stats.TxRate_Bps = mTxDataRate.load();
    stats.RxFIFOfilled = mRxFIFOfilled.load();
    stats.TxFIFOfilled = mTxFIFOfilled.load();
    return stats;
}

StreamerLTE::STATUS StreamerLTE::SPI_write(uint16_t address, uint16_t data)
{
    assert(mDataPort != nullptr);
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_BRDSPI_WR;
    ctrPkt.outBuffer.push_back((address >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(address & 0xFF);
    ctrPkt.outBuffer.push_back((data >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(data & 0xFF);
    mDataPort->TransferPacket(ctrPkt);
    return ctrPkt.status == 1 ? SUCCESS : FAILURE;
}
uint16_t StreamerLTE::SPI_read(uint16_t address)
{
    assert(mDataPort != nullptr);
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_BRDSPI_RD;
    ctrPkt.outBuffer.push_back((address >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(address & 0xFF);    
    mDataPort->TransferPacket(ctrPkt);
    return ctrPkt.inBuffer[2] * 256 + ctrPkt.inBuffer[3];
}
