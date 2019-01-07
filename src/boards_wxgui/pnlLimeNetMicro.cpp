#include "pnlLimeNetMicro.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"

using namespace std;

BEGIN_EVENT_TABLE(pnlLimeNetMicro, wxPanel)
END_EVENT_TABLE()

pnlLimeNetMicro::pnlLimeNetMicro(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    SetSizer(mainSizer);
    wxFlexGridSizer* lbSizer = new wxFlexGridSizer(0, 2, 5, 5);
    lbSizer->Add(new wxStaticText(this, wxID_ANY, _("RX RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString rxChoices;
    rxChoices.push_back(_("LNAL"));
    rxChoices.push_back(_("LNAH"));
    cmbRxPath = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, rxChoices, 1);
    cmbRxPath->SetSelection(0);
    Connect(cmbRxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), NULL, this);
    lbSizer->Add(cmbRxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    
    lbSizer->Add(new wxStaticText(this, wxID_ANY, _("TX RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString txChoices;
    txChoices.push_back(_("Band 1"));
    txChoices.push_back(_("Band 2"));
    cmbTxPath = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, txChoices, 1);
    cmbTxPath->SetSelection(0);
    Connect(cmbTxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), NULL, this);
    lbSizer->Add(cmbTxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    
    mainSizer->Add(lbSizer, 1, wxEXPAND | wxALL, 5);
    Bind(READ_ALL_VALUES, &pnlLimeNetMicro::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlLimeNetMicro::OnLoopbackChange, this, this->GetId());
}

void pnlLimeNetMicro::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();
}

pnlLimeNetMicro::~pnlLimeNetMicro()
{
    cmbRxPath->Disconnect(wxEVT_CHOICE, cmbRxPath->GetId(), wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), 0, this);
    cmbTxPath->Disconnect(wxEVT_CHOICE, cmbTxPath->GetId(), wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), 0, this);
}

void pnlLimeNetMicro::OnLoopbackChange(wxCommandEvent& event)
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    value |= cmbRxPath->GetSelection() == 1 ? 1<<9 : 1<<8;
    value |= cmbTxPath->GetSelection() == 1 ? 1<<13 : 1<<12;
    if(LMS_WriteFPGAReg(lmsControl,addr, value))
        wxMessageBox(_("Failed to write FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
}

void pnlLimeNetMicro::UpdatePanel()
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    if(LMS_ReadFPGAReg(lmsControl,addr, &value))
    {
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    cmbRxPath->SetSelection((value >> 9) & 0x1);
    cmbTxPath->SetSelection((value >> 13) & 0x1);
}

void pnlLimeNetMicro::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnlLimeNetMicro::OnWriteAll(wxCommandEvent &event)
{
    OnLoopbackChange(event);
}
