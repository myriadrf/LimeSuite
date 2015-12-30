/**
@file	SoapyIConnection.h
@brief	Header for Soapy SDR + IConnection bindings.
@author Lime Microsystems (www.limemicro.com)
*/

#include <SoapySDR/Device.hpp>
#include <ConnectionRegistry.h>

class SoapyIConnection : public SoapySDR::Device
{
public:
    SoapyIConnection(const ConnectionHandle &handle);

    ~SoapyIConnection(void);

    /*******************************************************************
     * Identification API
     ******************************************************************/

    std::string getDriverKey(void) const;

    std::string getHardwareKey(void) const;

    SoapySDR::Kwargs getHardwareInfo(void) const;

private:
    IConnection *_conn;
    std::string _moduleName;
};
