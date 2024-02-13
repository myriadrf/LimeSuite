#include "FX3.h"
#include "USBTransferContext_FX3.h"
#include "Logger.h"

using namespace lime;
using namespace std::literals::string_literals;

FX3::FX3(void* usbContext)
    : USBGeneric(usbContext)
{
#ifndef __unix__
    if (usbContext == nullptr)
    {
        USBDevicePrimary = new CCyFX3Device();
    }
    else
    {
        USBDevicePrimary = new CCyFX3Device(*(CCyFX3Device*)usbContext);
    }

    InCtrlEndPt3 = nullptr;
    OutCtrlEndPt3 = nullptr;
    InCtrlBulkEndPt = nullptr;
    OutCtrlBulkEndPt = nullptr;

    for (int i = 0; i < MAX_EP_CNT; ++i)
    {
        InEndPt[i] = OutEndPt[i] = nullptr;
    }
#endif
}

FX3::~FX3()
{
    Disconnect();
}

bool FX3::Connect(uint16_t vid, uint16_t pid, const std::string& serial)
{
    Disconnect();
#ifndef __unix__
    unsigned char index = 0;
    if (index > USBDevicePrimary->DeviceCount())
    {
        return ReportError(ERANGE, "FX3::Connect: Device index out of range");
    }

    if (USBDevicePrimary->Open(index) == false)
    {
        return ReportError(-1, "FX3::Connect: Failed to open device");
    }

    if (InCtrlEndPt3)
    {
        delete InCtrlEndPt3;
        InCtrlEndPt3 = nullptr;
    }
    InCtrlEndPt3 = new CCyControlEndPoint(*USBDevicePrimary->ControlEndPt);

    if (OutCtrlEndPt3)
    {
        delete OutCtrlEndPt3;
        OutCtrlEndPt3 = nullptr;
    }
    OutCtrlEndPt3 = new CCyControlEndPoint(*USBDevicePrimary->ControlEndPt);

    InCtrlEndPt3->ReqCode = CTR_R_REQCODE;
    InCtrlEndPt3->Value = CTR_R_VALUE;
    InCtrlEndPt3->Index = CTR_R_INDEX;
    InCtrlEndPt3->TimeOut = 3000;

    OutCtrlEndPt3->ReqCode = CTR_W_REQCODE;
    OutCtrlEndPt3->Value = CTR_W_VALUE;
    OutCtrlEndPt3->Index = CTR_W_INDEX;
    OutCtrlEndPt3->TimeOut = 3000;

    for (int i = 0; i < USBDevicePrimary->EndPointCount(); ++i)
    {
        auto adr = USBDevicePrimary->EndPoints[i]->Address;
        if (adr < CONTROL_BULK_OUT_ADDRESS)
        {
            OutEndPt[adr] = USBDevicePrimary->EndPoints[i];
            long len = OutEndPt[adr]->MaxPktSize * 64;
            OutEndPt[adr]->SetXferSize(len);
        }
        else if (adr < CONTROL_BULK_IN_ADDRESS)
        {
            adr &= 0xF;
            InEndPt[adr] = USBDevicePrimary->EndPoints[i];
            long len = InEndPt[adr]->MaxPktSize * 64;
            InEndPt[adr]->SetXferSize(len);
        }
    }

    InCtrlBulkEndPt = nullptr;
    for (int i = 0; i < USBDevicePrimary->EndPointCount(); ++i)
    {
        if (USBDevicePrimary->EndPoints[i]->Address == CONTROL_BULK_IN_ADDRESS)
        {
            InCtrlBulkEndPt = USBDevicePrimary->EndPoints[i];
            InCtrlBulkEndPt->TimeOut = 1000;
            break;
        }
    }

    OutCtrlBulkEndPt = nullptr;
    for (int i = 0; i < USBDevicePrimary->EndPointCount(); ++i)
    {
        if (USBDevicePrimary->EndPoints[i]->Address == CONTROL_BULK_OUT_ADDRESS)
        {
            OutCtrlBulkEndPt = USBDevicePrimary->EndPoints[i];
            OutCtrlBulkEndPt->TimeOut = 1000;
            break;
        }
    }

    bool isSuccessful = true;
    isConnected = true;
#else
    bool isSuccessful = USBGeneric::Connect(vid, pid, serial);
#endif
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
#else
    USBDevicePrimary->Close();
    for (int i = 0; i < MAX_EP_CNT; ++i)
    {
        InEndPt[i] = OutEndPt[i] = nullptr;
    }

    InCtrlBulkEndPt = nullptr;
    OutCtrlBulkEndPt = nullptr;

    if (InCtrlEndPt3)
    {
        delete InCtrlEndPt3;
        InCtrlEndPt3 = nullptr;
    }

    if (OutCtrlEndPt3)
    {
        delete OutCtrlEndPt3;
        OutCtrlEndPt3 = nullptr;
    }
#endif
    isConnected = false;
}

bool FX3::IsConnected()
{
#ifndef __unix__
    return USBDevicePrimary->IsOpen() && isConnected;
#else
    return isConnected;
#endif
}

#ifndef __unix__
int32_t FX3::BulkTransfer(uint8_t endPoint, uint8_t* data, int length, int32_t timeout_ms)
{
    switch (endPoint)
    {
    case FX3::CONTROL_BULK_OUT_ADDRESS: // Write
        if (OutCtrlBulkEndPt)
        {
            LONG longLength = static_cast<LONG>(length);
            if (OutCtrlBulkEndPt->XferData(data, longLength))
            {
                return length;
            }
        }
        break;
    case FX3::CONTROL_BULK_IN_ADDRESS: // Read
        if (InCtrlBulkEndPt)
        {
            LONG longLength = static_cast<LONG>(length);

            if (InCtrlBulkEndPt->XferData(data, longLength))
            {
                return length;
            }
        }
        break;
    default:
        throw std::logic_error("Invalid endpoint");
    }
    return 0;
}

int32_t FX3::ControlTransfer(int requestType, int request, int value, int index, uint8_t* data, uint32_t length, int32_t timeout_ms)
{
    switch (requestType)
    {
    case 0: // Write
        if (OutCtrlEndPt3)
        {
            LONG longLength = static_cast<LONG>(length);
            if (OutCtrlEndPt3->Write(data, longLength))
            {
                return length;
            }
        }
        break;
    case 1: // Read
        if (InCtrlEndPt3)
        {
            LONG longLength = static_cast<LONG>(length);

            if (InCtrlEndPt3->Read(data, longLength))
            {
                return length;
            }
        }
    default:
        throw std::logic_error("Invalid request type");
    }
    return 0;
}

int FX3::BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr)
{
    int index = GetUSBContextIndex();

    if (index < 0)
    {
        return -1;
    }

    USBTransferContext_FX3* FX3context = &static_cast<USBTransferContext_FX3*>(contexts)[index];

    switch (endPointAddr)
    {
    case FX3::STREAM_BULK_OUT_ADDRESS: // Tx/Write
        if (OutEndPt[STREAM_BULK_OUT_ADDRESS])
        {
            FX3context->EndPt = OutEndPt[STREAM_BULK_OUT_ADDRESS];
            FX3context->context = FX3context->EndPt->BeginDataXfer(buffer, length, FX3context->inOvLap);
        }

        break;
    case FX3::STREAM_BULK_IN_ADDRESS: // Rx/Read
        if (InEndPt[endPointAddr & 0xF])
        {
            FX3context->EndPt = InEndPt[endPointAddr & 0xF];
            FX3context->context = FX3context->EndPt->BeginDataXfer(buffer, length, FX3context->inOvLap);
        }
        break;
    default:
        throw std::logic_error("Invalid endpoint for an FX3 connection");
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

    bool status = FX3context->EndPt->WaitForXfer(FX3context->inOvLap, timeout_ms);
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

    long len = length;
    bool status = FX3context->EndPt->FinishDataXfer(buffer, len, FX3context->inOvLap, FX3context->context);
    FX3context->used = false;
    FX3context->Reset();

    if (!status)
    {
        return 0;
    }

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

        if (OutEndPt[i] && OutEndPt[i]->Address == endPointAddr)
        {
            OutEndPt[i]->Abort();
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
