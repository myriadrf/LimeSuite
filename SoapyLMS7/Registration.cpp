/**
@file	Registration.cpp
@brief	Soapy SDR + IConnection module registration.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include <SoapySDR/Registry.hpp>

using namespace lime;

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

    return args;
}

static SoapySDR::KwargsList findIConnection(const SoapySDR::Kwargs &matchArgs)
{
    SoapySDR::KwargsList results;
    for (const auto &handle : ConnectionRegistry::findConnections(argsToHandle(matchArgs)))
    {
        results.push_back(handleToArgs(handle));
    }
    return results;
}

static SoapySDR::Device *makeIConnection(const SoapySDR::Kwargs &args)
{
    return new SoapyLMS7(argsToHandle(args), args);
}

static SoapySDR::Registry registerIConnection("lime", &findIConnection, &makeIConnection, SOAPY_SDR_ABI_VERSION);
