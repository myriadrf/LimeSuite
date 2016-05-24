#include "pnluLimeSDR.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>

#include <ciso646>

#include <IConnection.h>
#include <ErrorReporting.h>

using namespace lime;
using namespace std;

BEGIN_EVENT_TABLE(pnluLimeSDR, wxPanel)
END_EVENT_TABLE()

pnluLimeSDR::pnluLimeSDR(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    mSerPort = nullptr;

    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 5);

    SetSizer(mainSizer);
    chkRFLB_A_EN = new wxCheckBox(this, wxNewId(), _("chkRFLB_A_EN"));
    Connect(chkRFLB_A_EN->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), NULL, this);
    mainSizer->Add(chkRFLB_A_EN, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkRFLB_B_EN = new wxCheckBox(this, wxNewId(), _("chkRFLB_B_EN"));
    Connect(chkRFLB_B_EN->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), NULL, this);
    mainSizer->Add(chkRFLB_B_EN, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();
}

void pnluLimeSDR::Initialize(IConnection* pControl)
{
    mSerPort = pControl;
}

pnluLimeSDR::~pnluLimeSDR()
{
    chkRFLB_A_EN->Disconnect(wxEVT_CHECKBOX, chkRFLB_A_EN->GetId(), wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), 0, this);
    chkRFLB_B_EN->Disconnect(wxEVT_CHECKBOX, chkRFLB_B_EN->GetId(), wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), 0, this);
}

void pnluLimeSDR::OnLoopbackChange(wxCommandEvent& event)
{   
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    value |= chkRFLB_A_EN->GetValue() << 0;
    value |= chkRFLB_B_EN->GetValue() << 1;
    if(mSerPort && mSerPort->WriteRegister(addr, value))
        wxMessageBox(GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
}

void pnluLimeSDR::UpdatePanel()
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    if(mSerPort && mSerPort->ReadRegister(addr, value))
    {
        wxMessageBox(GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    chkRFLB_A_EN->SetValue((value >> 0) & 0x1);
    chkRFLB_B_EN->SetValue((value >> 1) & 0x1);
}