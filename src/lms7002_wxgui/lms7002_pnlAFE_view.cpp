#include "lms7002_pnlAFE_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlAFE_view::lms7002_pnlAFE_view( wxWindow* parent )
:
pnlAFE_view( parent )
{

}

lms7002_pnlAFE_view::lms7002_pnlAFE_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlAFE_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkEN_G_AFE] = LMS7param(EN_G_AFE);
    wndId2Enum[cmbISEL_DAC_AFE] = LMS7param(ISEL_DAC_AFE);
    wndId2Enum[rgrMODE_INTERLEAVE_AFE] = LMS7param(MODE_INTERLEAVE_AFE);
    wndId2Enum[cmbMUX_AFE_1] = LMS7param(MUX_AFE_1);
    wndId2Enum[cmbMUX_AFE_2] = LMS7param(MUX_AFE_2);
    wndId2Enum[chkPD_AFE] = LMS7param(PD_AFE);
    wndId2Enum[chkPD_RX_AFE1] = LMS7param(PD_RX_AFE1);
    wndId2Enum[chkPD_RX_AFE2] = LMS7param(PD_RX_AFE2);
    wndId2Enum[chkPD_TX_AFE1] = LMS7param(PD_TX_AFE1);
    wndId2Enum[chkPD_TX_AFE2] = LMS7param(PD_TX_AFE2);

    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<8; ++i)
    {
        temp.push_back(wxString::Format(_("%i uA"), 325 + i * 75)); //nominal 625uA
    }
    cmbISEL_DAC_AFE->Set(temp);

    temp.clear();
    temp.push_back(_("PGA output is connected to ADC input"));
    temp.push_back(_("pdet_1 is connected to ADC ch. 1"));
    temp.push_back(_("BIAS_TOP test outputs will be connected to ADC ch.1 input"));
    temp.push_back(_("RSSI 1 output will be connected to ADC 1 input"));
    cmbMUX_AFE_1->Set(temp);

    temp.clear();
    temp.push_back(_("PGA output is connected to ADC input"));
    temp.push_back(_("pdet_2 is connected to ADC channel 2"));
    temp.push_back(_("RSSI 1 output will be connected to ADC 2 input"));
    temp.push_back(_("RSSI 2 output will be connected to ADC 2 input"));
    cmbMUX_AFE_2->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlAFE_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlAFE_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlAFE_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
