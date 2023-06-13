#include "cli/common.h"

using namespace lime;

SDRDevice               *device = nullptr;
SDRDevice::SDRConfig    config;

static int printHelp(void)
{
    std::cout << "Usage LimeDevice [options]" << std::endl;
    std::cout << "    --help\t This help" << std::endl;
    std::cout << "    --full\t Force print detailed device(s) info" << std::endl;
    return EXIT_SUCCESS;
}

/***********************************************************************
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"full", no_argument, 0, 'f'},
        {0, 0, 0,  0}
    };

    bool full(false);
    int long_index = 0;
    int option = 0;
    
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h': 
            return printHelp();
        case 'f':
            full = true;
        }
    }
    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0)
    {
        printf("No devices found\n");
        return -1;
    }
    std::cout << "Found " << handles.size() << " device(s) :" << std::endl;
    for (uint32_t i = 0; i < handles.size(); i++)
    {
        std::cout << i << ": " << handles[i].serialize() << std::endl;
        if  (full)
        {
            device = DeviceRegistry::makeDevice(handles.at(i));
            auto d = device->GetDescriptor();
            std::cout << '\t' << "Expansion name\t\t: " << d.expansionName << std::endl;
            std::cout << '\t' << "Firmware version\t: " << d.firmwareVersion << std::endl;
            std::cout << '\t' << "Gateware version\t: " << d.gatewareVersion << std::endl;
            std::cout << '\t' << "Gateware revision\t: " << d.gatewareRevision << std::endl;
            std::cout << '\t' << "Gateware target board\t:" << d.gatewareTargetBoard << std::endl;
            std::cout << '\t' << "Hardware version\t: " << d.hardwareVersion << std::endl;
            std::cout << '\t' << "Protocol version\t: " << d.protocolVersion << std::endl;
            std::cout << '\t' << "Serial number\t\t: " << d.serialNumber << std::endl;
            std::cout << "\t" << "SPI slave devices\t:" << std::endl;
            for (const auto nameIds : d.spiSlaveIds)
                std::cout << "\t\t\t\t  " <<  nameIds.first.c_str() << std::endl;
            std::cout << "\t" << "Memory devices\t\t:" << std::endl;
            for(const SDRDevice::DataStorage &mem : d.memoryDevices)
                std::cout << "\t\t\t\t  " << mem.name << std::endl;
        }
    }
    std::cout << std::endl;
    return EXIT_SUCCESS;
}
