#ifndef PLL_TEST_H
#define PLL_TEST_H

#include <stdint.h>
#include <functional>

namespace lime
{

class LMS7002M;

struct TestResult
{
    uint32_t successCount;
    uint32_t failureCount;
    void* data;
};

typedef std::function<bool(int testIndex, int testsCount, const TestResult* results)> SweepCallback;

int SweepCGEN_PLL(LMS7002M* lmsControl, const double freqStart, const double freqStep, const double freqEnd, const uint32_t tryCount, SweepCallback stepCallback);
int SweepSX_PLL(LMS7002M* lmsControl, const double freqStart, const double freqStep, const double freqEnd, const uint32_t tryCount, bool Tx, SweepCallback stepCallback);

}
#endif // PLL_TEST_H
