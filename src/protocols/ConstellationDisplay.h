#pragma once

#include <stdint.h>
#include "dataTypes.h"
#include <vector>
#include <thread>

#include "fifo.h"

class GNUPlotPipe;

class Constellation
{
public:
    struct RFData
    {
        uint64_t ts;
        std::vector<lime::complex16_t> samples;
        std::vector<double> fftBins;
    };

    Constellation(const char* title);
    ~Constellation();

    void Draw(const RFData &data);
    lime::RingFIFO mFifo;

    static void ProcessData(Constellation* pthis);
    void Start();
    void Stop();
    bool isTx;

    RFData src;
private:
    std::string name;
    std::atomic_bool stopProcessing;
    std::thread workThread;
    GNUPlotPipe* mPlot;
};