#include "cli/common.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"
#include "limesuite/StreamComposite.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <signal.h>
#include <thread>
#include "kissFFT/kiss_fft.h"
#include <condition_variable>
#include <mutex>
#include <getopt.h>

// #define USE_GNU_PLOT 1
#ifdef USE_GNU_PLOT
    #include "gnuPlotPipe.h"
#endif

using namespace lime;
using namespace std;

std::mutex globalGnuPlotMutex; // Seems multiple plot pipes can't be used concurrently

bool stopProgram(false);
void intHandler(int dummy)
{
    //std::cerr << "Stopping\n";
    stopProgram = true;
}

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
    cerr << "limeTRX [options]" << endl;
    cerr << "    -h, --help\t\t\t This help" << endl;
    cerr << "    -d, --device <name>\t\t\t Specifies which device to use" << endl;
    cerr << "    -c, --chip <indexes>\t\t Specify chip index, or index list for aggregation [0,1...]" << endl;
    cerr << "    -i, --input \"filepath\"\t\t Waveform file for samples transmitting" << endl;
    cerr << "    -o, --output \"filepath\"\t\t Waveform file for received samples" << endl;
    cerr << "    --looptx \t Loop tx samples transmission" << endl;
    cerr << "    -s, --samplesCount\t\t Number of samples to receive" << endl;
    cerr << "    -t, --time\t\t Time duration in milliseconds to receive" << endl;
    cerr << "    -f, --fft\t\t Display Rx FFT plot" << endl;
    cerr << "    --constellation\t\t Display IQ constellation plot" << endl;
    cerr << "    -l, --log\t\t Log verbosity: info, warning, error, verbose, debug" << endl;
    cerr << "    --mimo [channelCount]\t\t use multiple channels" << endl;
    cerr << "    --repeater [delaySamples]\t\t retransmit received samples with a delay" << endl;
    cerr << "    --linkFormat [I16, I12]\t\t Data transfer format" << endl;
    cerr << "    --syncPPS \t\t start sampling on next PPS" << endl;
    cerr << "    --rxSamplesInPacket \t\t number of samples in Rx packet" << endl;
    cerr << "    --txSamplesInPacket \t\t number of samples in Tx packet" << endl;
    cerr << "    --rxPacketsInBatch \t\t number of Rx packets in data transfer" << endl;
    cerr << "    --txPacketsInBatch \t\t number of Tx packets in data transfer" << endl;

    return EXIT_SUCCESS;
}

enum Args {
    HELP = 'h',
    DEVICE = 'd',
    CHIP = 'c',
    INPUTFILE = 'i',
    OUTPUT = 'o',
    SAMPLES_COUNT = 's',
    TIME = 't',
    FFT = 'f',
    CONSTELLATION = 'x',
    LOG = 'l',
    LOOPTX = 'r',
    MIMO = 200,
    REPEATER,
    LINKFORMAT,
    SYNCPPS,
    RXSAMPLESINPACKET,
    TXSAMPLESINPACKET,
    RXPACKETSINBATCH,
    TXPACKETSINBATCH
};

#ifdef USE_GNU_PLOT
/** @brief The fast Fourier transform diagram plotter */
class FFTPlotter
{
  public:
    /**
    @brief Construct a new FFTPlotter object.
    @param sampleRate The sample rate of the transform.
    @param fftSize The amount of samples per transform.
    @param persistent Whether the plot is persistent or not.
   */
    FFTPlotter(float sampleRate, int fftSize, bool persistent)
        : plot(persistent)
        , sampleRate(sampleRate)
        , doWork(false)
    {
        bins.resize(fftSize);
        plot.writef("set xrange[%f:%f]\n set yrange[%i:%i]\n", -sampleRate / 2, sampleRate / 2, -120, 0);
        plot.flush();
    }

    /** @brief Stop the plotter and destroy the FFTPlotter object. */
    ~FFTPlotter() { Stop(); }

    /** @brief Start the plotter. */
    void Start()
    {
        doWork = true;
        plotThread = std::thread(&FFTPlotter::PlotLoop, this);
    }

    /** @brief Stop the plotter. */
    void Stop()
    {
        doWork = false;
        if (plotThread.joinable())
        {
            plotDataReady.notify_one();
            plotThread.join();
        }
    }

    /**
      @brief Set the plot data.
      @param data The new data to plot.
     */
    void SubmitData(const vector<float>& data)
    {
        {
            std::unique_lock<std::mutex> lk(plotLock);
            bins = data;
        }
        plotDataReady.notify_one();
    }

  private:
    /** @brief The plot drawing loop. */
    void PlotLoop()
    {
        std::unique_lock<std::mutex> lk(plotLock);
        while (doWork)
        {
            if (plotDataReady.wait_for(lk, std::chrono::milliseconds(2000)) == std::cv_status::timeout)
            {
                printf("plot timeout\n");
                continue;
            }
            if (!doWork)
                break;

            std::unique_lock<std::mutex> glk(globalGnuPlotMutex);
            plot.write("plot '-' with lines\n");
            const int fftSize = bins.size();
            for (int j = fftSize / 2; j < fftSize; ++j)
                plot.writef("%f %f\n", sampleRate * (j - fftSize) / fftSize, bins[j]);
            for (int j = 0; j < fftSize / 2; ++j)
                plot.writef("%f %f\n", sampleRate * j / fftSize, bins[j]);
            plot.write("e\n");
            plot.flush();
        }
    }

    GNUPlotPipe plot; ///< The GNU Plot object
    std::vector<float> bins; ///< The data storage
    std::condition_variable plotDataReady; ///< Whether the plot data is ready or not
    std::mutex plotLock; ///< The plot lock
    std::thread plotThread; ///< The plotter thread
    float sampleRate; ///< The sample rate of the data
    bool doWork; ///< Whether to continue plotting or not
};

/** @brief The constellation diagram plotter */
class ConstellationPlotter
{
  public:
    /**
    @brief Construct a new Constellation Plotter object.
    @param range The half sidelength of the square to render.
    @param persistent Whether the plot is persistent or not.
   */
    ConstellationPlotter(int range, bool persistent)
        : plot(persistent)
        , doWork(false)
    {
        plot.writef("set size square\n set xrange[%i:%i]\n set yrange[%i:%i]\n", -range, range, -range, range);
        plot.flush();
    }

    /** @brief Stop the thread and destroy the Constellation Plotter object. */
    ~ConstellationPlotter() { Stop(); }

    /** @brief Start the plotter loop. */
    void Start()
    {
        doWork = true;
        plotThread = std::thread(&ConstellationPlotter::PlotLoop, this);
    }

    /** @brief Stop the plotter loop. */
    void Stop()
    {
        doWork = false;
        if (plotThread.joinable())
        {
            plotDataReady.notify_one();
            plotThread.join();
        }
    }

    /**
      @brief Submit data to the plotter.
      @param data The data to submit to the plotter.
     */
    void SubmitData(const vector<complex16_t>& data)
    {
        {
            std::unique_lock<std::mutex> lk(plotLock);
            samples = data;
        }
        plotDataReady.notify_one();
    }

  private:
    /** @brief The plot drawing loop. */
    void PlotLoop()
    {
        std::unique_lock<std::mutex> lk(plotLock);
        while (doWork)
        {
            if (plotDataReady.wait_for(lk, std::chrono::milliseconds(2000)) == std::cv_status::timeout)
            {
                printf("plot timeout\n");
                continue;
            }
            if (!doWork)
                break;

            std::unique_lock<std::mutex> glk(globalGnuPlotMutex);
            plot.write("plot '-' with points\n");
            const int samplesCount = samples.size();
            for (int j = 0; j < samplesCount; ++j)
                plot.writef("%i %i\n", samples[j].i, samples[j].q);
            plot.write("e\n");
            plot.flush();
        }
    }

    GNUPlotPipe plot; ///< The GNU Plot object
    std::vector<complex16_t> samples; ///< The stored samples
    std::condition_variable plotDataReady; ///< Whether the plot data is ready or not
    std::mutex plotLock; ///< The plot lock
    std::thread plotThread; ///< The plotter thread
    bool doWork; ///< Whether to continue plotting or not
};
#endif

static std::vector<int> ParseIntArray(const std::string& str)
{
    std::vector<int> numbers;
    size_t parsed = 0;
    while (parsed < str.length())
    {
        try
        {
            int nr = stoi(&str[parsed]);
            numbers.push_back(nr);
            size_t next = str.find_first_of(',', parsed);
            if (next == string::npos)
                return numbers;
            else
                parsed = next + 1;
        } catch (...)
        {
            return numbers;
        }
    }
    return numbers;
}

int main(int argc, char** argv)
{
    StreamComposite* composite = nullptr;
    char* devName = nullptr;
    char* rxFilename = nullptr;
    char* txFilename = nullptr;
    bool rx = true;
    bool tx = false;
    bool showFFT = false;
#ifdef USE_GNU_PLOT
    bool showConstelation = false;
#endif
    bool loopTx = false;
    int64_t samplesToCollect = 0;
    int64_t workTime = 0;
    std::vector<int> chipIndexes;
    int chipIndex = 0;
    int channelCount = 1;
    bool repeater = false;
    int64_t repeaterDelay = 0;
    bool syncPPS = false;
    int rxSamplesInPacket = 0;
    int txSamplesInPacket = 0;
    int rxPacketsInBatch = 0;
    int txPacketsInBatch = 0;
    bool useComposite = false;

    SDRDevice::StreamConfig::DataFormat linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
    static struct option long_options[] = { { "help", no_argument, 0, Args::HELP },
        { "device", required_argument, 0, Args::DEVICE },
        { "chip", required_argument, 0, Args::CHIP },
        { "input", required_argument, 0, Args::INPUTFILE },
        { "output", required_argument, 0, Args::OUTPUT },
        { "looptx", no_argument, 0, Args::LOOPTX },
        { "samplesCount", required_argument, 0, Args::SAMPLES_COUNT },
        { "time", required_argument, 0, Args::TIME },
        { "fft", no_argument, 0, Args::FFT },
#ifdef USE_GNU_PLOT
        { "constellation", no_argument, 0, Args::CONSTELLATION },
#endif
        { "log", required_argument, 0, Args::LOG },
        { "mimo", optional_argument, 0, Args::MIMO },
        { "repeater", optional_argument, 0, Args::REPEATER },
        { "linkFormat", required_argument, 0, Args::LINKFORMAT },
        { "syncPPS", no_argument, 0, Args::SYNCPPS },
        { "rxSamplesInPacket", required_argument, 0, Args::RXSAMPLESINPACKET },
        { "txSamplesInPacket", required_argument, 0, Args::TXSAMPLESINPACKET },
        { "rxPacketsInBatch", required_argument, 0, Args::RXPACKETSINBATCH },
        { "txPacketsInBatch", required_argument, 0, Args::TXPACKETSINBATCH },
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
                chipIndexes = ParseIntArray(optarg);

            if (chipIndexes.empty())
            {
                cerr << "Invalid chip index" << endl;
                return -1;
            }
            break;
        case Args::OUTPUT:
            if (optarg != NULL)
            {
                rx = true;
                rxFilename = optarg;
            }
            break;
        case Args::LOOPTX:
            loopTx = true;
            break;
        case Args::INPUTFILE:
            if (optarg != NULL)
            {
                tx = true;
                txFilename = optarg;
            }
            break;
        case Args::SAMPLES_COUNT:
            samplesToCollect = stoi(optarg);
            break;
        case Args::TIME:
            workTime = stoi(optarg);
            break;
        case Args::FFT:
            showFFT = true;
            break;
#ifdef USE_GNU_PLOT
        case Args::CONSTELLATION:
            showConstelation = true;
            break;
#endif
        case Args::LOG:
            if (optarg != NULL)
            {
                logVerbosity = strToLogLevel(optarg);
            }
            break;
        case Args::MIMO:
            if (optarg != NULL)
                channelCount = stoi(optarg);
            else
                channelCount = 2;
            break;
        case Args::REPEATER:
            repeater = true;
            tx = true;
            if (optarg != NULL)
            {
                repeaterDelay = stoi(optarg);
            }
            break;
        case Args::LINKFORMAT:
            if (optarg != NULL)
            {
                if (strcmp(optarg, "I16") == 0)
                    linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
                else if (strcmp(optarg, "I12") == 0)
                    linkFormat = SDRDevice::StreamConfig::DataFormat::I12;
                else
                {
                    cerr << "Invalid linkFormat " << optarg << std::endl;
                    return -1;
                }
            }
            break;
        case Args::SYNCPPS:
            syncPPS = true;
            break;
        case Args::RXSAMPLESINPACKET:
            rxSamplesInPacket = optarg != NULL ? stoi(optarg) : 0;
            break;
        case Args::TXSAMPLESINPACKET:
            txSamplesInPacket = optarg != NULL ? stoi(optarg) : 0;
            break;
        case Args::RXPACKETSINBATCH:
            rxPacketsInBatch = optarg != NULL ? stoi(optarg) : 0;
            break;
        case Args::TXPACKETSINBATCH:
            txPacketsInBatch = optarg != NULL ? stoi(optarg) : 0;
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
        // Samples data streaming configuration
        SDRDevice::StreamConfig stream;
        for (int i = 0; rx && i < channelCount; ++i)
        {
            stream.channels.at(TRXDir::Rx).push_back(i);
        }

        for (int i = 0; tx && i < channelCount; ++i)
        {
            stream.channels.at(TRXDir::Tx).push_back(i);
        }

        stream.format = SDRDevice::StreamConfig::DataFormat::I16;
        stream.linkFormat = linkFormat;

        if (syncPPS || rxSamplesInPacket || rxPacketsInBatch || txSamplesInPacket || txPacketsInBatch)
        {
            stream.extraConfig.waitPPS = syncPPS;
            stream.extraConfig.rx.samplesInPacket = rxSamplesInPacket;
            stream.extraConfig.tx.samplesInPacket = txSamplesInPacket;
            stream.extraConfig.rx.packetsInBatch = rxPacketsInBatch;
            stream.extraConfig.tx.packetsInBatch = txPacketsInBatch;
        }

        useComposite = chipIndexes.size() > 1;
        if (useComposite)
        {
            std::vector<StreamAggregate> aggregates(chipIndexes.size());
            for (size_t i = 0; i < chipIndexes.size(); ++i)
            {
                aggregates[i].device = device;
                aggregates[i].streamIndex = chipIndexes[i];
                int deviceChannelCount = device->GetDescriptor().rfSOC[chipIndexes[i]].channelCount;
                for (int j = 0; j < deviceChannelCount; ++j)
                    aggregates[i].channels.push_back(j);
            }
            composite = new StreamComposite(std::move(aggregates));
            composite->StreamSetup(stream);
        }
        else
        {
            chipIndex = chipIndexes.empty() ? 0 : chipIndexes[0];
            device->StreamSetup(stream, chipIndex);
        }
    } catch (std::runtime_error& e)
    {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        return -1;
    } catch (std::logic_error& e)
    {
        std::cout << "Failed to configure settings: " << e.what() << std::endl;
        return -1;
    }

    signal(SIGINT, intHandler);

    const int fftSize = 16384;
    std::vector<complex16_t> rxData[16];
    for (int i = 0; i < 16; ++i)
        rxData[i].resize(fftSize);

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
        inputFile.seekg(0, std::ios_base::end);
        auto cnt = inputFile.tellg();
        inputFile.seekg(0, std::ios_base::beg);
        cerr << "File size : " << cnt << " bytes." << endl;
        txData.resize(cnt / sizeof(complex16_t));
        inputFile.read((char*)txData.data(), cnt);
        inputFile.close();
    }

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
    float sampleRate = device->GetSampleRate(chipIndex, TRXDir::Rx, 0);
    if (sampleRate <= 0)
        sampleRate = 1; // sample rate readback not available, assign default value
    float frequencyLO = 0;

#ifdef USE_GNU_PLOT
    bool persistPlotWindows = false;
    int range = 32768;
    ConstellationPlotter constellationplot(range, persistPlotWindows);
    FFTPlotter fftplot(sampleRate, fftSize, persistPlotWindows);
#endif

    SDRDevice::StreamMeta rxMeta;
    SDRDevice::StreamMeta txMeta;
    txMeta.waitForTimestamp = true;
    txMeta.timestamp = sampleRate / 100; // send tx samples 10ms after start

#ifdef USE_GNU_PLOT
    if (showFFT)
        fftplot.Start();
    if (showConstelation)
        constellationplot.Start();
#endif
    if (useComposite)
        composite->StreamStart();
    else
        device->StreamStart(chipIndex);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto t1 = startTime - std::chrono::seconds(2); // rewind t1 to do update on first loop
    auto t2 = t1;

    while (!stopProgram)
    {
        if (workTime != 0 && (std::chrono::high_resolution_clock::now() - startTime) > std::chrono::milliseconds(workTime))
            break;
        if (samplesToCollect != 0 && totalSamplesReceived > samplesToCollect)
            break;

        int toSend = txData.size() - txSent > fftSize ? fftSize : txData.size() - txSent;
        if (tx && !repeater)
        {
            if (loopTx && toSend == 0)
            {
                txSent = 0;
                toSend = txData.size() - txSent > fftSize ? fftSize : txData.size() - txSent;
            }
            if (toSend > 0)
            {
                const complex16_t* txSamples[16];
                for (int i = 0; i < 16; ++i)
                    txSamples[i] = &txData[txSent];
                uint32_t samplesSent = useComposite ? composite->StreamTx(txSamples, toSend, &txMeta)
                                                    : device->StreamTx(chipIndex, txSamples, toSend, &txMeta);
                if (samplesSent > 0)
                {
                    txSent += samplesSent;
                    txMeta.timestamp += samplesSent;
                }
            }
        }

        complex16_t* rxSamples[16];
        for (int i = 0; i < 16; ++i)
            rxSamples[i] = rxData[i].data();
        uint32_t samplesRead = useComposite ? composite->StreamRx(rxSamples, fftSize, &rxMeta)
                                            : device->StreamRx(chipIndex, rxSamples, fftSize, &rxMeta);
        if (samplesRead == 0)
            continue;

        if (tx && repeater)
        {
            txMeta.timestamp = rxMeta.timestamp + samplesRead + repeaterDelay;
            txMeta.waitForTimestamp = true;
            txMeta.flushPartialPacket = true;
            if (useComposite)
                composite->StreamTx(rxSamples, samplesRead, &txMeta);
            else
                device->StreamTx(chipIndex, rxSamples, samplesRead, &txMeta);
        }

        // process samples
        totalSamplesReceived += samplesRead;
        if (rxFilename)
        {
            rxFile.write((char*)rxSamples[0], samplesRead * sizeof(lime::complex16_t));
        }

        t2 = std::chrono::high_resolution_clock::now();
        const bool doUpdate = t2 - t1 > std::chrono::milliseconds(500);
        if (doUpdate)
            t1 = t2;

        if (doUpdate)
        {
            if (showFFT)
            {
                for (unsigned i = 0; i < fftSize; ++i)
                {
                    m_fftCalcIn[i].r = rxSamples[0][i].i / 32768.0;
                    m_fftCalcIn[i].i = rxSamples[0][i].q / 32768.0;
                }
                kiss_fft(m_fftCalcPlan, (kiss_fft_cpx*)&m_fftCalcIn, (kiss_fft_cpx*)&m_fftCalcOut);
                for (unsigned int i = 0; i < fftSize; ++i)
                {
                    float amplitude =
                        ((m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i) / (fftSize * fftSize));

                    float output = amplitude > 0 ? 10 * log10(amplitude) : -150;
                    fftBins[i] = output;
                    // exlude DC from amplitude comparison, the 0 bin
                    if (output > peakAmplitude && i > 0)
                    {
                        peakAmplitude = output;
                        peakFrequency = i * sampleRate / fftSize;
                    }
                }
                if (peakFrequency > sampleRate / 2)
                    peakFrequency = peakFrequency - sampleRate;

                printf("Samples received: %li, Peak amplitude %.2f dBFS @ %.3f MHz\n",
                    totalSamplesReceived,
                    peakAmplitude,
                    (frequencyLO + peakFrequency) / 1e6);
                peakAmplitude = -1000;
#ifdef USE_GNU_PLOT
                fftplot.SubmitData(fftBins);
#endif
            }
            else
            {
                printf("Samples received: %li\n", totalSamplesReceived);
            }
        }

#ifdef USE_GNU_PLOT
        if (showConstelation && doUpdate)
            constellationplot.SubmitData(rxData[0]);
#endif
    }
#ifdef USE_GNU_PLOT
    // some sleep for GNU plot data to flush, otherwise sometimes cout spams  gnuplot "invalid command"
    this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
    if (useComposite)
        composite->StreamStop();
    else
        device->StreamStop(chipIndex);

    if (composite)
        delete composite;
    DeviceRegistry::freeDevice(device);

    rxFile.close();
    return 0;
}
