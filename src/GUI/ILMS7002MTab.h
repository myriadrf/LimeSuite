#pragma once

#include "wx/panel.h"
#include <map>

#include "lms7002_gui_utilities.h"
#include "LMS7002M_parameters.h"

#include "wx/combobox.h"
#include "wx/checkbox.h"
#include "wx/spinctrl.h"
#include "numericSlider.h"

namespace lime {
class SDRDevice;
}

class ILMS7002MTab : public wxPanel
{
  public:
    ILMS7002MTab(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    virtual void Initialize(lime::SDRDevice *pControl);
    virtual void UpdateGUI();
    void SetChannel(uint8_t channel);

  protected:
    virtual void ParameterChangeHandler(wxCommandEvent &event);
    virtual void SpinParameterChangeHandler(wxSpinEvent &event);

    virtual void WriteParam(const LMS7Parameter &param, uint16_t val);
    virtual int ReadParam(const LMS7Parameter &param);

    lime::SDRDevice *lmsControl;
    std::map<wxWindow *, LMS7Parameter> wndId2Enum;
    uint8_t mChannel;
};
