#include "lms7002_pnlXBUF_view.h"
#include "LMS7002M.h"
#include <map>
#include "lms7002_gui_utilities.h"

using namespace lime;

lms7002_pnlXBUF_view::lms7002_pnlXBUF_view( wxWindow* parent )
:
pnlXBUF_view( parent )
{

}

lms7002_pnlXBUF_view::lms7002_pnlXBUF_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlXBUF_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[chkBYP_XBUF_RX] = LMS7param(BYP_XBUF_RX);
    wndId2Enum[chkBYP_XBUF_TX] = LMS7param(BYP_XBUF_TX);
    wndId2Enum[chkEN_G_XBUF] = LMS7param(EN_G_XBUF);
    wndId2Enum[chkEN_OUT2_XBUF_TX] = LMS7param(EN_OUT2_XBUF_TX);
    wndId2Enum[chkEN_TBUFIN_XBUF_RX] = LMS7param(EN_TBUFIN_XBUF_RX);
    wndId2Enum[chkPD_XBUF_RX] = LMS7param(PD_XBUF_RX);
    wndId2Enum[chkPD_XBUF_TX] = LMS7param(PD_XBUF_TX);
    wndId2Enum[chkSLFB_XBUF_RX] = LMS7param(SLFB_XBUF_RX);
    wndId2Enum[chkSLFB_XBUF_TX] = LMS7param(SLFB_XBUF_TX);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlXBUF_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlXBUF_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlXBUF_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
