/**
@file	LMS7002M_statuses.h
@author Lime Microsystems (www.limemicro.com)
@brief 	LMS7002M control library statuses enumerations
*/

#ifndef LMS7API_STATUSES_H
#define LMS7API_STATUSES_H

const char liblms7_status_strings[][64] =
{
"success",
"failure",
"index out of range",
"too many values",
"connection manager is NULL",
"port not connected",
"frequency out of range",
"cannot deliver frequency",
"VCO is powered down",
"Bad SEL_PATH_RFE",
"Band not selected",
"file not found",
"file invalid format",
};

typedef int liblms7_status;

#define LIBLMS7_SUCCESS 0
#define LIBLMS7_FAILURE 1
#define LIBLMS7_INDEX_OUT_OF_RANGE 2
#define LIBLMS7_TOO_MANY_VALUES 3
#define LIBLMS7_NO_CONNECTION_MANAGER 4
#define LIBLMS7_NOT_CONNECTED 5
//#define LIBLMS7_FREQUENCY_OUT_OF_RANGE 6
#define LIBLMS7_CANNOT_DELIVER_FREQUENCY 7
#define LIBLMS7_VCO_IS_POWERED_DOWN 8
#define LIBLMS7_BAD_SEL_PATH 9
#define LIBLMS7_BAND_NOT_SELECTED 10
#define LIBLMS7_FILE_NOT_FOUND 11
#define LIBLMS7_FILE_INVALID_FORMAT 12

#define LIBLMS7_STATUS_COUNT 13

#endif
