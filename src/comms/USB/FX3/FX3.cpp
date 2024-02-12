#include "FX3.h"
#include "USBTransferContext_FX3.h"
#include "Logger.h"

using namespace lime;
using namespace std::literals::string_literals;

FX3::FX3(void* usbContext)
    : USBGeneric(usbContext)
{
}

FX3::~FX3()
{
    Disconnect();
}

bool FX3::Connect(uint16_t vid, uint16_t pid, const std::string& serial)
{
    Disconnect();
    bool isSuccessful = USBGeneric::Connect(vid, pid, serial);
    if (!isSuccessful)
    {
        return false;
    }

    contexts = new USBTransferContext_FX3[USB_MAX_CONTEXTS];
    return true;
}

void FX3::Disconnect()
{
    USBGeneric::Disconnect();
#ifdef __unix__
    if (dev_handle != nullptr)
    {
        libusb_release_interface(dev_handle, 0);
        libusb_close(dev_handle);
        dev_handle = nullptr;
    }
#endif
    isConnected = false;
}

#ifndef __unix__
int FX3::BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr)
{
    int index = GetUSBContextIndex();

    if (index < 0)
    {
        return -1;
    }

    USBTransferContext_FX3* FX3context = &static_cast<USBTransferContext_FX3*>(contexts)[index];

    if (InEndPt[endPointAddr & 0xF])
    {
        FX3context->EndPt = InEndPt[endPointAddr & 0xF];
        FX3context->context = FX3context->EndPt->BeginDataXfer(buffer, length, FX3context->inOvLap);
    }

    return index;
}

bool FX3::WaitForXfer(int contextHandle, uint32_t timeout_ms)
{
    if (contextHandle < 0)
    {
        return true;
    }

    USBTransferContext_FX3* FX3context = &static_cast<USBTransferContext_FX3*>(contexts)[contextHandle];

    if (!FX3context->used)
    {
        return true; //there is nothing to wait for (signal wait finished)
    }

    int status = 0;
    status = FX3context->EndPt->WaitForXfer(FX3context->inOvLap, timeout_ms);
    return status;
}

int FX3::FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle)
{
    if (contextHandle < 0)
    {
        return true;
    }

    USBTransferContext_FX3* FX3context = &static_cast<USBTransferContext_FX3*>(contexts)[contextHandle];

    if (!FX3context->used)
    {
        return 0;
    }

    int status = 0;
    long len = length;
    status = FX3context->EndPt->FinishDataXfer((unsigned char*)buffer, len, FX3context->inOvLap, FX3context->context);
    FX3context->used = false;
    FX3context->Reset();
    return len;
}

void FX3::AbortEndpointXfers(uint8_t endPointAddr)
{
    for (int i = 0; i < MAX_EP_CNT; i++)
    {
        if (InEndPt[i] && InEndPt[i]->Address == endPointAddr)
        {
            InEndPt[i]->Abort();
        }
    }

    WaitForXfers(endPointAddr);
}
#endif

int FX3::GetUSBContextIndex()
{
    std::unique_lock<std::mutex> lock{ contextsLock };

    USBTransferContext_FX3* FX3contexts = static_cast<USBTransferContext_FX3*>(contexts);

    if (FX3contexts == nullptr)
    {
        return -1;
    }

    int i = 0;
    bool contextFound = false;
    // Find not used context
    for (i = 0; i < USB_MAX_CONTEXTS; i++)
    {
        if (!FX3contexts[i].used)
        {
            contextFound = true;
            break;
        }
    }

    if (!contextFound)
    {
        lime::error("No contexts left for reading or sending data"s);
        return -1;
    }

    FX3contexts[i].used = true;

    return i;
}
