#ifndef PNL_ULIMESDR_H
#define PNL_ULIMESDR_H

#include <map>
#include <wx/panel.h>
class wxStaticText;
class wxFlexGridSizer;
class wxCheckBox;

namespace lime
{
class IConnection;
}

class pnluLimeSDR : public wxPanel
{
public:
    pnluLimeSDR(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lime::IConnection *pControl);
    virtual ~pnluLimeSDR();
    virtual void UpdatePanel();
    void OnLoopbackChange(wxCommandEvent &event);

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxCheckBox* chkRFLB_A_EN;
    wxCheckBox* chkRFLB_B_EN;
    lime::IConnection *mSerPort;
    DECLARE_EVENT_TABLE()
};

#endif
