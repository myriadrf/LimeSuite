/* 
 * File:   SteamerAPI.cpp
 * Author: ignas
 * 
 * Created on May 5, 2016, 11:01 AM
 */

#include "StreamerAPI.h"
#include "dataTypes.h"
#include <chrono>


static const int MAX_PACKETS_BATCH = 16;
static const int SAMPLES12_PACKET = 1360;
static const int SAMPLES16_PACKET = 1020;

StreamerAPI::StreamerAPI(lime::IConnection* port)
{
    
    rx_handles = nullptr;
    rx_buffers = nullptr;
    rx_running = false;
    streamPort = port;

    tx_handles = nullptr;
    tx_buffers = nullptr;
    tx_bufferUsed = nullptr; 
    tx_running = false;
 
    packetsToBatch = 2;
    channelsCount = 1;
    streamConf.dataFmt == lms_stream_conf_t::LMS_FMT_F32;
    streamConf.linkFmt == lms_stream_conf_t::LMS_LINK_12BIT;
    streamConf.fifoSize = 0;
}



StreamerAPI::~StreamerAPI()
{
    StopRx();
    StopTx();
}

 int StreamerAPI::SetupStream(lms_stream_conf_t conf)
 {
    streamConf = conf;
     
    int buffersCount = 1; // must be power of 2
    while (buffersCount < streamConf.numTransfers)
        buffersCount <<=1;
    streamConf.numTransfers = buffersCount; 
    
    packetsToBatch = 1+(conf.transferSize-1)/sizeof(lime::PacketLTE);
    if (packetsToBatch > MAX_PACKETS_BATCH)
        packetsToBatch = MAX_PACKETS_BATCH;
    
    if (streamConf.channels == 3)
        channelsCount = 2;
 }

int StreamerAPI::RecvStream(void **data,size_t numSamples, lms_stream_meta_t *meta, unsigned timeout_ms)
{
    const long bufferSize = packetsToBatch*SAMPLES12_PACKET; 
    static int16_t buffer[MAX_PACKETS_BATCH*SAMPLES12_PACKET*2];
    static uint64_t ts = 0;
    uint64_t rx_meta = 0;
    static int index = bufferSize;
    int ret;
    
    rx_lock.lock();
 
    if (streamConf.dataFmt == lms_stream_conf_t::LMS_FMT_F32)
        ret = RecvStreamFloat((float**)data, buffer, numSamples, &ts, index, &rx_meta, timeout_ms);
    else
        ret = RecvStreamInt16((int16_t**)data, buffer, numSamples, &ts, index, &rx_meta, timeout_ms);
                
    if (meta->end_of_burst)
        index = bufferSize;  
    
   meta->timestamp = ts+index/channelsCount-ret;
   rx_lock.unlock();
   return ret;
}

int StreamerAPI::RecvStreamFloat(float **data, int16_t* buffer, size_t numSamples, uint64_t* ts, int &index, uint64_t* rx_meta, unsigned timeout_ms)
{
    const long bufferSize = packetsToBatch*SAMPLES12_PACKET; 
    
    for (int i = 0; i < numSamples ; i++)
    {
        if (index == bufferSize)
        {       
            if ((_read(buffer,ts,rx_meta,timeout_ms))!=bufferSize)
            {
                return i;   
            }    
            index = 0;
        }
        
        for (int ch = 0;ch<channelsCount;ch++)
        {
            data[ch][2*i] = (float)buffer[2*index]/2047.0;
            data[ch][2*i+1] = (float)buffer[2*index+1]/2047.0;
            index++;
        }
    }
    return numSamples;
}


int StreamerAPI::RecvStreamInt16(int16_t **data, int16_t* buffer, size_t numSamples, uint64_t* ts, int &index, uint64_t* rx_meta, unsigned timeout_ms)
{
    const long bufferSize = packetsToBatch*SAMPLES12_PACKET; 
    
    for (int i = 0; i < numSamples ; i++)
    {
        if (index == bufferSize)
        {
            index = 0;
            if ((_read(buffer,ts,rx_meta,timeout_ms))!=bufferSize)
            {
                return i;   
            }           
        }
        
        for (int ch = 0;ch<channelsCount;ch++)
        {
            data[ch][2*i] = buffer[2*index];
            data[ch][2*i+1] = buffer[2*index+1];
            index++;
        }
    }
    return numSamples;
}

int StreamerAPI::SendStream(const void **data,size_t numSamples, lms_stream_meta_t *meta, unsigned timeout_ms)
{
    const long bufferSize = packetsToBatch*SAMPLES12_PACKET; 
    static int16_t buffer[MAX_PACKETS_BATCH*SAMPLES12_PACKET*2];
    uint64_t tx_meta = 0;
    static int index = 0;
    
    if(meta->has_timestamp == false)
        tx_meta |= (1 << 4); //ignore timestamp
    
    tx_lock.lock();
    int ret;
    
    if (streamConf.dataFmt == lms_stream_conf_t::LMS_FMT_F32)
        ret = SendStreamFloat((const float**)data, buffer, numSamples, meta->timestamp, index, tx_meta, timeout_ms);
    else
        ret = SendStreamInt16((const int16_t**)data, buffer, numSamples, meta->timestamp, index, tx_meta, timeout_ms);
    
    if (meta->end_of_burst)
    {
        for (int i = index; i <bufferSize;i++)
        {
            buffer[2*i] = 0;
            buffer[2*i+1] = 0;     
        }    
       uint64_t ts = meta->timestamp + numSamples - index/channelsCount;
       _write(buffer,ts,tx_meta,timeout_ms);
       index = 0; 
    }
    
   tx_lock.unlock();
   return ret;
}


int StreamerAPI::SendStreamInt16(const int16_t **data, int16_t* buffer, size_t numSamples, uint64_t ts, int &index, uint64_t tx_meta, unsigned timeout_ms)
{
    const long bufferSize = packetsToBatch*SAMPLES12_PACKET; 

    for (int i = 0; i < numSamples ; i++)
    {
       for (int ch = 0;ch<channelsCount;ch++)
        {
           buffer[2*index] = data[ch][2*i];
           buffer[2*index+1] = data[ch][2*i+1];
           index++;
        }
        
        if (index == bufferSize)
        {
             ts = ts + i - bufferSize/channelsCount;
             index = 0;
             if (_write(buffer,ts,tx_meta,timeout_ms)!=bufferSize)
             {
                return i;
             }
        }
    }  
   return numSamples;
}

int StreamerAPI::SendStreamFloat(const float **data, int16_t* buffer, size_t numSamples, uint64_t ts, int &index, uint64_t tx_meta, unsigned timeout_ms)
{
    const long bufferSize = packetsToBatch*SAMPLES12_PACKET; 

    for (int i = 0; i < numSamples ; i++)
    {
       for (int ch = 0;ch<channelsCount;ch++)
        {
           buffer[2*index] = data[ch][2*i]*2047.0;
           buffer[2*index+1] = data[ch][2*i+1]*2047.0;
           index++;
        }
        
        if (index == bufferSize)
        {
             ts = ts + i - bufferSize/channelsCount;
             index = 0;
             if (_write(buffer,ts,tx_meta,timeout_ms)!=bufferSize)
             {
                return i;
             }
        }
    }  
   return numSamples;
}


int StreamerAPI::_read(int16_t *data, uint64_t *timestamp, uint64_t *metadata, unsigned timeout)
{
    if (rx_running == false)
        return 0;
    
    long bufferSize = packetsToBatch*sizeof(lime::PacketLTE);
    const int buffersCountMask = streamConf.numTransfers - 1;
    static int bi = 0;
    int n = 0;

    if (streamPort->WaitForReading(rx_handles[bi], timeout) == false)
    {
        return -1;
    }

    long bytesReceived = streamPort->FinishDataReading(&rx_buffers[bi*bufferSize], bufferSize, rx_handles[bi]);
    if (bytesReceived > 0)
    {    
	lime::PacketLTE* pkt= (lime::PacketLTE*)&rx_buffers[bi*bufferSize];
        
        *timestamp = pkt->counter; 
        *metadata = *((uint64_t*)(&pkt->reserved));
        for (int p = 0; p<packetsToBatch;p++)
        {     
            for (int i = 0; i<sizeof(pkt[p].data);i+=3)
            {
                //I sample
                int16_t sample;
                sample = (pkt[p].data[i +1] & 0x0F) << 8;
                sample |= (pkt[p].data[i] & 0xFF);
                sample = sample << 4;
                sample = sample >> 4;
                data[n++] = sample;

                //Q sample
                sample = pkt[p].data[i + 2] << 4;
                sample |= (pkt[p].data[i + 1] >> 4) & 0x0F;
                sample = sample << 4;
                sample = sample >> 4;
                data[n++] = sample;
            }
        }
        
    }
    else return -1;

    // Re-submit this request to keep the queue full
    memset(&rx_buffers[bi*bufferSize], 0, bufferSize);
    rx_handles[bi] = streamPort->BeginDataReading(&rx_buffers[bi*bufferSize], bufferSize);
    bi = (bi + 1) & buffersCountMask;
    return n/2;    
}

void StreamerAPI::ResetUSBFIFO()
{
	lime::LMS64CProtocol* port = dynamic_cast<lime::LMS64CProtocol *>(streamPort);
// TODO : USB FIFO reset command for IConnection
    if (port == nullptr) return;
	lime::LMS64CProtocol::GenericPacket ctrPkt;
    ctrPkt.cmd = lime::CMD_USB_FIFO_RST;
    ctrPkt.outBuffer.push_back(0x01);
    port->TransferPacket(ctrPkt);
    ctrPkt.outBuffer[0] = 0x00;
    port->TransferPacket(ctrPkt);
}
    
int StreamerAPI::Start()
{   
    uint32_t dataRd = 0;
    // Stop Tx Rx if they were active
    streamPort->ReadRegister(0x000A,dataRd);
    streamPort->WriteRegister(0x000A, dataRd & ~0x3);
    //reset hardware timestamp to 0
    streamPort->ReadRegister(0x0009,dataRd);
    streamPort->WriteRegister(0x0009, dataRd & ~0x3);
    streamPort->WriteRegister(0x0009, dataRd | 0x3);
    streamPort->WriteRegister(0x0009, dataRd & ~0x3);

    //enable MIMO mode, 12 bit compressed values
    if (channelsCount == 2)
    {
        streamPort->WriteRegister(0x0008, (1<<8)|(0x0002));
        streamPort->WriteRegister(0x0007, 0x0003);
    }
    else
    {
        streamPort->WriteRegister(0x0008, (1<<8)|(0x0002));
        streamPort->WriteRegister(0x0007, 0x0001);
    }

    //USB FIFO reset
    ResetUSBFIFO();

    //switch on Rx
    streamPort->ReadRegister(0x000A,dataRd);
    streamPort->WriteRegister(0x000A, dataRd | 0x1);   
}

int StreamerAPI::Stop()
{
    uint32_t dataRd = 0;
    streamPort->ReadRegister(0x000A,dataRd);
    streamPort->WriteRegister(0x000A, dataRd & ~0x3);
	return 0;
}


int StreamerAPI::StartRx()
{ 
    rx_lock.lock();
    
    if (rx_running)
    {
        rx_lock.unlock();
        return 0;
    }
    
    unsigned bufferSize = packetsToBatch*sizeof(lime::PacketLTE);  
     
    if (rx_handles != nullptr)
        delete [] rx_handles;
    rx_handles = new int[streamConf.numTransfers];
    memset(rx_handles, 0, streamConf.numTransfers*sizeof(int));
    
    if (rx_buffers != nullptr)
        delete [] rx_buffers;
    rx_buffers = new char[streamConf.numTransfers*bufferSize];
    memset(rx_buffers,0,streamConf.numTransfers*bufferSize);
    
    rx_running = true;
    if (tx_running == false)
        Start();
    
    for (int i = 0; i<streamConf.numTransfers; ++i)
        rx_handles[i] = streamPort->BeginDataReading(&rx_buffers[i*bufferSize], bufferSize);
        
    rx_lock.unlock();
    return 0;
}

int StreamerAPI::StartTx()
{
    tx_lock.lock();

    if (tx_running)
    {
        tx_lock.unlock();
        return 0;
    }

    
    int bufferSize = packetsToBatch*sizeof(lime::PacketLTE);
    
    if (tx_handles != nullptr)
        delete [] tx_handles;
    tx_handles = new int[streamConf.numTransfers];
    memset(tx_handles, 0, streamConf.numTransfers*sizeof(int));
    
    if (tx_buffers != nullptr)
        delete [] tx_buffers;
    tx_buffers = new char[streamConf.numTransfers*bufferSize];
    memset(tx_buffers,0,streamConf.numTransfers*bufferSize);
    
    if (tx_bufferUsed != nullptr)
        delete [] tx_bufferUsed;
    tx_bufferUsed = new bool[streamConf.numTransfers];
    memset(tx_bufferUsed,0,streamConf.numTransfers);
    tx_running = true;
    if (rx_running == false)
        Start();
    tx_lock.unlock();
	return 0;
}

int StreamerAPI::StopRx()
{
    rx_lock.lock();
    int bufferSize = packetsToBatch*sizeof(lime::PacketLTE);
    rx_running = false;
    streamPort->AbortReading();
    
    if (rx_handles != nullptr)
    for (int j = 0; j<streamConf.numTransfers; j++)
    {
        long bytesToRead = bufferSize;
        streamPort->WaitForReading(rx_handles[j], 30);
        streamPort->FinishDataReading(&rx_buffers[j*bufferSize], bytesToRead, rx_handles[j]); 
    }
    
    if (rx_handles != nullptr)
    {
        delete [] rx_handles;
        rx_handles = nullptr;
    }
    
    if (rx_buffers != nullptr)
    {
        delete [] rx_buffers;
        rx_buffers = nullptr;
    }
        
    if (tx_running == false)
        Stop();  
    rx_lock.unlock();
	return 0;
}

int StreamerAPI::StopTx()
{
    tx_lock.lock();
    int bufferSize = packetsToBatch*sizeof(lime::PacketLTE);
    tx_running = false;
    // Wait for all the queued requests to be cancelled
    streamPort->AbortSending();
    
    if (tx_bufferUsed != nullptr)
        for (int j = 0; j<streamConf.numTransfers; j++)
        {
            long bytesToSend = bufferSize;
            if (tx_bufferUsed[j])
            {
                streamPort->WaitForSending(tx_handles[j], 30);
                streamPort->FinishDataSending(&tx_buffers[j*bufferSize], bytesToSend, tx_handles[j]);
            }
        }
    
    if (tx_handles != nullptr)
    {
        delete [] tx_handles;
        tx_handles = nullptr;
    }
    
    if (tx_buffers != nullptr)
    {
        delete [] tx_buffers;
        tx_buffers = nullptr;
    }
    
    if (tx_bufferUsed != nullptr)
    {
        delete [] tx_bufferUsed; 
        tx_bufferUsed = nullptr;
    }
        
    if (rx_running == false)
        Stop(); 
    tx_lock.unlock();
	return 0;
}


int StreamerAPI::_write(int16_t *data, uint64_t timestamp, uint64_t meta, unsigned timeout)
{
    if (tx_running == false)
        return 0;

    long bufferSize = packetsToBatch*sizeof(lime::PacketLTE);
    const int buffersCountMask = streamConf.numTransfers - 1;
    static int bi = 0; //buffer index
    
    if (tx_bufferUsed[bi])
    {
        if (streamPort->WaitForSending(tx_handles[bi], timeout) == false)
        {
            return -1;
        }
        
        int bytesSent;
        if ((bytesSent = streamPort->FinishDataSending(&tx_buffers[bi*bufferSize], bufferSize, tx_handles[bi]))<bufferSize)
            return bytesSent;

        tx_bufferUsed[bi] = false;
    }
    
    int n = 0;
    for (int p = 0; p < packetsToBatch;p++)
    {
        uint32_t statusFlags = 0;

        lime::PacketLTE* pkt = (lime::PacketLTE*)&tx_buffers[bi*bufferSize];
        pkt[p].counter = timestamp;
        
        pkt[p].reserved[0] = (meta & 0xFF);
       
        for (int i = 0; i<sizeof(pkt[p].data);i+=3)
        {
                //I sample
                pkt[p].data[i] = data[n] & 0xFF;
                pkt[p].data[i + 1] = (data[n] >> 8) & 0x0F;
                n++;
                //Q sample
                pkt[p].data[i + 1] |= (data[n] << 4) & 0xF0;
                pkt[p].data[i + 2] = (data[n] >> 4) & 0xFF;
                n++;
        }
        timestamp += SAMPLES12_PACKET/channelsCount;
    }

    tx_handles[bi] = streamPort->BeginDataSending(&tx_buffers[bi*bufferSize], bufferSize);
    tx_bufferUsed[bi] = true;

    bi = (bi + 1) & buffersCountMask;   
    return packetsToBatch*SAMPLES12_PACKET;
}



StreamerFIFO::StreamerFIFO(lime::IConnection* dataPort):StreamerAPI(dataPort)
{
    mRxFIFO = new lime::LMS_SamplesFIFO(1, 1);
    mTxFIFO = new lime::LMS_SamplesFIFO(1, 1);
    stopRx = false;
    stopTx = false;
}

StreamerFIFO::~StreamerFIFO()
{
    delete mRxFIFO;
    delete mTxFIFO;
}



int StreamerFIFO::SetupStream(lms_stream_conf_t conf)
{
    streamConf = conf;
    if (streamConf.channels == 3)
        channelsCount = 2;
    else
        channelsCount = 1;
    return 0;
}

int StreamerFIFO::StartRx()
{
    if (rx_running == true)
        return 0;
    stopRx.store(false);
    StreamerLTE_ThreadData threadRxArgs;
    threadRxArgs.dataPort = streamPort;
    threadRxArgs.FIFO = mRxFIFO;
    threadRxArgs.terminate = &stopRx;
    threadRxArgs.dataRate_Bps = &mRxDataRate;
    if (threadRx.joinable())
        threadRx.join();
    mRxFIFO->Reset(2*4096, channelsCount);
    threadRx = std::thread(ReceivePackets, threadRxArgs); 
    rx_running = true;
    if (tx_running == false)
        Start(); 
}

int StreamerFIFO::StartTx()
{
    if (tx_running == true)
        return 0;
    stopTx.store(false);
    StreamerLTE_ThreadData threadTxArgs;
    threadTxArgs.dataPort = streamPort;
    threadTxArgs.FIFO = mTxFIFO;
    threadTxArgs.terminate = &stopTx;
    threadTxArgs.dataRate_Bps = &mTxDataRate;
    mTxFIFO->Reset(2*4096, channelsCount);
    threadTx = std::thread(TransmitPackets, threadTxArgs);
    tx_running = true;
    if (rx_running == false)
        Start(); 
}

int StreamerFIFO::StopRx()
{
    if (rx_running == false)
        return 0;
    stopRx.store(true);
    if (threadRx.joinable())
        threadRx.join();
    rx_running = false;
    if (tx_running == false)
        Stop();  
}

int StreamerFIFO::StopTx()
{
    if (tx_running == false)
        return 0;
    stopTx.store(true);
    if (threadTx.joinable())
     threadTx.join();
    tx_running = false;
    if (rx_running == false)
        Stop();  
}

int StreamerFIFO::RecvStream(void **samples,size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms)
{ 
    if (streamConf.dataFmt == lms_stream_conf_t::LMS_FMT_F32)
    {
       lime::complex16_t** buffers = new lime::complex16_t*[channelsCount];
       for (int i = 0; i<channelsCount;i++)
           buffers[i] = new lime::complex16_t[sample_count];
           
       int ret = mRxFIFO->pop_samples(buffers, sample_count, channelsCount, &meta->timestamp, timeout_ms);
       for (int i = 0; i < sample_count;i++)
       {
        for (int ch = 0;ch<channelsCount;ch++)
        {
            ((float*)samples[ch])[2*i] = (float)buffers[ch][i].i/2047.0;
            ((float*)samples[ch])[2*i+1] = (float)buffers[ch][i].q/2047.0;
        }
       }
       
       for (int i = 0; i<channelsCount;i++)
           delete [] buffers[i];
       delete [] buffers;
       
       return ret;
    }
    return mRxFIFO->pop_samples((lime::complex16_t**)samples, sample_count, channelsCount, &meta->timestamp, timeout_ms);   
}

int StreamerFIFO::SendStream(const void **samples,size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms)
{
    if (streamConf.dataFmt == lms_stream_conf_t::LMS_FMT_F32)
    {
       lime::complex16_t** buffers = new lime::complex16_t*[channelsCount];
       for (int i = 0; i<channelsCount;i++)
           buffers[i] = new lime::complex16_t[sample_count];
       
        int ret = mTxFIFO->push_samples((const lime::complex16_t**)buffers, sample_count, channelsCount, meta->timestamp, timeout_ms, 0);
        for (int i = 0; i < sample_count;i++)
        {
            for (int ch = 0;ch<channelsCount;ch++)
            {
                buffers[ch][i].i = ((float*)samples[ch])[2*i]*2047.0;
                buffers[ch][i].q = ((float*)samples[ch])[2*i+1]*2047.0;
            }
        }
        for (int i = 0; i<channelsCount;i++)
           delete [] buffers[i];
           
        delete [] buffers;
        return ret;
    }
    return mTxFIFO->push_samples((const lime::complex16_t**)samples, sample_count, channelsCount, meta->timestamp, timeout_ms, 0);
}

/** @brief Function dedicated for receiving data samples from board
    @param rxFIFO FIFO to store received data
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void StreamerFIFO::ReceivePackets(const StreamerLTE_ThreadData &args)
{
    auto dataPort = args.dataPort;
    auto rxFIFO = args.FIFO;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;

    //at this point Rx must be enabled in FPGA
    unsigned long rxDroppedSamples = 0;
    const int channelsCount = rxFIFO->GetChannelsCount();
    uint32_t samplesCollected = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();

    const int bufferSize = 65536;
    const int buffersCount = 16; // must be power of 2
    const int buffersCountMask = buffersCount - 1;
    int handles[buffersCount];
    memset(handles, 0, sizeof(int)*buffersCount);
    std::vector<char>buffers;
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
    lime::PacketFrame tempPacket;
    tempPacket.Initialize(channelsCount);

    for (int i = 0; i<buffersCount; ++i)
        handles[i] = dataPort->BeginDataReading(&buffers[i*bufferSize], bufferSize);

    int bi = 0;
    unsigned long totalBytesReceived = 0; //for data rate calculation
    int m_bufferFailures = 0;
    int16_t sample;

    uint32_t samplesReceived = 0;

    bool currentRxCmdValid = false;
    lime::RxCommand currentRxCmd;


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
            for (int pktIndex = 0; pktIndex < bytesReceived / sizeof(lime::PacketLTE); ++pktIndex)
            {
                lime::PacketLTE* pkt = (lime::PacketLTE*)&buffers[bi*bufferSize];
                tempPacket.first = 0;
                tempPacket.timestamp = pkt[pktIndex].counter;

                int statusFlags = 0;
                uint8_t* pktStart = (uint8_t*)pkt[pktIndex].data;
                const int stepSize = channelsCount * 3;
                size_t numPktBytes = sizeof(pkt->data);

                auto byte0 = pkt[pktIndex].reserved[0];

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

                uint32_t samplesPushed = rxFIFO->push_samples((const lime::complex16_t**)tempPacket.samples, samplesCollected, channelsCount, tempPacket.timestamp, 10, statusFlags);

                samplesCollected = 0;
            }
        }
        else
        {
            ++m_bufferFailures;
        }

        t2 = std::chrono::high_resolution_clock::now();
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

/** @brief Functions dedicated for transmitting packets to board
    @param txFIFO data source FIFO
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void StreamerFIFO::TransmitPackets(const StreamerLTE_ThreadData &args)
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
    std::vector<char> buffers;
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
    lime::complex16_t **outSamples = new lime::complex16_t*[channelsCount];
    const int samplesInPacket = lime::PacketFrame::maxSamplesInPacket / channelsCount;
    for (int i = 0; i < channelsCount; ++i)
    {
        outSamples[i] = new lime::complex16_t[samplesInPacket];
    }

    int m_bufferFailures = 0;
    long bytesSent = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
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
            lime::PacketLTE* pkt = (lime::PacketLTE*)&buffers[bi*bufferSize];
            pkt[i].counter = timestamp;
            if(statusFlags!=0)
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
        }

        bytesToSend[bi] = sizeof(lime::PacketLTE)*i;
        handles[bi] = dataPort->BeginDataSending(&buffers[bi*bufferSize], bytesToSend[bi]);
        bufferUsed[bi] = true;

        t2 = std::chrono::high_resolution_clock::now();
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




   
