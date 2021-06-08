#ifndef PNL_ULIMESDR_H
#define PNL_ULIMESDR_H

#include "lime/LimeSuite.h"
#include <map>
#include <wx/panel.h>
#include <wx/choice.h>
class wxStaticText;
class wxFlexGridSizer;
class wxCheckBox;

class pnluLimeSDR : public wxPanel
{
public:
    pnluLimeSDR(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t *pControl);
    virtual ~pnluLimeSDR();
    virtual void UpdatePanel();
    void OnLoopbackChange(wxCommandEvent &event);

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxCheckBox* chkTxLBSH;
    wxCheckBox* chkTxLBAT;
    wxChoice* cmbRxPath;
    wxChoice* cmbTxPath;
    wxStaticText* txtLB;
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};

#endif
