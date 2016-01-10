/**
@file	Streaming.h
@brief	Soapy SDR + IConnection streaming interfaces.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyIConnection.h"
#include <IConnection.h>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Time.hpp>

/*******************************************************************
 * Stream data structure
 ******************************************************************/
struct IConnectionStream
{
    size_t streamID;
    int direction;
    std::atomic<double> *rate;
};

/*******************************************************************
 * Stream information
 ******************************************************************/
std::vector<std::string> SoapyIConnection::getStreamFormats(const int direction, const size_t channel) const
{
    std::vector<std::string> formats;
    formats.push_back(SOAPY_SDR_CF32);
    formats.push_back(SOAPY_SDR_CS16);
    return formats;
}

std::string SoapyIConnection::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
{
    fullScale = 1024;
    return SOAPY_SDR_CS16;
}

SoapySDR::ArgInfoList SoapyIConnection::getStreamArgsInfo(const int direction, const size_t channel) const
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
SoapySDR::Stream *SoapyIConnection::setupStream(
    const int direction,
    const std::string &format,
    const std::vector<size_t> &channels,
    const SoapySDR::Kwargs &args)
{
    StreamConfig config;
    config.isTx = (direction == SOAPY_SDR_TX);
    config.channels = channels;
    if (format == SOAPY_SDR_CF32) config.format = StreamConfig::STREAM_COMPLEX_FLOAT32;
    else if (format == SOAPY_SDR_CS16) config.format = StreamConfig::STREAM_12_BIT_IN_16;
    else throw std::runtime_error("SoapyIConnection::setupStream(format="+format+") unsupported format");

    //optional buffer length if specified
    if (args.count("bufferLength") != 0)
    {
        config.bufferLength = std::stoul(args.at("bufferLength"));
    }

    //optional link format if specified
    if (args.count("linkFormat") != 0)
    {
        const auto linkFormat = args.at("linkFormat");
        if (linkFormat == SOAPY_SDR_CS16) config.linkFormat = StreamConfig::STREAM_12_BIT_IN_16;
        else if (linkFormat == SOAPY_SDR_CS12) config.linkFormat = StreamConfig::STREAM_12_BIT_COMPRESSED;
        else throw std::runtime_error("SoapyIConnection::setupStream(linkFormat="+format+") unsupported format");
    }

    //create the stream
    size_t streamID(~0);
    const auto errorMsg = _conn->SetupStream(streamID, config);
    if (not errorMsg.empty()) throw std::runtime_error("SoapyIConnection::setupStream() failed: " + errorMsg);

    //store result into opaque stream object
    auto stream = new IConnectionStream;
    stream->streamID = streamID;
    stream->direction = direction;
    stream->rate = &_streamRates[direction][channels.front()];
    return (SoapySDR::Stream *)stream;
}

void SoapyIConnection::closeStream(SoapySDR::Stream *stream)
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;

    _conn->CloseStream(streamID);
}

size_t SoapyIConnection::getStreamMTU(SoapySDR::Stream *stream) const
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;
    return _conn->GetStreamSize(streamID);
}

int SoapyIConnection::activateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs,
    const size_t numElems)
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;

    StreamMetadata metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, *(icstream->rate));
    metadata.endOfBurst = (flags & SOAPY_SDR_END_BURST) != 0;
    auto ok = _conn->ControlStream(streamID, true, numElems, metadata);
    return ok?0:SOAPY_SDR_STREAM_ERROR;
}

int SoapyIConnection::deactivateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs)
{
    auto icstream = (IConnectionStream *)stream;
    auto streamID = icstream->streamID;

    StreamMetadata metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, *(icstream->rate));
    metadata.endOfBurst = (flags & SOAPY_SDR_END_BURST) != 0;
    auto ok = _conn->ControlStream(streamID, false, 0, metadata);
    return ok?0:SOAPY_SDR_STREAM_ERROR;
}

/*******************************************************************
 * Stream API
 ******************************************************************/
int SoapyIConnection::readStream(
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
    auto ret = _conn->ReadStream(streamID, buffs, numElems, timeoutUs/1000, metadata);

    //output metadata
    flags = 0;
    if (metadata.endOfBurst) flags |= SOAPY_SDR_END_BURST;
    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, *(icstream->rate));;

    //return num read or error code
    if (ret == 0) return SOAPY_SDR_TIMEOUT;
    return (ret > 0)? ret : SOAPY_SDR_STREAM_ERROR;
}

int SoapyIConnection::writeStream(
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
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, *(icstream->rate));
    metadata.endOfBurst = (flags & SOAPY_SDR_END_BURST) != 0;

    auto ret = _conn->WriteStream(streamID, buffs, numElems, timeoutUs/1000, metadata);

    //return num written or error code
    if (ret == 0) return SOAPY_SDR_TIMEOUT;
    return (ret > 0)? ret : SOAPY_SDR_STREAM_ERROR;
}

int SoapyIConnection::readStreamStatus(
    SoapySDR::Stream *stream,
    size_t &chanMask,
    int &flags,
    long long &timeNs,
    const long timeoutUs)
{
    return SOAPY_SDR_NOT_SUPPORTED;
}
