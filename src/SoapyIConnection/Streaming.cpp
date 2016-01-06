/**
@file	Streaming.h
@brief	Soapy SDR + IConnection streaming interfaces.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyIConnection.h"
#include <SoapySDR/Formats.hpp>

/*******************************************************************
 * Stream data structure
 ******************************************************************/
struct IConnectionStream
{
    int streamId;
    int direction;
    //format, etc...
    //TODO fill me in
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
    
}

void SoapyIConnection::closeStream(SoapySDR::Stream *stream)
{
    
}

size_t SoapyIConnection::getStreamMTU(SoapySDR::Stream *stream) const
{
    
}

int SoapyIConnection::activateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs,
    const size_t numElems)
{
    
}

int SoapyIConnection::deactivateStream(
    SoapySDR::Stream *stream,
    const int flags,
    const long long timeNs)
{
    
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
    
}

int SoapyIConnection::writeStream(
    SoapySDR::Stream *stream,
    const void * const *buffs,
    const size_t numElems,
    int &flags,
    const long long timeNs,
    const long timeoutUs)
{
    
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
