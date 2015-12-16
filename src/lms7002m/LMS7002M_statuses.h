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

enum liblms7_status
{
	LIBLMS7_SUCCESS = 0,
	LIBLMS7_FAILURE,
	LIBLMS7_INDEX_OUT_OF_RANGE,
    LIBLMS7_TOO_MANY_VALUES,
    LIBLMS7_NO_CONNECTION_MANAGER,
    LIBLMS7_NOT_CONNECTED,
    LIBLMS7_FREQUENCY_OUT_OF_RANGE,    
    LIBLMS7_CANNOT_DELIVER_FREQUENCY,
    LIBLMS7_VCO_IS_POWERED_DOWN,
    LIBLMS7_BAD_SEL_PATH,
    LIBLMS7_BAND_NOT_SELECTED,
    LIBLMS7_FILE_NOT_FOUND,
    LIBLMS7_FILE_INVALID_FORMAT,

    LIBLMS7_STATUS_COUNT
};

const char* liblms7_status2string(liblms7_status status);

#endif
