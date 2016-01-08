/**
@file	Streaming.h
@brief	Soapy SDR + IConnection streaming interfaces.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyIConnection.h"
#include <SoapySDR/Formats.hpp>
#include "LMS_StreamBoard.h"
#include <LMS7002M.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

/*******************************************************************
 * Stream data structure
 ******************************************************************/
struct IConnectionStream
{
    LMS7002M *rfic;
    //int streamId;
    int direction;
    //format, etc...
    LMS_StreamBoard *streamBoard;
};

#define STREAM_MTU 16384 //for now

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
    //TODO this may not be the case for every advice (exampled packed complex 12 bit)
    fullScale = 1024;
    return SOAPY_SDR_CS16;
}

SoapySDR::ArgInfoList SoapyIConnection::getStreamArgsInfo(const int direction, const size_t channel) const
{
    SoapySDR::ArgInfoList argInfos;
    //TODO nothing yet
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
    if (direction == SOAPY_SDR_TX) throw std::runtime_error("SoapyIConnection::setupStream() no TX yet");
    if (format != SOAPY_SDR_CF32) throw std::runtime_error("SoapyIConnection::setupStream() only complex floats now");
    if (channels.size() != 2 or channels.at(0) != 0 or channels.at(1) != 1) throw std::runtime_error("SoapyIConnection::setupStream() only channels 0, 1 supported");

    auto icstream = new IConnectionStream();
    icstream->rfic = _rfics.at(channels.at(0)/2);
    icstream->direction = direction;
    icstream->streamBoard = new LMS_StreamBoard(_conn);
    return (SoapySDR::Stream *)icstream;
}

void SoapyIConnection::closeStream(SoapySDR::Stream *stream)
{
    auto icstream = (IConnectionStream *)stream;
    delete icstream->streamBoard;
    delete icstream;
}

size_t SoapyIConnection::getStreamMTU(SoapySDR::Stream *stream) const
{
    return STREAM_MTU;
}

int SoapyIConnection::activateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs,
    const size_t numElems)
{
    auto icstream = (IConnectionStream *)stream;
    if (icstream->direction == SOAPY_SDR_RX)
    {
        auto st = LMS_StreamBoard::ConfigurePLL(_conn, icstream->rfic->GetReferenceClk_TSP_MHz(LMS7002M::Tx), icstream->rfic->GetReferenceClk_TSP_MHz(LMS7002M::Rx), 90);
        st = icstream->streamBoard->StartReceiving(STREAM_MTU);
        if (st != LMS_StreamBoard::SUCCESS)
        {
            return SOAPY_SDR_STREAM_ERROR;
        }
    }
    return 0;
}

int SoapyIConnection::deactivateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs)
{
    auto icstream = (IConnectionStream *)stream;
    if (icstream->direction == SOAPY_SDR_RX)
    {
        auto st = icstream->streamBoard->StopReceiving();
        if (st != LMS_StreamBoard::SUCCESS)
        {
            return SOAPY_SDR_STREAM_ERROR;
        }
    }
    return 0;
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

    const auto exitTime(std::chrono::high_resolution_clock::now() + std::chrono::microseconds(timeoutUs));
    do
    {
        auto data = icstream->streamBoard->GetIncomingData();
        if (data.samplesI.size() != 0)
        {
            //TODO
            // * both buffers
            // * remainder when numElems is smaller
            size_t N = std::min(data.samplesI.size(), numElems);
            auto ch0_data = (std::complex<float> *)buffs[0];
            for (size_t i = 0; i < N; i++)
            {
                ch0_data[i] = std::complex<float>(data.samplesI[i], data.samplesQ[i]);
            }
            return N;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    } while (std::chrono::high_resolution_clock::now() < exitTime);

    return SOAPY_SDR_TIMEOUT;
}

int SoapyIConnection::writeStream(
    SoapySDR::Stream *stream,
    const void * const *buffs,
    const size_t numElems,
    int &flags,
    const long long timeNs,
    const long timeoutUs)
{
    return SOAPY_SDR_NOT_SUPPORTED;
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
