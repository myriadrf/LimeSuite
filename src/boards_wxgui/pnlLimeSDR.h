#ifndef PNL_LIMESDR_H
#define PNL_LIMESDR_H

#include "lime/LimeSuite.h"
#include <vector>
#include <wx/panel.h>
class wxStaticText;
class wxFlexGridSizer;
class wxCheckBox;
class wxStaticBoxSizer;

namespace lime
{
class IConnection;
}

class pnlLimeSDR : public wxPanel
{
public:
    pnlLimeSDR(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t* pControl);
    virtual ~pnlLimeSDR();
    virtual void UpdatePanel();
    void OnGPIOChange(wxCommandEvent &event);
    void OnUsrGPIODirChange(wxCommandEvent &event);
    void OnUsrGPIOChange(wxCommandEvent &event);

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxCheckBox* chkRFLB_A_EN;
    wxCheckBox* chkRFLB_B_EN;
    wxCheckBox* chkTX1_2_LB_SH;
    wxCheckBox* chkTX1_2_LB_AT;
    wxCheckBox* chkTX2_2_LB_SH;
    wxCheckBox* chkTX2_2_LB_AT;
    wxStaticText* gpioIn[16];
    wxCheckBox* gpioDir[16];
    wxCheckBox* gpioOut[16];
    wxFlexGridSizer* controlsSizer;
    wxFlexGridSizer* mainSizer;
    lms_device_t *lmsControl;
    int gpioCnt;
    DECLARE_EVENT_TABLE()
};

#endif
