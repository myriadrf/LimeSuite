#ifndef LIME_USBTRANSFERCONTEXT_FX3_H
#define LIME_USBTRANSFERCONTEXT_FX3_H

#include "USBTransferContext.h"

#ifndef __unix__
    #include "windows.h"
    #include "CyAPI.h"
#endif

namespace lime {

class USBTransferContext_FX3 : public USBTransferContext
{
  public:
    USBTransferContext_FX3();

#ifndef __unix__
    ~USBTransferContext_FX3();
#endif

    bool Reset() override;

#ifndef __unix__
    PUCHAR context;
    CCyUSBEndPoint* EndPt;
    OVERLAPPED* inOvLap;
#endif
};

} // namespace lime

#endif // LIME_USBTRANSFERCONTEXT_FX3_H