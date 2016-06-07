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

    if ((info = LMS_GetDeviceInfo(lmsControl))!=nullptr)
    {
        lblDeviceCtr->SetLabel(info->deviceName);
        lblExpansionCtr->SetLabel(info->expansionName);
        lblFirmwareCtr->SetLabel(info->firmwareVersion);
        lblHardwareCtr->SetLabel(info->hardwareVersion);
        lblProtocolCtr->SetLabel(info->protocolVersion);
        lblGatewareCtr->SetLabel(info->gatewareVersion);
        lblGatewareRevCtr->SetLabel(info->gatewareRevision);
        lblGatewareTargetCtr->SetLabel(info->gatewareTargetBoard);

        lblDeviceData->SetLabel(info->deviceName);
        lblExpansionData->SetLabel(info->expansionName);
        lblFirmwareData->SetLabel(info->firmwareVersion);
        lblHardwareData->SetLabel(info->hardwareVersion);
        lblProtocolData->SetLabel(info->protocolVersion);
        lblGatewareData->SetLabel(info->gatewareVersion);
        lblGatewareRevData->SetLabel(info->gatewareRevision);
        lblGatewareTargetData->SetLabel(info->gatewareTargetBoard);
    }
    else
    {
        lblDeviceCtr->SetLabel(_("???"));
        lblExpansionCtr->SetLabel(_("???"));
        lblFirmwareCtr->SetLabel(_("???"));
        lblHardwareCtr->SetLabel(_("???"));
        lblProtocolCtr->SetLabel(_("???"));
        lblGatewareCtr->SetLabel(_("???"));
        lblGatewareRevCtr->SetLabel(_("???"));
        lblGatewareTargetCtr->SetLabel(_("???"));

        lblDeviceData->SetLabel(_("???"));
        lblExpansionData->SetLabel(_("???"));
        lblFirmwareData->SetLabel(_("???"));
        lblHardwareData->SetLabel(_("???"));
        lblProtocolData->SetLabel(_("???"));
        lblGatewareData->SetLabel(_("???"));
        lblGatewareRevData->SetLabel(_("???"));
        lblGatewareTargetData->SetLabel(_("???"));
    }


}
