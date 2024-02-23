#include "FFT.h"

#include <assert.h>
#include <chrono>

namespace lime {

FFT::FFT(uint32_t size)
    : samplesFIFO(size * 128)
{
    m_fftCalcIn.resize(size);
    m_fftCalcOut.resize(size);
    m_fftCalcPlan = kiss_fft_alloc(size, 0, 0, 0);

    doWork.store(true, std::memory_order_relaxed);
    mWorkerThread = std::thread(&FFT::ProcessLoop, this);

    GenerateWindowCoefficients(WindowFunctionType::BLACKMAN_HARRIS, m_fftCalcIn.size(), mWindowCoefs);
}

FFT::~FFT()
{
    doWork.store(false, std::memory_order_relaxed);
    inputAvailable.notify_one();
    if (mWorkerThread.joinable())
        mWorkerThread.join();
    kiss_fft_free(m_fftCalcPlan);
}

int FFT::PushSamples(const complex32f_t* samples, uint32_t count)
{
    int produced = samplesFIFO.Produce(samples, count);
    inputAvailable.notify_one();
    return produced;
}

void FFT::SetResultsCallback(FFT::CallbackType fptr, void* userData)
{
    resultsCallback = fptr;
    mUserData = userData;
}

std::vector<float> FFT::Calc(const std::vector<complex32f_t>& samples, WindowFunctionType window)
{
    const int fftSize = samples.size();
    std::vector<float> coefs;
    GenerateWindowCoefficients(window, fftSize, coefs);

    std::vector<kiss_fft_cpx> fftIn(fftSize);
    std::vector<kiss_fft_cpx> fftOut(fftSize);
    std::vector<float> bins(fftSize);
    kiss_fft_cfg plan = kiss_fft_alloc(fftSize, 0, 0, 0);

    for (int i = 0; i < fftSize; ++i)
    {
        fftIn[i].r = samples[i].i * coefs[i];
        fftIn[i].i = samples[i].q * coefs[i];
    }
    kiss_fft(plan, fftIn.data(), fftOut.data());
    for (int i = 0; i < fftSize; ++i)
    {
        bins[i] = (fftOut[i].r * fftOut[i].r + fftOut[i].i * fftOut[i].i) / (fftSize * fftSize);
    }
    kiss_fft_free(plan);
    return bins;
}

void FFT::ConvertToDBFS(std::vector<float>& bins)
{
    for (float& amplitude : bins)
        amplitude = amplitude > 0 ? 10 * log10(amplitude) : -150;
}

void FFT::Calculate(const complex16_t* src, uint32_t count, std::vector<float>& outputBins)
{
    assert(count == m_fftCalcIn.size());
    for (uint32_t i = 0; i < count; ++i)
    {
        m_fftCalcIn[i].r = src[i].i / 32768.0 * mWindowCoefs[i];
        m_fftCalcIn[i].i = src[i].q / 32768.0 * mWindowCoefs[i];
    }
    kiss_fft(m_fftCalcPlan, m_fftCalcIn.data(), m_fftCalcOut.data());
    outputBins.resize(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        float amplitude = (m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i) / (count * count);
        outputBins[i] = amplitude > 0 ? 10 * log10(amplitude) : -150;
    }
}

void FFT::Calculate(const complex32f_t* src, uint32_t count, std::vector<float>& outputBins)
{
    assert(count == m_fftCalcIn.size());
    for (uint32_t i = 0; i < count; ++i)
    {
        m_fftCalcIn[i].r = src[i].i * mWindowCoefs[i];
        m_fftCalcIn[i].i = src[i].q * mWindowCoefs[i];
    }
    kiss_fft(m_fftCalcPlan, m_fftCalcIn.data(), m_fftCalcOut.data());
    outputBins.resize(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        float amplitude = (m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i) / (count * count);
        outputBins[i] = amplitude > 0 ? 10 * log10(amplitude) : -150;
    }
}

void FFT::ProcessLoop()
{
    std::vector<complex32f_t> samples(m_fftCalcIn.size());
    std::vector<float> fftBins(m_fftCalcOut.size());
    std::vector<float> avgOutput(m_fftCalcOut.size());
    uint32_t samplesReady = 0;

    std::unique_lock<std::mutex> lk(inputMutex);

    int resultsDone = 0;
    while (doWork.load(std::memory_order_relaxed) == true)
    {
        if (inputAvailable.wait_for(lk, std::chrono::milliseconds(2000)) == std::cv_status::timeout)
        {
            printf("plot timeout\n");
            continue;
        }

        int samplesNeeded = samples.size() - samplesReady;
        int ret = samplesFIFO.Consume(samples.data() + samplesReady, samplesNeeded);
        samplesReady += ret;

        if (samplesReady == samples.size())
        {
            // auto t1 = std::chrono::high_resolution_clock::now();
            Calculate(samples.data(), samples.size(), fftBins);
            ++resultsDone;
            samplesReady = 0;

            for (uint32_t i = 0; i < fftBins.size(); ++i)
                avgOutput[i] += fftBins[i] * fftBins[i];

            if (resultsDone == avgCount)
            {
                if (resultsCallback)
                {
                    // to log scale
                    for (size_t i = 0; i < fftBins.size(); ++i)
                        avgOutput[i] = sqrt(avgOutput[i] / avgCount);
                    for (size_t i = 0; i < fftBins.size(); ++i)
                    {
                        fftBins[i] = avgOutput[i] > 0 ? 10 * log10(avgOutput[i]) : -150;
                    }
                    resultsCallback(fftBins, mUserData);
                }
                resultsDone = 0;
                memset(avgOutput.data(), 0, avgOutput.size() * sizeof(float));
            }
            // auto t2 = std::chrono::high_resolution_clock::now();
            // auto timePeriod = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            //printf("FFT update %lius\n", timePeriod);
        }
    }
}

void FFT::GenerateWindowCoefficients(WindowFunctionType func, uint32_t N /*coef count*/, std::vector<float>& windowFcoefs)
{
    float amplitudeCorrection = 1;
    windowFcoefs.resize(N);
    float a0 = 0.35875;
    float a1 = 0.48829;
    float a2 = 0.14128;
    float a3 = 0.01168;
    float PI = 3.14159265359;
    switch (func)
    {
    case WindowFunctionType::BLACKMAN_HARRIS:
        for (uint32_t i = 0; i < N; ++i)
            windowFcoefs[i] =
                a0 - a1 * cos((2 * PI * i) / (N - 1)) + a2 * cos((4 * PI * i) / (N - 1)) - a3 * cos((6 * PI * i) / (N - 1));
        break;
    case WindowFunctionType::HAMMING:
        amplitudeCorrection = 0;
        a0 = 0.54;
        for (uint32_t i = 0; i < N; ++i)
            windowFcoefs[i] = a0 - (1 - a0) * cos((2 * PI * i) / (N));
        break;
    case WindowFunctionType::HANNING:
        amplitudeCorrection = 0;
        for (uint32_t i = 0; i < N; ++i)
            windowFcoefs[i] = 0.5 * (1 - cos((2 * PI * i) / (N)));
        break;
    case WindowFunctionType::NONE:
    default:
        for (uint32_t i = 0; i < N; ++i)
            windowFcoefs[i] = 1;
        return;
    }
    for (uint32_t i = 0; i < N; ++i)
        amplitudeCorrection += windowFcoefs[i];
    amplitudeCorrection = 1.0 / (amplitudeCorrection / N);
    for (uint32_t i = 0; i < N; ++i)
        windowFcoefs[i] *= amplitudeCorrection;
}

} // namespace lime
