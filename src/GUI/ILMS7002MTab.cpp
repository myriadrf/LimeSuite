#include "ILMS7002MTab.h"

#include "lms7002_gui_utilities.h"

#include "SDRDevice.h"
#include "LimeSuite.h"

using namespace lime;

ILMS7002MTab::ILMS7002MTab(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                           long style)
    : wxPanel(parent, id, pos, size, style), mChannel(0)
{
}

void ILMS7002MTab::Initialize(lime::SDRDevice *pControl)
{
    lmsControl = pControl;
}

void ILMS7002MTab::UpdateGUI()
{
    if (lmsControl == nullptr)
        return;
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum, mChannel);
}

void ILMS7002MTab::ParameterChangeHandler(wxCommandEvent &event)
{
    assert(lmsControl != nullptr);
    LMS7Parameter parameter;
    try {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow *>(event.GetEventObject()));
    }
    catch (std::exception &e) {
        std::cout << "Control element(ID = " << event.GetId()
                  << ") don't have assigned LMS parameter." << std::endl;
        return;
    }
    lmsControl->WriteParam(parameter, event.GetInt(), mChannel);
}

void ILMS7002MTab::SpinParameterChangeHandler(wxSpinEvent &event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void ILMS7002MTab::SetChannel(uint8_t channel)
{
    mChannel = channel;
}

void ILMS7002MTab::WriteParam(const LMS7Parameter &param, uint16_t val)
{
    lmsControl->WriteParam(param, val, mChannel);
}

int ILMS7002MTab::ReadParam(const LMS7Parameter &param)
{
    return lmsControl->ReadParam(param, mChannel);
}
