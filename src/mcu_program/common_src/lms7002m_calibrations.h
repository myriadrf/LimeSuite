#ifndef LMS7002M_CALIBRATIONS_H
#define LMS7002M_CALIBRATIONS_H

#include "typedefs.h"
namespace lime {
class LMS7002M;
}

void SetupCalibrations(lime::LMS7002M* chip, double BW);

int16_t ReadAnalogDC(const uint16_t addr);
void UpdateRSSIDelay();
uint32_t GetRSSI();
void LoadDC_REG_TX_IQ();
uint8_t CalibrateTx(bool extLoopback);
uint8_t CalibrateTxExternalLoop();
uint8_t CalibrateRx(bool extLoopback, bool dcOnly);
uint8_t CalibrateRxExternalLoop();
void CalibrateRxDCAuto();
uint8_t RunAGC(uint32_t wantedRSSI);
int16_t clamp(int16_t value, int16_t minBound, int16_t maxBound);

#endif
