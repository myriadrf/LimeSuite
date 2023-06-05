#include "cli/common.h"

using namespace lime;

SDRDevice               *device = nullptr;
SDRDevice::SDRConfig    config;

static int printHelp(void)
{
    std::cout << "Usage LimeConfig [options]" << std::endl;
    std::cout << "    --help\t\t\t This help" << std::endl;
    std::cout << "    --debug\t\t\t Force debug output" << std::endl;
    std::cout << "    --index <i>\t\t\t Specifies device index (0-... default 0)" << std::endl;
    std::cout << "    --load <file>\t\t Load LMS7002M registers from file <file>" << std::endl;
    std::cout << "    --save <file> \t\t Save LMS7002M registers to file <file>" << std::endl;
    std::cout << "    --lms <x>\t\t\t Specifies LMS7002M chip index <x> (default 0)" << std::endl;
    std::cout << "    --reset\t\t\t Perform LMS7002M reset" << std::endl;

    return EXIT_SUCCESS;
}

/***********************************************************************
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"debug", no_argument, 0, 'd'},
        {"index", required_argument, 0, 'i'},
        {"load", required_argument, 0, 'l'},
        {"save", required_argument, 0, 's'},
        {"lms", required_argument, 0, 'm'},
        {"reset", no_argument, 0, 'r'},
        {0, 0, 0,  0}
    };

    bool debug(false),load(true),reset(false);
    int dev_index = 0;
    int lms_index = 0;
    int long_index = 0;
    int option = 0;
    char *f = NULL;
    
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
        case 'l':
            if (optarg != NULL) {f = optarg;load = true;}
            break;
        case 's':
            if (optarg != NULL) {f = optarg;load = false;}
            break;
        case 'm':
            if (optarg != NULL) lms_index = std::stod(optarg);
            break;
        case 'r':
            reset = true;
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
    device = DeviceRegistry::makeDevice(handles.at(dev_index));
    LMS7002M *chip = static_cast<LMS7002M*>(device->GetInternalChip (lms_index));
    if  (reset) chip->ResetChip ();
    if  (!f)
        return EXIT_SUCCESS;
    if  (load) 
            chip->LoadConfig (f); 
        else 
            chip->SaveConfig (f);
    return EXIT_SUCCESS;
}

