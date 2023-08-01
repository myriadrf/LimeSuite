#include "cli/common.h"

using namespace std;
using namespace lime;

static int printHelp(void)
{
    cout << "limeDevice [options]" << endl;
    cout << "    -h, --help\t This help" << endl;
    cout << "    -f, --full\t Force print detailed device(s) info" << endl;
    return EXIT_SUCCESS;
}

void PrintDeviceDetails(SDRDevice* device)
{
    auto d = device->GetDescriptor();
    cout << '\t' << "Expansion name\t\t: " << d.expansionName << endl;
    cout << '\t' << "Firmware version\t: " << d.firmwareVersion << endl;
    cout << '\t' << "Gateware version\t: " << d.gatewareVersion << endl;
    cout << '\t' << "Gateware revision\t: " << d.gatewareRevision << endl;
    cout << '\t' << "Gateware target board\t: " << d.gatewareTargetBoard << endl;
    cout << '\t' << "Hardware version\t: " << d.hardwareVersion << endl;
    cout << '\t' << "Protocol version\t: " << d.protocolVersion << endl;
    cout << '\t' << "Serial number\t\t: " << d.serialNumber << endl;
    cout << "\t" << "SPI slave devices\t:" << endl;
    for (const auto &nameIds : d.spiSlaveIds)
        cout << "\t\t\t\t  " <<  nameIds.first.c_str() << endl;
    cout << "\t" << "Memory devices\t\t:" << endl;
    for (const SDRDevice::DataStorage &mem : d.memoryDevices)
        cout << "\t\t\t\t  " << mem.name << endl;
}

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
    cout << "Found " << handles.size() << " device(s) :" << endl;
    for (uint32_t i = 0; i < handles.size(); i++)
    {
        cout << i << ": " << handles[i].Serialize() << endl;
        if (full)
        {
            SDRDevice* device = DeviceRegistry::makeDevice(handles.at(i));
            if (!device)
            {
                cout << "\tFailed to connect" << endl;
                continue;
            }
            PrintDeviceDetails(device);
            DeviceRegistry::freeDevice(device);
        }
    }
    cout << endl;
    return EXIT_SUCCESS;
}
