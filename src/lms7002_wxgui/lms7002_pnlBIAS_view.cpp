#include "lms7002_pnlBIAS_view.h"
#include "LMS7002M.h"
#include <map>
#include "lms7002_gui_utilities.h"

lms7002_pnlBIAS_view::lms7002_pnlBIAS_view( wxWindow* parent )
:
pnlBIAS_view( parent )
{

}

lms7002_pnlBIAS_view::lms7002_pnlBIAS_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlBIAS_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkPD_BIAS_MASTER] = PD_BIAS_MASTER;
    wndId2Enum[cmbMUX_BIAS_OUT] = MUX_BIAS_OUT;
    wndId2Enum[chkPD_FRP_BIAS] = PD_FRP_BIAS;
    wndId2Enum[chkPD_F_BIAS] = PD_F_BIAS;
    wndId2Enum[chkPD_PTRP_BIAS] = PD_PTRP_BIAS;
    wndId2Enum[chkPD_PT_BIAS] = PD_PT_BIAS;
    wndId2Enum[cmbRP_CALIB_BIAS] = RP_CALIB_BIAS;

    wxArrayString temp;
    temp.clear();
    temp.push_back(_("NO test mode"));
    temp.push_back(_("vr_ext_bak and vr_cal_ref=600mV passed to ADC input MUX"));
    temp.push_back(_("BIAS_TOP test outputs will be connected to ADC channel 1 input"));
    temp.push_back(_("RSSI 1 output will be connected to ADC 1 input"));
    cmbMUX_BIAS_OUT->Set(temp);

    temp.clear();
    for (int i = 0; i<32; ++i)
    {
        temp.push_back(wxString::Format(_("%i"), i));
    }
    cmbRP_CALIB_BIAS->Set(temp);
    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlBIAS_view::Initialize(LMS7002M* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlBIAS_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlBIAS_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
