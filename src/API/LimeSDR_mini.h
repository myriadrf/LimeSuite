/*
 * File:   LimeSDR_mini.h
 * Author: Ignas J
 *
 * Created on September 18, 2017
 */

#ifndef LIMESDR_MINI_H
#define	LIMESDR_MINI_H
#include "lms7_device.h"

namespace lime {

class LMS7_LimeSDR_mini : public LMS7_Device
{
public:
    LMS7_LimeSDR_mini(lime::IConnection* conn, LMS7_Device *obj = nullptr);
    int Init() override;
    unsigned GetNumChannels(const bool tx = false) const override;
    int SetFrequency(bool isTx, unsigned chan, double f_Hz) override;
    Range GetRxPathBand(unsigned path, unsigned chan) const override;
    std::vector<std::string> GetPathNames(bool dir_tx, unsigned chan) const override;
    int SetPath(bool tx, unsigned chan, unsigned path) override;
    std::vector<std::string> GetProgramModes() const override;
private:
};

}

#endif	/* LIMESDR_MINI_H */

