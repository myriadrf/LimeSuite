#include "lms7002_pnlCalibrations_view.h"
#include "lms7002_gui_utilities.h"
#include "LMS7002M.h"
#include "ErrorReporting.h"
#include <wx/msgdlg.h>
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include <wx/busyinfo.h>
using namespace lime;

lms7002_pnlCalibrations_view::lms7002_pnlCalibrations_view( wxWindow* parent )
:
pnlCalibrations_view( parent )
{

}

lms7002_pnlCalibrations_view::lms7002_pnlCalibrations_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : pnlCalibrations_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[cmbIQCORR_TXTSP] = LMS7param(IQCORR_TXTSP);
    wndId2Enum[cmbDCCORRI_TXTSP] = LMS7param(DCCORRI_TXTSP);
    wndId2Enum[cmbDCCORRQ_TXTSP] = LMS7param(DCCORRQ_TXTSP);
    wndId2Enum[cmbGCORRI_TXTSP] = LMS7param(GCORRI_TXTSP);
    wndId2Enum[cmbGCORRQ_TXTSP] = LMS7param(GCORRQ_TXTSP);
    wndId2Enum[cmbGCORRI_RXTSP] = LMS7param(GCORRI_RXTSP);
    wndId2Enum[cmbGCORRQ_RXTSP] = LMS7param(GCORRQ_RXTSP);
    wndId2Enum[cmbIQCORR_RXTSP] = LMS7param(IQCORR_RXTSP);
    wndId2Enum[chkEN_DCOFF_RXFE_RFE] = LMS7param(EN_DCOFF_RXFE_RFE);
    wndId2Enum[cmbDCOFFI_RFE] = LMS7param(DCOFFI_RFE);
    wndId2Enum[cmbDCOFFQ_RFE] = LMS7param(DCOFFQ_RFE);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
    chkUseExtLoopback->Hide();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateRx(wxCommandEvent& event)
{
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    int status;
    {
        wxBusyInfo wait("Please wait, calibrating receiver...");
        uint16_t ch;
        LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
        status = LMS_Calibrate(lmsControl,LMS_CH_RX,ch-1,bandwidth_MHz * 1e6,chkUseExtLoopback->IsChecked());
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Rx calibration: %s"), wxString::From8BitData(GetLastErrorMessage())));
    else
    {
        wxMessageBox(_("Rx Calibration Finished"), _("Info"), wxOK, this);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Rx Calibrated"));
        wxPostEvent(this, evt);
    }
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateTx( wxCommandEvent& event )
{
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    int status;
    {
        wxBusyInfo wait("Please wait, calibrating transmitter...");
        uint16_t ch;
        LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
        LMS_Calibrate(lmsControl,LMS_CH_TX,ch-1,bandwidth_MHz * 1e6,chkUseExtLoopback->IsChecked());
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Tx calibration: %s"), wxString::From8BitData(GetLastErrorMessage())));
    else
    {
        wxMessageBox(_("Tx Calibration Finished"), _("Info"), wxOK, this);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Tx Calibrated"));
        wxPostEvent(this, evt);
    }
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateAll( wxCommandEvent& event )
{
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    int status;
    {
        wxBusyInfo wait("Please wait, calibrating transmitter...");
        LMS_Calibrate(lmsControl,LMS_CH_TX,ch-1,bandwidth_MHz * 1e6,chkUseExtLoopback->IsChecked());
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Tx calibration: %s"), wxString::From8BitData(GetLastErrorMessage())));
    {
        wxBusyInfo wait("Please wait, calibrating receiver...");
        LMS_Calibrate(lmsControl,LMS_CH_TX,ch-1,bandwidth_MHz * 1e6,chkUseExtLoopback->IsChecked());
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Rx calibration: %s"), wxString::From8BitData(GetLastErrorMessage())));

    wxMessageBox(_("Calibration Finished"), _("Info"), wxOK, this);
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlCalibrations_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlCalibrations_view::ParameterChangeHandler(wxCommandEvent& event)
{
    assert(lmsControl != nullptr);
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }
    if(LMS7ParameterCompare(parameter,LMS7param(DCOFFI_RFE))==0 || LMS7ParameterCompare(parameter,LMS7param(DCOFFQ_RFE))==0)
    {
        int16_t value = (event.GetInt() < 0) << 6;
        value |= abs(event.GetInt()) & 0x2F;
        LMS_WriteParam(lmsControl,parameter,value);
    }
    else
        LMS_WriteParam(lmsControl,parameter,event.GetInt());
}

void lms7002_pnlCalibrations_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    int16_t value;
    LMS_ReadParam(lmsControl,LMS7param(IQCORR_RXTSP),(uint16_t*)&value);
    int bitsToShift = (15 - LMS7param(IQCORR_RXTSP).msb - LMS7param(IQCORR_RXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_RXTSP->SetValue(value);

    LMS_ReadParam(lmsControl,LMS7param(IQCORR_TXTSP),(uint16_t*)&value);
    bitsToShift = (15 - LMS7param(IQCORR_TXTSP).msb - LMS7param(IQCORR_TXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_TXTSP->SetValue(value);

    LMS_ReadParam(lmsControl,LMS7param(DCOFFI_RFE),(uint16_t*)&value);
    int16_t dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFI_RFE->SetValue(dcvalue);
    LMS_ReadParam(lmsControl,LMS7param(DCOFFQ_RFE),(uint16_t*)&value);
    dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFQ_RFE->SetValue(dcvalue);

    int8_t dccorr;
    LMS_ReadParam(lmsControl,LMS7param(DCCORRI_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRI_TXTSP->SetValue(dccorr);
    LMS_ReadParam(lmsControl,LMS7param(DCCORRQ_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRQ_TXTSP->SetValue(dccorr);
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblCGENrefClk->SetLabel(wxString::Format(_("%f"), freq));
}
