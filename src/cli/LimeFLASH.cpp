#include "cli/common.h"

using namespace lime;

SDRDevice               *device = nullptr;
SDRDevice::SDRConfig    config;

static int printHelp(void)
{
    std::cout << "Usage LimeFLASH [options]" << std::endl;
    std::cout << "    --help\t\t\t This help" << std::endl;
    std::cout << "    --index <i>\t\t\t Specifies device index (0-... default 0)" << std::endl;
    std::cout << "    --ram <fname>\t\t RAM programming" << std::endl;
    std::cout << "    --flash <fname>\t\t FLASH programming" << std::endl;

    return EXIT_SUCCESS;
}
bool CallBack (size_t bsent, size_t btotal, const char* statusMessage)
{
    return true;
}
/***********************************************************************
 * main entry point
 **********************************************************************/
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"index", required_argument, 0, 'i'},
        {"ram", required_argument, 0, 'r'},
        {"flash", required_argument, 0, 'f'},
        {0, 0, 0,  0}
    };

    int dev_index = 0;
    int long_index = 0;
    int option = 0;
    char *fram = NULL,*fflash = NULL;
    std::vector<char> data;
    std::ifstream inf;
    
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h': 
            return printHelp();
        case 'i':
            if (optarg != NULL) dev_index = std::stod(optarg);
            break;
        case 'r':
            if (optarg != NULL) fram = optarg;
            break;
        case 'f':
            if (optarg != NULL) fflash = optarg;
            break;
        }
    }
    if  (!fram && !fflash) {printHelp (); return EXIT_SUCCESS;}
    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0) 
    {
        std::cout << "No devices found" << std::endl;
        return -1;
    }
    device = DeviceRegistry::makeDevice(handles.at(dev_index));
//    const SDRDevice::Descriptor &desc = device->GetDescriptor();

    if  (fram)
        inf.open (fram, std::ifstream::in | std::ifstream::binary);
    else
        inf.open (fflash, std::ifstream::in | std::ifstream::binary);
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
    if (device->UploadMemory (fram?0:1, data.data(), data.size(), CallBack))
    {
        std::cout << "Error programming device." << std::endl;
        return -1;
    }
    std::cout << "Ok." << std::endl;

    return EXIT_SUCCESS;
}

