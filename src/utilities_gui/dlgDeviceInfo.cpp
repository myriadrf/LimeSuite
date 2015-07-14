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
        LMSinfo info = ctrPort->GetInfo();
        lblDeviceCtr->SetLabel(wxString::From8BitData(GetDeviceName(info.device)));
        lblExpansionCtr->SetLabel(wxString::From8BitData(GetExpansionBoardName(info.expansion)));
        lblFirmwareCtr->SetLabel(wxString::Format(_("%i"), info.firmware));
        lblHardwareCtr->SetLabel(wxString::Format(_("%i"), info.hardware));
        lblProtocolCtr->SetLabel(wxString::Format(_("%i"), info.protocol));
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
        LMSinfo info = dataPort->GetInfo();
        lblDeviceData->SetLabel(wxString::From8BitData(GetDeviceName(info.device)));
        lblExpansionData->SetLabel(wxString::From8BitData(GetExpansionBoardName(info.expansion)));
        lblFirmwareData->SetLabel(wxString::Format(_("%i"), info.firmware));
        lblHardwareData->SetLabel(wxString::Format(_("%i"), info.hardware));
        lblProtocolData->SetLabel(wxString::Format(_("%i"), info.protocol));
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
