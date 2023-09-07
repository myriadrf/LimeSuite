#include "cli/common.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <signal.h>
#include <thread>
#include "kissFFT/kiss_fft.h"
#define USE_GNU_PLOT 1
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

using namespace lime;
using namespace std;

bool stopProgram(false);
void intHandler(int dummy) {
    //std::cerr << "Stoppping\n";
    stopProgram = true;
}

static SDRDevice::LogLevel logVerbosity = SDRDevice::ERROR;
static SDRDevice::LogLevel strToLogLevel(const char* str)
{
    if (strstr("debug", str))
        return SDRDevice::DEBUG;
    else if (strstr("verbose", str))
        return SDRDevice::VERBOSE;
    else if (strstr("error", str))
        return SDRDevice::ERROR;
    else if (strstr("warning", str))
        return SDRDevice::WARNING;
    else if (strstr("info", str))
        return SDRDevice::INFO;
    return SDRDevice::ERROR;
}
static void LogCallback(SDRDevice::LogLevel lvl, const char* msg)
{
    if (lvl > logVerbosity)
        return;
    printf("%s\n", msg);
}

static int printHelp(void)
{
    cerr << "limeSPI [options]" << endl;
    cerr << "    -h, --help\t\t\t This help" << endl;
    cerr << "    -d, --device <name>\t\t\t Specifies which device to use" << endl;
    cerr << "    -c, --chip <name>\t\t Selects destination chip" << endl;
    cerr << "    -i, --input \"filepath\"\t\t Waveform file for samples transmitting" << endl;
    cerr << "    -o, --output \"filepath\"\t\t Waveform file for received samples" << endl;
    cerr << "    -s, --samplesCount\t\t Number of samples to receive" << endl;
    cerr << "    -t, --time\t\t Time duration in milliseconds to receive" << endl;
    cerr << "    -f, --fft\t\t Display Rx FFT plot" << endl;
    cerr << "    --contellation\t\t Display IQ constellation plot" << endl;
    cerr << "    -l, --log\t\t Log verbosity: info, warning, error, verbose, debug" << endl;

    return EXIT_SUCCESS;
}

enum Args
{
    HELP = 'h',
    DEVICE = 'd',
    CHIP = 'c',
    INPUT = 'i',
    OUTPUT = 'o',
    SAMPLES_COUNT = 's',
    TIME = 't',
    FFT = 'f',
    CONSTELLATION = 'x',
    LOG = 'l'
};

int main(int argc, char** argv)
{
    char* devName = nullptr;
    char* rxFilename = nullptr;
    char* txFilename = nullptr;
    bool rx = true;
    bool tx = false;
    bool showFFT = false;
    bool showConstelation = false;
    int64_t samplesToCollect = 0;
    int64_t workTime = 0;
    int chipIndex = 0;
    static struct option long_options[] = {
        {"help", no_argument, 0, Args::HELP},
        {"device", required_argument, 0, Args::DEVICE},
        {"chip", required_argument, 0, Args::CHIP},
        {"input", required_argument, 0, Args::INPUT},
        {"output", required_argument, 0, Args::OUTPUT},
        {"samplesCount", required_argument, 0, Args::SAMPLES_COUNT},
        {"time", required_argument, 0, Args::TIME},
        {"fft", no_argument, 0, Args::FFT},
        {"constellation", no_argument, 0, Args::CONSTELLATION},
        {"log", required_argument, 0, Args::LOG},
        {0, 0, 0,  0}
    };

    int long_index = 0;
    int option = 0;
    while ((option = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (option)
        {
        case Args::HELP:
            return printHelp();
        case Args::DEVICE:
            if (optarg != NULL) devName = optarg;
            break;
        case Args::CHIP:
            if (optarg != NULL) chipIndex = stoi(optarg);
            break;
        case Args::OUTPUT:
            if (optarg != NULL) {rx = true; rxFilename = optarg; }
            break;
        case Args::INPUT:
            if (optarg != NULL) {tx = true; txFilename = optarg; }
            break;
        case Args::FFT: showFFT = true; break;
        case Args::CONSTELLATION: showConstelation = true; break;
        case Args::LOG:
            if (optarg != NULL) {logVerbosity = strToLogLevel(optarg); }
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
    //device->Init();

    try {
        // Samples data streaming configuration
        SDRDevice::StreamConfig stream;
        stream.rxCount = rx ? 1 : 0; // rx channels count
        stream.rxChannels[0] = 0;
        stream.txCount = tx ? 1 : 0;

        stream.format = SDRDevice::StreamConfig::DataFormat::I16;
        stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
        device->StreamSetup(stream, chipIndex);
        device->StreamStart(chipIndex);
    }
    catch ( std::runtime_error &e) {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        return -1;
    }
    catch ( std::logic_error &e) {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        return -1;
    }

    signal(SIGINT, intHandler);

    const int fftSize = 16384;
    complex16_t** rxSamples = new complex16_t*[2]; // allocate two channels for simplicity
    for (int i=0; i<2; ++i)
        rxSamples[i] = new complex16_t[fftSize];

    std::vector<complex16_t> txData;
    int64_t txSent = 0;
    if (tx && txFilename)
    {
        std::ifstream inputFile;
        inputFile.open(txFilename, std::ifstream::in | std::ifstream::binary);
        if (!inputFile)
        {
            cerr << "Failed to open file: " << txFilename << endl;
            return -1;
        }
        inputFile.seekg (0,std::ios_base::end);
        auto cnt = inputFile.tellg();
        inputFile.seekg (0,std::ios_base::beg);
        cerr << "File size : " << cnt << " bytes." << endl;
        txData.resize(cnt/sizeof(complex16_t));
        inputFile.read((char*)txData.data(), cnt);
        inputFile.close();
    }
    const complex16_t* txSamples[2] = {txData.data(), txData.data()};

    auto startTime = std::chrono::high_resolution_clock::now();
    auto t1 = startTime - std::chrono::seconds(2); // rewind t1 to do update on first loop
    auto t2 = t1;


    int64_t totalSamplesReceived = 0;

    std::vector<float> fftBins(fftSize);
    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx m_fftCalcIn[fftSize];
    kiss_fft_cpx m_fftCalcOut[fftSize];
    fftBins[0] = 0;

    std::ofstream rxFile;
    if (rxFilename)
    {
        std::cout << "Rx data to file: " << rxFilename << std::endl;
        rxFile.open(rxFilename, std::ofstream::out | std::ofstream::binary);
    }

    float peakAmplitude = 0;
    float peakFrequency = 0;
    float sampleRate = device->GetSampleRate(chipIndex, TRXDir::Rx);
    float frequencyLO = 0;

#ifdef USE_GNU_PLOT
    bool persistPlotWindows = false;
    GNUPlotPipe gp(persistPlotWindows);
    int range = 32768;
    gp.writef("set size square\n set xrange[%i:%i]\n set yrange[%i:%i]\n", -range, range, -range, range);

    GNUPlotPipe fftplot(persistPlotWindows);
    fftplot.writef("set xrange[%f:%f]\n set yrange[%i:%i]\n", -sampleRate/2, sampleRate/2, -120, 0);
#endif

    SDRDevice::StreamMeta rxMeta;
    SDRDevice::StreamMeta txMeta;
    txMeta.useTimestamp = true;
    while (!stopProgram)
    {
        if (workTime != 0 && std::chrono::high_resolution_clock::now() - startTime < std::chrono::milliseconds(workTime))
            break;
        if (samplesToCollect != 0 && totalSamplesReceived > samplesToCollect)
            break;

        int toSend = txData.size()-txSent > fftSize ? fftSize : txData.size()-txSent;
        if (tx && toSend > 0)
        {
            int samplesSent = device->StreamTx(chipIndex, txSamples, toSend, &txMeta);
            txSent += samplesSent;
            txMeta.timestamp += samplesSent;
        }

        int samplesRead = device->StreamRx(chipIndex, rxSamples, fftSize, &rxMeta);
        if (samplesRead <= 0)
            continue;


        // process samples
        totalSamplesReceived += samplesRead;
        if (rxFilename)
        {
            rxFile.write((char*)rxSamples[0], samplesRead*sizeof(lime::complex16_t));
        }

        t2 = std::chrono::high_resolution_clock::now();
        const bool doUpdate = t2-t1 > std::chrono::milliseconds(500);
        if (doUpdate)
            t1 = t2;

        if (doUpdate)
        {
            for (unsigned i = 0; i < fftSize; ++i)
            {
                m_fftCalcIn[i].r = rxSamples[0][i].i/32768.0;
                m_fftCalcIn[i].i = rxSamples[0][i].q/32768.0;
            }
            kiss_fft(m_fftCalcPlan, (kiss_fft_cpx*)&m_fftCalcIn, (kiss_fft_cpx*)&m_fftCalcOut);
            for (unsigned int i = 1; i<fftSize; ++i)
            {
                float amplitude = ((m_fftCalcOut[i].r*m_fftCalcOut[i].r + m_fftCalcOut[i].i*m_fftCalcOut[i].i)/(fftSize*fftSize));

                float output = amplitude > 0 ? 10*log10(amplitude) : -150;
                fftBins[i] = output;
                if (output > peakAmplitude)
                {
                    peakAmplitude = output;
                    peakFrequency = i * sampleRate / fftSize;
                }
            }
            if (peakFrequency > sampleRate / 2)
                peakFrequency = peakFrequency - sampleRate;

            printf("Samples received: %li, Peak amplitude %.2f dBFS @ %.3f MHz\n",
                totalSamplesReceived, peakAmplitude, (frequencyLO+peakFrequency)/1e6);
            peakAmplitude = -1000;
            if (showFFT)
            {
    #ifdef USE_GNU_PLOT
                fftplot.write("plot '-' with lines\n");
                const float halfSampleRate = sampleRate;
                for (int j = fftSize/2; j < fftSize; ++j)
                    fftplot.writef("%f %f\n", halfSampleRate*(j-fftSize)/fftSize, fftBins[j]);
                for (int j = 0; j < fftSize/2; ++j)
                    fftplot.writef("%f %f\n", halfSampleRate*j/fftSize, fftBins[j]);
                fftplot.write("e\n");
                fftplot.flush();
    #endif
            }
        }

        if (showConstelation && doUpdate)
        {
#ifdef USE_GNU_PLOT
            gp.write("plot '-' with points\n");
            for (int j = 0; j < samplesRead; ++j)
                gp.writef("%i %i\n", rxSamples[0][j].i, rxSamples[0][j].q);
            gp.write("e\n");
            gp.flush();
#endif
        }
    }
#ifdef USE_GNU_PLOT
    // some sleep for GNU plot data to flush, otherwise sometimes cout spams  gnuplot "invalid command"
    this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
    DeviceRegistry::freeDevice(device);

    rxFile.close();

    for (int i=0; i<2; ++i)
        delete[] rxSamples[i];
    delete[] rxSamples;
    return 0;
}
