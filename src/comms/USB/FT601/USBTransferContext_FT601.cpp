#include "USBTransferContext_FT601.h"

using namespace lime;

USBTransferContext_FT601::USBTransferContext_FT601()
    : USBTransferContext()
#ifndef __unix__
    , inOvLap(new OVERLAPPED())
    , endPointAddr(0)
#endif
{
#ifndef __unix__
    context = NULL;
#endif
}
