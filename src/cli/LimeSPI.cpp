#include "cli/common.h"

using namespace lime;

SDRDevice               *device = nullptr;
SDRDevice::SDRConfig    config;

static int printHelp(void)
{
    std::cout << "Usage LimeSPI [options]" << std::endl;
    std::cout << "    --help\t\t\t This help" << std::endl;
    std::cout << "    --lms\t\t\t Force to use LMS7002M registers instead of FPGA (default)" << std::endl;
    std::cout << "    --debug\t\t\t Force debug output" << std::endl;
    std::cout << "    --index <i>\t\t\t Specifies device index (0-... default 0)" << std::endl;
    std::cout << "    --addr <XXXX>\t\t Set Operating address (default 0)" << std::endl;
    std::cout << "    --write <YYYY> \t\t writes to SPI register specified with --addr value YYYY" << std::endl;
    std::cout << "    --value <xxxx>\t\t\t Specifies value <xxxx> to be written (default 0)" << std::endl;

    return EXIT_SUCCESS;
}

/***********************************************************************
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"lms", no_argument, 0, 'l'},
        {"debug", no_argument, 0, 'd'},
        {"index", required_argument, 0, 'i'},
        {"write", no_argument, 0, 'w'},
        {"addr", required_argument, 0, 'a'},
        {"value", required_argument, 0, 'v'},
        {0, 0, 0,  0}
    };

    bool lms(false),debug(false),read_option(true);
    long addr = 0;
    int dev_index = 0;
    int long_index = 0;
    int option = 0;
    uint32_t devAddr = 0;
    long value = 0;
    uint32_t mosi;
    uint32_t miso = 0;
    
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h': 
            return printHelp();
        case 'l':
            lms = true;
            break;
        case 'd':
            debug = true;
            break;
        case 'i':
            if (optarg != NULL) dev_index = std::stod(optarg);
            break;        
        case 'w':
            read_option = false;
            break;
        case 'a':
            if (optarg != NULL) addr = std::stod(optarg);
            break;
        case 'v':
            if (optarg != NULL) value = std::stod(optarg);
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
    if (lms) devAddr = 0; else devAddr = 1; // FPGA - 1
    device = DeviceRegistry::makeDevice(handles.at(dev_index));
    if  (read_option)
    {
        if  (debug)
        {
            std::cout << "Performning read operation from ";
            if  (lms)
                std::cout << "LMS7002M addr 0x" << std::hex << std::uppercase << addr << " Got value 0x:";
            else
                std::cout << "FPGA addr 0x" << std::hex << std::uppercase << addr << " Got value : 0x";
        }
        mosi = addr;
        device->SPI(devAddr, &mosi, &miso, 1);     
        if  (!debug) std::cout << "0x";
        std::cout << std::hex << miso;
    } else { // perform write operation
        if  (debug)
        {
            std::cout << "Performning write operation to ";
            if  (lms)
                std::cout << "LMS7002M addr 0x" << std::hex << std::uppercase << addr << " value 0x" << std::hex << value;
            else
                std::cout << "FPGA addr 0x" << std::hex << std::uppercase << addr << " value : 0x" << std::hex << value;
        }
        mosi = (1 << 31) | addr << 16 | value;
        device->SPI(devAddr, &mosi, nullptr, 1);
    }
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

