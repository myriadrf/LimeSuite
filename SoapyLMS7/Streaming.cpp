/**
@file	Streaming.cpp
@brief	Soapy SDR + IConnection streaming interfaces.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include "LMS7002M.h"
#include <IConnection.h>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Time.hpp>
#include <thread>
#include <algorithm> //min/max
#include "ErrorReporting.h"

using namespace lime;

/*******************************************************************
 * Stream data structure
 ******************************************************************/
struct IConnectionStream
{
    std::vector<size_t> streamID;
    int direction;
    size_t elemSize;
    size_t elemMTU;
    bool enabled;

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
    fullScale = 2048;
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

    StreamConfig config;
    config.isTx = (direction == SOAPY_SDR_TX);
    config.performanceLatency = 0.5;

    //default to channel 0, if none were specified
    const std::vector<size_t> &channelIDs = channels.empty() ? std::vector<size_t>{0} : channels;

    for(size_t i=0; i<channelIDs.size(); ++i)
    {
        config.channelID = channelIDs[i];
        if (format == SOAPY_SDR_CF32) config.format = StreamConfig::STREAM_COMPLEX_FLOAT32;
        else if (format == SOAPY_SDR_CS16) config.format = StreamConfig::STREAM_12_BIT_IN_16;
        else if (format == SOAPY_SDR_CS12) config.format = StreamConfig::STREAM_12_BIT_COMPRESSED;
        else throw std::runtime_error("SoapyLMS7::setupStream(format="+format+") unsupported format");

        //optional buffer length if specified
        if (args.count("bufferLength") != 0)
        {
            config.bufferLength = std::stoul(args.at("bufferLength"));
        }
        //optional packets latency, 0-maximum throughput, 1-lowest latency
        if (args.count("latency") != 0)
        {
            config.performanceLatency = std::stof(args.at("latency"));
            if(config.performanceLatency<0)
                config.performanceLatency = 0;
            else if(config.performanceLatency > 1)
                config.performanceLatency = 1;
        }

        //create the stream
        size_t streamID(~0);
        const int status = _conn->SetupStream(streamID, config);
        if (status != 0)
            throw std::runtime_error("SoapyLMS7::setupStream() failed: " + std::string(GetLastErrorMessage()));
        stream->streamID.push_back(streamID);
        stream->elemMTU = _conn->GetStreamSize(streamID);
        stream->enabled = false;
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
    if (icstream->enabled)
    {
        for(auto i : streamID)
            _conn->ControlStream(i, false);
    }

    for(auto i : streamID)
        _conn->CloseStream(i);
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

    if (_conn->GetHardwareTimestampRate() == 0.0)
        throw std::runtime_error("SoapyLMS7::activateStream() - the sample rate has not been configured!");

    //perform self calibration with current bandwidth settings
    //this is for the set-it-and-forget-it style of use case
    //where boards are configured, the stream is setup,
    //and the configuration is maintained throughout the run
    while (not _channelsToCal.empty())
    {
        auto dir  = _channelsToCal.begin()->first;
        auto ch  = _channelsToCal.begin()->second;
        if (dir == SOAPY_SDR_RX) getRFIC(ch)->CalibrateRx(_actualBw.at(dir).at(ch));
        if (dir == SOAPY_SDR_TX) getRFIC(ch)->CalibrateTx(_actualBw.at(dir).at(ch));
        _channelsToCal.erase(_channelsToCal.begin());
    }

    //stream requests used with rx
    icstream->flags = flags;
    icstream->timeNs = timeNs;
    icstream->numElems = numElems;
    icstream->hasCmd = true;

    if (not icstream->enabled)
    {
        for(auto i : streamID)
        {
            int status = _conn->ControlStream(i, true);
            if(status != 0) return SOAPY_SDR_STREAM_ERROR;
        }
        icstream->enabled = true;
    }

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

    StreamMetadata metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, _conn->GetHardwareTimestampRate());
    metadata.hasTimestamp = (flags & SOAPY_SDR_HAS_TIME) != 0;
    metadata.endOfBurst = (flags & SOAPY_SDR_END_BURST) != 0;

    if (icstream->enabled)
    {
        for(auto i : streamID)
        {
            int status = _conn->ControlStream(i, false);
            if(status != 0) return SOAPY_SDR_STREAM_ERROR;
        }
        icstream->enabled = false;
    }

    return 0;
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
    const auto &streamID = icstream->streamID;

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

    ReadStreamAgain:
    StreamMetadata metadata;
    int status = 0;
    int bufIndex = 0;
    for(auto i : streamID)
    {
        status = _conn->ReadStream(i, buffs[bufIndex++], numElems, timeoutUs/1000, metadata);
        if(status == 0) return SOAPY_SDR_TIMEOUT;
        if(status < 0) return SOAPY_SDR_STREAM_ERROR;
    }

    //the command had a time, so we need to compare it to received time
    if ((icstream->flags & SOAPY_SDR_HAS_TIME) != 0 and metadata.hasTimestamp)
    {
        const uint64_t cmdTicks = SoapySDR::timeNsToTicks(icstream->timeNs, _conn->GetHardwareTimestampRate());

        //our request time is now late, clear command and return error code
        if (cmdTicks < metadata.timestamp)
        {
            icstream->hasCmd = false;
            return SOAPY_SDR_TIME_ERROR;
        }

        //our request time is not in this received buffer, try again
        if (cmdTicks >= (metadata.timestamp + status))
        {
            if (std::chrono::high_resolution_clock::now() > exitTime) return SOAPY_SDR_TIMEOUT;
            goto ReadStreamAgain;
        }

        //otherwise our request is in this buffer, advance memory
        const size_t numOff = (cmdTicks - metadata.timestamp);
        metadata.timestamp += numOff;
        status -= numOff;
        const size_t elemSize = icstream->elemSize;
        for (size_t i = 0; i < streamID.size(); i++)
        {
            const size_t memStart = size_t(buffs[i])+(numOff*elemSize);
            std::memmove(buffs[i], (const void *)memStart, status*elemSize);
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
            metadata.endOfBurst = true;
        }
    }

    //output metadata
    flags = 0;
    if (metadata.endOfBurst) flags |= SOAPY_SDR_END_BURST;
    if (metadata.hasTimestamp) flags |= SOAPY_SDR_HAS_TIME;
    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, _conn->GetHardwareTimestampRate());

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
    auto icstream = (IConnectionStream *)stream;
    const auto &streamID = icstream->streamID;

    //input metadata
    StreamMetadata metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, _conn->GetHardwareTimestampRate());
    metadata.hasTimestamp = (flags & SOAPY_SDR_HAS_TIME) != 0;
    metadata.endOfBurst = (flags & SOAPY_SDR_END_BURST) != 0;

    int ret = 0;
    int bufIndex = 0;
    for(auto i : streamID)
    {
        ret = _conn->WriteStream(i, buffs[bufIndex++], numElems, timeoutUs/1000, metadata);
        if(ret == 0) return SOAPY_SDR_TIMEOUT;
        if(ret < 0) return SOAPY_SDR_STREAM_ERROR;
    }

    //return num written or error code
    return (ret > 0)? ret : SOAPY_SDR_STREAM_ERROR;
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

    StreamMetadata metadata;
    flags = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while (1)
    {
        for(auto i : streamID)
        {
            int ret = _conn->ReadStreamStatus(i, timeoutUs/1000, metadata);
            if (ret != 0)
            {
                //handle the default not implemented case and return not supported
                if (GetLastError() == EPERM) return SOAPY_SDR_NOT_SUPPORTED;
                return SOAPY_SDR_TIMEOUT;
            }

            //stop when event is detected
            if (metadata.endOfBurst || metadata.lateTimestamp || metadata.packetDropped)
                goto found;
        }
        //check timeout
        std::chrono::duration<double> seconds = std::chrono::high_resolution_clock::now()-start;
        if (seconds.count()> (double)timeoutUs/1e6)
            return SOAPY_SDR_TIMEOUT;
        //sleep to avoid high CPU load
        if (timeoutUs >= 2000)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        else
            std::this_thread::sleep_for(std::chrono::microseconds(1+timeoutUs/2));
    }

    found:
    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, _conn->GetHardwareTimestampRate());
    //output metadata
    if (metadata.endOfBurst) flags |= SOAPY_SDR_END_BURST;
    if (metadata.hasTimestamp) flags |= SOAPY_SDR_HAS_TIME;

    if (metadata.lateTimestamp) return SOAPY_SDR_TIME_ERROR;
    if (metadata.packetDropped) return SOAPY_SDR_OVERFLOW;

    return 0;
}
