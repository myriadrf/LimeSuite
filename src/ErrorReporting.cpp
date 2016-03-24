/**
@file ErrorReporting.cpp
@author Lime Microsystems
@brief API for reporting error codes and error messages.
*/

#include "ErrorReporting.h"
#include <cstring> //strerror

#ifdef _MSC_VER
//#define thread_local __declspec( thread )
#endif

#define MAX_MSG_LEN 1024
thread_local int _reportedErrorCode;
thread_local char _reportedErrorMessage[MAX_MSG_LEN];

static const char *errToStr(const int err)
{
    thread_local char buff[MAX_MSG_LEN];
    #ifdef _MSC_VER
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buff, sizeof(buff), NULL);
    return buff;
    #else
    //http://linux.die.net/man/3/strerror_r
    #if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE) || __APPLE__
    strerror_r(err, buff, sizeof(buff));
    #else
    //this version may decide to use its own internal string
    return strerror_r(err, buff, sizeof(buff));
    #endif
    return buff;
    #endif
}

int lime::GetLastError(void)
{
    return _reportedErrorCode;
}

const char *lime::GetLastErrorMessage(void)
{
    return _reportedErrorMessage;
}

void lime::ReportError(const int code)
{
    lime::ReportError(code, errToStr(code));
}

void lime::ReportError(const int code, const char *message)
{
    _reportedErrorCode = code;
    strncpy(_reportedErrorMessage, message, MAX_MSG_LEN);
    _reportedErrorMessage[MAX_MSG_LEN-1] = '\0';
}

void lime::ReportError(const int code, const std::string &message)
{
    lime::ReportError(code, message.c_str());
}
