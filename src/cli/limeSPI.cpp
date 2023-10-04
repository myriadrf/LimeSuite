#include "cli/common.h"

#include <assert.h>
#include <cstring>

using namespace std;
using namespace lime;

static int32_t FindChipSelectByName(SDRDevice* device, const char* chipName)
{
    if (!device)
        return -1;
    const auto chipMap = device->GetDescriptor().spiSlaveIds;
    if (!chipName)
    {
        cerr << "specify SPI chip select, -c, --chip :" << endl;
        for (const auto &nameIds : chipMap)
            cerr << "\t" <<  nameIds.first.c_str() << endl;
        return -1;
    }

    auto iter = chipMap.find(std::string(chipName));
    if (iter == chipMap.end())
    {
        cerr << "Device does not contain target chip (" << chipName << "). Available list:" << endl;
        for (const auto &nameIds : chipMap)
            cerr << "\t" <<  nameIds.first.c_str() << endl;
        return -1;
    }
    return iter->second;
}

static void PrintMISO(std::ostream& stream, const std::vector<uint32_t>& miso)
{
    stream << std::hex << std::setfill('0');
    for(uint32_t value : miso)
        stream << std::setw(8) << value << std::endl;
}

static uint32_t hex2int(const char* hexstr)
{
    assert(hexstr);
    uint32_t value = 0;
    sscanf(hexstr, "%X", &value);
    return value;
}

static int parseWriteInput(char* hexstr, std::vector<uint32_t>& mosi)
{
    static const char* delimiters = " \n,";
    mosi.clear();
    char* token = std::strtok(hexstr, delimiters);
    const uint32_t spiWriteBit = 1 << 31;
    int tokenCount = 0;
    while (token)
    {
        int tokenLength = strlen(token);
        if (tokenLength <= 8 && tokenLength > 4) // write instruction
        {
            uint32_t value = hex2int(token);
            mosi.push_back(spiWriteBit | value);
        }
        else
            std::cerr << "Invalid input value: " << token << std::endl;
        ++tokenCount;
        token = std::strtok(nullptr, delimiters);
    }
    return tokenCount;
}

static int parseReadInput(char* hexstr, std::vector<uint32_t>& mosi)
{
    static const char* delimiters = " \n,";
    mosi.clear();
    char* token = std::strtok(hexstr, delimiters);
    int tokenCount = 0;
    while (token)
    {
        int tokenLength = strlen(token);
        if (tokenLength <= 4 && tokenLength > 0) // read instruction
        {
            uint32_t value = hex2int(token);
            mosi.push_back(value);
        }
        else
            std::cerr << "Invalid input value: " << token << std::endl;
        ++tokenCount;
        token = std::strtok(nullptr, delimiters);
    }
    return tokenCount;
}

static int printHelp(void)
{
    cerr << "limeSPI [options]" << endl;
    cerr << "    -h, --help\t\t\t This help" << endl;
    cerr << "    -d, --device <name>\t\t\t Specifies which device to use" << endl;
    cerr << "    -c, --chip <name>\t\t Selects destination chip" << endl;
    cerr << "    -r, --read \"data or filepath\"\t\t space/newline/comma delimited 16bit hexadecimal addresses for reading" << endl;
    cerr << "    -w, --write \"data or filepath\"\t\t space/newline/comma delimited 32bit hexadecimal values for writing" << endl;
    cerr << "    -f, --file\t\t Use --read/--write argument as filename" << endl;

    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    char* devName = nullptr;
    char* chipName = nullptr;
    char* hexInput = nullptr;
    bool hexInputIsFilename = false;
    bool isWrite = false;
    bool isRead = false;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"device", required_argument, 0, 'd'},
        {"chip", required_argument, 0, 'c'},
        {"read", required_argument, 0, 'r'},
        {"write", required_argument, 0, 'w'},
        {"file", optional_argument, 0, 'f'},
        {0, 0, 0,  0}
    };

    int long_index = 0;
    int option = 0;
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h':
            return printHelp();
        case 'd':
            if (optarg != NULL) devName = optarg;
            break;
        case 'c':
            if (optarg != NULL) chipName = optarg;
            break;
        case 'r':
            if (optarg != NULL) {isRead = true; hexInput = optarg; }
            break;
        case 'w':
            if (optarg != NULL) {isWrite = true; hexInput = optarg; }
            break;
        case 'f':
            hexInputIsFilename = true;
            break;
        }
    }

    if (isRead && isWrite)
    {
        cerr << "use --read, OR --write, can't do both at the same time" << endl;
        return EXIT_FAILURE;
    }

    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0)
    {
        cerr << "No devices found" << endl;
        return EXIT_FAILURE;
    }

    SDRDevice* device;
    if (devName)
        device = ConnectUsingNameHint(devName);
    else
    {
        if (handles.size() > 1)
        {
            cerr << "Multiple devices detected, specify which one to use with -d, --device" << endl;
            return EXIT_FAILURE;
        }
        // device not specified, use the only one available
        device = DeviceRegistry::makeDevice(handles.at(0));
    }

    if (!device)
    {
        cerr << "Device connection failed" << endl;
        return EXIT_FAILURE;
    }

    int32_t chipSelect = FindChipSelectByName(device, chipName);
    if (chipSelect < 0)
        return EXIT_FAILURE;

    std::vector<char> buffer;
    if (hexInputIsFilename)
    {
        //read file
        const std::string filename(hexInput);
        std::ifstream inputFile(filename);
        if (!inputFile.is_open())
        {
            cerr << "Failed to open file: " << filename << endl;
            return EXIT_FAILURE;
        }
        inputFile.seekg(0,std::ios::end);
        long fileSize = inputFile.tellg();
        inputFile.seekg(0,std::ios::beg);

        buffer.resize(fileSize+1); // +1 to add null termination
        inputFile.read(&buffer[0], fileSize);
        buffer[fileSize] = 0;
        hexInput = buffer.data();
    }

    if (!hexInput)
    {
        cerr << "No input provided" << endl;
        return EXIT_FAILURE;
    }

    std::vector<uint32_t> mosi;
    if (isWrite)
        parseWriteInput(hexInput, mosi);
    else if (isRead)
        parseReadInput(hexInput, mosi);

    std::vector<uint32_t> miso(mosi.size());

    try {
        device->SPI(chipSelect, mosi.data(), miso.data(), mosi.size());
    } catch (std::runtime_error &e) {
        cerr << "SPI failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    // fill in register addresses for convenience and reusage as write input
    for (size_t i=0; i<miso.size(); ++i)
        miso[i] |= mosi[i] << 16;

    if (isRead)
        PrintMISO(cout, miso);

    return EXIT_SUCCESS;
}

