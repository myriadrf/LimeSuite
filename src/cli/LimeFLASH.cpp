#include "cli/common.h"

using namespace lime;

SDRDevice *device = nullptr;
SDRDevice::SDRConfig config;
bool abortflag(false);

static int printHelp(void)
{
    std::cout << "Usage LimeFLASH [options] program.bin" << std::endl;
    std::cout << "    --help\t\t\t This help" << std::endl;
    std::cout << "    --index <i>\t\t\t Specifies device index (0-...). Required if multiple devices" << std::endl;
    std::cout << "    --target <TARGET>\t\t <TARGET> programming. \"-\" to list targets" << std::endl;

    return EXIT_SUCCESS;
}
/***********************************************************************/
void inthandler (int sig)
{
    abortflag = true;
}
/***********************************************************************/
int findMemDevId (std::string target)
{
    auto d = device->GetDescriptor();

    if (target[0] == '-')
        std::cout << "Memory devices :" << std::endl;
    for (const SDRDevice::DataStorage &mem : d.memoryDevices)
        if (target[0] == '-')
            std::cout << "\t" << mem.name << std::endl;                    
        else
            if (mem.name == target)
                return (mem.id);
                
    return -1;
}

/***********************************************************************/
bool CallBack (size_t bsent, size_t btotal, const char* statusMessage)
{
    std::cout << "Sent " << bsent << " of " << btotal << " bytes\r";
    if (abortflag)
    {
        std::cout << "aborting..." << std::endl;
        return false;
    }
    return true;
}
/***********************************************************************
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"board", required_argument, 0, 'b'},
        {"target", required_argument, 0, 't'},
        {0, 0, 0,  0}
    };

    int dev_index = -1;
    int long_index = 0;
    int option = 0;
    std::string target;
    std::vector<char> data;
    std::ifstream inf;
    bool printlist(false);
    int devid;

    while ((option = getopt_long_only (argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h': 
            return printHelp ();
        case 'b':
            if (optarg != NULL) dev_index = std::stod (optarg);
            break;
        case 't':
            if (optarg != NULL) {
                if (optarg[0] == '-')
                    printlist =  true;
                else
                    target = optarg;
            }
            break;
        }
    }       
    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0) 
    {
        std::cout << "No devices found" << std::endl;
        return -1;
    } else {
        if  (handles.size () > 1 && dev_index == -1)
        {
            std::cout << "Multiple devices detected and no --index option specified" << std::endl;
            return -1;
        } else
            dev_index = 0;
    }
    device = DeviceRegistry::makeDevice(handles.at(dev_index));
    if (printlist)
    {
        findMemDevId ("-");
        return EXIT_SUCCESS;    
    }
    if (target.empty ())
    {
        std::cout << "Target must be specified." << std::endl;
        printHelp ();
        return -1;
    }
    if (optind == argc) {std::cout << "File must be specified." << std::endl;printHelp (); return -1;}
    if ((devid = findMemDevId (target)) == - 1)
    {
        std::cout << "Memory device " << target << " not found" << std::endl;    
        return -1;
    }
    inf.open (argv[optind], std::ifstream::in | std::ifstream::binary);
    if  (!inf)
    {
        std::cout << "Unable to open file." << std::endl;
        return -1;
    }
    inf.seekg (0,std::ios_base::end);
    auto cnt = inf.tellg ();
    inf.seekg (0,std::ios_base::beg);
    std::cout << "File size : " << cnt << " bytes." << std::endl;
    data.resize (cnt);
    inf.read (data.data (),cnt);
    inf.close ();
    std::cout << "Memory device id : " << devid << std::endl;
    signal (SIGINT, inthandler);
    if (!device->UploadMemory (devid, data.data(), data.size(), CallBack))
    {
        std::cout << "Error programming device." << std::endl;
        return -1;
    }
    std::cout << "Programming Ok." << std::endl;

    return EXIT_SUCCESS;
}

