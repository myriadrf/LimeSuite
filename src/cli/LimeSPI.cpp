#include "cli/common.h"

using namespace lime;

SDRDevice               *device = nullptr;
SDRDevice::SDRConfig    config;
std::ofstream           outfile;

/************************************************************************/
static int printHelp(void)
{
    std::cout << "Usage LimeSPI [options]" << std::endl;
    std::cout << "    --help\t\t\t This help" << std::endl;
    std::cout << "    --board <i>\t\t\t Specifies device index (0-... required if devices > 1)" << std::endl;
    std::cout << "    --target <TARGET>\t\t Selects target TARGET (- to list targets, required)" << std::endl;
    std::cout << "    --debug\t\t\t Force debug output" << std::endl;
    std::cout << "    --addr <XXXX>\t\t Set Operating address (default 0)" << std::endl;
    std::cout << "    --value <xxxx>\t\t Specifies value <xxxx> to be written" << std::endl;
    std::cout << "    --operation <r|w>\t\t Operation - read/write" << std::endl;
    std::cout << "    --raw <xxxx>\t\t Specifies raw SPI value <xxxx> to be sent" << std::endl;
    std::cout << "    --input <fname>\t\t Specifies input data file" << std::endl;
    std::cout << "    --output <fname>\t\t Specifies ouput data file" << std::endl;

    return EXIT_SUCCESS;
}
/************************************************************************/
void processRaw (uint32_t rawdata, uint32_t devAddr, bool debug)
{
    uint32_t mosi;
    uint32_t miso = 0;

    if  (debug) 
         std::cout << "Processing raw data 0x" << std::hex << std::setfill('0') << std::setw(8) << rawdata << std::endl;
    if  (rawdata & (1 << 31)) // write
    {
        mosi = rawdata;
        if (debug)
        {
            std::cout << "Write operation. Addr : 0x" << std::hex << std::setfill('0') << std::setw(4) << ((mosi >> 16) & 0x7fff) << " data : 0x" << std::hex << (mosi & 0xffff) << std::endl;
        }
        mosi = rawdata;
        device->SPI(devAddr, &mosi, nullptr, 1);
    } 
    else    
    {
        mosi = (rawdata >> 16) & 0xffff;
        device->SPI(devAddr, &mosi, &miso, 1);        
        if (debug)
        {
            std::cout << "Read operation. Addr : 0x" << std::hex << std::setfill('0') << std::setw(4) << ((mosi)& 0xffff) << std::endl;
        }
        if (outfile.is_open ())
            outfile << "0x" << std::hex << std::setfill('0') << std::setw(4) << mosi << std::hex << std::setfill('0') << std::setw(4) << miso << std::endl;
        else
            std::cout << "0x" << std::hex << std::setfill('0') << std::setw(4) << mosi << std::hex << std::setfill('0') << std::setw(4) << miso << std::endl;        
    }
}
/************************************************************************/
void processFile (std::string infname, uint32_t devAddr, bool debug)
{
    std::ifstream infile;
    uint32_t rawdata;
    
    infile.open (infname);
    if (!infile.is_open ())
    {
        std::cout << "Can not open file " << infname << std::endl;
        return;
    }
    for (std::string str; std::getline(infile, str);)
    {
        if (debug && str.size ())
            std::cout << "Processing : " << str << std::endl;
        if (str.size ())
        {
            rawdata = std::stod(str);
            processRaw (rawdata, devAddr, debug);
        }
    }
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
        {"board", required_argument, 0, 'b'},
        {"addr", required_argument, 0, 'a'},
        {"value", required_argument, 0, 'v'},
        {"operation", required_argument, 0, 'p'},
        {"raw", required_argument, 0, 'r'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {0, 0, 0,  0}
    };

    bool debug(false),printlist(false),value_specified(false),raw(false);
    long addr = 0;
    int dev_index = -1;
    int long_index = 0;
    int option = 0, opts = 0;
    uint32_t devAddr = 0;
    long value = 0;
    uint32_t rawdata;
    std::string target, operation, ifname;

    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h': 
            return printHelp();
        case 'd':
            debug = true;
            break;
        case 'b':
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
        case 'p':
            if (optarg != NULL) {operation = optarg[0];opts++;}
            break;
        case 'r':
            if (optarg != NULL) {rawdata = std::stod(optarg);raw = true;opts++;}
            break;
        case 'i':
            if (optarg != NULL) {ifname = optarg;opts++;}
            break;
        case 'o':
            if (optarg != NULL) outfile.open (optarg, std::ios_base::app);
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
        std::cout << "Multiple devices detected and no --board specified " << std::endl;
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
        return EXIT_SUCCESS;    
    }
    if  (operation[0] != 'r' && operation[0] != 'w' && !raw && !ifname.size ())
    {
        std::cout << "Operation must be specified --operation r, --operation w, --raw or --input" << std::endl;
        printHelp ();
        return -1;
    }
    if (opts > 1)
    {
        std::cout << "Allowed only one operation --operation r, --operation w, --raw or --input" << std::endl;
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
    if (raw)
    {
        processRaw (rawdata, devAddr, debug); 
        return EXIT_SUCCESS;
    }
    if (ifname.size())
    {
        processFile (ifname, devAddr, debug);
        return EXIT_SUCCESS;
    }
    if  (operation[0] == 'r')
    {
        rawdata = addr << 16;
        processRaw (rawdata, devAddr, debug);
    } else { // perform write operation
        if  (!value_specified)
        {
            std::cout << "Value must bu specified." << std::endl;
            printHelp ();
            return -1;
        }
        rawdata = (1 << 31) | addr << 16 | value;
        processRaw (rawdata, devAddr, debug);
    }
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

