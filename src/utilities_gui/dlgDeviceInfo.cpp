#include "dlgDeviceInfo.h"
#include "lmsComms.h"

dlgDeviceInfo::dlgDeviceInfo(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
dlgDeviceInfo_view( parent, id, title, pos, size, styles)
{
    ctrPort = nullptr;
    dataPort = nullptr;
}

void dlgDeviceInfo::Initialize(LMScomms* pCtrPort, LMScomms* pDataPort)
{
    ctrPort = pCtrPort;
    dataPort = pDataPort;
}

void dlgDeviceInfo::OnGetInfo( wxCommandEvent& event )
{    
    if (ctrPort != nullptr && ctrPort->IsOpen() == true)
    {
        auto info = ctrPort->GetDeviceInfo();
        lblDeviceCtr->SetLabel(info.deviceName);
        lblExpansionCtr->SetLabel(info.expansionName);
        lblFirmwareCtr->SetLabel(info.firmwareVersion);
        lblHardwareCtr->SetLabel(info.hardwareVersion);
        lblProtocolCtr->SetLabel(info.protocolVersion);
    }
    else
    {
        lblDeviceCtr->SetLabel("???");
        lblExpansionCtr->SetLabel("???");
        lblFirmwareCtr->SetLabel("???");
        lblHardwareCtr->SetLabel("???");
        lblProtocolCtr->SetLabel("???");
    }

    if (dataPort != nullptr && dataPort->IsOpen() == true)
    {
        auto info = dataPort->GetDeviceInfo();
        lblDeviceData->SetLabel(info.deviceName);
        lblExpansionData->SetLabel(info.expansionName);
        lblFirmwareData->SetLabel(info.firmwareVersion);
        lblHardwareData->SetLabel(info.hardwareVersion);
        lblProtocolData->SetLabel(info.protocolVersion);
    }
    else
    {
        lblDeviceData->SetLabel("???");
        lblExpansionData->SetLabel("???");
        lblFirmwareData->SetLabel("???");
        lblHardwareData->SetLabel("???");
        lblProtocolData->SetLabel("???");
    }
}
