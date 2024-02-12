#include "cli/common.h"

#include "limesuite/LMS7002M.h"
#include <cassert>
#include <cstring>
#include <getopt.h>

using namespace std;
using namespace lime;

static SDRDevice::LogLevel logVerbosity = SDRDevice::LogLevel::ERROR;
static SDRDevice::LogLevel strToLogLevel(const char* str)
{
    if (strstr("debug", str))
        return SDRDevice::LogLevel::DEBUG;
    else if (strstr("verbose", str))
        return SDRDevice::LogLevel::VERBOSE;
    else if (strstr("error", str))
        return SDRDevice::LogLevel::ERROR;
    else if (strstr("warning", str))
        return SDRDevice::LogLevel::WARNING;
    else if (strstr("info", str))
        return SDRDevice::LogLevel::INFO;
    return SDRDevice::LogLevel::ERROR;
}
static void LogCallback(SDRDevice::LogLevel lvl, const char* msg)
{
    if (lvl > logVerbosity)
        return;
    printf("%s\n", msg);
}

static int printHelp(void)
{
    cerr << "limeConfig [options]" << endl;
    cerr << "    -h, --help\t\t\t This help" << endl;
    cerr << "    -d, --device <name>\t\t Specifies which device to use" << endl;
    cerr << "    -c, --chip <name>\t\t Selects destination chip" << endl;
    cerr << "    -l, --log\t\t Log verbosity: info, warning, error, verbose, debug" << endl;
    cerr << "    -i, --initialize\t\t Reset and initialize entire device" << endl;

    cerr << "    --refclk\t\t Reference clock in Hz" << endl;
    cerr << "    --samplerate\t Sampling rate in Hz" << endl;
    cerr << "    --rxen=[0,1]\t Enable receiver" << endl;
    cerr << "    --rxlo\t\t Receiver center frequency in Hz" << endl;
    cerr << "    --rxpath\t\t Receiver antenna path" << endl;
    cerr << "    --rxlpf\t\t Receiver low pass filter bandwidth in Hz" << endl;
    cerr << "    --rxoversample\t Receiver decimation 1,2,4,8..." << endl;
    cerr << "    --rxtestsignal=[0,1]\t Enables receiver test signal if available" << endl;

    cerr << "    --txen=[0,1]\t\t Enable transmitter" << endl;
    cerr << "    --txlo\t\t Transmitter center frequency in Hz" << endl;
    cerr << "    --txpath\t\t Transmitter antenna path" << endl;
    cerr << "    --txlpf\t\t Transmitter low pass filter bandwidth in Hz" << endl;
    cerr << "    --txoversample\t Transmitter interpolation 1,2,4,8..." << endl;
    cerr << "    --txtestsignal=[0,1]\t Enables transmitter test signal if available" << endl;

    cerr << "    --ini\t Path to LMS7002M .ini configuration file to use as a base" << endl;

    return EXIT_SUCCESS;
}

enum Args {
    HELP = 'h',
    DEVICE = 'd',
    CHIP = 'c',
    LOG = 'l',
    INIT = 'i',

    REFCLK = 200,
    SAMPLERATE,
    RXEN,
    RXLO,
    RXPATH,
    RXLPF,
    RXOVERSAMPLE,
    RXTESTSIGNAL,
    TXEN,
    TXLO,
    TXPATH,
    TXLPF,
    TXOVERSAMPLE,
    TXTESTSIGNAL,
    INIFILE
};

int main(int argc, char** argv)
{
    char* devName = nullptr;
    int moduleId = 0;
    bool initializeBoard = false;
    char* iniFilename = nullptr;

    SDRDevice::SDRConfig config;
    config.channel[0].rx.oversample = 2;
    config.channel[0].tx.oversample = 2;

    static struct option long_options[] = { { "help", no_argument, 0, Args::HELP },
        { "device", required_argument, 0, Args::DEVICE },
        { "chip", required_argument, 0, Args::CHIP },
        { "log", required_argument, 0, Args::LOG },
        { "initialize", no_argument, 0, Args::INIT },
        { "refclk", required_argument, 0, Args::REFCLK },
        { "samplerate", required_argument, 0, Args::SAMPLERATE },
        { "rxen", required_argument, 0, Args::RXEN },
        { "rxlo", required_argument, 0, Args::RXLO },
        { "rxpath", required_argument, 0, Args::RXPATH },
        { "rxlpf", required_argument, 0, Args::RXLPF },
        { "rxoversample", required_argument, 0, Args::RXOVERSAMPLE },
        { "rxtestsignal", required_argument, 0, Args::RXTESTSIGNAL },
        { "txen", required_argument, 0, Args::TXEN },
        { "txlo", required_argument, 0, Args::TXLO },
        { "txpath", required_argument, 0, Args::TXPATH },
        { "txlpf", required_argument, 0, Args::TXLPF },
        { "txoversample", required_argument, 0, Args::TXOVERSAMPLE },
        { "txtestsignal", required_argument, 0, Args::TXTESTSIGNAL },
        { "ini", required_argument, 0, Args::INIFILE },
        { 0, 0, 0, 0 } };

    int long_index = 0;
    int option = 0;
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case Args::HELP:
            return printHelp();
        case Args::DEVICE:
            if (optarg != NULL)
                devName = optarg;
            break;
        case Args::CHIP:
            if (optarg != NULL)
                moduleId = stoi(optarg);
            break;
        case Args::LOG:
            if (optarg != NULL)
            {
                logVerbosity = strToLogLevel(optarg);
            }
            break;
        case Args::INIT:
            initializeBoard = true;
            break;
        case REFCLK:
            config.referenceClockFreq = stof(optarg);
            break;
        case SAMPLERATE:
            config.channel[0].rx.sampleRate = stof(optarg);
            config.channel[0].tx.sampleRate = config.channel[0].rx.sampleRate;
            break;
        case RXEN:
            config.channel[0].rx.enabled = stoi(optarg) != 0;
            break;
        case RXLO:
            config.channel[0].rx.centerFrequency = stof(optarg);
            break;
        case RXPATH:
            config.channel[0].rx.path = stoi(optarg);
            break;
        case RXLPF:
            config.channel[0].rx.lpf = stof(optarg);
            break;
        case RXOVERSAMPLE:
            config.channel[0].rx.oversample = stoi(optarg);
            break;
        case RXTESTSIGNAL:
            config.channel[0].rx.testSignal.enabled = stoi(optarg) != 0;
            break;
        case TXEN:
            config.channel[0].tx.enabled = stoi(optarg) != 0;
            break;
        case TXLO:
            config.channel[0].tx.centerFrequency = stof(optarg);
            break;
        case TXPATH:
            config.channel[0].tx.path = stoi(optarg);
            break;
        case TXLPF:
            config.channel[0].tx.lpf = stof(optarg);
            break;
        case TXOVERSAMPLE:
            config.channel[0].tx.oversample = stoi(optarg);
            break;
        case TXTESTSIGNAL:
            config.channel[0].tx.testSignal.enabled = stoi(optarg) != 0;
            break;
        case INIFILE:
            iniFilename = optarg;
            break;
        }
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

    device->SetMessageLogCallback(LogCallback);
    try
    {
        if (initializeBoard)
            device->Init();
        if (iniFilename)
        {
            std::string configFilepath;
            config.skipDefaults = true;
            std::string cwd(argv[0]);
            const size_t slash0Pos = cwd.find_last_of("/\\");
            if (slash0Pos != std::string::npos)
                cwd.resize(slash0Pos);

            if (iniFilename[0] != '/') // is not global path
            {
                configFilepath = cwd + "/" + iniFilename;
            }
            else
            {
                configFilepath = iniFilename;
            }

            LMS7002M* chip = static_cast<LMS7002M*>(device->GetInternalChip(moduleId));
            if (!chip)
            {
                DeviceRegistry::freeDevice(device);
                cerr << "Failed to get internal chip: " << moduleId << endl;
                return EXIT_FAILURE;
            }
            if (chip->LoadConfig(configFilepath) != OpStatus::SUCCESS)
            {
                cerr << "Error loading file: " << configFilepath << endl;
                return EXIT_FAILURE;
            }

            for (int i = 0; i < device->GetDescriptor().rfSOC[moduleId].channelCount; ++i)
                config.channel[i] = config.channel[0];
        }
        device->Configure(config, moduleId);
    } catch (std::runtime_error& e)
    {
        DeviceRegistry::freeDevice(device);
        cerr << "Config failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    DeviceRegistry::freeDevice(device);

    return EXIT_SUCCESS;
}
