#ifndef LIME_USBTRANSFERCONTEXT_FT601_H
#define LIME_USBTRANSFERCONTEXT_FT601_H

#include "USBTransferContext.h"

#ifndef __unix__
    #include "windows.h"
    #include "FTD3XXLibrary/FTD3XX.h"
#endif

namespace lime {

class USBTransferContext_FT601 : public USBTransferContext
{
  public:
    USBTransferContext_FT601();

#ifndef __unix__
    PUCHAR context;
    OVERLAPPED inOvLap;
    uint8_t endPointAddr;
#endif
};

} // namespace lime

#endif // LIME_USBTRANSFERCONTEXT_FT601_H