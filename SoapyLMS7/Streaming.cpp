/**
@file	Streaming.cpp
@brief	Soapy SDR + IConnection streaming interfaces.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include "lms7_device.h"
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Time.hpp>
#include <thread>
#include <iostream>
#include <algorithm> //min/max
#include "Logger.h"
#include "Streamer.h"

using namespace lime;

/*******************************************************************
 * Stream data structure
 ******************************************************************/
struct IConnectionStream
{
    std::vector<StreamChannel*> streamID;
    int direction;
    size_t elemSize;
    size_t elemMTU;
    bool skipCal;

    //rx cmd requests
    bool hasCmd;
    int flags;
    long long timeNs;
    size_t numElems;
};

/*******************************************************************
 * Stream information
 ******************************************************************/
std::vector<std::string> SoapyLMS7::getStreamFormats(const int direction, const size_t channel) const
{
    std::vector<std::string> formats;
    formats.push_back(SOAPY_SDR_CF32);
    formats.push_back(SOAPY_SDR_CS12);
    formats.push_back(SOAPY_SDR_CS16);
    return formats;
}

std::string SoapyLMS7::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
{
    fullScale = 32767;
    return SOAPY_SDR_CS16;
}

SoapySDR::ArgInfoList SoapyLMS7::getStreamArgsInfo(const int direction, const size_t channel) const
{
    SoapySDR::ArgInfoList argInfos;

    //buffer length
    {
        SoapySDR::ArgInfo info;
        info.value = "0";
        info.key = "bufferLength";
        info.name = "Buffer Length";
        info.description = "The buffer transfer size over the link.";
        info.units = "samples";
        info.type = SoapySDR::ArgInfo::INT;
        argInfos.push_back(info);
    }

    //link format
    {
        SoapySDR::ArgInfo info;
        info.value = SOAPY_SDR_CS16;
        info.key = "linkFormat";
        info.name = "Link Format";
        info.description = "The format of the samples over the link.";
        info.type = SoapySDR::ArgInfo::STRING;
        info.options.push_back(SOAPY_SDR_CS16);
        info.options.push_back(SOAPY_SDR_CS12);
        info.optionNames.push_back("Complex int16");
        info.optionNames.push_back("Complex int12");
        argInfos.push_back(info);
    }

    //skip calibrations
    {
        SoapySDR::ArgInfo info;
        info.value = "false";
        info.key = "skipCal";
        info.name = "Skip Calibration";
        info.description = "Skip automatic activation calibration.";
        info.type = SoapySDR::ArgInfo::BOOL;
        argInfos.push_back(info);
    }

    //align phase of Rx channels
    {
        SoapySDR::ArgInfo info;
        info.value = "false";
        info.key = "alignPhase";
        info.name = "align phase";
        info.description = "Attempt to align phase of Rx channels.";
        info.type = SoapySDR::ArgInfo::BOOL;
        argInfos.push_back(info);
    }

    return argInfos;
}

/*******************************************************************
 * Stream config
 ******************************************************************/
SoapySDR::Stream *SoapyLMS7::setupStream(
    const int direction,
    const std::string &format,
    const std::vector<size_t> &channels,
    const SoapySDR::Kwargs &args)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    //store result into opaque stream object
    auto stream = new IConnectionStream;
    stream->direction = direction;
    stream->elemSize = SoapySDR::formatToSize(format);
    stream->hasCmd = false;
    stream->skipCal = args.count("skipCal") != 0 and args.at("skipCal") == "true";

    StreamConfig config;
    config.align = args.count("alignPhase") != 0 and args.at("alignPhase") == "true";
    config.isTx = (direction == SOAPY_SDR_TX);
    config.performanceLatency = 0.5;
    config.bufferLength = 0; //auto

    //default to channel 0, if none were specified
    const std::vector<size_t> &channelIDs = channels.empty() ? std::vector<size_t>{0} : channels;
    for(size_t i=0; i<channelIDs.size(); ++i)
    {
        config.channelID = channelIDs[i];
        if (format == SOAPY_SDR_CF32) config.format = StreamConfig::FMT_FLOAT32;
        else if (format == SOAPY_SDR_CS16) config.format = StreamConfig::FMT_INT16;
        else if (format == SOAPY_SDR_CS12) config.format = StreamConfig::FMT_INT12;
        else throw std::runtime_error("SoapyLMS7::setupStream(format="+format+") unsupported format");

        //optional buffer length if specified (from device args)
        const auto devArgsBufferLength = _deviceArgs.find(config.isTx?"txBufferLength":"rxBufferLength");
        if (devArgsBufferLength != _deviceArgs.end())
        {
            config.bufferLength = std::stoul(devArgsBufferLength->second);
        }

        //optional buffer length if specified (takes precedent)
        if (args.count("bufferLength") != 0)
        {
            config.bufferLength = std::stoul(args.at("bufferLength"));
        }

        //optional packets latency, 1-maximum throughput, 0-lowest latency
        if (args.count("latency") != 0)
        {
            config.performanceLatency = std::stof(args.at("latency"));
            if(config.performanceLatency<0)
                config.performanceLatency = 0;
            else if(config.performanceLatency > 1)
                config.performanceLatency = 1;
        }

        //create the stream
        StreamChannel* streamID = lms7Device->SetupStream(config);
        if (streamID == 0)
            throw std::runtime_error("SoapyLMS7::setupStream() failed: " + std::string(GetLastErrorMessage()));
        stream->streamID.push_back(streamID);
        stream->elemMTU = streamID->GetStreamSize();
    }

    //calibrate these channels when activated
    for (const auto &ch : channelIDs)
    {
        _channelsToCal.emplace(direction, ch);
    }
    return (SoapySDR::Stream *)stream;
}

void SoapyLMS7::closeStream(SoapySDR::Stream *stream)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto icstream = (IConnectionStream *)stream;
    const auto &streamID = icstream->streamID;

    //disable stream if left enabled
    for(auto i : streamID)
        i->Stop();

    for(auto i : streamID)
        lms7Device->DestroyStream(i);
}

size_t SoapyLMS7::getStreamMTU(SoapySDR::Stream *stream) const
{
    auto icstream = (IConnectionStream *)stream;
    return icstream->elemMTU;
}

int SoapyLMS7::activateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs,
    const size_t numElems)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto icstream = (IConnectionStream *)stream;
    const auto &streamID = icstream->streamID;
    if (sampleRate[SOAPY_SDR_TX] == 0.0 && sampleRate[SOAPY_SDR_RX] == 0.0)
        throw std::runtime_error("SoapyLMS7::activateStream() - the sample rate has not been configured!");
    if (sampleRate[SOAPY_SDR_RX] <= 0.0)
        sampleRate[SOAPY_SDR_RX] = lms7Device->GetRate(LMS_CH_RX, 0);
    //perform self calibration with current bandwidth settings
    //this is for the set-it-and-forget-it style of use case
    //where boards are configured, the stream is setup,
    //and the configuration is maintained throughout the run
    while (not _channelsToCal.empty() and not icstream->skipCal)
    {
        bool dir  = _channelsToCal.begin()->first;
        auto ch  = _channelsToCal.begin()->second;
        auto bw = mChannels[dir].at(ch).rf_bw > 0 ? mChannels[dir].at(ch).rf_bw : sampleRate[dir];
        lms7Device->Calibrate(dir== SOAPY_SDR_TX, ch, bw>2.5e6 ? bw : 2.5e6, 0);
        _channelsToCal.erase(_channelsToCal.begin());
    }
    //stream requests used with rx
    icstream->flags = flags;
    icstream->timeNs = timeNs;
    icstream->numElems = numElems;
    icstream->hasCmd = true;

    for(auto i : streamID)
    {
        int status = i->Start();
        if(status != 0) return SOAPY_SDR_STREAM_ERROR;
    }
    activeStreams.insert(stream);
    return 0;
}

int SoapyLMS7::deactivateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto icstream = (IConnectionStream *)stream;
    const auto &streamID = icstream->streamID;
    icstream->hasCmd = false;

    for(auto i : streamID)
    {
        int status = i->Stop();
        if(status != 0) return SOAPY_SDR_STREAM_ERROR;
    }
    activeStreams.erase(stream);
    return 0;
}

/*******************************************************************
 * Stream alignment helper for multiple channels
 ******************************************************************/
static inline void fastForward(
    char *buff, size_t &numWritten, const size_t elemSize,
    const uint64_t oldHeadTime, const uint64_t desiredHeadTime)
{
    const size_t numPop = std::min<size_t>(desiredHeadTime - oldHeadTime, numWritten);
    const size_t numMove = (numWritten-numPop);
    numWritten -= numPop;
    std::memmove(buff, buff+(numPop*elemSize), numMove*elemSize);
}

int SoapyLMS7::_readStreamAligned(
    IConnectionStream *stream,
    char * const *buffs,
    size_t numElems,
    uint64_t requestTime,
    StreamChannel::Metadata &md,
    const long timeoutMs)
{
    const auto &streamID = stream->streamID;
    const size_t elemSize = stream->elemSize;
    std::vector<size_t> numWritten(streamID.size(), 0);

    for (size_t i = 0; i < streamID.size(); i += (numWritten[i]>=numElems)?1:0)
    {
        size_t &N = numWritten[i];
        const uint64_t expectedTime(requestTime + N);
        if (numElems <= N)
            continue;
        int status = streamID[i]->Read(buffs[i]+(elemSize*N), numElems-N,&md, timeoutMs);
        if (status == 0) return SOAPY_SDR_TIMEOUT;
        if (status < 0) return SOAPY_SDR_STREAM_ERROR;

        //update accounting
        const size_t elemsRead = size_t(status);
        const size_t prevN = N;
        N += elemsRead; //num written total

        //unspecified request time, set the new head condition
        if (requestTime == 0) goto updateHead;

        //good contiguous read, read again for remainder
        if (expectedTime == md.timestamp) continue;

        //request time is later, fast forward buffer
        if (md.timestamp < expectedTime)
        {
            if (prevN != 0)
            {
                SoapySDR::log(SOAPY_SDR_ERROR, "readStream() experienced non-monotonic timestamp");
                return SOAPY_SDR_CORRUPTION;
            }
            fastForward(buffs[i], N, elemSize, md.timestamp, requestTime);
            if (i == 0 and N != 0) numElems = N; //match size on other channels
            continue; //read again into the remaining buffer
        }

        //overflow in the middle of a contiguous buffer
        //fast-forward all prior channels and restart loop
        if (md.timestamp > expectedTime)
        {
            for (size_t j = 0; j < i; j++)
            {
                fastForward(buffs[j], numWritten[j], elemSize, requestTime, md.timestamp);
            }
            fastForward(buffs[i], N, elemSize, md.timestamp-prevN, md.timestamp);
            i = 0; //start over at ch0
        }

        updateHead:
        requestTime = md.timestamp;
        numElems = elemsRead;
    }

    md.timestamp = requestTime;
    return int(numElems);
}

/*******************************************************************
 * Stream API
 ******************************************************************/
int SoapyLMS7::readStream(
    SoapySDR::Stream *stream,
    void * const *buffs,
    size_t numElems,
    int &flags,
    long long &timeNs,
    const long timeoutUs)
{
    auto icstream = (IConnectionStream *)stream;

    const auto exitTime = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(timeoutUs);

    //wait for a command from activate stream up to the timeout specified
    if (not icstream->hasCmd)
    {
        while (std::chrono::high_resolution_clock::now() < exitTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return SOAPY_SDR_TIMEOUT;
    }

    //handle the one packet flag by clipping
    if ((flags & SOAPY_SDR_ONE_PACKET) != 0)
    {
        numElems = std::min(numElems, icstream->elemMTU);
    }

    StreamChannel::Metadata metadata;
    const uint64_t cmdTicks = ((icstream->flags & SOAPY_SDR_HAS_TIME) != 0)?SoapySDR::timeNsToTicks(icstream->timeNs, sampleRate[SOAPY_SDR_RX]):0;
    int status = _readStreamAligned(icstream, (char * const *)buffs, numElems, cmdTicks, metadata, timeoutUs/1000);
    if (status < 0) return status;

    //the command had a time, so we need to compare it to received time
    if ((icstream->flags & SOAPY_SDR_HAS_TIME) != 0 and (metadata.flags & RingFIFO::SYNC_TIMESTAMP) != 0)
    {
        //our request time is now late, clear command and return error code
        if (cmdTicks < metadata.timestamp)
        {
            icstream->hasCmd = false;
            return SOAPY_SDR_TIME_ERROR;
        }

        //_readStreamAligned should guarantee this condition
        if (cmdTicks != metadata.timestamp)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR,
                "readStream() alignment algorithm failed\n"
                "Request time = %lld, actual time = %lld",
                (long long)cmdTicks, (long long)metadata.timestamp);
            return SOAPY_SDR_STREAM_ERROR;
        }

        icstream->flags &= ~SOAPY_SDR_HAS_TIME; //clear for next read
    }

    //handle finite burst request commands
    if (icstream->numElems != 0)
    {
        //Clip to within the request size when over,
        //and reduce the number of elements requested.
        status = std::min<size_t>(status, icstream->numElems);
        icstream->numElems -= status;

        //the burst completed, done with the command
        if (icstream->numElems == 0)
        {
            icstream->hasCmd = false;
            metadata.flags |= RingFIFO::END_BURST;
        }
    }

    //output metadata
    flags = 0;
    if ((metadata.flags & RingFIFO::END_BURST) != 0) flags |= SOAPY_SDR_END_BURST;
    if ((metadata.flags & RingFIFO::SYNC_TIMESTAMP) != 0) flags |= SOAPY_SDR_HAS_TIME;
    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, sampleRate[SOAPY_SDR_RX]);

    //return num read or error code
    return (status >= 0) ? status : SOAPY_SDR_STREAM_ERROR;
}

int SoapyLMS7::writeStream(
    SoapySDR::Stream *stream,
    const void * const *buffs,
    const size_t numElems,
    int &flags,
    const long long timeNs,
    const long timeoutUs)
{
    if ((flags & SOAPY_SDR_HAS_TIME) && (timeNs <= 0))
        return SOAPY_SDR_TIME_ERROR;
    auto icstream = (IConnectionStream *)stream;
    const auto &streamID = icstream->streamID;

    //input metadata
    StreamChannel::Metadata metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, sampleRate[SOAPY_SDR_RX]);
    metadata.flags = (flags & SOAPY_SDR_HAS_TIME) ? lime::RingFIFO::SYNC_TIMESTAMP : 0;
    metadata.flags |= (flags & SOAPY_SDR_END_BURST) ? lime::RingFIFO::END_BURST : 0;

    //write the 0th channel: get number of samples written
    int status = streamID[0]->Write(buffs[0], numElems, &metadata, timeoutUs/1000);
    if (status == 0) return SOAPY_SDR_TIMEOUT;
    if (status < 0) return SOAPY_SDR_STREAM_ERROR;

    //write subsequent channels with the same size and large timeout
    //we should always be able to do a matching buffer write quickly
    //or there is an unknown internal issue with the stream fifo
    for (size_t i = 1; i < streamID.size(); i++)
    {
        int status_i = streamID[i]->Write(buffs[i], status, &metadata, 1000/*1s*/);
        if (status_i != status)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "Multi-channel stream alignment failed!");
            return SOAPY_SDR_CORRUPTION;
        }
    }

    //return num written
    return status;
}

int SoapyLMS7::readStreamStatus(
    SoapySDR::Stream *stream,
    size_t &chanMask,
    int &flags,
    long long &timeNs,
    const long timeoutUs)
{
    auto icstream = (IConnectionStream *)stream;
    const auto &streamID = icstream->streamID;

    int ret = 0;
    flags = 0;
    lime::StreamChannel::Info metadata;
    auto start = std::chrono::high_resolution_clock::now();
    while (1)
    {
        for(auto i : streamID)
        {
            metadata = i->GetInfo();

            if (metadata.droppedPackets) ret = SOAPY_SDR_TIME_ERROR;
            else if (metadata.overrun) ret = SOAPY_SDR_OVERFLOW;
            else if (metadata.underrun) ret = SOAPY_SDR_UNDERFLOW;
        }
        if (ret) break;
        //check timeout
        std::chrono::duration<double> seconds = std::chrono::high_resolution_clock::now()-start;
        if (seconds.count()> (double)timeoutUs/1e6)
            return SOAPY_SDR_TIMEOUT;
        //sleep to avoid high CPU load
        if (timeoutUs >= 1000000)
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        else
            std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
    }

    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, sampleRate[SOAPY_SDR_RX]);
    //output metadata
    flags |= SOAPY_SDR_HAS_TIME;
    return ret;
}
