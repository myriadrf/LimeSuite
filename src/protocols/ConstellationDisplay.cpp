#include "ConstellationDisplay.h"

#include "gnuPlotPipe.h"
#include "kiss_fft.h"
#include <iostream>
std::mutex plotMutex;

enum plotType
{
    Constellation,
    FFT,
    TIME
};

static const plotType type = Constellation;

Constellation::Constellation(const char* title) : mFifo(), stopProcessing(false),
    isTx(false), name(title)
{
    mPlot = new GNUPlotPipe(false);

    mPlot->writef("set title '%s'\n", title);
    mPlot->write("set grid xtics ytics\n");
    const int axisrange = 33000;
    if(type == plotType::TIME)
    {
        mPlot->writef("set yrange [-%i:%i]\n", axisrange, axisrange);
    }
    else if(type == plotType::Constellation)
    {
        mPlot->writef("set xrange [-%i:%i]\n", axisrange, axisrange);
        mPlot->writef("set yrange [-%i:%i]\n", axisrange, axisrange);
    }
    else if(type == FFT)
    {
        mPlot->write("set yrange [-200:0]\n");
        mPlot->write("set yrange [-100:0]\n");
        mPlot->write("set style fill transparent solid 0.04 noborder\n");
        mPlot->write("set style line 1 lc rgb 'black' pt 7\n");
    }
}

Constellation::~Constellation()
{
    delete mPlot;
}

void Constellation::Draw(const RFData &data)
{
    src = data;

    std::lock_guard<std::mutex> lock(plotMutex);
    if(type == plotType::TIME)
    {
        mPlot->writef("set title '%s: ts-%li'\n", name.c_str(), data.ts);
        mPlot->write("plot '-' u 1:2 with lines ls 1, '-' u 1:2 with lines ls 1\n");

        for(int i=0; i<data.samples.size(); ++i)
            mPlot->writef("%li %i\n", data.ts+i, data.samples[i].i);
        mPlot->write("e\n");
        for(int i=0; i<data.samples.size(); ++i)
            mPlot->writef("%li %i\n", data.ts+i, data.samples[i].q);
    }
    else if(type == plotType::Constellation)
    {
        mPlot->write("plot '-' u 1:2 with points ls 1\n");

        for(int i=0; i<data.samples.size(); ++i)
            mPlot->writef("%i %i\n", data.samples[i].i, data.samples[i].q);
    }
    else if(type == FFT)
    {
        mPlot->write("plot '-' u 1:2 with lines\n");

        for(int i=0; i<data.samples.size(); ++i)
            mPlot->writef("%i %f\n", i, data.fftBins[i]);
    }

    mPlot->write("e\n");
    mPlot->flush();
}

void Constellation::Start()
{
    stopProcessing.store(false);
    workThread = std::thread(ProcessData, this);
}

void Constellation::Stop()
{
    stopProcessing.store(true);
    workThread.join();
    mFifo.Clear();
}

void Constellation::ProcessData(Constellation* pthis)
{
    printf("constellation start\n");
    double dbOffset = true ? 93.319 : 69.2369;
    const int fftSize = 510;//16384/2;
    const int avgCount = 250;//250*4;

    RFData localDataResults;
    //localDataResults.nyquist_Hz = 7.68e6;

    localDataResults.samples.resize(fftSize);
    localDataResults.fftBins.resize(fftSize);

    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];

    int fftCounter = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    while (pthis->stopProcessing.load() == false)
    {
        do
        {
            uint64_t timestamp = 0;
            int samplesPopped = pthis->mFifo.pop_samples(localDataResults.samples.data(), fftSize, &timestamp, 1000);
            localDataResults.ts = timestamp;
            //printf("pop %i\n", samplesPopped);
            //reset fftBins for accumulation
            if(type == FFT)
            {
                if(fftCounter==0)
                {
                    memset(localDataResults.fftBins.data(), 0, sizeof(double) * localDataResults.fftBins.size());


                    //memset(localDataResults.samples.data(), 0, sizeof(lime::complex16_t) * fftSize);
                    //localDataResults.fftBins.clear();
                    //localDataResults.samples.clear();
                }
                for (unsigned i = 0; i < fftSize; ++i)
                {
                    m_fftCalcIn[i].r = localDataResults.samples[i].i;
                    m_fftCalcIn[i].i = localDataResults.samples[i].q;
                }

                kiss_fft(m_fftCalcPlan, m_fftCalcIn, m_fftCalcOut);

                int output_index = 0;
                for (unsigned i = fftSize / 2 + 1; i < fftSize; ++i)
                    localDataResults.fftBins[output_index++] +=
                        pow((m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i) / (fftSize*fftSize), 2);
                for (unsigned i = 0; i < fftSize / 2 + 1; ++i)
                    localDataResults.fftBins[output_index++] +=
                        pow((m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i) / (fftSize*fftSize), 2);
            }
            else if(type == plotType::Constellation)
            {

            }
        } while (++fftCounter < avgCount && pthis->stopProcessing.load() == false);

        if (fftCounter >= avgCount)
        {
            //normalize fft
            if(type == FFT)
            {
                for (unsigned s = 0; s < fftSize; ++s)
                {
                    const double div = (double)fftCounter;//*fftSize*fftSize;
                    //localDataResults.fftBins[s] /= div;
                    localDataResults.fftBins[s] = sqrt(localDataResults.fftBins[s] / div);
                    localDataResults.fftBins[s] = (localDataResults.fftBins[s] != 0 ? (10 * log10(localDataResults.fftBins[s])) - dbOffset : -100);
                }
            }

            fftCounter = 0;
            t2 = std::chrono::high_resolution_clock::now();
            auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            if (timePeriod >= 1000 && !pthis->isTx)
            {
                t1 = t2;
                //printf("DRAW %s\n", pthis->name.c_str());
                pthis->Draw(localDataResults);
            }
        }
    }
    kiss_fft_free(m_fftCalcPlan);
    delete [] m_fftCalcIn;
    delete [] m_fftCalcOut;
}
