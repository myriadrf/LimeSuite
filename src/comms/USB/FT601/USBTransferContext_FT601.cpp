#include "USBTransferContext_FT601.h"

using namespace lime;

/** @brief Constructs a new USBTransferContext_FT601 object */
USBTransferContext_FT601::USBTransferContext_FT601()
    : USBTransferContext()
{
#ifndef __unix__
    context = NULL;
#endif
}
