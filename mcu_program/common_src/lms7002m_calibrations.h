#ifndef LMS7002M_CALIBRATIONS_H
#define LMS7002M_CALIBRATIONS_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int16_t ReadAnalogDC(const uint16_t addr);
extern uint16_t GetRSSI();
extern void LoadDC_REG_TX_IQ();
extern uint8_t CalibrateTx();
extern uint8_t CalibrateRx();

#ifdef __cplusplus
}
#endif


#endif
