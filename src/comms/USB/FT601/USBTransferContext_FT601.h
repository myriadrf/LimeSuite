#ifndef LIME_USBTRANSFERCONTEXT_FT601_H
#define LIME_USBTRANSFERCONTEXT_FT601_H

#include "comms/USB/USBTransferContext.h"

#ifndef __unix__
    #include "FTD3XXLibrary/FTD3XX.h"
#endif

namespace lime {

/** @brief A class for a USB transfer context for devices using the FTDI FT601 USB controller. */
class USBTransferContext_FT601 : public USBTransferContext
{
  public:
    USBTransferContext_FT601();

#ifndef __unix__
    LPOVERLAPPED inOvLap;
    uint8_t endPointAddr;
#endif
};

} // namespace lime

#endif // LIME_USBTRANSFERCONTEXT_FT601_H
