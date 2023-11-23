#include "USBTransferContext_FX3.h"

using namespace lime;

USBTransferContext_FX3::USBTransferContext_FX3()
    : USBTransferContext()
{
#ifndef __unix__
    inOvLap = new OVERLAPPED;
    memset(inOvLap, 0, sizeof(OVERLAPPED));
    inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
    context = NULL;
    EndPt = nullptr;
#endif
}

#ifndef __unix__
USBTransferContext_FX3::~USBTransferContext_FX3()
{
    CloseHandle(inOvLap->hEvent);
    delete inOvLap;
}
#endif

bool USBTransferContext_FX3::Reset()
{
    if (used)
    {
        return false;
    }
#ifndef __unix__
    CloseHandle(inOvLap->hEvent);
    memset(inOvLap, 0, sizeof(OVERLAPPED));
    inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
#endif
    return true;
}
