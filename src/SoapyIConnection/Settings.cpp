/**
@file	Settings.h
@brief	Soapy SDR + IConnection config settings.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyIConnection.h"
#include <IConnection.h>

/*******************************************************************
 * Constructor/destructor
 ******************************************************************/
SoapyIConnection::SoapyIConnection(const ConnectionHandle &handle):
    _conn(nullptr)
{
    _moduleName = handle.module;
    _conn = ConnectionRegistry::makeConnection(handle);
}

SoapyIConnection::~SoapyIConnection(void)
{
    ConnectionRegistry::freeConnection(_conn);
}

/*******************************************************************
 * Identification API
 ******************************************************************/
std::string SoapyIConnection::getDriverKey(void) const
{
    return _moduleName;
}

std::string SoapyIConnection::getHardwareKey(void) const
{
    return _conn->GetDeviceInfo().deviceName;
}

SoapySDR::Kwargs SoapyIConnection::getHardwareInfo(void) const
{
    auto devinfo = _conn->GetDeviceInfo();
    SoapySDR::Kwargs info;
    info["expansionName"] = devinfo.expansionName;
    info["firmwareVersion"] = devinfo.firmwareVersion;
    info["hardwareVersion"] = devinfo.hardwareVersion;
    info["protocolVersion"] = devinfo.protocolVersion;
    return info;
}
