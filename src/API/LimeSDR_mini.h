/*
 * File:   LimeSDR_mini.h
 * Author: Ignas J
 *
 * Created on September 18, 2017
 */

#ifndef LIMESDR_MINI_H
#define	LIMESDR_MINI_H
#include "lms7_device.h"

class LMS7_LimeSDR_mini : public LMS7_Device
{
public:
    LMS7_LimeSDR_mini(LMS7_Device *obj = nullptr);
    int Init() override;
    unsigned GetNumChannels(const bool tx = false) const override;
    int SetRxFrequency(size_t chan, double f_Hz) override;
    int SetTxFrequency(size_t chan, double f_Hz) override;
    lms_range_t GetRxPathBand(size_t path, size_t chan) const override;
    std::vector<std::string> GetPathNames(bool dir_tx, unsigned chan) const override;
    int SetPath(bool tx, unsigned chan, unsigned path) override;
private:
};

#endif	/* LIMESDR_MINI_H */

