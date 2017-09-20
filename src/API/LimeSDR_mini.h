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
    size_t GetNumChannels(const bool tx = false) const override;
private:
};

#endif	/* LIMESDR_MINI_H */

