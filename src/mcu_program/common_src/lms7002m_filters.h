#ifndef LMS7002M_FILTERS_TUNING_H
#define LMS7002M_FILTERS_TUNING_H

#include "typedefs.h"

uint8_t TuneTxFilter(const float_type bandwidth);
uint8_t TuneRxFilter(const float_type rx_lpf_freq_RF);

#endif
