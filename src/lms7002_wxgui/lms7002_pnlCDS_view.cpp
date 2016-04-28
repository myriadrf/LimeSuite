#include "lms7002_pnlCDS_view.h"
#include "LMS7002M.h"
#include <map>
#include "lms7002_gui_utilities.h"

using namespace lime;

lms7002_pnlCDS_view::lms7002_pnlCDS_view( wxWindow* parent )
:
pnlCDS_view( parent )
{

}

lms7002_pnlCDS_view::lms7002_pnlCDS_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlCDS_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkCDSN_MCLK1] = LMS7param(CDSN_MCLK1);
    wndId2Enum[chkCDSN_MCLK2] = LMS7param(CDSN_MCLK2);
    wndId2Enum[chkCDSN_RXALML] = LMS7param(CDSN_RXALML);
    wndId2Enum[chkCDSN_RXATSP] = LMS7param(CDSN_RXATSP);
    wndId2Enum[chkCDSN_RXBLML] = LMS7param(CDSN_RXBLML);
    wndId2Enum[chkCDSN_RXBTSP] = LMS7param(CDSN_RXBTSP);
    wndId2Enum[chkCDSN_TXALML] = LMS7param(CDSN_TXALML);
    wndId2Enum[chkCDSN_TXATSP] = LMS7param(CDSN_TXATSP);
    wndId2Enum[chkCDSN_TXBLML] = LMS7param(CDSN_TXBLML);
    wndId2Enum[chkCDSN_TXBTSP] = LMS7param(CDSN_TXBTSP);
    wndId2Enum[rgrCDS_MCLK1] = LMS7param(CDS_MCLK1);
    wndId2Enum[rgrCDS_MCLK2] = LMS7param(CDS_MCLK2);
    wndId2Enum[rgrCDS_RXALML] = LMS7param(CDS_RXALML);
    wndId2Enum[rgrCDS_RXATSP] = LMS7param(CDS_RXATSP);
    wndId2Enum[rgrCDS_RXBLML] = LMS7param(CDS_RXBLML);
    wndId2Enum[rgrCDS_RXBTSP] = LMS7param(CDS_RXBTSP);
    wndId2Enum[rgrCDS_TXALML] = LMS7param(CDS_TXALML);
    wndId2Enum[rgrCDS_TXATSP] = LMS7param(CDS_TXATSP);
    wndId2Enum[rgrCDS_TXBLML] = LMS7param(CDS_TXBLML);
    wndId2Enum[rgrCDS_TXBTSP] = LMS7param(CDS_TXBTSP);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlCDS_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlCDS_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlCDS_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
