/**
@file Logger.cpp
@author Lime Microsystems
@brief API for reporting error codes and error messages.
*/

#include "Logger.h"
#include <cstdio>

static void defaultLogHandler(const lime::LogLevel level, const char *message)
{
    fprintf(stderr, "[%s] %s\n", lime::logLevelToName(level), message);
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
    case lime::CRITICAL: return "CRITICAL";
    case lime::ERROR: return "ERROR";
    case lime::WARNING: return "WARNING";
    case lime::INFO: return "INFO";
    case lime::DEBUG: return "DEBUG";
    }
    return "";
}
