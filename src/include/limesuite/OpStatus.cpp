#include "OpStatus.h"

namespace lime {

const char* ToCString(OpStatus value)
{
    switch(value)
    {
    case OpStatus::SUCCESS: return "success";
    case OpStatus::ERROR: return "error";
    case OpStatus::NOT_IMPLEMENTED: return "not implemented";
    case OpStatus::IO_FAILURE: return "input/output failure";
    case OpStatus::INVALID_VALUE: return "invalid value";
    case OpStatus::FILE_NOT_FOUND: return "file not found";
    case OpStatus::OUT_OF_RANGE: return "value out of range";
    case OpStatus::NOT_SUPPORTED: return "not supported";
    case OpStatus::TIMEOUT: return "timeout";
    case OpStatus::BUSY: return "busy";
    }
}

} // namespace lime
