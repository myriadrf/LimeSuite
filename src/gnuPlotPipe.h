#ifndef GNUPLOT_PIPE_H
#define GNUPLOT_PIPE_H

#include <iostream>
#include <stdio.h>
#include <cstdio>

class GNUPlotPipe
{
public:
    GNUPlotPipe(bool persistent = true)
    {
        if(persistent)
            pipeHandle = popen("gnuplot -persistent", "w");
        else
            pipeHandle = popen("gnuplot", "w");
    }
    ~GNUPlotPipe()
    {
    	write("exit\n");
        pclose(pipeHandle);
    }

    void write(const char* str)
    {
        fprintf(pipeHandle, "%s", str);
        fflush(pipeHandle);
    }

    void writef(const char* format, ...)
    {
        static char temp[512];
        va_list argList;
        va_start(argList, format);
        vsnprintf(temp, 512, format, argList);
        write(temp);
        va_end(argList);
    }

    void flush()
    {
        fflush(pipeHandle);
    }
protected:
    FILE *pipeHandle;
};

#endif
