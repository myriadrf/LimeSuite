#pragma once

#include <wx/panel.h>

class wxButton;
class wxChoice;

namespace lime {

class DeviceConnectionPanel : public wxPanel
{
  public:
    DeviceConnectionPanel(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL);
    ~DeviceConnectionPanel();

  protected:
    void SendDisconnectEvent(wxCommandEvent& inEvent);
    void SendHandleChangeEvent(wxCommandEvent& inEvent);

    void EnumerateDevicesToChoice();
    wxChoice* cmbDevHandle;
    wxButton* btnDisconnect;
};

} // namespace lime