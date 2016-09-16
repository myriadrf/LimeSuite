/**
@file	Streaming.cpp
@brief	Soapy SDR + IConnection streaming interfaces.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include <IConnection.h>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Time.hpp>
#include "ErrorReporting.h"

using namespace lime;

/*******************************************************************
 * Stream data structure
 ******************************************************************/
struct IConnectionStream
{
    std::vector<size_t> streamID;
    int direction;
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

    StreamConfig config;
    config.isTx = (direction == SOAPY_SDR_TX);
    for(size_t i=0; i<channels.size(); ++i)
    {
        config.channelID = channels[i];
        if (format == SOAPY_SDR_CF32) config.format = StreamConfig::STREAM_COMPLEX_FLOAT32;
        else if (format == SOAPY_SDR_CS16) config.format = StreamConfig::STREAM_12_BIT_IN_16;
        else if (format == SOAPY_SDR_CS12) config.format = StreamConfig::STREAM_12_BIT_COMPRESSED;
        else throw std::runtime_error("SoapyLMS7::setupStream(format="+format+") unsupported format");

        //optional buffer length if specified
        if (args.count("bufferLength") != 0)
        {
            config.bufferLength = std::stoul(args.at("bufferLength"));
        }

        //create the stream
        size_t streamID(~0);
        const int status = _conn->SetupStream(streamID, config);
        if (status != 0)
            throw std::runtime_error("SoapyLMS7::setupStream() failed: " + std::string(GetLastErrorMessage()));
        stream->streamID.push_back(streamID);
    }
    return (SoapySDR::Stream *)stream;
}

void SoapyLMS7::closeStream(SoapySDR::Stream *stream)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;

    for(auto i : streamID)
        _conn->CloseStream(i);
}

size_t SoapyLMS7::getStreamMTU(SoapySDR::Stream *stream) const
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;
    for(auto i : streamID)
    {
        return _conn->GetStreamSize(i);
    }
    return 0;
}

int SoapyLMS7::activateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs,
    const size_t numElems)
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;

    if (_conn->GetHardwareTimestampRate() == 0.0)
        throw std::runtime_error("SoapyLMS7::activateStream() - the sample rate has not been configured!");

    StreamMetadata metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, _conn->GetHardwareTimestampRate());
    metadata.hasTimestamp = (flags & SOAPY_SDR_HAS_TIME) != 0;
    metadata.endOfBurst = (flags & SOAPY_SDR_END_BURST) != 0;
    for(auto i : streamID)
    {
        int status = _conn->ControlStream(i, true);
        if(status != 0)
            return SOAPY_SDR_STREAM_ERROR;
    }
    return 0;
}

int SoapyLMS7::deactivateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs)
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;

    StreamMetadata metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, _conn->GetHardwareTimestampRate());
    metadata.hasTimestamp = (flags & SOAPY_SDR_HAS_TIME) != 0;
    metadata.endOfBurst = (flags & SOAPY_SDR_END_BURST) != 0;
    for(auto i : streamID)
    {
        int status = _conn->ControlStream(i, false);
        if(status != 0)
            return SOAPY_SDR_STREAM_ERROR;
    }
    return 0;
}

/*******************************************************************
 * Stream API
 ******************************************************************/
int SoapyLMS7::readStream(
    SoapySDR::Stream *stream,
    void * const *buffs,
    const size_t numElems,
    int &flags,
    long long &timeNs,
    const long timeoutUs)
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;

    StreamMetadata metadata;
    int status = 0;
    int bufIndex = 0;
    for(auto i : streamID)
    {
        status = _conn->ReadStream(i, buffs[bufIndex++], numElems, timeoutUs/1000, metadata);
        if(status == 0) return SOAPY_SDR_TIMEOUT;
        if(status < 0) return SOAPY_SDR_STREAM_ERROR;
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
    auto streamID = icstream->streamID;

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
    auto streamID = icstream->streamID;

    StreamMetadata metadata;
    for(auto i : streamID)
    {
        int ret = _conn->ReadStreamStatus(i, timeoutUs/1000, metadata);
        if (ret != 0)
            return SOAPY_SDR_TIMEOUT;
    }

    //output metadata
    flags = 0;
    if (metadata.endOfBurst) flags |= SOAPY_SDR_END_BURST;
    if (metadata.hasTimestamp) flags |= SOAPY_SDR_HAS_TIME;
    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, _conn->GetHardwareTimestampRate());

    if (metadata.lateTimestamp) return SOAPY_SDR_TIME_ERROR;
    if (metadata.packetDropped) return SOAPY_SDR_OVERFLOW;
    return 0;
}
