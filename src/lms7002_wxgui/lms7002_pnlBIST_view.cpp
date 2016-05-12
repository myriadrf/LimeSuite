#include "lms7002_pnlBIST_view.h"
#include "LMS7002M.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlBIST_view::lms7002_pnlBIST_view( wxWindow* parent )
:
pnlBIST_view( parent )
{

}

lms7002_pnlBIST_view::lms7002_pnlBIST_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlBIST_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkBENC] = BENC;
    wndId2Enum[chkBENR] = BENR;
    wndId2Enum[chkBENT] = BENT;
    wndId2Enum[chkBSTART] = BSTART;
    wndId2Enum[chkSDM_TSTO_CGEN] = EN_SDM_TSTO_CGEN;
    wndId2Enum[chkSDM_TSTO_SXR] = EN_SDM_TSTO_SXR;
    wndId2Enum[chkSDM_TSTO_SXT] = EN_SDM_TSTO_SXT;
    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlBIST_view::Initialize(LMS7002M* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlBIST_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlBIST_view::onbtnReadSignature( wxCommandEvent& event )
{
    int value;
    value = lmsControl->Get_SPI_Reg_bits(BSIGC, true);
    lblBSIGC->SetLabel(wxString::Format(_("0x%0.6X"), value));
    value = lmsControl->Get_SPI_Reg_bits(BSIGR, true);
    lblBSIGR->SetLabel(wxString::Format(_("0x%0.6X"), value));
    value = lmsControl->Get_SPI_Reg_bits(BSIGT, true);
    lblBSIGT->SetLabel(wxString::Format(_("0x%0.6X"), value));
    value = lmsControl->Get_SPI_Reg_bits(BSTATE, true);
    lblBSTATE->SetLabel(wxString::Format(_("0x%X"), value));
}

void lms7002_pnlBIST_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
