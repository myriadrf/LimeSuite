#include "lms7002_pnlBIST_view.h"
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
    wndId2Enum[chkBENC] = LMS7param(BENC);
    wndId2Enum[chkBENR] = LMS7param(BENR);
    wndId2Enum[chkBENT] = LMS7param(BENT);
    wndId2Enum[chkBSTART] = LMS7param(BSTART);
    wndId2Enum[chkSDM_TSTO_CGEN] = LMS7param(EN_SDM_TSTO_CGEN);
    wndId2Enum[chkSDM_TSTO_SXR] = LMS7param(EN_SDM_TSTO_SXR);
    wndId2Enum[chkSDM_TSTO_SXT] = LMS7param(EN_SDM_TSTO_SXT);
    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlBIST_view::Initialize(lms_device_t* pControl)
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
    LMS_WriteParam(lmsControl,parameter,event.GetInt());
}

void lms7002_pnlBIST_view::onbtnReadSignature( wxCommandEvent& event )
{

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(BSIGC),&value);
    lblBSIGC->SetLabel(wxString::Format(_("0x%0.6X"), value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGR),&value);
    lblBSIGR->SetLabel(wxString::Format(_("0x%0.6X"), value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGT),&value);
    lblBSIGT->SetLabel(wxString::Format(_("0x%0.6X"), value));
    LMS_ReadParam(lmsControl,LMS7param(BSTATE),&value);
    lblBSTATE->SetLabel(wxString::Format(_("0x%X"), value));
}

void lms7002_pnlBIST_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
