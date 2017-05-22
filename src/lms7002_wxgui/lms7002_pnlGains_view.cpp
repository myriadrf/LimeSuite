#include "lms7002_pnlGains_view.h"
#include "numericSlider.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;
using namespace LMS7002_WXGUI;

static indexValueMap g_lna_rfe_IndexValuePairs;
static indexValueMap g_tia_rfe_IndexValuePairs;

lms7002_pnlGains_view::lms7002_pnlGains_view( wxWindow* parent )
:
pnlGains_view( parent )
{

}

lms7002_pnlGains_view::lms7002_pnlGains_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlGains_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkTRX_GAIN_SRC] = LMS7param(TRX_GAIN_SRC);
    wndId2Enum[cmbG_PGA_RBB] = LMS7param(G_PGA_RBB_R3);
    wndId2Enum[cmbG_LNA_RFE] = LMS7param(G_LNA_RFE_R3);
    wndId2Enum[cmbG_TIA_RFE] = LMS7param(G_TIA_RFE_R3);
    wndId2Enum[cmbCG_IAMP_TBB] = LMS7param(CG_IAMP_TBB_R3);
    wndId2Enum[cmbLOSS_LIN_TXPAD_TRF] = LMS7param(LOSS_LIN_TXPAD_R3);
    wndId2Enum[cmbLOSS_MAIN_TXPAD_TRF] = LMS7param(LOSS_MAIN_TXPAD_R3);
    wndId2Enum[cmbC_CTL_PGA_RBB] = LMS7param(C_CTL_PGA_RBB_R3);

    wxArrayString temp;

    temp.clear();
    for(int i=0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbLOSS_LIN_TXPAD_TRF->Append(temp);
    cmbLOSS_MAIN_TXPAD_TRF->Append(temp);

    temp.clear();
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i dB"), -12 + i));
    cmbG_PGA_RBB->Set(temp);

    for (int i = 0; i < 15; ++i)
        g_lna_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-30"));
    temp.push_back(_("Gmax-27"));
    temp.push_back(_("Gmax-24"));
    temp.push_back(_("Gmax-21"));
    temp.push_back(_("Gmax-18"));
    temp.push_back(_("Gmax-15"));
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-9"));
    temp.push_back(_("Gmax-6"));
    temp.push_back(_("Gmax-5"));
    temp.push_back(_("Gmax-4"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax-2"));
    temp.push_back(_("Gmax-1"));
    temp.push_back(_("Gmax"));
    cmbG_LNA_RFE->Set(temp);

    for (int i = 0; i < 3; ++i)
        g_tia_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax"));
    cmbG_TIA_RFE->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlGains_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_IsOpen(lmsControl,0) && LMS_ReadParam(lmsControl,LMS7param(MASK),&value)==0 && value == 0)
    {
        chkTRX_GAIN_SRC->Enable(false);
        chkTRX_GAIN_SRC->SetValue(false);
    }
    else
        chkTRX_GAIN_SRC->Enable(true);

}

void lms7002_pnlGains_view::ParameterChangeHandler(wxCommandEvent& event)
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
    long value = event.GetInt();
    if (event.GetEventObject() == cmbG_LNA_RFE)
    {
        value = index2value(value, g_lna_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbG_TIA_RFE)
    {
        value = index2value(value, g_tia_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == chkTRX_GAIN_SRC)
    {
        cmbG_LNA_RFE->Enable(value);
        cmbG_TIA_RFE->Enable(value);
        cmbG_PGA_RBB->Enable(value);
        cmbC_CTL_PGA_RBB->Enable(value);
        cmbLOSS_LIN_TXPAD_TRF->Enable(value);
        cmbLOSS_MAIN_TXPAD_TRF->Enable(value);
        cmbCG_IAMP_TBB->Enable(value);
    }
    LMS_WriteParam(lmsControl,parameter,value);
}

void lms7002_pnlGains_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(G_LNA_RFE_R3),&value);
    cmbG_LNA_RFE->SetSelection( value2index(value, g_lna_rfe_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(G_TIA_RFE_R3),&value);
    cmbG_TIA_RFE->SetSelection( value2index(value, g_tia_rfe_IndexValuePairs));

    value = chkTRX_GAIN_SRC->GetValue();
    cmbG_LNA_RFE->Enable(value);
    cmbG_TIA_RFE->Enable(value);
    cmbG_PGA_RBB->Enable(value);
    cmbC_CTL_PGA_RBB->Enable(value);
    cmbLOSS_LIN_TXPAD_TRF->Enable(value);
    cmbLOSS_MAIN_TXPAD_TRF->Enable(value);
    cmbCG_IAMP_TBB->Enable(value);

}
