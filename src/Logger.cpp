/**
@file Logger.cpp
@author Lime Microsystems
@brief API for reporting error codes and error messages.
*/

#include "Logger.h"
#include <cstdio>

static void defaultLogHandler(const lime::LogLevel level, const char *message)
{
#ifdef NDEBUG
    if (level == lime::LOG_LEVEL_DEBUG)
        return;
#endif
    fprintf(stderr, "%s\n", message);
}

static lime::LogHandler logHandler(&defaultLogHandler);

void lime::log(const LogLevel level, const char *format, va_list argList)
{
    char buff[4096];
    int ret = vsnprintf(buff, sizeof(buff), format, argList);
    if (ret > 0) logHandler(level, buff);
}

void lime::registerLogHandler(const LogHandler handler)
{
    logHandler = handler;
}

const char *lime::logLevelToName(const LogLevel level)
{
    switch(level)
    {
    case lime::LOG_LEVEL_CRITICAL: return "CRITICAL";
    case lime::LOG_LEVEL_ERROR: return "ERROR";
    case lime::LOG_LEVEL_WARNING: return "WARNING";
    case lime::LOG_LEVEL_INFO: return "INFO";
    case lime::LOG_LEVEL_DEBUG: return "DEBUG";
    }
    return "";
}
