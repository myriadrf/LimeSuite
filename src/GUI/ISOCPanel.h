#pragma once

#include <wx/panel.h>

class ISOCPanel : public wxPanel
{
  public:
    ISOCPanel(wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long styles = 0)
        : wxPanel(parent, id, pos, size, styles){};
    virtual ~ISOCPanel(){};

    virtual void ChangeSOC(void* socPtr){};
    virtual void UpdateGUI(){};

  private:
    ISOCPanel() = delete;
};
