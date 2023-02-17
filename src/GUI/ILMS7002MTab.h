#pragma once

#include "wx/panel.h"
#include <map>

#include "lms7002_gui_utilities.h"
#include "limesuite/LMS7002M_parameters.h"

#include "wx/combobox.h"
#include "wx/checkbox.h"
#include "wx/spinctrl.h"
#include "numericSlider.h"

#include "LMS7002M.h"

class ILMS7002MTab : public wxPanel
{
  public:
    typedef lime::LMS7002M ControllerType;

    ILMS7002MTab(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    virtual void Initialize(ControllerType *pControl);
    virtual void UpdateGUI();
    void SetChannel(uint8_t channel);

  protected:
    int LMS_ReadLMSReg(ControllerType* lms, uint16_t address, uint16_t* value);
    int LMS_WriteLMSReg(ControllerType* lms, uint16_t address, uint16_t value);
    virtual void ParameterChangeHandler(wxCommandEvent &event);
    virtual void SpinParameterChangeHandler(wxSpinEvent &event);

    virtual void WriteParam(const LMS7Parameter &param, uint16_t val);
    virtual int ReadParam(const LMS7Parameter &param);
    int LMS_ReadParam(ControllerType *lmsControl, const LMS7Parameter &param, uint16_t* value);

    ControllerType *lmsControl;
    std::map<wxWindow *, LMS7Parameter> wndId2Enum;
    uint8_t mChannel;
};
