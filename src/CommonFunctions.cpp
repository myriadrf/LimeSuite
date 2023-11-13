#include "CommonFunctions.h"
#include <stdarg.h>
#include <string>

namespace lime {

const std::string strFormat(const char* format, ...)
{
    constexpr uint BUFFER_SIZE = 256;
    char ctemp[BUFFER_SIZE];

    va_list args;

    va_start(args, format);
    vsnprintf(ctemp, BUFFER_SIZE, format, args);
    va_end(args);

    return std::string(ctemp);
}

} // namespace lime