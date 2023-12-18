#include "cli/common.h"

using namespace std;
using namespace lime;

SDRDevice* device = nullptr;
bool terminateProgress(false);

void inthandler(int sig)
{
    if (terminateProgress == true)
    {
        cerr << "Force exiting" << endl;
        exit(-1);
    }
    terminateProgress = true;
}

static void PrintMemoryDevices(SDRDevice::Descriptor descriptor)
{
    for (const auto& memoryDevice : descriptor.memoryDevices)
    {
        cerr << '\t' << memoryDevice.first << endl;
    }
}

static const std::shared_ptr<SDRDevice::DataStorage> FindMemoryDeviceByName(SDRDevice* device, const char* targetName)
{
    if (!device)
    {
        return std::make_shared<SDRDevice::DataStorage>(nullptr, eMemoryDevice::COUNT);
    }

    const auto descriptor = device->GetDescriptor();
    const auto memoryDevices = descriptor.memoryDevices;
    if (!targetName)
    {
        // if name is not specified, but only single choice is available, use that.
        if (memoryDevices.size() == 1)
        {
            return memoryDevices.begin()->second;
        }

        cerr << "specify memory device target, -t, --target :" << endl;
        PrintMemoryDevices(descriptor);

        return std::make_shared<SDRDevice::DataStorage>(nullptr, eMemoryDevice::COUNT);
    }

    try
    {
        return memoryDevices.at(targetName);
    } catch (const std::out_of_range& e)
    {
        std::cerr << e.what() << '\n';
    }

    cerr << "Device does not contain target device (" << targetName << "). Available list:" << endl;
    PrintMemoryDevices(descriptor);

    return std::make_shared<SDRDevice::DataStorage>(nullptr, eMemoryDevice::COUNT);
}

static auto lastProgressUpdate = std::chrono::steady_clock::now();
bool progressCallBack(size_t bsent, size_t btotal, const char* statusMessage)
{
    float percentage = 100.0 * bsent / btotal;
    const bool hasCompleted = bsent == btotal;
    auto now = std::chrono::steady_clock::now();
    // no need to spam the text with each callback
    if (now - lastProgressUpdate > std::chrono::milliseconds(10) || hasCompleted)
    {
        lastProgressUpdate = now;
        printf("[%3.0f%%] bytes written %li/%li\r", percentage, bsent, btotal);
        fflush(stdout);
    }
    if (hasCompleted)
        cout << endl;
    if (terminateProgress)
    {
        printf("\nAborting programing will corrupt firmware and will need external programmer to fix it. Are you sure? [y/n]: ");
        fflush(stdout);
        std::string answer;
        while (1)
        {
            std::getline(std::cin, answer);
            if (answer[0] == 'y')
            {
                cout << "\naborting..." << endl;
                return true;
            }
            else if (answer[0] == 'n')
            {
                terminateProgress = false;
                cout << "\ncontinuing..." << endl;
                return false;
            }
            else
                cout << "Invalid option(" << answer << "), [y/n]: ";
        }
    }
    return false;
}

static int printHelp(void)
{
    cerr << "Usage: LimeFLASH [OPTIONS] [FILE]" << endl;
    cerr << "  OPTIONS:" << endl;
    cerr << "    -h, --help\t\t\t This help" << endl;
    cerr << "    -d, --device <name>\t\t\t Specifies which device to use" << endl;
    cerr << "    -t, --target <TARGET>\t <TARGET> programming. \"-\" to list targets" << endl;
    cerr << endl;
    cerr << "  FILE: input file path." << endl;

    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    char* filePath = nullptr;
    char* devName = nullptr;
    char* targetName = nullptr;
    signal(SIGINT, inthandler);

    static struct option long_options[] = { { "help", no_argument, 0, 'h' },
        { "device", required_argument, 0, 'd' },
        { "target", required_argument, 0, 't' },
        { 0, 0, 0, 0 } };

    int long_index = 0;
    int option = 0;
    std::string target;

    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case 'h':
            return printHelp();
        case 'd':
            if (optarg != NULL)
                devName = optarg;
            break;
        case 't':
            if (optarg != NULL)
            {
                targetName = optarg;
            }
            break;
        }
    }

    if (optind < argc)
        filePath = argv[optind];
    else
    {
        cerr << "File path not specified." << endl;
        printHelp();
        return -1;
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

    auto memorySelect = FindMemoryDeviceByName(device, targetName);
    if (memorySelect->ownerDevice == nullptr)
    {
        DeviceRegistry::freeDevice(device);
        return EXIT_FAILURE;
    }

    std::vector<char> data;
    std::ifstream inputFile;
    inputFile.open(filePath, std::ifstream::in | std::ifstream::binary);
    if (!inputFile)
    {
        DeviceRegistry::freeDevice(device);
        cerr << "Failed to open file: " << filePath << endl;
        return EXIT_FAILURE;
    }
    inputFile.seekg(0, std::ios_base::end);
    auto cnt = inputFile.tellg();
    inputFile.seekg(0, std::ios_base::beg);
    cerr << "File size : " << cnt << " bytes." << endl;
    data.resize(cnt);
    inputFile.read(data.data(), cnt);
    inputFile.close();

    if (memorySelect->ownerDevice->UploadMemory(memorySelect->memoryDeviceType, 0, data.data(), data.size(), progressCallBack) != 0)
    {
        DeviceRegistry::freeDevice(device);
        cout << "Device programming failed." << endl;
        return EXIT_FAILURE;
    }
    if (terminateProgress)
        cerr << "Programming aborted." << endl;
    else
        cerr << "Programming completed." << endl;

    DeviceRegistry::freeDevice(device);
    return EXIT_SUCCESS;
}
