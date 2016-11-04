#include "pnlLimeSDR.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/statbox.h>
#include "lms7suiteEvents.h"

#include <ciso646>

#include <ErrorReporting.h>

using namespace lime;
using namespace std;

BEGIN_EVENT_TABLE(pnlLimeSDR, wxPanel)
END_EVENT_TABLE()

pnlLimeSDR::pnlLimeSDR(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    mainSizer = new wxFlexGridSizer(0, 2, 5, 5);
    controlsSizer = new wxFlexGridSizer(0, 2, 5, 5);

    SetSizer(mainSizer);
    chkRFLB_A_EN = new wxCheckBox(this, wxNewId(), _("RF loopback ch.A"));
    chkRFLB_A_EN->SetToolTip(_("[RFLB_A_EN] External RF loopback TxBAND2->RxLNAH channel A"));
    Connect(chkRFLB_A_EN->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkRFLB_A_EN, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkRFLB_B_EN = new wxCheckBox(this, wxNewId(), _("RF loopback ch.A"));
    chkRFLB_B_EN->SetToolTip(_("[RFLB_B_EN] External RF loopback TxBAND2->RxLNAH channel B"));
    Connect(chkRFLB_B_EN->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkRFLB_B_EN, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    chkTX1_2_LB_SH = new wxCheckBox(this, wxNewId(), _("Ch.A shunt"));
    chkTX1_2_LB_SH->SetToolTip(_("[TX1_2_LB_SH]"));
    Connect(chkTX1_2_LB_SH->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX1_2_LB_SH, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkTX2_2_LB_SH = new wxCheckBox(this, wxNewId(), _("Ch.B shunt"));
    chkTX2_2_LB_SH->SetToolTip(_("[TX2_2_LB_SH]"));
    Connect(chkTX2_2_LB_SH->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX2_2_LB_SH, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkTX1_2_LB_AT = new wxCheckBox(this, wxNewId(), _("Ch.A attenuator"));
    chkTX1_2_LB_AT->SetToolTip(_("[TX1_2_LB_AT]"));
    Connect(chkTX1_2_LB_AT->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX1_2_LB_AT, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkTX2_2_LB_AT = new wxCheckBox(this, wxNewId(), _("Ch.B attenuator"));
    chkTX2_2_LB_AT->SetToolTip(_("[TX2_2_LB_AT]"));
    Connect(chkTX2_2_LB_AT->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX2_2_LB_AT, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    groupSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("External loopback controls") ), wxVERTICAL );
    lblWarning = new wxStaticText(this, wxID_ANY, _(""));
    lblWarning->Hide();
    groupSizer->Add(lblWarning, 0, wxALIGN_LEFT | wxALIGN_TOP, 5);
    groupSizer->Add(controlsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    mainSizer->Add(groupSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();

    Bind(READ_ALL_VALUES, &pnlLimeSDR::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlLimeSDR::OnGPIOChange, this, this->GetId());
}

void pnlLimeSDR::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    if(lmsControl)
    {
        auto info = LMS_GetDeviceInfo(lmsControl);
        int hw =  std::stoi(info->hardwareVersion);
        if(hw < 3)
        {
            auto controls = controlsSizer->GetChildren();
            for(auto i : controls)
                i->GetWindow()->Disable();
            lblWarning->SetLabel(wxString::Format(_("Requires HW:>2, your HW:%s"), hw));
            lblWarning->Show();
        }
        else
        {
            lblWarning->SetLabel(_(""));
            lblWarning->Hide();
            auto controls = controlsSizer->GetChildren();
            for(auto i : controls)
                i->GetWindow()->Enable();
        }
    }
    
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();
}

pnlLimeSDR::~pnlLimeSDR()
{
    chkRFLB_A_EN->Disconnect(wxEVT_CHECKBOX, chkRFLB_A_EN->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);
    chkRFLB_B_EN->Disconnect(wxEVT_CHECKBOX, chkRFLB_B_EN->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);
    chkTX1_2_LB_SH->Disconnect(wxEVT_CHECKBOX, chkTX1_2_LB_SH->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
    chkTX1_2_LB_AT->Disconnect(wxEVT_CHECKBOX, chkTX1_2_LB_AT->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
    chkTX2_2_LB_SH->Disconnect(wxEVT_CHECKBOX, chkTX2_2_LB_SH->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
    chkTX2_2_LB_AT->Disconnect(wxEVT_CHECKBOX, chkTX2_2_LB_AT->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
}

void pnlLimeSDR::OnGPIOChange(wxCommandEvent& event)
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    value |= chkRFLB_A_EN->GetValue() << 0;
    value |= chkTX1_2_LB_AT->GetValue() << 1;
    value |= chkTX1_2_LB_SH->GetValue() << 2;

    value |= chkRFLB_B_EN->GetValue() << 4;
    value |= chkTX2_2_LB_AT->GetValue() << 5;
    value |= chkTX2_2_LB_SH->GetValue() << 6;

    if(lmsControl && LMS_WriteFPGAReg(lmsControl, addr, value))
        wxMessageBox(LMS_GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
}

void pnlLimeSDR::UpdatePanel()
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    if(lmsControl && LMS_ReadFPGAReg(lmsControl, addr, &value))
    {
        wxMessageBox(LMS_GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    chkRFLB_A_EN->SetValue((value >> 0) & 0x1);
    chkTX1_2_LB_AT->SetValue((value >> 1) & 0x1);
    chkTX1_2_LB_SH->SetValue((value >> 2) & 0x1);

    chkRFLB_B_EN->SetValue((value >> 4) & 0x1);
    chkTX2_2_LB_AT->SetValue((value >> 5) & 0x1);
    chkTX2_2_LB_SH->SetValue((value >> 6) & 0x1);
}

void pnlLimeSDR::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnlLimeSDR::OnWriteAll(wxCommandEvent &event)
{
    OnGPIOChange(event);
}
