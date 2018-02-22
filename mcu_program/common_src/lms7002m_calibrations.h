#ifndef LMS7002M_CALIBRATIONS_H
#define LMS7002M_CALIBRATIONS_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int16_t ReadAnalogDC(const uint16_t addr);
extern void UpdateRSSIDelay();
extern uint16_t GetRSSI();
extern void LoadDC_REG_TX_IQ();
extern uint8_t CalibrateTx(bool extLoopback);
extern uint8_t CalibrateTxExternalLoop();
extern uint8_t CalibrateRx(bool extLoopback);
extern uint8_t CalibrateRxExternalLoop();

#ifdef __cplusplus
}
#endif


#endif
