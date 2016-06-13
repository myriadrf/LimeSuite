/**
    @file LimeUtil.cpp
    @author Lime Microsystems
    @brief Command line test app
*/

#include <VersionInfo.h>
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <iostream>
#include <cstdlib>
#include <ciso646>
#include <getopt.h>

using namespace lime;

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
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

/***********************************************************************
 * print info
 **********************************************************************/
static int printInfo(void)
{
    std::cout << "Library Version: v" << lime::GetLibraryVersion() << std::endl;
    std::cout << "Build timestamp: " << lime::GetBuildTimestamp() << std::endl;
    std::cout << "Connections: ";
    for (const auto &name : ConnectionRegistry::moduleNames()) std::cout << name << ", ";
    std::cout << std::endl;
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
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"info", optional_argument, 0, 'i'},
        {"find", optional_argument, 0, 'f'},
        {"make", optional_argument, 0, 'm'},
        {0, 0, 0,  0}
    };
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
        }
    }

    //unknown or unspecified options, do help...
    return printHelp();
}
