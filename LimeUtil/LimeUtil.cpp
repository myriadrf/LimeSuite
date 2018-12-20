/**
    @file LimeUtil.cpp
    @author Lime Microsystems
    @brief Command line test app
*/

#include <VersionInfo.h>
#include <SystemResources.h>
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <iostream>
#include <cstdlib>
#include <ciso646>
#include <getopt.h>
#include <fstream>
#include "Logger.h"
#include "LMS64CProtocol.h"
#include "lms7_device.h"
#include "device_constants.h"

using namespace lime;

int deviceTestTiming(const std::string &argStr);
int deviceCalSweep(
    const std::string &argStr,
    const double start,
    const double stop,
    const double step,
    const double bw,
    const std::string &dir,
    const std::string &chans);

/***********************************************************************
 * print help
 **********************************************************************/
static int printHelp(void)
{
    std::cout << "Usage LimeUtil [options]" << std::endl;
    std::cout << "  Options summary:" << std::endl;
    std::cout << "    --help \t\t\t\t Print this help message" << std::endl;
    std::cout << "    --info \t\t\t\t Print module information" << std::endl;
    std::cout << "    --find[=\"module=foo,serial=bar\"] \t Discover available devices" << std::endl;
    std::cout << "    --make[=\"module=foo,serial=bar\"] \t Create a device instance" << std::endl;
    std::cout << "    --force \t\t\t\t Force operaton" << std::endl;
    std::cout << std::endl;
    std::cout << "  Advanced options:" << std::endl;
    std::cout << "    --args[=\"module=foo,serial=bar\"] \t Arguments for the options below" << std::endl;
    std::cout << "    --update          \t\t\t Automatic firmware sync + flash" << std::endl;
    std::cout << "    --fpga=\"filename\" \t\t\t Program FPGA gateware to flash" << std::endl;
    std::cout << "    --fw=\"filename\"   \t\t\t Program FX3  firmware to flash" << std::endl;
    std::cout << "    --timing          \t\t\t Time interfaces and operations" << std::endl;
    std::cout << std::endl;
    std::cout << "  Calibrations sweep:" << std::endl;
    std::cout << "    --cal[=\"module=foo,serial=bar\"]  \t Calibrate device, optional device args..." << std::endl;
    std::cout << "    --start[=freqStart]                \t Frequency start for the sweep(Hz)" << std::endl;
    std::cout << "    --stop[=freqStop]                  \t Frequency stop for the sweep(Hz)" << std::endl;
    std::cout << "    --step[=freqStep, default=1MHz]    \t Frequency step for the sweep(Hz)" << std::endl;
    std::cout << "    --bw[=bandwidth, default=30MHz]    \t Desired calibration bandwidth(Hz)" << std::endl;
    std::cout << "    --dir[=direction, default=BOTH]    \t Calibration direction, RX, TX, BOTH" << std::endl;
    std::cout << "    --chans[=channels, default=ALL]    \t Calibration channels, 0, 1, ALL" << std::endl;
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

/***********************************************************************
 * print info
 **********************************************************************/
static int printInfo(void)
{
    std::cout << "######################################################" << std::endl;
    std::cout << "## LimeSuite information summary" << std::endl;
    std::cout << "######################################################" << std::endl;
    std::cout << std::endl;

    std::cout << "Version information:" << std::endl;
    std::cout << "  Library version:\tv" << lime::GetLibraryVersion() << std::endl;
    std::cout << "  Build timestamp:\t" << lime::GetBuildTimestamp() << std::endl;
    std::cout << "  Interface version:\tv" << lime::GetAPIVersion() << std::endl;
    std::cout << "  Binary interface:\t" << lime::GetABIVersion() << std::endl;
    std::cout << std::endl;

    std::cout << "System resources:" << std::endl;
    std::cout << "  Installation root:\t" << lime::getLimeSuiteRoot() << std::endl;
    std::cout << "  User home directory:\t" << lime::getHomeDirectory() << std::endl;
    std::cout << "  App data directory:\t" << lime::getAppDataDirectory() << std::endl;
    std::cout << "  Config directory:\t" << lime::getConfigDirectory() << std::endl;
    std::cout << "  Image search paths:" << std::endl;
    for (const auto &name : lime::listImageSearchPaths()) std::cout << "     - " << name << std::endl;
    std::cout << std::endl;

    std::cout << "Supported connections:" << std::endl;
    for (const auto &name : ConnectionRegistry::moduleNames()) std::cout << "   * " << name << std::endl;
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

/***********************************************************************
 * find devices
 **********************************************************************/
static int findDevices(void)
{
    std::string argStr;
    if (optarg != NULL) argStr = optarg;
    ConnectionHandle hint(argStr);

    auto handles = ConnectionRegistry::findConnections(hint);
    for (const auto &handle : handles)
    {
        std::cout << "  * [" << handle.serialize() << "]" << std::endl;
    }

    std::cout << std::endl;
    return EXIT_SUCCESS;
}

/***********************************************************************
 * make a device
 **********************************************************************/
static int makeDevice(void)
{
    std::string argStr;
    if (optarg != NULL) argStr = optarg;
    ConnectionHandle handle(argStr);

    std::cout << "Make device " << argStr << std::endl;
    auto conn = ConnectionRegistry::makeConnection(handle);
    if (conn == nullptr)
    {
        std::cout << "No available device!" << std::endl;
        return EXIT_FAILURE;
    }
    if (not conn->IsOpen())
    {
        std::cout << "Connection not open!" << std::endl;
        ConnectionRegistry::freeConnection(conn);
        return EXIT_FAILURE;
    }

    auto info = conn->GetDeviceInfo();
    std::cout << "  Device name: " << info.deviceName << std::endl;
    std::cout << "  Expansion name: " << info.expansionName << std::endl;
    std::cout << "  Firmware version: " << info.firmwareVersion << std::endl;
    std::cout << "  Hardware version: " << info.hardwareVersion << std::endl;
    std::cout << "  Protocol version: " << info.protocolVersion << std::endl;
    std::cout << "  Gateware version: " << info.gatewareVersion << std::endl;
    std::cout << "  Gateware revision: " << info.gatewareRevision << std::endl;
    std::cout << "  Gateware target: " << info.gatewareTargetBoard << std::endl;
    std::cout << "  Serial number: " << std::hex << "0x" << info.boardSerialNumber << std::dec << std::endl;

    std::cout << "  Free connection... " << std::flush;
    ConnectionRegistry::freeConnection(conn);
    std::cout << "OK" << std::endl;
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

/***********************************************************************
 * Program update (sync images and flash support)
 **********************************************************************/
static int programUpdate(const bool force, const std::string &argStr)
{
    auto handles = ConnectionRegistry::findConnections(argStr);
    if(handles.size() == 0)
    {
        std::cout << "No devices found" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Connected to [" << handles[0].ToString() << "]" << std::endl;
    auto conn = ConnectionRegistry::makeConnection(handles[0]);

    auto progCallback = [](int bsent, int btotal, const char* progressMsg)
    {
        printf("[%3i%%] %5i/%5i Bytes %s\r", int(100.0*bsent/btotal+0.5), bsent, btotal, progressMsg);
        fflush(stdout);
        return 0;
    };

    auto status = conn->ProgramUpdate(true/*yes download*/, force, progCallback);

    std::cout << std::endl;
    if(status == 0)
    {
        std::cout << "Programming update complete!" << std::endl;
    }
    else
    {
        std::cout << "Programming update failed! : " << GetLastErrorMessage() << std::endl;
    }

    ConnectionRegistry::freeConnection(conn);
    return (status==0)?EXIT_SUCCESS:EXIT_FAILURE;
}

/***********************************************************************
 * Program gateware
 **********************************************************************/
static int programGateware(const std::string &argStr)
{
    //load file
    std::ifstream file;
    file.open(optarg, std::ios::in | std::ios::binary);
    if(not file.good())
    {
        std::cout << "File not found: " << optarg << std::endl;
        return EXIT_FAILURE;
    }

    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> progData(fileSize, 0);
    file.read(progData.data(), fileSize);

    auto handles = ConnectionRegistry::findConnections(argStr);
    if(handles.size() == 0)
    {
        std::cout << "No devices found" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Connected to [" << handles[0].ToString() << "]" << std::endl;
    auto device = LMS7_Device::CreateDevice(handles[0]);

    auto progCallback = [](int bsent, int btotal, const char* progressMsg)
    {
        printf("[%3i%%] %5i/%5i Bytes %s\r", int(100.0*bsent/btotal+0.5), bsent, btotal, progressMsg);
        fflush(stdout);
        return 0;
    };

    auto status = device->Program(program_mode::fpgaFlash, progData.data(), progData.size(), progCallback);
    std::cout << std::endl;
    if(status != 0)
        std::cout << "Programming failed! : " << GetLastErrorMessage() << std::endl;
    delete device;
    return (status==0)?EXIT_SUCCESS:EXIT_FAILURE;
}

/***********************************************************************
 * Program gateware
 **********************************************************************/
static int programFirmware(const std::string &argStr)
{
    //load file
    std::ifstream file;
    file.open(optarg, std::ios::in | std::ios::binary);
    if(not file.good())
    {
        std::cout << "File not found: " << optarg << std::endl;
        return EXIT_FAILURE;
    }

    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> progData(fileSize, 0);
    file.read(progData.data(), fileSize);

    auto handles = ConnectionRegistry::findConnections(argStr);
    if(handles.size() == 0)
    {
        std::cout << "No devices found" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Connected to [" << handles[0].ToString() << "]" << std::endl;
    auto device = LMS7_Device::CreateDevice(handles[0]);

    auto progCallback = [](int bsent, int btotal, const char* progressMsg)
    {
        printf("[%3i%%] %5i/%5i Bytes %s\r", int(100.0*bsent/btotal+0.5), bsent, btotal, progressMsg);
        fflush(stdout);
        return 0;
    };

    auto status = device->Program(program_mode::fx3Flash, progData.data(), progData.size(), progCallback);
    std::cout << std::endl;
    if(status != 0)
        std::cout << "Programming failed! : " << GetLastErrorMessage() << std::endl;
    delete device;
    return (status==0)?EXIT_SUCCESS:EXIT_FAILURE;
}

/***********************************************************************
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"info", optional_argument, 0, 'i'},
        {"find", optional_argument, 0, 'f'},
        {"make", optional_argument, 0, 'm'},
        {"force",      no_argument, 0, 'F'},
        {"args", optional_argument, 0, 'a'},
        {"update",     no_argument, 0, 'u'},
        {"fpga", required_argument, 0, 'g'},
        {"fw",   required_argument, 0, 'w'},
        {"timing",     no_argument, 0, 't'},
        {"cal",     optional_argument, 0, 'l'},
        {"start",   required_argument, 0, 's'},
        {"stop",    required_argument, 0, 'p'},
        {"step",    required_argument, 0, 'e'},
        {"bw",      required_argument, 0, 'b'},
        {"dir",     required_argument, 0, 'd'},
        {"chans",   required_argument, 0, 'c'},
        {0, 0, 0,  0}
    };

    std::string argStr, dir("BOTH"), chans("ALL");
    double start(0.0), stop(0.0), step(1e6), bw(30e6);
    bool testTiming(false), calSweep(false), update(false), force(false);
    int long_index = 0;
    int option = 0;
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h': return printHelp();
        case 'i': return printInfo();
        case 'f': return findDevices();
        case 'm': return makeDevice();
        case 'a':
            if (optarg != NULL) argStr = optarg;
            break;
        case 'u': update = true; break;
        case 'g': return programGateware(argStr);
        case 'w': return programFirmware(argStr);
        case 't': testTiming = true; break;
        case 'l':
            calSweep = true;
            if (optarg != NULL) argStr = optarg;
            break;
        case 's': if (optarg != NULL) start = std::stod(optarg); break;
        case 'p': if (optarg != NULL) stop = std::stod(optarg); break;
        case 'e': if (optarg != NULL) step = std::stod(optarg); break;
        case 'b': if (optarg != NULL) bw = std::stod(optarg); break;
        case 'd': if (optarg != NULL) dir = optarg; break;
        case 'c': if (optarg != NULL) chans = optarg; break;
        case 'F': force = true; break;
        }
    }

    if (testTiming) return deviceTestTiming(argStr);
    if (calSweep) return deviceCalSweep(argStr, start, stop, step, bw, dir, chans);
    if (update) return programUpdate(force, argStr);

    //unknown or unspecified options, do help...
    return printHelp();
}
