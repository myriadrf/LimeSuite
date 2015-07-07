#include "lms7002_pnlCDS_view.h"
#include "LMS7002M.h"
#include <map>
#include "lms7002_gui_utilities.h"

lms7002_pnlCDS_view::lms7002_pnlCDS_view( wxWindow* parent )
:
pnlCDS_view( parent )
{

}

lms7002_pnlCDS_view::lms7002_pnlCDS_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlCDS_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkCDSN_MCLK1] = CDSN_MCLK1;
    wndId2Enum[chkCDSN_MCLK2] = CDSN_MCLK2;
    wndId2Enum[chkCDSN_RXALML] = CDSN_RXALML;
    wndId2Enum[chkCDSN_RXATSP] = CDSN_RXATSP;
    wndId2Enum[chkCDSN_RXBLML] = CDSN_RXBLML;
    wndId2Enum[chkCDSN_RXBTSP] = CDSN_RXBTSP;
    wndId2Enum[chkCDSN_TXALML] = CDSN_TXALML;
    wndId2Enum[chkCDSN_TXATSP] = CDSN_TXATSP;
    wndId2Enum[chkCDSN_TXBLML] = CDSN_TXBLML;
    wndId2Enum[chkCDSN_TXBTSP] = CDSN_TXBTSP;
    wndId2Enum[rgrCDS_MCLK1] = CDS_MCLK1;
    wndId2Enum[rgrCDS_MCLK2] = CDS_MCLK2;
    wndId2Enum[rgrCDS_RXALML] = CDS_RXALML;
    wndId2Enum[rgrCDS_RXATSP] = CDS_RXATSP;
    wndId2Enum[rgrCDS_RXBLML] = CDS_RXBLML;
    wndId2Enum[rgrCDS_RXBTSP] = CDS_RXBTSP;
    wndId2Enum[rgrCDS_TXALML] = CDS_TXALML;
    wndId2Enum[rgrCDS_TXATSP] = CDS_TXATSP;
    wndId2Enum[rgrCDS_TXBLML] = CDS_TXBLML;
    wndId2Enum[rgrCDS_TXBTSP] = CDS_TXBTSP;
}

void lms7002_pnlCDS_view::Initialize(LMS7002M* pControl)
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
    lmsControl->Modify_SPI_Reg_bits(parameter, event.GetInt());
}

void lms7002_pnlCDS_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
