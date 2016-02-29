#include "lms7002_pnlCalibrations_view.h"
#include "lms7002_gui_utilities.h"
#include "LMS7002M.h"
#include <wx/msgdlg.h>
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include <wx/busyinfo.h>
lms7002_pnlCalibrations_view::lms7002_pnlCalibrations_view( wxWindow* parent )
:
pnlCalibrations_view( parent )
{

}

lms7002_pnlCalibrations_view::lms7002_pnlCalibrations_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : pnlCalibrations_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[cmbIQCORR_TXTSP] = IQCORR_TXTSP;
    wndId2Enum[cmbDCCORRI_TXTSP] = DCCORRI_TXTSP;
    wndId2Enum[cmbDCCORRQ_TXTSP] = DCCORRQ_TXTSP;
    wndId2Enum[cmbGCORRI_TXTSP] = GCORRI_TXTSP;
    wndId2Enum[cmbGCORRQ_TXTSP] = GCORRQ_TXTSP;
    wndId2Enum[cmbGCORRI_RXTSP] = GCORRI_RXTSP;
    wndId2Enum[cmbGCORRQ_RXTSP] = GCORRQ_RXTSP;
    wndId2Enum[cmbIQCORR_RXTSP] = IQCORR_RXTSP;
    wndId2Enum[chkEN_DCOFF_RXFE_RFE] = EN_DCOFF_RXFE_RFE;
    wndId2Enum[cmbDCOFFI_RFE] = DCOFFI_RFE;
    wndId2Enum[cmbDCOFFQ_RFE] = DCOFFQ_RFE;

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateRx(wxCommandEvent& event)
{
    lmsControl->EnableCalibrationByMCU(true);
    double bandwidth = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth);
    liblms7_status status;
    {
        wxBusyInfo wait("Please wait, calibrating receiver...");
        status = lmsControl->CalibrateRx(bandwidth);
    }
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Rx calibration: %s"), wxString::From8BitData(liblms7_status2string(status))));
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
    lmsControl->EnableCalibrationByMCU(true);
    double bandwidth = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth);
    liblms7_status status;
    {
        wxBusyInfo wait("Please wait, calibrating transmitter...");
        status = lmsControl->CalibrateTx(bandwidth);
    }
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Tx calibration: %s"), wxString::From8BitData(liblms7_status2string(status))));
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
    lmsControl->EnableCalibrationByMCU(true);
    double bandwidth = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth);    
    liblms7_status status;
    {
        wxBusyInfo wait("Please wait, calibrating transmitter...");
        status = lmsControl->CalibrateTx(bandwidth);
    }
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Tx calibration: %s"), wxString::From8BitData(liblms7_status2string(status))));
    {
        status = lmsControl->CalibrateRx(bandwidth);
        wxBusyInfo wait("Please wait, calibrating receiver...");
    }
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Rx calibration: %s"), wxString::From8BitData(liblms7_status2string(status))));

    wxMessageBox(_("Calibration Finished"), _("Info"), wxOK, this);
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::Initialize(LMS7002M* pControl)
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
    lmsControl->Modify_SPI_Reg_bits(parameter, event.GetInt());
}

void lms7002_pnlCalibrations_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    int16_t value = lmsControl->Get_SPI_Reg_bits(IQCORR_RXTSP);
    int bitsToShift = (15 - IQCORR_RXTSP.msb - IQCORR_RXTSP.lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_RXTSP->SetValue(value);

    value = lmsControl->Get_SPI_Reg_bits(IQCORR_TXTSP);
    bitsToShift = (15 - IQCORR_TXTSP.msb - IQCORR_TXTSP.lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;    
    cmbIQCORR_TXTSP->SetValue(value);

    value = lmsControl->Get_SPI_Reg_bits(DCOFFI_RFE);
    int16_t dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFI_RFE->SetValue(dcvalue);
    value = lmsControl->Get_SPI_Reg_bits(DCOFFQ_RFE);
    dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFQ_RFE->SetValue(dcvalue);

    int8_t dccorr = lmsControl->Get_SPI_Reg_bits(DCCORRI_TXTSP);
    cmbDCCORRI_TXTSP->SetValue(dccorr);
    dccorr = lmsControl->Get_SPI_Reg_bits(DCCORRQ_TXTSP);
    cmbDCCORRQ_TXTSP->SetValue(dccorr);

    lblCGENrefClk->SetLabel(wxString::Format(_("%f"), lmsControl->GetReferenceClk_SX(LMS7002M::Rx)));
}