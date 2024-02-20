#include "commonTypes.h"

namespace lime {

const char* ToCString(TRXDir dir)
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