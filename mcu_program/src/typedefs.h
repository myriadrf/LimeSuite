/**
@file	typedefs.h
@author Lime Microsystems (www.limemicro.com)
@brief 	Variables types definitions
*/

#ifndef LMS7002M_TYPEDEFS_H
#define LMS7002M_TYPEDEFS_H

#ifdef PCHOST //for convenience when compiling for PC
#include <stdint.h>
#define code
#define xdata

#else
typedef unsigned long uint32_t;
typedef long int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef bit bool;
#define false 0
#define true 1

#endif

#endif
