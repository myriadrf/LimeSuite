#include "lms7002_pnlTBB_view.h"
#include "LMS7002M.h"
#include "ErrorReporting.h"
#include <map>
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7suiteEvents.h"
using namespace lime;

lms7002_pnlTBB_view::lms7002_pnlTBB_view( wxWindow* parent )
:
pnlTBB_view( parent )
{

}

lms7002_pnlTBB_view::lms7002_pnlTBB_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlTBB_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkBYPLADDER_TBB] = LMS7param(BYPLADDER_TBB);
    wndId2Enum[cmbCCAL_LPFLAD_TBB] = LMS7param(CCAL_LPFLAD_TBB);
    wndId2Enum[cmbCG_IAMP_TBB] = LMS7param(CG_IAMP_TBB);
    wndId2Enum[chkEN_G_TBB] = LMS7param(EN_G_TBB);
    wndId2Enum[cmbICT_IAMP_FRP_TBB] = LMS7param(ICT_IAMP_FRP_TBB);
    wndId2Enum[cmbICT_IAMP_GG_FRP_TBB] = LMS7param(ICT_IAMP_GG_FRP_TBB);
    wndId2Enum[cmbICT_LPFH_F_TBB] = LMS7param(ICT_LPFH_F_TBB);
    wndId2Enum[cmbICT_LPFLAD_F_TBB] = LMS7param(ICT_LPFLAD_F_TBB);
    wndId2Enum[cmbICT_LPFLAD_PT_TBB] = LMS7param(ICT_LPFLAD_PT_TBB);
    wndId2Enum[cmbICT_LPFS5_F_TBB] = LMS7param(ICT_LPFS5_F_TBB);
    wndId2Enum[cmbICT_LPFS5_PT_TBB] = LMS7param(ICT_LPFS5_PT_TBB);
    wndId2Enum[cmbICT_LPF_H_PT_TBB] = LMS7param(ICT_LPF_H_PT_TBB);
    wndId2Enum[cmbLOOPB_TBB] = LMS7param(LOOPB_TBB);
    wndId2Enum[chkPD_LPFH_TBB] = LMS7param(PD_LPFH_TBB);
    wndId2Enum[chkPD_LPFIAMP_TBB] = LMS7param(PD_LPFIAMP_TBB);
    wndId2Enum[chkPD_LPFLAD_TBB] = LMS7param(PD_LPFLAD_TBB);
    wndId2Enum[chkPD_LPFS5_TBB] = LMS7param(PD_LPFS5_TBB);
    wndId2Enum[cmbRCAL_LPFH_TBB] = LMS7param(RCAL_LPFH_TBB);
    wndId2Enum[cmbRCAL_LPFLAD_TBB] = LMS7param(RCAL_LPFLAD_TBB);
    wndId2Enum[cmbRCAL_LPFS5_TBB] = LMS7param(RCAL_LPFS5_TBB);
    wndId2Enum[cmbTSTIN_TBB] = LMS7param(TSTIN_TBB);
    wndId2Enum[chkEN_DIR_TBB] = LMS7param(EN_DIR_TBB);

    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<32; ++i)
    {
        temp.push_back(wxString::Format(_("%i"), i));
    }
    cmbICT_LPFH_F_TBB->Set(temp);
    cmbICT_LPFLAD_F_TBB->Set(temp);
    cmbICT_LPFLAD_PT_TBB->Set(temp);
    cmbICT_LPFS5_F_TBB->Set(temp);
    cmbICT_LPFS5_PT_TBB->Set(temp);
    cmbICT_LPF_H_PT_TBB->Set(temp);

    temp.clear();
    temp.push_back(_("Disabled"));
    temp.push_back(_("DAC current output"));
    temp.push_back(_("LPFLAD ladder output"));
    temp.push_back(_("TBB output"));
    temp.push_back(_("DAC current output (IQ swap)"));
    temp.push_back(_("LPFLAD ladder output (IQ swap)"));
    temp.push_back(_("TBB output (IQ swap)"));
    cmbLOOPB_TBB->Set(temp);

    temp.clear();
    temp.push_back("Disabled");
    temp.push_back("to Highband filter");
    temp.push_back("to Lowband filter");
    temp.push_back("to current amplifier");
    cmbTSTIN_TBB->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlTBB_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlTBB_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlTBB_view::ParameterChangeHandler(wxCommandEvent& event)
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
    LMS_WriteParam(lmsControl,parameter,event.GetInt());
}

void lms7002_pnlTBB_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    //check if B channel is enabled
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&value);
    if (value >= 2)
    {
        LMS_ReadParam(lmsControl,LMS7param(MIMO_SISO),&value);
        if (value != 0)
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
    }
}

void lms7002_pnlTBB_view::OnFilterSelectionChange( wxCommandEvent& event )
{
    txtFilterFrequency2->Disable();
	switch (rgrFilterSelection->GetSelection())
	{
	case 0:
		lblFilterInputName->SetLabelText("High BW (MHz)");
		break;
	case 1:
		lblFilterInputName->SetLabelText("Ladder BW (MHz)");
		txtFilterFrequency2->Enable();
		break;
	}
}

void lms7002_pnlTBB_view::OnbtnTuneFilter( wxCommandEvent& event )
{
    double input1;
    double input2;
    txtFilterFrequency->GetValue().ToDouble(&input1);
    txtFilterFrequency2->GetValue().ToDouble(&input2);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    float_type freq[2];
    int status;
    switch (rgrFilterSelection->GetSelection())
    {
    case 0:
        freq[0] = input1*1e6;
        status = LMS_TuneFilter(lmsControl,ch-1,LMS_TX_LPF_HIGHBAND,freq);
        break;
    case 1:
        freq[0] = input1*1e6;
        freq[1] = input2*1e6;
        status = LMS_TuneFilter(lmsControl,ch-1,LMS_TX_LPF_LOWCHAIN,freq);
        
        break;
    }
    if (status != 0)
    {
        wxMessageBox(wxString(_("Tx Filter tune: ")) + wxString::From8BitData(GetLastErrorMessage()), _("Error"));
    }
    else switch (rgrFilterSelection->GetSelection())
    {
    case 0:
        wxMessageBox(_("Tx High band calibration finished"), _("INFO"));
        break;
    case 1:
        wxMessageBox(_("Tx Low band calibration finished"), _("INFO"));
        break;

    }
    UpdateGUI();
}

void lms7002_pnlTBB_view::OnbtnTuneFilterTest( wxCommandEvent& event )
{
    double input1;
    double input2;
    txtLadderFrequency->GetValue().ToDouble(&input1);
    txtRealpoleFrequency->GetValue().ToDouble(&input2);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    float_type freq;
    int status;
    switch (rgrFilterSelectionTest->GetSelection())
    {
    case 0:
        freq = input1*1e6;
        status = LMS_TuneFilter(lmsControl,ch-1,LMS_TX_LPF_LADDER,&freq);
        break;
    case 1:
        freq = input1*1e6;
        status = LMS_TuneFilter(lmsControl,ch-1,LMS_TX_LPF_REALPOLE,&freq);
        break;
    }
    if (status != 0)
    {
        wxMessageBox(wxString(_("Tx Filter tune: ")) + wxString::From8BitData(GetLastErrorMessage()), _("Error"));
    }
    else switch (rgrFilterSelectionTest->GetSelection())
    {
    case 0:
        {
        wxMessageBox(_("Tx Ladder calibration finished"), _("INFO"));
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Tx Ladder calibrated"));
        wxPostEvent(this, evt);
        break;
        }
    case 1:
        {
        wxMessageBox(_("Tx Realpole calibration finished"), _("INFO"));
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Tx Realpole calibrated"));
        wxPostEvent(this, evt);
        break;
        }
    }
    UpdateGUI();
}
