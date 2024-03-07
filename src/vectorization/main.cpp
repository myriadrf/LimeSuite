#include <chrono>
#include <random>
#include <stdio.h>
#include "samplesConversion.h"
#include "limesuite/complex.h"

using namespace lime;
using namespace std;
using namespace std::chrono;

void TestDeinterleaving()
{
    constexpr int inputSize = 512;
    complex32f_t destB[inputSize];
    complex32f_t destA[inputSize];
    complex16_t src[inputSize];

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(-32768, 32767);

    for (int i = 0; i < inputSize; ++i)
    {
        src[i].real(dist(mt));
        src[i].imag(dist(mt));
    }

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    t1 = chrono::high_resolution_clock::now();
    t2 = t1;
    uint64_t counterSlow = 0;
    while ((t2 - t1) < milliseconds(1000))
    {
        PathSelection(destA, src, inputSize);
        ++counterSlow;
        t2 = chrono::high_resolution_clock::now();
    }
    printf("Rx SISO c16 -> c32f : %.2f MSps\n", inputSize * counterSlow / 1e6);

    t1 = chrono::high_resolution_clock::now();
    t2 = t1;
    counterSlow = 0;
    while ((t2 - t1) < milliseconds(1000))
    {
        PathSelectionUnzip(destA, destB, src, inputSize);
        ++counterSlow;
        t2 = chrono::high_resolution_clock::now();
    }
    printf("Rx MIMO c16 -> c32f : %.2f MSps\n", (inputSize / 2) * counterSlow / 1e6);
}

void TestInterleaving()
{
    constexpr int inputSize = 512;
    complex32f_t srcA[inputSize];
    complex32f_t srcB[inputSize];
    complex16_t dest[inputSize];

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (int i = 0; i < inputSize; ++i)
    {
        srcA[i].real(dist(mt));
        srcA[i].imag(dist(mt));
        srcB[i].real(dist(mt));
        srcB[i].imag(dist(mt));
    }

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    t1 = chrono::high_resolution_clock::now();
    t2 = t1;
    uint64_t counterSlow = 0;
    while ((t2 - t1) < milliseconds(1000))
    {
        PathSelection(dest, srcA, inputSize);
        ++counterSlow;
        t2 = chrono::high_resolution_clock::now();
    }
    printf("Tx SISO c32f -> c16 : %.2f MSps\n", inputSize * counterSlow / 1e6);

    t1 = chrono::high_resolution_clock::now();
    t2 = t1;
    counterSlow = 0;
    while ((t2 - t1) < milliseconds(1000))
    {
        PathSelectionZip(dest, srcA, srcB, inputSize / 2);
        ++counterSlow;
        t2 = chrono::high_resolution_clock::now();
    }
    printf("Tx MIMO c32f -> c16 : %.2f MSps\n", (inputSize / 2) * counterSlow / 1e6);
}

int main(int argc, char** argv)
{
    TestDeinterleaving();
    TestInterleaving();
    return 0;
}