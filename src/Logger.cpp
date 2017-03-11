/**
@file Logger.cpp
@author Lime Microsystems
@brief API for reporting error codes and error messages.
*/

#include "Logger.h"
#include <cstdio>

static void defaultLogHandler(const lime::LogLevel level, const char *message)
{
    switch(level)
    {
    case lime::CRITICAL: fprintf(stderr, "[CRITICAL] %s\n", message); return;
    case lime::ERROR: fprintf(stderr, "[ERROR] %s\n", message); return;
    case lime::WARNING: fprintf(stderr, "[WARNING] %s\n", message); return;
    case lime::INFO: fprintf(stderr, "[INFO] %s\n", message); return;
    case lime::DEBUG: fprintf(stderr, "[DEBUG] %s\n", message); return;
    }
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
