/**
@file	LMS7002M_statuses.cpp
@author Lime Microsystems (www.limemicro.com)
*/

#include "LMS7002M_statuses.h"

const char* undefinedStatusStr = "undefined status";

const char* liblms7_status2string(liblms7_status status)
{
    if (status >= 0 && status < LIBLMS7_STATUS_COUNT)
        return liblms7_status_strings[status];
    else
        return undefinedStatusStr;
}
