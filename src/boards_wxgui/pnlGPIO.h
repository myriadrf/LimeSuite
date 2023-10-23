#ifndef PNL_GPIO_H
#define PNL_GPIO_H

#include "limesuite/SDRDevice.h"
#include <wx/panel.h>

class wxStaticText;
class wxCheckBox;

namespace lime
{
class IConnection;
}

class pnlGPIO : public wxPanel
{
public:
    pnlGPIO(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lime::SDRDevice* pControl);
    virtual ~pnlGPIO();
    virtual void UpdatePanel();
    void OnUsrGPIODirChange(wxCommandEvent &event);
    void OnUsrGPIOChange(wxCommandEvent &event);
protected:
    wxStaticText* gpioIn[16];
    wxCheckBox* gpioDir[16];
    wxCheckBox* gpioOut[16];

    lime::SDRDevice *device;
    int gpioCount;
};

#endif
