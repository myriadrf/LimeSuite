#include "USBTransferContext.h"

using namespace lime;

USBTransferContext::USBTransferContext()
{
#ifdef __unix__
    transfer = libusb_alloc_transfer(0);
    bytesXfered = 0;
    done = 0;
#endif
}

USBTransferContext::~USBTransferContext()
{
#ifdef __unix__
    if (transfer)
    {
        libusb_free_transfer(transfer);
    }
#endif
}

bool USBTransferContext::Reset()
{
    return !used;
}
