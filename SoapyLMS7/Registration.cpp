/**
@file	Registration.cpp
@brief	Soapy SDR + IConnection module registration.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Logger.hpp>
#include "Logger.h"

using namespace lime;

static void limeSuiteLogHandler(const lime::LogLevel level, const char *message)
{
    switch(level)
    {
    case lime::LOG_LEVEL_CRITICAL: SoapySDR::log(SOAPY_SDR_CRITICAL, message); return;
    case lime::LOG_LEVEL_ERROR: SoapySDR::log(SOAPY_SDR_ERROR, message); return;
    case lime::LOG_LEVEL_WARNING: SoapySDR::log(SOAPY_SDR_WARNING, message); return;
    case lime::LOG_LEVEL_INFO: SoapySDR::log(SOAPY_SDR_INFO, message); return;
    case lime::LOG_LEVEL_DEBUG: SoapySDR::log(SOAPY_SDR_DEBUG, message); return;
    }
}

static ConnectionHandle argsToHandle(const SoapySDR::Kwargs &args)
{
    ConnectionHandle handle;

    //load handle with key/value provided options
    if (args.count("module") != 0) handle.module = args.at("module");
    if (args.count("media") != 0) handle.media = args.at("media");
    if (args.count("name") != 0) handle.name = args.at("name");
    if (args.count("addr") != 0) handle.addr = args.at("addr");
    if (args.count("serial") != 0) handle.serial = args.at("serial");
    if (args.count("index") != 0) handle.index = std::stoi(args.at("index"));

    return handle;
}

static SoapySDR::Kwargs handleToArgs(const ConnectionHandle &handle)
{
    SoapySDR::Kwargs args;

    //convert the handle into key/value pairs
    if (not handle.module.empty()) args["module"] = handle.module;
    if (not handle.media.empty()) args["media"] = handle.media;
    if (not handle.name.empty()) args["name"] = handle.name;
    if (not handle.addr.empty()) args["addr"] = handle.addr;
    if (not handle.serial.empty()) args["serial"] = handle.serial;
    if (handle.index != -1) args["index"] = std::to_string(handle.index);

    //label connection for drop-downs and lists
    args["label"] = handle.ToString();

    return args;
}

static SoapySDR::KwargsList findIConnection(const SoapySDR::Kwargs &matchArgs)
{
    lime::registerLogHandler(&limeSuiteLogHandler);
    SoapySDR::KwargsList results;
    for (const auto &handle : ConnectionRegistry::findConnections(argsToHandle(matchArgs)))
    {
        results.push_back(handleToArgs(handle));
    }
    return results;
}

static SoapySDR::Device *makeIConnection(const SoapySDR::Kwargs &args)
{
    lime::registerLogHandler(&limeSuiteLogHandler);
    return new SoapyLMS7(argsToHandle(args), args);
}

static SoapySDR::Registry registerIConnection("lime", &findIConnection, &makeIConnection, SOAPY_SDR_ABI_VERSION);
