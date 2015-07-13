#ifndef LMS7002M_HPM7_WXGUI_H
#define LMS7002M_HPM7_WXGUI_H

/**
@file HPM7_wxgui.h
@author Lime Microsystems
*/

#include <vector>
#include <wx/frame.h>
class wxStaticText;
class wxFlexGridSizer;
class wxButton;
class wxStaticBoxSizer;
class wxComboBox;
class wxCheckBox;

#include <vector>
class LMScomms;

class HPM7_wxgui : public wxFrame
{
public:
    HPM7_wxgui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long styles = 0);
    virtual void Initialize(LMScomms* serPort);
    virtual ~HPM7_wxgui();

protected:
    std::vector<long> tunerIds;
    std::vector<wxCheckBox*> chkEB;
    std::vector<wxCheckBox*> chkTP;
    std::vector<wxComboBox*> cmbSSC1;
    std::vector<wxComboBox*> cmbSSC2;    
    wxButton* btnUpdateAll;
    std::vector<wxCheckBox*> chkGPIO;
    wxComboBox* cmbDAC_A;
    wxComboBox* cmbDAC_B;

private:    
    void DownloadAll(wxCommandEvent& event);
    void OnGPIOchange(wxCommandEvent& event);
    void OnTunerSSC1change(wxCommandEvent& event);    
    void OnTunerSSC2change(wxCommandEvent& event);    
    void OnDACchange(wxCommandEvent& event);
    std::vector<long> chEBids;
    std::vector<long> chTPids;
    std::vector<long> cmbSSC1ids;
    std::vector<long> cmbSSC2ids;

protected:
    LMScomms* m_serPort;

    DECLARE_EVENT_TABLE()
};

#endif
