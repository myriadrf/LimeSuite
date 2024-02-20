#include "CommonFunctions.h"
#include <stdarg.h>
#include <string>

namespace lime {

static constexpr int BUFFER_SIZE = 256;

const std::string strFormat(const char* format, ...)
{
    char ctemp[BUFFER_SIZE];

    va_list args;

    va_start(args, format);
    vsnprintf(ctemp, BUFFER_SIZE, format, args);
    va_end(args);

    return std::string(ctemp);
}

std::string ToString(TRXDir dir)
{
    switch (dir)
    {
    case TRXDir::Rx:
        return "Rx";
    case TRXDir::Tx:
        return "Tx";
    }
}

} // namespace lime
