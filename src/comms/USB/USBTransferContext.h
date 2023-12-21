#ifndef LIME_USBTRANSFERCONTEXT_H
#define LIME_USBTRANSFERCONTEXT_H

#include <atomic>
#include <condition_variable>
#include <mutex>

#ifdef __unix__
    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    #include <libusb.h>
    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif
#endif

namespace lime {

/** @brief Base class for a USB transfer context. */
class USBTransferContext
{
  public:
    explicit USBTransferContext();
    virtual ~USBTransferContext();
    virtual bool Reset();

    bool isTransferUsed; ///< A flag to mark if this transfer is currently being used.

#ifdef __unix__
    libusb_transfer* transfer;
    long bytesXfered;
    std::atomic<bool> done;
    std::mutex transferLock;
    std::condition_variable cv;
#endif
};

} // namespace lime

#endif // LIME_USBTRANSFERCONTEXT_H
