/**
@file	Streaming.cpp
@brief	Soapy SDR + IConnection streaming interfaces.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Time.hpp>

#include <algorithm>
#include <iostream>
#include <ratio>
#include <thread>

#include "limesuite/Logger.h"

using namespace lime;

/*******************************************************************
 * Stream data structure
 ******************************************************************/
struct IConnectionStream {
    SDRDevice* ownerDevice;
    int direction;
    size_t elemSize;
    size_t elemMTU;
    bool skipCal;

    // Rx command requests
    bool hasCmd;
    int flags;
    long long timeNs;
    size_t numElems;

    SDRDevice::StreamConfig streamConfig;
};

/*******************************************************************
 * Stream information
 ******************************************************************/
std::vector<std::string> SoapyLMS7::getStreamFormats(const int direction, const size_t channel) const
{
    return { SOAPY_SDR_CF32, SOAPY_SDR_CS12, SOAPY_SDR_CS16 };
}

std::string SoapyLMS7::getNativeStreamFormat(const int direction, const size_t channel, double& fullScale) const
{
    fullScale = 32767;
    return SOAPY_SDR_CS16;
}

SoapySDR::ArgInfoList SoapyLMS7::getStreamArgsInfo(const int direction, const size_t channel) const
{
    SoapySDR::ArgInfoList argInfos;

    // Buffer length
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

    // Latency
    {
        SoapySDR::ArgInfo info;
        info.value = "0.5";
        info.key = "latency";
        info.name = "Latency";
        info.description = "Latency vs. performance";
        info.type = SoapySDR::ArgInfo::FLOAT;
        argInfos.push_back(info);
    }

    // Link format
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

    // Skip calibrations
    {
        SoapySDR::ArgInfo info;
        info.value = "false";
        info.key = "skipCal";
        info.name = "Skip Calibration";
        info.description = "Skip automatic activation calibration.";
        info.type = SoapySDR::ArgInfo::BOOL;
        argInfos.push_back(info);
    }

    // Align phase of Rx channels
    {
        SoapySDR::ArgInfo info;
        info.value = "false";
        info.key = "alignPhase";
        info.name = "Align phase";
        info.description = "Attempt to align phase of Rx channels.";
        info.type = SoapySDR::ArgInfo::BOOL;
        argInfos.push_back(info);
    }

    return argInfos;
}

/*******************************************************************
 * Stream config
 ******************************************************************/
SoapySDR::Stream* SoapyLMS7::setupStream(
    const int direction, const std::string& format, const std::vector<size_t>& channels, const SoapySDR::Kwargs& args)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    // Store result into opaque stream object
    auto stream = new IConnectionStream;
    stream->direction = direction;
    stream->elemSize = SoapySDR::formatToSize(format);
    stream->hasCmd = false;
    stream->skipCal = args.count("skipCal") != 0 and args.at("skipCal") == "true";

    SDRDevice::StreamConfig& config = streamConfig;
    config.alignPhase = args.count("alignPhase") != 0 and args.at("alignPhase") == "true";
    // config.performanceLatency = 0.5;
    config.bufferSize = 0; // Auto

    TRXDir dir = (direction == SOAPY_SDR_TX) ? TRXDir::Tx : TRXDir ::Rx;

    // Default to channel 0, if none were specified
    const std::vector<size_t>& channelIDs = channels.empty() ? std::vector<size_t>{ 0 } : channels;
    config.channels[dir] = std::vector<uint8_t>(channelIDs.begin(), channelIDs.end());

    if (format == SOAPY_SDR_CF32)
    {
        config.format = SDRDevice::StreamConfig::DataFormat::F32;
    }
    else if (format == SOAPY_SDR_CS16)
    {
        config.format = SDRDevice::StreamConfig::DataFormat::I16;
    }
    else if (format == SOAPY_SDR_CS12)
    {
        config.format = SDRDevice::StreamConfig::DataFormat::I12;
    }
    else
    {
        throw std::runtime_error("SoapyLMS7::setupStream(format=" + format + ") unsupported stream format");
    }

    config.linkFormat =
        config.format == SDRDevice::StreamConfig::DataFormat::F32 ? SDRDevice::StreamConfig::DataFormat::I16 : config.format;

    // Optional link format
    if (args.count("linkFormat"))
    {
        auto linkFormat = args.at("linkFormat");
        if (linkFormat == SOAPY_SDR_CS16)
        {
            config.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
        }
        else if (linkFormat == SOAPY_SDR_CS12)
        {
            config.linkFormat = SDRDevice::StreamConfig::DataFormat::I12;
        }
        else
        {
            throw std::runtime_error("SoapyLMS7::setupStream(linkFormat=" + linkFormat + ") unsupported link format");
        }
    }

    // TODO: reimplement if relevant
    // Optional buffer length if specified (from device args)

    // const auto devArgsBufferLength = _deviceArgs.find(config.isTx ? "txBufferLength" : "rxBufferLength");
    // if (devArgsBufferLength != _deviceArgs.end())
    // {
    //     config.bufferLength = std::stoul(devArgsBufferLength->second);
    // }

    // TODO: reimplement if relevant
    // Optional buffer length if specified (takes precedent)

    // if (args.count("bufferLength") != 0)
    // {
    //     config.bufferLength = std::stoul(args.at("bufferLength"));
    // }

    // TODO: reimplement if relevant
    // Optional packets latency, 1-maximum throughput, 0-lowest latency

    // if (args.count("latency") != 0)
    // {
    //     config.performanceLatency = std::stof(args.at("latency"));
    //     if (config.performanceLatency < 0)
    //     {
    //         config.performanceLatency = 0;
    //     }
    //     else if (config.performanceLatency > 1)
    //     {
    //         config.performanceLatency = 1;
    //     }
    // }

    // Create the stream
    auto returnValue = sdrDevice->StreamSetup(config, 0);
    if (returnValue != OpStatus::SUCCESS)
    {
        throw std::runtime_error("SoapyLMS7::setupStream() failed: " + std::string(GetLastErrorMessage()));
    }

    auto samplesPerPacket = config.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360;
    // TODO: figure out a way to actually get the correct packet per batch count
    stream->elemMTU = 8 / config.channels.at(dir).size() * samplesPerPacket;

    stream->ownerDevice = sdrDevice;

    // Calibrate these channels when activated
    // for (const auto& ch : channelIDs)
    // {
    // sdrDevice->Calibrate(0, dir, ch, sdrDevice->GetFrequency(0, dir, ch));
    // lastSavedConfiguration.channel[ch].GetDirection(dir).calibrate = true;
    // _channelsToCal.emplace(direction, ch);
    // }

    stream->streamConfig = config;

    return reinterpret_cast<SoapySDR::Stream*>(stream);
}

void SoapyLMS7::closeStream(SoapySDR::Stream* stream)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto icstream = reinterpret_cast<IConnectionStream*>(stream);

    const auto& ownerDevice = icstream->ownerDevice;
    ownerDevice->StreamStop(0);
    delete icstream;
}

size_t SoapyLMS7::getStreamMTU(SoapySDR::Stream* stream) const
{
    auto icstream = reinterpret_cast<IConnectionStream*>(stream);
    return icstream->elemMTU;
}

int SoapyLMS7::activateStream(SoapySDR::Stream* stream, const int flags, const long long timeNs, const size_t numElems)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto icstream = reinterpret_cast<IConnectionStream*>(stream);
    const auto& ownerDevice = icstream->ownerDevice;
    if (sampleRate[SOAPY_SDR_TX] == 0.0 && sampleRate[SOAPY_SDR_RX] == 0.0)
    {
        throw std::runtime_error("SoapyLMS7::activateStream() - the sample rate has not been configured!");
    }

    if (sampleRate[SOAPY_SDR_RX] <= 0.0)
    {
        sampleRate[SOAPY_SDR_RX] = sdrDevice->GetSampleRate(0, TRXDir::Rx, 0);
    }

    // Perform self calibration with current bandwidth settings.
    // This is for the set-it-and-forget-it style of use case
    // where boards are configured, the stream is setup,
    // and the configuration is maintained throughout the run.

    // TODO: Double check
    // while (not _channelsToCal.empty() and not icstream->skipCal)
    // {
    //     bool dir = _channelsToCal.begin()->first;
    //     auto ch = _channelsToCal.begin()->second;
    //     auto bw = mChannels[dir].at(ch).rf_bw > 0 ? mChannels[dir].at(ch).rf_bw : sampleRate[dir];
    //     bw = bw > 2.5e6 ? bw : 2.5e6;
    //     sdrDevice->Calibrate(dir == SOAPY_SDR_TX ? TRXDir::Tx : TRXDir::Rx, ch, bw, 0);
    //     settingsCache.at(direction).at(ch).calibrationBandwidth = bw;
    //     _channelsToCal.erase(_channelsToCal.begin());
    // }
    // Stream requests used with rx
    icstream->flags = flags;
    icstream->timeNs = timeNs;
    icstream->numElems = numElems;
    icstream->hasCmd = true;

    ownerDevice->StreamStart(0);
    activeStreams.insert(stream);
    return 0;
}

int SoapyLMS7::deactivateStream(SoapySDR::Stream* stream, const int flags, const long long timeNs)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto icstream = reinterpret_cast<IConnectionStream*>(stream);
    const auto& ownerDevice = icstream->ownerDevice;
    icstream->hasCmd = false;

    ownerDevice->StreamStop(0);

    activeStreams.erase(stream);
    return 0;
}

/*******************************************************************
 * Stream API
 ******************************************************************/
int SoapyLMS7::readStream(
    SoapySDR::Stream* stream, void* const* buffs, size_t numElems, int& flags, long long& timeNs, const long timeoutUs)
{
    auto icstream = reinterpret_cast<IConnectionStream*>(stream);

    const auto exitTime = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(timeoutUs);

    // Wait for a command from activate stream up to the timeout specified
    if (not icstream->hasCmd)
    {
        while (std::chrono::high_resolution_clock::now() < exitTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return SOAPY_SDR_TIMEOUT;
    }

    // Handle the one packet flag by clipping
    if ((flags & SOAPY_SDR_ONE_PACKET) != 0)
    {
        numElems = std::min(numElems, icstream->elemMTU);
    }

    SDRDevice::StreamMeta metadata;
    const uint64_t cmdTicks =
        ((icstream->flags & SOAPY_SDR_HAS_TIME) != 0) ? SoapySDR::timeNsToTicks(icstream->timeNs, sampleRate[SOAPY_SDR_RX]) : 0;

    int status = 0;
    switch (icstream->streamConfig.format)
    {
    case SDRDevice::StreamConfig::DataFormat::I16:
    case SDRDevice::StreamConfig::DataFormat::I12:
        status = icstream->ownerDevice->StreamRx(0, reinterpret_cast<complex16_t* const*>(buffs), numElems, &metadata);
        break;
    case SDRDevice::StreamConfig::DataFormat::F32:
        status = icstream->ownerDevice->StreamRx(0, reinterpret_cast<complex32f_t* const*>(buffs), numElems, &metadata);
        break;
    }

    flags = 0;

    if (status == 0)
    {
        return SOAPY_SDR_TIMEOUT;
    }
    if (status < 0)
    {
        return SOAPY_SDR_STREAM_ERROR;
    }

    const uint64_t expectedTime(cmdTicks + status);

    if (metadata.timestamp < expectedTime)
    {
        SoapySDR::log(SOAPY_SDR_ERROR, "readStream() experienced non-monotonic timestamp");
        return SOAPY_SDR_CORRUPTION;
    }

    // The command had a time, so we need to compare it to received time
    if ((icstream->flags & SOAPY_SDR_HAS_TIME) != 0 and metadata.waitForTimestamp)
    {
        // Our request time is now late, clear command and return error code
        if (cmdTicks < metadata.timestamp)
        {
            icstream->hasCmd = false;
            return SOAPY_SDR_TIME_ERROR;
        }

        // _readStreamAligned should guarantee this condition
        if (cmdTicks != metadata.timestamp)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR,
                "readStream() alignment algorithm failed\n"
                "Request time = %lld, actual time = %lld",
                cmdTicks,
                metadata.timestamp);
            return SOAPY_SDR_STREAM_ERROR;
        }

        icstream->flags &= ~SOAPY_SDR_HAS_TIME; // Clear for next read
    }

    // Handle finite burst request commands
    if (icstream->numElems != 0)
    {
        // Clip to within the request size when over,
        // and reduce the number of elements requested.
        status = std::min<std::size_t>(status, icstream->numElems);
        icstream->numElems -= status;

        // The burst completed, done with the command
        if (icstream->numElems == 0)
        {
            icstream->hasCmd = false;
            metadata.flushPartialPacket = true;
        }
    }

    // Output metadata
    if (metadata.flushPartialPacket)
    {
        flags |= SOAPY_SDR_END_BURST;
    }

    if (metadata.waitForTimestamp)
    {
        flags |= SOAPY_SDR_HAS_TIME;
    }

    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, sampleRate[SOAPY_SDR_RX]);

    // Return num read or error code
    return (status >= 0) ? status : SOAPY_SDR_STREAM_ERROR;
}

int SoapyLMS7::writeStream(SoapySDR::Stream* stream,
    const void* const* buffs,
    const size_t numElems,
    int& flags,
    const long long timeNs,
    const long timeoutUs)
{
    if ((flags & SOAPY_SDR_HAS_TIME) && (timeNs <= 0))
    {
        return SOAPY_SDR_TIME_ERROR;
    }

    auto icstream = reinterpret_cast<IConnectionStream*>(stream);
    const auto& ownerDevice = icstream->ownerDevice;

    // Input metadata
    SDRDevice::StreamMeta metadata;
    metadata.timestamp = SoapySDR::timeNsToTicks(timeNs, sampleRate[SOAPY_SDR_RX]);
    metadata.waitForTimestamp = (flags & SOAPY_SDR_HAS_TIME);
    metadata.flushPartialPacket = (flags & SOAPY_SDR_END_BURST);

    int status = 0;
    switch (icstream->streamConfig.format)
    {
    case SDRDevice::StreamConfig::DataFormat::I16:
    case SDRDevice::StreamConfig::DataFormat::I12:
        status = ownerDevice->StreamTx(0, reinterpret_cast<const complex16_t* const*>(buffs), numElems, &metadata);
        break;
    case SDRDevice::StreamConfig::DataFormat::F32:
        status = ownerDevice->StreamTx(0, reinterpret_cast<const complex32f_t* const*>(buffs), numElems, &metadata);
        break;
    }

    if (status == 0)
    {
        return SOAPY_SDR_TIMEOUT;
    }
    if (status < 0)
    {
        return SOAPY_SDR_STREAM_ERROR;
    }

    // Return num written
    return status;
}

int SoapyLMS7::readStreamStatus(SoapySDR::Stream* stream, size_t& chanMask, int& flags, long long& timeNs, const long timeoutUs)
{
    auto icstream = reinterpret_cast<IConnectionStream*>(stream);
    const auto& ownerDevice = icstream->ownerDevice;

    int ret = 0;
    flags = 0;
    SDRDevice::StreamStats metadata;
    auto start = std::chrono::high_resolution_clock::now();
    while (1)
    {
        if (icstream->direction == SOAPY_SDR_TX)
        {
            ownerDevice->StreamStatus(0, nullptr, &metadata);
        }
        else
        {
            ownerDevice->StreamStatus(0, &metadata, nullptr);
        }

        if (metadata.loss)
        {
            ret = SOAPY_SDR_TIME_ERROR;
        }
        else if (metadata.overrun)
        {
            ret = SOAPY_SDR_OVERFLOW;
        }
        else if (metadata.underrun)
        {
            ret = SOAPY_SDR_UNDERFLOW;
        }

        if (ret)
        {
            break;
        }
        // Check timeout
        std::chrono::duration<double, std::micro> microseconds = std::chrono::high_resolution_clock::now() - start;
        if (microseconds.count() >= timeoutUs)
        {
            return SOAPY_SDR_TIMEOUT;
        }
        // Sleep to avoid high CPU load
        if (timeoutUs >= 1000000)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
        }
    }

    timeNs = SoapySDR::ticksToTimeNs(metadata.timestamp, sampleRate[SOAPY_SDR_RX]);
    // Output metadata
    flags |= SOAPY_SDR_HAS_TIME;
    return ret;
}
