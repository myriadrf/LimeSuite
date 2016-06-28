#include "LMS_StreamBoard.h"
#include "IConnection.h"
#include "LMS64CProtocol.h" //TODO remove when reset usb is abstracted
#include <assert.h>
#include <iostream>
#include "kiss_fft.h"
#include <fstream>
#include "windowFunction.h"

using namespace std;
using namespace lime;

LMS_StreamBoard::LMS_StreamBoard(IConnection* dataPort)
{
    mRxFrameStart.store(false);
    mDataPort = dataPort;
    mRxFIFO = new LMS_StreamBoard_FIFO<SamplesPacket>(1024*2);
    mTxFIFO = new LMS_StreamBoard_FIFO<SamplesPacket>(1024*2);
    mStreamRunning.store(false);
    mTxCyclicRunning.store(false);
    captureToFile = false;
    samplesToCapture = 0;
    captureFilename = "";
    windowFunction = 0;
}

LMS_StreamBoard::~LMS_StreamBoard()
{
    StopReceiving();
    delete mRxFIFO;
    delete mTxFIFO;
}

/** @brief Crates threads for packets receiving, processing and transmitting
*/
LMS_StreamBoard::Status LMS_StreamBoard::StartReceiving(unsigned int fftSize)
{
    if (mStreamRunning.load() == true)
        return FAILURE;
    if (mDataPort->IsOpen() == false)
        return FAILURE;
    mRxFIFO->reset();
    stopRx.store(false);
    stopProcessing.store(false);
    threadRx = std::thread(ReceivePackets, this);
    threadProcessing = std::thread(ProcessPackets, this, fftSize);
    mStreamRunning.store(true);
    return SUCCESS;
}

/** @brief Stops receiving, processing and transmitting threads
*/
LMS_StreamBoard::Status LMS_StreamBoard::StopReceiving()
{
    if (mStreamRunning.load() == false)
        return FAILURE;
    stopTx.store(true);
    stopProcessing.store(true);
    stopRx.store(true);
    threadProcessing.join();
    threadRx.join();
    mStreamRunning.store(false);
    return SUCCESS;
}

static void ResetUSBFIFO(LMS64CProtocol* port)
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

/** @brief Function dedicated for receiving data samples from board
*/
void LMS_StreamBoard::ReceivePackets(LMS_StreamBoard* pthis)
{
    if(pthis->mDataPort == nullptr)
    {
    #ifndef NDEBUG
        printf("ReceivePackets: port not connected");
    #endif
        return;
    }


    SamplesPacket pkt;
    int samplesCollected = 0;
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

    ResetUSBFIFO(dynamic_cast<LMS64CProtocol *>(pthis->mDataPort));

    //start Rx
    uint16_t regVal = pthis->Reg_read(0x000A);
    pthis->Reg_write(0x000A, regVal | 0x1);

    for (int i = 0; i<buffers_count; ++i)
        handles[i] = pthis->mDataPort->BeginDataReading(&buffers[i*buffer_size], buffer_size);

    int bi = 0;
    int packetsReceived = 0;
    unsigned long BytesReceived = 0;
    int m_bufferFailures = 0;
    short sample;

    bool frameStart = pthis->mRxFrameStart.load();
    while (pthis->stopRx.load() == false)
    {
        if (pthis->mDataPort->WaitForReading(handles[bi], 1000) == false)
            ++m_bufferFailures;

        long bytesToRead = buffer_size;
        long bytesReceived = pthis->mDataPort->FinishDataReading(&buffers[bi*buffer_size], bytesToRead, handles[bi]);
        if (bytesReceived > 0)
        {
            ++packetsReceived;
            BytesReceived += bytesReceived;
            char* bufStart = &buffers[bi*buffer_size];

            for (int p = 0; p < bytesReceived; p+=2)
            {
                if (samplesCollected == 0) //find frame start
                {
                    int frameStartOffset = FindFrameStart(&bufStart[p], bytesReceived-p, frameStart);
                    if (frameStartOffset < 0)
                        break; //frame start was not found, move on to next buffer
                    p += frameStartOffset;
                }
                sample = (bufStart[p+1] & 0x0F);
                sample = sample << 8;
                sample |= (bufStart[p] & 0xFF);
                sample = sample << 4;
                sample = sample >> 4;
                pkt.iqdata[samplesCollected] = sample;
                ++samplesCollected;
                if (pkt.samplesCount == samplesCollected)
                {
                    samplesCollected = 0;
                    if (pthis->mRxFIFO->push_back(pkt, 200) == false)
                        ++m_bufferFailures;
                }
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
            // periodically update frame start, user might change it during operation
            frameStart = pthis->mRxFrameStart.load();
            float m_dataRate = 1000.0*BytesReceived / timePeriod;
            t1 = t2;
            BytesReceived = 0;
            LMS_StreamBoard_FIFO<SamplesPacket>::Status rxstats = pthis->mRxFIFO->GetStatus();

            pthis->mRxDataRate.store(m_dataRate);
            pthis->mRxFIFOfilled.store(100.0*rxstats.filledElements / rxstats.maxElements);
#ifndef NDEBUG
            printf("Rx: %.1f%% \t rate: %.0f kB/s failures:%i\n", 100.0*rxstats.filledElements / rxstats.maxElements, m_dataRate / 1000.0, m_bufferFailures);
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

    regVal = pthis->Reg_read(0x000A);
    pthis->Reg_write(0x000A, regVal & ~0x1);

    delete[] buffers;
#ifndef NDEBUG
    printf("Rx finished\n");
#endif
}

/** @brief Function dedicated for processing incomming data and calculating FFT
*/
void LMS_StreamBoard::ProcessPackets(LMS_StreamBoard* pthis, unsigned int fftSize)
{
    DataToGUI localDataResults;
    localDataResults.nyquist_Hz = 7.68 * 1000000;
    localDataResults.samplesI.resize(fftSize, 0);
    localDataResults.samplesQ.resize(fftSize, 0);
    localDataResults.fftBins_dbFS.resize(fftSize, 0);
    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];

    vector<int16_t> capturedSamples;
    bool captureToFile = pthis->captureToFile;
    if(captureToFile)
        capturedSamples.reserve(2*pthis->samplesToCapture);

    const int framesInPacket = 16384;
    const int packetsCountForFFT = fftSize / framesInPacket + ((fftSize % framesInPacket) != 0);

    vector<float> wndFunc;
    float ampCorr;
    GenerateWindowCoefficients(pthis->windowFunction, fftSize, wndFunc, ampCorr);

    SamplesPacket* pkt = new SamplesPacket[packetsCountForFFT];
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

        if(captureToFile && capturedSamples.size() < pthis->samplesToCapture*2)
        {
            for(int i=0; i<fftSize && capturedSamples.size() < pthis->samplesToCapture*2; ++i)
            {
                capturedSamples.push_back(pkt[i / (2*framesInPacket)].iqdata[(2 * i) % (2*framesInPacket)]); //I
                capturedSamples.push_back(pkt[i / (2*framesInPacket)].iqdata[(2 * i + 1) % (2*framesInPacket)]); //Q
            }
        }

        unsigned int samplesUsed = 0;
        for (int i = 0; i < fftSize; ++i)
        {
            localDataResults.samplesI[i] = pkt[i / (2*framesInPacket)].iqdata[(2 * i) % (2*framesInPacket)];
            localDataResults.samplesQ[i] = pkt[i / (2*framesInPacket)].iqdata[(2 * i + 1) % (2*framesInPacket)];
            m_fftCalcIn[i].r = localDataResults.samplesI[i] * wndFunc[i] * ampCorr;
            m_fftCalcIn[i].i = localDataResults.samplesQ[i] * wndFunc[i] * ampCorr;
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

    }
    delete[] pkt;
    if(pthis->captureToFile)
    {
        std::ofstream fout;
        fout.open(pthis->captureFilename.c_str());
        fout << "AI\tAQ" << endl;
        int samplesCnt = capturedSamples.size();
        for(int i=0; i<samplesCnt; i+=2)
            fout << capturedSamples[i] << "\t" << capturedSamples[i+1] << endl;
    }
#ifndef NDEBUG
    printf("Processing finished\n");
#endif
}

/** @brief Function dedicated for transmitting samples to board
*/
void LMS_StreamBoard::TransmitPackets(LMS_StreamBoard* pthis)
{
    if(pthis->mDataPort == nullptr)
    {
    #ifndef NDEBUG
        printf("ReceivePackets: port not connected");
    #endif
        return;
    }

    const int packetsToBatch = 16;
    const int buffer_size = sizeof(SamplesPacket)*packetsToBatch;
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

    SamplesPacket* pkt = new SamplesPacket[packetsToBatch];
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

        memcpy(&buffers[bi*buffer_size], &pkt[0], sizeof(SamplesPacket)*packetsToBatch);
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
            cout << "Tx rate: " << m_dataRate / 1000 << " kB/s\t failures: " << m_bufferFailures << endl;
#endif
            LMS_StreamBoard_FIFO<SamplesPacket>::Status txstats = pthis->mTxFIFO->GetStatus();
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
LMS_StreamBoard::DataToGUI LMS_StreamBoard::GetIncomingData()
{
    std::unique_lock<std::mutex> lck(mLockIncomingPacket);
    return mIncomingPacket;
}

/** @brief Returns data rate info and Tx Rx FIFO fill percentage
*/
LMS_StreamBoard::ProgressStats LMS_StreamBoard::GetStats()
{
    ProgressStats stats;
    stats.RxRate_Bps = mRxDataRate.load();
    stats.TxRate_Bps = mTxDataRate.load();
    stats.RxFIFOfilled = mRxFIFOfilled.load();
    stats.TxFIFOfilled = mTxFIFOfilled.load();
    return stats;
}

/** @brief Helper function to write board spi regiters
    @param address spi address
    @param data register value
*/
LMS_StreamBoard::Status LMS_StreamBoard::Reg_write(uint16_t address, uint16_t data)
{
    assert(mDataPort != nullptr);
    int status = mDataPort->WriteRegister(address, data);
    return status == 0 ? SUCCESS : FAILURE;
}

/** @brief Helper function to read board spi registers
    @param address spi address
    @return register value
*/
uint16_t LMS_StreamBoard::Reg_read(uint16_t address)
{
    assert(mDataPort != nullptr);
    uint32_t dataRd = 0;
    int status = mDataPort->ReadRegister(address, dataRd);
    if (status == 0)
        return dataRd & 0xFFFF;
    else
        return 0;
}

/** @brief Changes which frame start to look for when receiving data
*/
void LMS_StreamBoard::SetRxFrameStart(const bool startValue)
{
    mRxFrameStart.store(startValue);
}

/** @brief Searches for frame start index in given buffer
    @return frameStart index in buffer, -1 if frame start was not found
    Frame start indicator is 13th bit inside each I and Q sample
*/
int LMS_StreamBoard::FindFrameStart(const char* buffer, const int bufLen, const bool frameStart)
{
    int startIndex = -1;
    for (int i = 0; i < bufLen; i+=2)
        if ((buffer[i+1] & 0x10)>0 == frameStart)
        {
            startIndex = i;
            break;
        }
    return startIndex;
}

/** @brief Starts a thread for continuous cyclic transmitting of given samples
    @param isamples I channel samples
    @param qsamples Q channel samples
    @param framesCount number of samples in given arrays
    @return 0:success, other:failure
*/
LMS_StreamBoard::Status LMS_StreamBoard::StartCyclicTransmitting(const int16_t* isamples, const int16_t* qsamples, uint32_t framesCount)
{
    if (!mDataPort || mDataPort->IsOpen() == false)
        return FAILURE;

    stopTxCyclic.store(false);
    threadTxCyclic = std::thread([](LMS_StreamBoard* pthis)
    {
        const int buffer_size = 65536;
        const int buffers_count = 16; // must be power of 2
        const int buffers_count_mask = buffers_count - 1;
        int handles[buffers_count];
        memset(handles, 0, sizeof(int)*buffers_count);
        char *buffers = NULL;
        buffers = new char[buffers_count*buffer_size];
        if (buffers == 0)
        {
            printf("error allocating buffers\n");
            return 0;
        }
        memset(buffers, 0, buffers_count*buffer_size);

        //timers for data rate calculation
        auto t1 = chrono::high_resolution_clock::now();
        auto t2 = chrono::high_resolution_clock::now();

        int bi = 0; //buffer index

        //setup output data
        int dataIndex = 0;
        for (int i = 0; i < buffers_count; ++i)
        {
            for (int j = 0; j < buffer_size; ++j)
            {
                buffers[i*buffer_size + j] = pthis->mCyclicTransmittingSourceData[dataIndex];
                ++dataIndex;
                if (dataIndex > pthis->mCyclicTransmittingSourceData.size())
                    dataIndex = 0;
            }
        }

        for (int i = 0; i < buffers_count; ++i)
            handles[i] = pthis->mDataPort->BeginDataSending(&buffers[i*buffer_size], buffer_size);

        int m_bufferFailures = 0;
        int bytesSent = 0;
        int totalBytesSent = 0;
        int sleepTime = 200;
        while (pthis->stopTxCyclic.load() != true)
        {
            if (pthis->mDataPort->WaitForSending(handles[bi], 1000) == false)
            {
                ++m_bufferFailures;
            }
            long bytesToSend = buffer_size;
            bytesSent = pthis->mDataPort->FinishDataSending(&buffers[bi*buffer_size], bytesToSend, handles[bi]);
            if (bytesSent > 0)
                totalBytesSent += bytesSent;
            else
            {
                ++m_bufferFailures;
            }

            t2 = chrono::high_resolution_clock::now();
            long timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            if (timePeriod >= 1000)
            {
                pthis->mTxDataRate.store(1000.0*totalBytesSent / timePeriod);
                t1 = t2;
                totalBytesSent = 0;
#ifndef NDEBUG
                printf("Upload rate: %f\t failures:%i\n", 1000.0*totalBytesSent / timePeriod, m_bufferFailures);
#endif
                m_bufferFailures = 0;
            }

            //fill up next buffer
            for (int j = 0; j < buffer_size; ++j)
            {
                buffers[bi*buffer_size + j] = pthis->mCyclicTransmittingSourceData[dataIndex];
                ++dataIndex;
                if (dataIndex >= pthis->mCyclicTransmittingSourceData.size())
                    dataIndex = 0;
            }

            // Re-submit this request to keep the queue full
            handles[bi] = pthis->mDataPort->BeginDataSending(&buffers[bi*buffer_size], buffer_size);
            bi = (bi + 1) & buffers_count_mask;
        }

        // Wait for all the queued requests to be cancelled
        pthis->mDataPort->AbortSending();
        for (int j = 0; j < buffers_count; j++)
        {
            long bytesToSend = buffer_size;
            pthis->mDataPort->WaitForSending(handles[j], 1000);
            pthis->mDataPort->FinishDataSending(&buffers[j*buffer_size], bytesToSend, handles[j]);
        }
#ifndef NDEBUG
        printf("Cyclic transmitting FULLY STOPPED\n");
#endif
        delete[] buffers;
        return 0;
    }, this);
    mTxCyclicRunning.store(true);

    return LMS_StreamBoard::SUCCESS;
}

/** @brief Stops cyclic transmitting thread
*/
LMS_StreamBoard::Status LMS_StreamBoard::StopCyclicTransmitting()
{
    stopTxCyclic.store(true);
    if (mTxCyclicRunning.load() == true)
    {
        threadTxCyclic.join();
        mTxCyclicRunning.store(false);
    }
    return LMS_StreamBoard::SUCCESS;
}

void LMS_StreamBoard::SetCaptureToFile(bool enable, const char* filename, uint32_t samplesCount)
{
    captureToFile = enable;
    captureFilename = filename;
    samplesToCapture = samplesCount;
}

void LMS_StreamBoard::SetWidowFunction(int func)
{
    windowFunction = func;
}
