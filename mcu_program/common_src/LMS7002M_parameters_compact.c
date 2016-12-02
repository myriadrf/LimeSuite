#include "LMS7002M_parameters_compact.h"

ROM const LMS7Parameter DCOFFI_RFE = {0x010E, 13<<4 | 7};
ROM const LMS7Parameter DCOFFQ_RFE = {0x010E, 6<<4 | 0};
ROM const LMS7Parameter DCCORRI_TXTSP = {0x0204, 15<<4 | 8};
ROM const LMS7Parameter DCCORRQ_TXTSP = {0x0204, 7<<4 | 0};
ROM const LMS7Parameter IQCORR_TXTSP = {0x0203, 11<<4 | 0};
ROM const LMS7Parameter IQCORR_RXTSP = {0x0403, 11<<4 | 0};
ROM const LMS7Parameter GCORRI_TXTSP = {0x0202, 10<<4 | 0};
ROM const LMS7Parameter GCORRQ_TXTSP = {0x0201, 10<<4 | 0};
ROM const LMS7Parameter GCORRI_RXTSP = {0x0402, 10<<4 | 0};
ROM const LMS7Parameter GCORRQ_RXTSP = {0x0401, 10<<4 | 0};
