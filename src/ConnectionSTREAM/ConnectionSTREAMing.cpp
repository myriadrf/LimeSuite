/**
    @file ConnectionSTREAMing.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection (streaming API)
*/

#include "ConnectionSTREAM.h"
#include "LMS_StreamBoard.h"
#include "StreamerLTE.h"
#include "fifo.h" //from StreamerLTE
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>

using namespace lime;

#define STREAM_MTU 16384 //for now

/***********************************************************************
 * Custom StreamerLTE for streaming API hooks
 **********************************************************************/

struct StreamerLTECustom : StreamerLTE
{
    StreamerLTECustom(LMS64CProtocol *dataPort, const bool isTx, const size_t channelsCount, const StreamDataFormat /*format*/, const bool convertFloat):
        StreamerLTE(dataPort),
        isTx(isTx),
        channelsCount(channelsCount),
        //format(format), //TODO fix STREAM_12_BIT_IN_16
        format(STREAM_12_BIT_COMPRESSED),
        convertFloat(convertFloat),
        workThread(nullptr),
        mFIFO(nullptr)
    {
        std::cout << "StreamerLTECustom(isTx=" << isTx << ", channelsCount=" << channelsCount << ")" << std::endl;
        if (workThread != nullptr) return;

        mFIFO = new LMS_SamplesFIFO(2*4096, channelsCount);
        for (size_t i = 0; i < channelsCount; i++)
        {
            mFIFOBuffers.push_back(new complex16_t[STREAM_MTU]);
        }

        //switch off Rx
        uint16_t regVal = Reg_read(dataPort, 0x0005);
        Reg_write(dataPort, 0x0005, regVal & ~0x6);

        //enable MIMO mode, 12 bit compressed values
        if (channelsCount == 2)
        {
            Reg_write(dataPort, 0x0001, 0x0003);
            Reg_write(dataPort, 0x0007, 0x000A);
        }
        else
        {
            Reg_write(dataPort, 0x0001, 0x0001);
            Reg_write(dataPort, 0x0007, 0x0008);
        }

        //USB FIFO reset
        //TODO direction specific reset using isTx
        LMS64CProtocol::GenericPacket ctrPkt;
        ctrPkt.cmd = CMD_USB_FIFO_RST;
        ctrPkt.outBuffer.push_back(0x01);
        dataPort->TransferPacket(ctrPkt);
        ctrPkt.outBuffer[0] = 0x00;
        dataPort->TransferPacket(ctrPkt);

        stopThread = false;
        if (format == STREAM_12_BIT_COMPRESSED)
        {
            if (!isTx) workThread = new std::thread(ReceivePackets, dataPort, mFIFO, &stopThread, &rate_Bps);
            if (isTx) workThread = new std::thread(TransmitPackets, dataPort, mFIFO, &stopThread, &rate_Bps);
        }
        else
        {
            if (!isTx) workThread = new std::thread(ReceivePacketsUncompressed, dataPort, mFIFO, &stopThread, &rate_Bps);
            if (isTx) workThread = new std::thread(TransmitPacketsUncompressed, dataPort, mFIFO, &stopThread, &rate_Bps);
        }
    }

    ~StreamerLTECustom(void)
    {
        std::cout << "~StreamerLTECustom " << std::endl;
        if (workThread == nullptr) return;
        stopThread = true;

        workThread->join();
        delete workThread;
        workThread = nullptr;

        delete mFIFO;
        mFIFO = nullptr;

        for (size_t i = 0; i < channelsCount; i++)
        {
            delete [] mFIFOBuffers[i];
        }
        mFIFOBuffers.clear();
    }

    void Enable(LMS64CProtocol *dataPort, const bool enable)
    {
        std::cout << "StreamerLTECustom::Enable " << enable << std::endl;
        if (enable)
        {
            //switch on Rx
            auto regVal = Reg_read(dataPort, 0x0005);
            bool async = false;
            Reg_write(dataPort, 0x0005, (regVal & ~0x20) | 0x6 | (async << 4));
        }
        else
        {
            //stop Tx Rx if they were active
            auto regVal = Reg_read(dataPort, 0x0005);
            Reg_write(dataPort, 0x0005, regVal & ~0x6);
        }
    }

    const bool isTx;
    const size_t channelsCount;
    const StreamDataFormat format;
    const bool convertFloat;
    std::atomic<bool> stopThread;
    std::atomic<uint32_t> rate_Bps;
    std::thread *workThread;
    LMS_SamplesFIFO *mFIFO;
    std::vector<complex16_t *> mFIFOBuffers;
};

/***********************************************************************
 * Streaming API implementation
 **********************************************************************/

std::string ConnectionSTREAM::SetupStream(size_t &streamID, const StreamConfig &config)
{
    streamID = ~0;

    //API format check
    bool convertFloat = false;
    if (config.format == StreamConfig::STREAM_COMPLEX_FLOAT32) convertFloat = true;
    else if (config.format == StreamConfig::STREAM_12_BIT_IN_16) convertFloat = false;
    else return "ConnectionSTREAM::setupStream() only complex floats or int16";

    //check channel config
    //provide a default channel 0 if none specified
    std::vector<size_t> channels(config.channels);
    if (channels.empty()) channels.push_back(0);
    if (channels.size() > 2) return "SoapyIConnection::setupStream() 2 channels max";
    if (channels.front() > 1) return "SoapyIConnection::setupStream() channels must be [0, 1]";
    if (channels.back() > 1) return "SoapyIConnection::setupStream() channels must be [0, 1]";
    bool pos0isA = channels.front() == 0;
    bool pos1isA = channels.back() == 0;

    //determine sample positions based on channels
    auto s0 = pos1isA?LMS7002M::AI:LMS7002M::BI;
    auto s1 = pos1isA?LMS7002M::AQ:LMS7002M::BQ;
    auto s2 = pos0isA?LMS7002M::AI:LMS7002M::BI;
    auto s3 = pos0isA?LMS7002M::AQ:LMS7002M::BQ;
    if (channels.size() == 1) s0 = s3;

    //configure LML based on channel config
    LMS7002M rfic;
    rfic.SetConnection(this);
    if (config.isTx) rfic.ConfigureLML_BB2RF(s0, s1, s2, s3);
    else             rfic.ConfigureLML_RF2BB(s0, s1, s2, s3);

    //check link format
    auto linkFormat = StreamerLTE::STREAM_12_BIT_IN_16;
    switch (config.linkFormat)
    {
    case StreamConfig::STREAM_12_BIT_IN_16: linkFormat = StreamerLTE::STREAM_12_BIT_IN_16; break;
    case StreamConfig::STREAM_12_BIT_COMPRESSED: linkFormat = StreamerLTE::STREAM_12_BIT_COMPRESSED; break;
    default: return "SoapyIConnection::setupStream() unsupported link format";
    }

    streamID = size_t(new StreamerLTECustom(this, config.isTx, channels.size(), linkFormat, convertFloat));
    return ""; //success
}

void ConnectionSTREAM::CloseStream(const size_t streamID)
{
    auto *stream = (LMS_StreamBoard *)streamID;
    delete stream;
}

size_t ConnectionSTREAM::GetStreamSize(const size_t streamID)
{
    return STREAM_MTU;
}

bool ConnectionSTREAM::ControlStream(const size_t streamID, const bool enable, const size_t burstSize, const StreamMetadata &metadata)
{
    auto *stream = (StreamerLTECustom *)streamID;
    stream->Enable(this, enable);
    return true;
}

int ConnectionSTREAM::ReadStream(const size_t streamID, void * const *buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata)
{
    auto *stream = (StreamerLTECustom *)streamID;

    size_t samplesCount = std::min<size_t>(length, STREAM_MTU);

    complex16_t **popBuffer(nullptr);
    if (stream->convertFloat) popBuffer = (complex16_t **)stream->mFIFOBuffers.data();
    else popBuffer = (complex16_t **)buffs;

    size_t sampsPopped = stream->mFIFO->pop_samples(
        popBuffer,
        samplesCount,
        stream->channelsCount,
        &metadata.timestamp,
        timeout_ms);
    samplesCount = (std::min)(samplesCount, sampsPopped);

    if (stream->convertFloat)
    {
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto buffIn = stream->mFIFOBuffers[i];
            auto buffOut = (std::complex<float> *)buffs[i];
            for (size_t j = 0; j < samplesCount; j++)
            {
                buffOut[j] = std::complex<float>(
                    float(buffIn[j].i)/2048,
                    float(buffIn[j].q)/2048);
            }
        }
    }

    return samplesCount;
}

int ConnectionSTREAM::WriteStream(const size_t streamID, const void * const *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata)
{
    auto *stream = (StreamerLTECustom *)streamID;

    size_t samplesCount = std::min<size_t>(length, STREAM_MTU);

    const complex16_t **pushBuffer(nullptr);
    if (stream->convertFloat)
    {
        pushBuffer = (const complex16_t **)stream->mFIFOBuffers.data();
        for (size_t i = 0; i < stream->channelsCount; i++)
        {
            auto buffIn = (const std::complex<float> *)buffs[i];
            auto bufOut = stream->mFIFOBuffers[i];
            for (size_t j = 0; j < samplesCount; j++)
            {
                bufOut[j].i = int16_t(buffIn[j].real()*2048);
                bufOut[j].q = int16_t(buffIn[j].imag()*2048);
            }
        }
    }
    else pushBuffer = (const complex16_t **)buffs;

    size_t sampsPushed = stream->mFIFO->push_samples(
        pushBuffer,
        samplesCount,
        stream->channelsCount,
        metadata.timestamp,
        timeout_ms);
    samplesCount = (std::min)(samplesCount, sampsPushed);

    return samplesCount;
}

void ConnectionSTREAM::UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate)
{
    //std::cout << "LMS_StreamBoard::ConfigurePLL(tx=" << txRate/1e6 << "MHz, rx=" << rxRate/1e6  << "MHz)" << std::endl;
    LMS_StreamBoard::ConfigurePLL(this, txRate/1e6, rxRate/1e6, 90);
}
