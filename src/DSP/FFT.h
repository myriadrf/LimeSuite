#pragma once

#include "limesuite/complex.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "RingBuffer.h"
#include "../external/kissFFT/kiss_fft.h"

namespace lime {

class FFT
{
  public:
    typedef void (*CallbackType)(const std::vector<float>& bins, void* userData);
    FFT(uint32_t size);
    ~FFT();

    int PushSamples(const complex32f_t* samples, uint32_t count);
    void SetResultsCallback(FFT::CallbackType fptr, void* userData);

    enum class WindowFunctionType { NONE = 0, BLACKMAN_HARRIS, HAMMING, HANNING };
    static void GenerateWindowCoefficients(WindowFunctionType type, uint32_t coefCount, std::vector<float>& coefs);
    static std::vector<float> Calc(const std::vector<complex32f_t>& samples, WindowFunctionType window = WindowFunctionType::NONE);
    static void ConvertToDBFS(std::vector<float>& bins);

  private:
    void Calculate(const complex16_t* src, uint32_t count, std::vector<float>& outputBins);
    void Calculate(const complex32f_t* src, uint32_t count, std::vector<float>& outputBins);
    void ProcessLoop();

    RingBuffer<complex32f_t> samplesFIFO;

    std::thread mWorkerThread;

    kiss_fft_cfg m_fftCalcPlan;
    std::vector<float> mWindowCoefs;
    std::vector<kiss_fft_cpx> m_fftCalcIn;
    std::vector<kiss_fft_cpx> m_fftCalcOut;

    std::atomic<bool> doWork;
    std::condition_variable inputAvailable;
    std::mutex inputMutex;

    CallbackType resultsCallback;
    void* mUserData;

    int avgCount = 100;
};

} // namespace lime