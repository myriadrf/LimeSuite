/*
 * File:   LimeSDR.h
 * Author: Ignas J
 *
 * Created on September 18, 2017
 */

#ifndef LIMESDR_H
#define	LIMESDR_H
#include "lms7_device.h"

class LMS7_LimeSDR : public LMS7_Device
{
public:
    LMS7_LimeSDR(lime::IConnection* conn, LMS7_Device *obj = nullptr);
private:
};

#endif	/* LIMESDR_H */

