/**
@file Logger.cpp
@author Lime Microsystems
@brief API for reporting error codes and error messages.
*/

#include "Logger.h"
#include <cstdio>
#include <cstring> //strerror

#ifdef _MSC_VER
    #define thread_local __declspec(thread)
    #include <Windows.h>
#endif

#ifdef __APPLE__
    #define thread_local __thread
#endif

#define MAX_MSG_LEN 1024
thread_local int _reportedErrorCode;
thread_local char _reportedErrorMessage[MAX_MSG_LEN];

static const char* errToStr(const int errnum)
{
    thread_local static char buff[MAX_MSG_LEN];
#ifdef _MSC_VER
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errnum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&buff,
        sizeof(buff),
        NULL);
    return buff;
#else
    //http://linux.die.net/man/3/strerror_r
    #if !(defined(__GLIBC__) && defined(__GNU_SOURCE))
    auto result = strerror_r(errnum, buff, sizeof(buff));
    (void)result;
    #else
    //this version may decide to use its own internal string
    return strerror_r(errnum, buff, sizeof(buff));
    #endif
    return buff;
#endif
}

const char* lime::GetLastErrorMessage(void)
{
    return _reportedErrorMessage;
}

int lime::ReportError(const int errnum)
{
    return lime::ReportError(errnum, errToStr(errnum));
}

lime::OpStatus lime::ReportError(const lime::OpStatus errnum)
{
    return lime::ReportError(errnum, ToCString(errnum));
}

int lime::ReportError(const int errnum, const char* format, va_list argList)
{
    _reportedErrorCode = errnum;
    vsnprintf(_reportedErrorMessage, MAX_MSG_LEN, format, argList);
    lime::log(LogLevel::ERROR, _reportedErrorMessage);
    return errnum;
}

lime::OpStatus lime::ReportError(const lime::OpStatus errnum, const char* format, va_list argList)
{
    vsnprintf(_reportedErrorMessage, MAX_MSG_LEN, format, argList);
    lime::log(LogLevel::ERROR, _reportedErrorMessage);
    return errnum;
}

static void defaultLogHandler(const lime::LogLevel level, const char* message)
{
    if (level > lime::LogLevel::ERROR)
        return;
    fprintf(stderr, "%s\n", message);
}

static lime::LogHandler logHandler(&defaultLogHandler);

void lime::log(const LogLevel level, const char* format, va_list argList)
{
    char buff[4096];
    int ret = vsnprintf(buff, sizeof(buff), format, argList);
    if (ret > 0)
        logHandler(level, buff);
}

void lime::registerLogHandler(const LogHandler handler)
{
    logHandler = handler ? handler : defaultLogHandler;
}

const char* lime::logLevelToName(const LogLevel level)
{
    switch (level)
    {
    case lime::LogLevel::CRITICAL:
        return "CRITICAL";
    case lime::LogLevel::ERROR:
        return "ERROR";
    case lime::LogLevel::WARNING:
        return "WARNING";
    case lime::LogLevel::INFO:
        return "INFO";
    case lime::LogLevel::DEBUG:
        return "DEBUG";
    }
    return "";
}
