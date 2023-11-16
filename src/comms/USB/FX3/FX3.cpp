#include "FX3.h"
#include "USBTransferContext_FX3.h"

using namespace lime;

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

    if (InEndPt[endPointAddr & 0xF])
    {
        contexts[index].EndPt = InEndPt[endPointAddr & 0xF];
        contexts[index].context = contexts[index].EndPt->BeginDataXfer(buffer, length, contexts[index].inOvLap);
    }

    return index;
}

bool FX3::WaitForXfer(int contextHandle, uint32_t timeout_ms)
{
    if (contextHandle >= 0 && contexts[contextHandle].used == true)
    {
        int status = 0;
        status = contexts[contextHandle].EndPt->WaitForXfer(contexts[contextHandle].inOvLap, timeout_ms);
        return status;
    }

    return true; //there is nothing to wait for (signal wait finished)
}

int FX3::FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle)
{
    if (contextHandle >= 0 && contexts[contextHandle].used == true)
    {
        int status = 0;
        long len = length;
        status = contexts[contextHandle].EndPt->FinishDataXfer(
            (unsigned char*)buffer, len, contexts[contextHandle].inOvLap, contexts[contextHandle].context);
        contexts[contextHandle].used = false;
        contexts[contextHandle].reset();
        return len;
    }

    return 0;
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
