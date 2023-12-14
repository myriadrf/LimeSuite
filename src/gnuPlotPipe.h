#ifndef GNUPLOT_PIPE_H
#define GNUPLOT_PIPE_H

#include <cstdio>
#include <cstdarg>

/// @brief A class to pipe data into GNUPlot
class GNUPlotPipe
{
  public:
    /// @brief Creates the GNUPlot pipe.
    /// @param persistent Whether the pipe is persistent or not.
    GNUPlotPipe(bool persistent = true)
    {
#ifdef __unix__
        if (persistent)
            pipeHandle = popen("gnuplot -persistent -noraise", "we");
        else
            pipeHandle = popen("gnuplot -noraise", "we");
#else
        if (persistent)
            pipeHandle = _popen("gnuplot -persistent", "w");
        else
            pipeHandle = _popen("gnuplot", "w");
#endif
    }

    /// @brief Closes the GNUPlot pipe.
    ~GNUPlotPipe()
    {
        write("exit\n");
#ifdef __unix__
        pclose(pipeHandle);
#else
        _pclose(pipeHandle);
#endif
    }

    /// @brief Writes data into the pipe.
    /// @param str The C-string of data to write.
    void write(const char* str)
    {
        fprintf(pipeHandle, "%s", str);
        //fflush(pipeHandle);
    }

    /// @brief Writes formatted data into the pipe.
    /// @param format The format string.
    /// @param ... The format arguments parameters.
    void writef(const char* format, ...)
    {
        static char temp[512];
        va_list argList;
        va_start(argList, format);
        vsnprintf(temp, 512, format, argList);
        write(temp);
        va_end(argList);
    }

    /// @brief Flushes the pipe.
    void flush() { fflush(pipeHandle); }

  protected:
    FILE* pipeHandle;
};

#endif
