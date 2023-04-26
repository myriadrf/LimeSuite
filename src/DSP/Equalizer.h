#pragma once

#include <stdint.h>
#include <string.h>
#include "Register.h"

namespace lime {

class ISPI;

class Equalizer
{
public:
    struct Config
    {
        struct CFR // (Crest Factor Reduction) controls
        {
            bool bypass;
            bool sleep;
            bool bypassGain;
            uint8_t order;
            uint8_t interpolation;
            uint16_t threshold;
            uint16_t thresholdGain;
        } cfr[2];
        struct FIR // post-CFR FIR
        {
            bool sleep;
            bool bypass;
            int16_t coefficients[32];
            uint8_t coefficientsCount;
        } fir[2];
        bool bypassRxEqualizer[2];
        bool bypassTxEqualizer[2];

        Config() {
            memset(this, 0, sizeof(Config));
        }
    };

    Equalizer(ISPI* comms, uint32_t spiBusAddr);
    ~Equalizer();
    void Configure(const Equalizer::Config &cfg);

    void SetOversample(uint8_t oversample);
    uint8_t GetOversample();
private:
    ISPI *m_Comms;
    uint32_t mSPIbusAddr;
    void WriteRegister(const Register &reg, uint16_t value);
    uint16_t ReadRegister(const Register &reg);

    void SetFIRCoefficients(const int16_t *coefficients, uint16_t count);
    void UpdateHannCoeff(uint16_t Filt_N);
};

}
