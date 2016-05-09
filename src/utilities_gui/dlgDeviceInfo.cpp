#include "dlgDeviceInfo.h"


dlgDeviceInfo::dlgDeviceInfo(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
dlgDeviceInfo_view( parent, id, title, pos, size, styles)
{
    lmsControl = nullptr;

}

void dlgDeviceInfo::Initialize(lms_device_t* lms)
{
    lmsControl = lms;
}

void dlgDeviceInfo::OnGetInfo( wxCommandEvent& event )
{
    const lms_dev_info_t* info;
    
    if ((info = LMS_GetDeviceInfo(lmsControl))==0)
    {
        lblDeviceCtr->SetLabel(info->deviceName);
        lblExpansionCtr->SetLabel(info->expansionName);
        lblFirmwareCtr->SetLabel(info->firmwareVersion);
        lblHardwareCtr->SetLabel(info->hardwareVersion);
        lblProtocolCtr->SetLabel(info->protocolVersion);
        
        lblDeviceData->SetLabel(info->deviceName);
        lblExpansionData->SetLabel(info->expansionName);
        lblFirmwareData->SetLabel(info->firmwareVersion);
        lblHardwareData->SetLabel(info->hardwareVersion);
        lblProtocolData->SetLabel(info->protocolVersion);
    }
    else
    {
        lblDeviceCtr->SetLabel("???");
        lblExpansionCtr->SetLabel("???");
        lblFirmwareCtr->SetLabel("???");
        lblHardwareCtr->SetLabel("???");
        lblProtocolCtr->SetLabel("???");
        
        lblDeviceData->SetLabel("???");
        lblExpansionData->SetLabel("???");
        lblFirmwareData->SetLabel("???");
        lblHardwareData->SetLabel("???");
        lblProtocolData->SetLabel("???");
    }


}