#ifndef PNL_LIMESDR_H
#define PNL_LIMESDR_H

#include "lime/LimeSuite.h"
#include <map>
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

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxCheckBox* chkRFLB_A_EN;
    wxCheckBox* chkRFLB_B_EN;
    wxCheckBox* chkTX1_2_LB_SH;
    wxCheckBox* chkTX1_2_LB_AT;
    wxCheckBox* chkTX2_2_LB_SH;
    wxCheckBox* chkTX2_2_LB_AT;
    wxStaticText* lblWarning;
    wxFlexGridSizer* controlsSizer;
    wxStaticBoxSizer *groupSizer;
    wxFlexGridSizer* mainSizer;
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};

#endif
