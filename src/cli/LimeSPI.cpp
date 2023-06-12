#include "cli/common.h"

using namespace lime;

SDRDevice               *device = nullptr;
SDRDevice::SDRConfig    config;

static int printHelp(void)
{
    std::cout << "Usage LimeSPI [options]" << std::endl;
    std::cout << "    --help\t\t\t This help" << std::endl;
    std::cout << "    --index <i>\t\t\t Specifies device index (0-... required if devices > 1)" << std::endl;
    std::cout << "    --target <TARGET>\t\t Selects target TARGET (- to list targets, required)" << std::endl;
    std::cout << "    --debug\t\t\t Force debug output" << std::endl;
    std::cout << "    --addr <XXXX>\t\t Set Operating address (default 0)" << std::endl;
    std::cout << "    --value <xxxx>\t\t Specifies value <xxxx> to be written" << std::endl;
    std::cout << "    --operation <r|w>\t\t Operation - read/write" << std::endl;
    return EXIT_SUCCESS;
}

/***********************************************************************
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"target", required_argument, 0, 't'},
        {"debug", no_argument, 0, 'd'},
        {"index", required_argument, 0, 'i'},
        {"addr", required_argument, 0, 'a'},
        {"value", required_argument, 0, 'v'},
        {"operation", required_argument, 0, 'o'},
        {0, 0, 0,  0}
    };

    bool debug(false),printlist(false),value_specified(false);
    long addr = 0;
    int dev_index = -1;
    int long_index = 0;
    int option = 0;
    uint32_t devAddr = 0;
    long value = 0;
    uint32_t mosi;
    uint32_t miso = 0;
    std::string target;
    std::string operation;
      
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h': 
            return printHelp();
        case 'd':
            debug = true;
            break;
        case 'i':
            if (optarg != NULL) dev_index = std::stod(optarg);
            break;        
        case 'a':
            if (optarg != NULL) addr = std::stod(optarg);
            break;
        case 'v':
            if (optarg != NULL) {value = std::stod(optarg);value_specified = true;}
            break;
        case 't':
            if (optarg != NULL)
            {
                if (optarg[0] == '-')
                    printlist =  true;
                else
                    target = optarg;
            }
            break;
        case 'o':
            if (optarg != NULL) operation = optarg[0];
            break;
        }
    }
    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0)
    {
        printf("No devices found\n");
        return -1;
    }
    if (debug) std::cout << "Found " << handles.size() << " device(s)" << std::endl;
    if (handles.size() > 1 && dev_index == -1)
    {
        std::cout << "Multiple devices detected and no --index specified " << std::endl;
        return -1;
    }
    if (dev_index == -1) dev_index = 0;
    device = DeviceRegistry::makeDevice(handles.at(dev_index));
    auto descr = device->GetDescriptor();  
    if (printlist)
    {
       std::cout << "SPI slave devices list :" << std::endl;
       for (const auto nameIds : descr.spiSlaveIds)
           std::cout << "\t" <<  nameIds.first.c_str() << std::endl;
        return -1;    
    }
    if  (operation[0] != 'r' && operation[0] != 'w')
    {
        std::cout << "Must be specified --operation r OR --operation w" << std::endl;
        printHelp ();
        return -1;
    }    
    if  (target.empty ())
    {
        std::cout << "Target must be specified." << std::endl;
        printHelp ();
        return -1;
    }
    auto iter = descr.spiSlaveIds.find(std::string(target));
    if (iter == descr.spiSlaveIds.end())
    {
        std::cout << "SPI device " << target << " do not exists." << std::endl;
        return -1;
    }
    devAddr = iter->second;
    if  (operation[0] == 'r')
    {
        mosi = addr;
        device->SPI(devAddr, &mosi, &miso, 1);     
        if  (debug) 
            std::cout << "Reading from " << target << " addr.: 0x" << std::hex << std::uppercase << addr << " value ";
        std::cout << "0x" << std::hex << miso;
    } else { // perform write operation
        if  (!value_specified)
        {
            std::cout << "Value must bu specified." << std::endl;
            printHelp ();
            return -1;
        }
        if  (debug)
        {
            std::cout << "Performning write operation to ";
            std::cout << target << " addr 0x" << std::hex << std::uppercase << addr << " value : 0x" << std::hex << value;
        }
        mosi = (1 << 31) | addr << 16 | value;
        device->SPI(devAddr, &mosi, nullptr, 1);
    }
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

