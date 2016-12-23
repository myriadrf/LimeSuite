#ifndef LMS7002M_CONTROLS_MCU_H
#define LMS7002M_CONTROLS_MCU_H

#define LMS7002M_Rx 0
#define LMS7002M_Tx 1

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void SetNCOFrequency(const bool tx, const float freq_Hz);

extern float_type GetFrequencyCGEN();
extern uint8_t SetFrequencyCGEN(const float_type freq);
extern float_type GetFrequencySX(const bool Tx);
extern uint8_t SetFrequencySX(const bool Tx, const float_type freq);

enum VCO_ID
{
    VCO_CGEN = 0,
    VCO_SXR,
    VCO_SXT
};

extern uint8_t TuneVCO(const uint8_t module); // 0-cgen, 1-SXR, 2-SXT
extern uint16_t pow2(const uint8_t power);
#ifdef __cplusplus
}
#endif

#endif //LMS7002M_CONTROLS_MCU_H
