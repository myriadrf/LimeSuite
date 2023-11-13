#include "pnlX3.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"
#include "limesuite/SDRDevice.h"

#include <ciso646>

using namespace std;
using namespace lime;

BEGIN_EVENT_TABLE(pnlX3, wxPanel)
END_EVENT_TABLE()

int pnlX3::LMS_ReadFPGAReg(lime::SDRDevice* device, uint32_t address, uint16_t* val)
{
    if (!device)
        return -1;
    const uint32_t mosi = address;
    uint32_t miso = 0;

    try
    {
        device->SPI(chipSelect, &mosi, &miso, 1);
        *val = miso & 0xFFFF;
        return 0;
    } catch (...)
    {
        return -1;
    }
}

int pnlX3::LMS_WriteFPGAReg(lime::SDRDevice* device, uint32_t address, uint16_t val)
{
    if (!device)
        return -1;

    const uint32_t mosi = (1 << 31) | address << 16 | val;
    try
    {
        device->SPI(chipSelect, &mosi, nullptr, 1);
        return 0;
    } catch (...)
    {
        return -1;
    }
}

int pnlX3::LMS_WriteCustomBoardParam(lime::SDRDevice* device, int32_t param_id, double val, const std::string& units)
{
    if (device == nullptr)
    {
        return -1;
    }

    try
    {
        return device->CustomParameterWrite(&param_id, &val, 1, units);
    } catch (...)
    {
        return -1;
    }
}

int pnlX3::LMS_ReadCustomBoardParam(lime::SDRDevice* device, int32_t param_id, double* val, std::string& units)
{
    if (device == nullptr)
    {
        return -1;
    }

    std::vector<std::string> str{ units };
    try
    {
        int ret = device->CustomParameterRead(&param_id, val, 1, str);
        units = str[0];
        return ret;
    } catch (...)
    {
        return -1;
    }
}

pnlX3::pnlX3(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, int style, wxString name)
{
    chipSelect = -1;
    device = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    // LMS1
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    // LMS 1 Ch0
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("TX1_EN  ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    Lms1tx1En = new wxCheckBox(this, wxNewId(), _("Enable"));
    Connect(Lms1tx1En->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer->Add(Lms1tx1En, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RFSW_TX1 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms1Tx1Path = new wxChoice(this, wxNewId());
    cmbLms1Tx1Path->Append("TX1_2 -> TX1 (J8)");
    cmbLms1Tx1Path->Append("TX1_1 -> TX1 (J8)");
    cmbLms1Tx1Path->SetSelection(0);
    Connect(cmbLms1Tx1Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer->Add(cmbLms1Tx1Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("TX1DAC ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxSize spinBoxSize(64, -1);
    long spinBoxStyle = wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER;
    spinDac1 = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 65535, 0);
    Connect(spinDac1->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlX3::OnDacChange), NULL, this);
    Connect(spinDac1->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlX3::OnDacChange), NULL, this);
    Connect(spinDac1->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlX3::OnDacChange), NULL, this);
    mainSizer->Add(spinDac1, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RFSW_RX1 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms1Rx1Path = new wxChoice(this, wxNewId());
    cmbLms1Rx1Path->Append("RX1_L <- RX1 (J1)");
    cmbLms1Rx1Path->Append("RX1_H <- RX1 (J1)");
    cmbLms1Rx1Path->SetSelection(0);
    Connect(cmbLms1Rx1Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer->Add(cmbLms1Rx1Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    // LMS 1 Ch1
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("TX2_EN  ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    Lms1tx2En = new wxCheckBox(this, wxNewId(), _("Enable"));
    Connect(Lms1tx2En->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer->Add(Lms1tx2En, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RFSW_TX2 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms1Tx2Path = new wxChoice(this, wxNewId());
    cmbLms1Tx2Path->Append("TX2_2 -> TX2 (J9)");
    cmbLms1Tx2Path->Append("TX2_1 -> TX2 (J9)");
    cmbLms1Tx2Path->SetSelection(0);
    Connect(cmbLms1Tx2Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer->Add(cmbLms1Tx2Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("TX2DAC ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    spinDac2 = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 65535, 0);
    Connect(spinDac2->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlX3::OnDacChange), NULL, this);
    Connect(spinDac2->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlX3::OnDacChange), NULL, this);
    Connect(spinDac2->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlX3::OnDacChange), NULL, this);
    mainSizer->Add(spinDac2, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RFSW_RX2 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms1Rx2Path = new wxChoice(this, wxNewId());
    cmbLms1Rx2Path->Append("RX2_L <- RX2 (J2)");
    cmbLms1Rx2Path->Append("RX2_H <- RX2 (J2)");
    cmbLms1Rx2Path->SetSelection(0);
    Connect(cmbLms1Rx2Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer->Add(cmbLms1Rx2Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    // start B.J.
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("LMS1 TDD")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    rbLMS1LowFreq = new wxRadioButton(this, wxNewId(), wxT("Low Freq."), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    rbLMS1LowFreq->SetValue(true);
    rbLMS1LowFreq->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlX3::OnLMS1LowFreq), NULL, this);
    mainSizer->Add(rbLMS1LowFreq, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    rbLMS1HighFreq = new wxRadioButton(this, wxNewId(), wxT("High Freq."), wxDefaultPosition, wxDefaultSize, 0);
    rbLMS1HighFreq->SetValue(false);
    rbLMS1HighFreq->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlX3::OnLMS1HighFreq), NULL, this);
    mainSizer->Add(rbLMS1HighFreq, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    m_bLMS1LowFreq = 1;

    rbLMS1DownLink = new wxRadioButton(this, wxNewId(), wxT("DL"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    rbLMS1DownLink->SetValue(true);
    rbLMS1DownLink->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlX3::OnLMS1DownLink), NULL, this);
    mainSizer->Add(rbLMS1DownLink, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    rbLMS1UpLink = new wxRadioButton(this, wxNewId(), wxT("UL"), wxDefaultPosition, wxDefaultSize, 0);
    rbLMS1UpLink->SetValue(false);
    rbLMS1UpLink->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlX3::OnLMS1UpLink), NULL, this);
    mainSizer->Add(rbLMS1UpLink, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    m_bLMS1DownLink = 1;

    cbLMS1ChA = new wxCheckBox(this, wxNewId(), _("ChA"));
    mainSizer->Add(cbLMS1ChA, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    cbLMS1ChA->SetValue(false);
    cbLMS1ChB = new wxCheckBox(this, wxNewId(), _("ChB"));
    cbLMS1ChB->SetValue(false);
    mainSizer->Add(cbLMS1ChB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);

    btnLMS1Settings = new wxButton(this, wxNewId(), _T("Configure"));
    mainSizer->Add(btnLMS1Settings, 1, wxALIGN_LEFT | wxEXPAND, 5);
    Connect(btnLMS1Settings->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlX3::OnLMS1Configure);
    // end B.J.

    auto lms1Sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("LMS1")), wxVERTICAL);
    lms1Sizer->Add(mainSizer, 0, 0, 5);

    // LMS2
    wxFlexGridSizer* mainSizer2 = new wxFlexGridSizer(0, 2, 5, 5);

    // LMS2 Ch0
    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("TX1_EN  ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    Lms2tx1En = new wxCheckBox(this, wxNewId(), _("Enable"));
    Connect(Lms2tx1En->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(Lms2tx1En, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_TRX1T ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Trx1TPath = new wxChoice(this, wxNewId());
    cmbLms2Trx1TPath->Append("TX1_1 -> RFSW_TRX1");
    cmbLms2Trx1TPath->Append("TX1_1 -> Ground");
    cmbLms2Trx1TPath->SetSelection(0);
    Connect(cmbLms2Trx1TPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Trx1TPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_TRX1 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Trx1Path = new wxChoice(this, wxNewId());
    cmbLms2Trx1Path->Append("RFSW_RX1IN <- TRX1 (J10)");
    cmbLms2Trx1Path->Append("RFSW_TRX1T -> TRX1 (J10)");
    cmbLms2Trx1Path->SetSelection(0);
    Connect(cmbLms2Trx1Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Trx1Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RX1_LNA  ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    Lms2Lna1En = new wxCheckBox(this, wxNewId(), _("Enable"));
    Connect(Lms2Lna1En->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(Lms2Lna1En, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_RX1C ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Rx1CPath = new wxChoice(this, wxNewId());
    cmbLms2Rx1CPath->Append("RX1_H <- RFSW_RX1IN (LNA)");
    cmbLms2Rx1CPath->Append("RX1_H <- LMS3 (TX1_1)");
    cmbLms2Rx1CPath->SetSelection(0);
    Connect(cmbLms2Rx1CPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Rx1CPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_RX1IN ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Rx1InPath = new wxChoice(this, wxNewId());
    cmbLms2Rx1InPath->Append("RFSW_RX1C <- RX1 (J11)");
    cmbLms2Rx1InPath->Append("RFSW_RX1C <- RSFW_TRX1");
    cmbLms2Rx1InPath->SetSelection(0);
    Connect(cmbLms2Rx1InPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Rx1InPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    // LMS2 Ch1
    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("TX2_EN  ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    Lms2tx2En = new wxCheckBox(this, wxNewId(), _("Enable"));
    Connect(Lms2tx2En->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(Lms2tx2En, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_TRX2T ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Trx2TPath = new wxChoice(this, wxNewId());
    cmbLms2Trx2TPath->Append("TX2_1 -> Ground");
    cmbLms2Trx2TPath->Append("TX2_1 -> RFSW_TRX2");
    cmbLms2Trx2TPath->SetSelection(0);
    Connect(cmbLms2Trx2TPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Trx2TPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_TRX2 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Trx2Path = new wxChoice(this, wxNewId());
    cmbLms2Trx2Path->Append("RFSW_RX2IN <- TRX2 (J12)");
    cmbLms2Trx2Path->Append("RFSW_TRX2T -> TRX2 (J12)");
    cmbLms2Trx2Path->SetSelection(0);
    Connect(cmbLms2Trx2Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Trx2Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RX2_LNA  ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    Lms2Lna2En = new wxCheckBox(this, wxNewId(), _("Enable"));
    Connect(Lms2Lna2En->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(Lms2Lna2En, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_RX2C ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Rx2CPath = new wxChoice(this, wxNewId());
    cmbLms2Rx2CPath->Append("RX2_H <- RFSW_RX2IN (LNA)");
    cmbLms2Rx2CPath->Append("RX2_H <- LMS3 (TX2_1)");
    cmbLms2Rx2CPath->SetSelection(0);
    Connect(cmbLms2Rx2CPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Rx2CPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("RFSW_RX2IN ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms2Rx2InPath = new wxChoice(this, wxNewId());
    cmbLms2Rx2InPath->Append("RFSW_RX2C <- RX2 (J13)");
    cmbLms2Rx2InPath->Append("RFSW_RX2C <- RSFW_TRX2");
    cmbLms2Rx2InPath->SetSelection(0);
    Connect(cmbLms2Rx2InPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer2->Add(cmbLms2Rx2InPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    // start B.J.
    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("LMS2 TDD")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer2->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

    rbLMS2DownLink = new wxRadioButton(this, wxNewId(), wxT("DL"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    rbLMS2DownLink->SetValue(true);
    rbLMS2DownLink->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlX3::OnLMS2DownLink), NULL, this);
    mainSizer2->Add(rbLMS2DownLink, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    rbLMS2UpLink = new wxRadioButton(this, wxNewId(), wxT("UL"), wxDefaultPosition, wxDefaultSize, 0);
    rbLMS2UpLink->SetValue(false);
    rbLMS2UpLink->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlX3::OnLMS2UpLink), NULL, this);
    mainSizer2->Add(rbLMS2UpLink, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    m_bLMS2DownLink = 1;

    cbLMS2ChA = new wxCheckBox(this, wxNewId(), _("ChA"));
    mainSizer2->Add(cbLMS2ChA, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    cbLMS2ChA->SetValue(false);
    cbLMS2ChB = new wxCheckBox(this, wxNewId(), _("ChB"));
    mainSizer2->Add(cbLMS2ChB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    cbLMS2ChB->SetValue(false);

    btnLMS2Settings = new wxButton(this, wxNewId(), _T("Configure"));
    mainSizer2->Add(btnLMS2Settings, 1, wxALIGN_LEFT | wxEXPAND, 5);
    Connect(btnLMS2Settings->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlX3::OnLMS2Configure);
    // end B.J.

    auto lms2Sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("LMS2")), wxVERTICAL);
    lms2Sizer->Add(mainSizer2, 0, 0, 5);

    // LMS3
    wxFlexGridSizer* mainSizer3 = new wxFlexGridSizer(0, 2, 5, 5);
    mainSizer3->Add(new wxStaticText(this, wxID_ANY, _("RFSW1_RX1 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms3Rx1Path = new wxChoice(this, wxNewId());
    cmbLms3Rx1Path->Append("RX1_H <- RX_IN (J6)");
    cmbLms3Rx1Path->Append("RX1_H <- LMS2 (TX1_1)");
    cmbLms3Rx1Path->SetSelection(0);
    Connect(cmbLms3Rx1Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer3->Add(cmbLms3Rx1Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer3->Add(new wxStaticText(this, wxID_ANY, _("RFSW1_RX2 ")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbLms3Rx2Path = new wxChoice(this, wxNewId());
    cmbLms3Rx2Path->Append("RX2_H <- RX_IN (J7)");
    cmbLms3Rx2Path->Append("RX1_H <- LMS2 (TX2_1)");
    cmbLms3Rx2Path->SetSelection(0);
    Connect(cmbLms3Rx2Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlX3::OnInputChange), NULL, this);
    mainSizer3->Add(cmbLms3Rx2Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    auto lms3Sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("LMS3")), wxVERTICAL);
    lms3Sizer->Add(mainSizer3, 0, 0, 5);

    // Main RF sizer
    wxStaticBoxSizer* mainBoxSizer;
    mainBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("RF controls")), wxHORIZONTAL);
    mainBoxSizer->Add(lms1Sizer, 0, 0, 5);
    mainBoxSizer->Add(lms2Sizer, 0, 0, 5);
    mainBoxSizer->Add(lms3Sizer, 0, 0, 5);
    mainBoxSizer->Fit(this);
    mainBoxSizer->SetSizeHints(this);
    SetSizer(mainBoxSizer);
    Layout();

    Bind(READ_ALL_VALUES, &pnlX3::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlX3::OnWriteAll, this, this->GetId());
}

void pnlX3::Initialize(lime::SDRDevice* pControl)
{
    chipSelect = -1;
    device = pControl;
    if (device == nullptr)
        return;

    const SDRDevice::Descriptor& desc = device->GetDescriptor();
    const std::string targetSPI = "FPGA";
    for (const auto& nameIds : desc.spiSlaveIds)
    {
        if (nameIds.first == targetSPI)
        {
            chipSelect = nameIds.second;
            break;
        }
    }
}

pnlX3::~pnlX3()
{
    // LMS1
    cmbLms1Rx1Path->Disconnect(wxEVT_CHOICE, cmbLms1Rx1Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms1Tx1Path->Disconnect(wxEVT_CHOICE, cmbLms1Tx1Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms1Rx2Path->Disconnect(wxEVT_CHOICE, cmbLms1Rx2Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms1Tx2Path->Disconnect(wxEVT_CHOICE, cmbLms1Tx2Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);

    spinDac1->Disconnect(wxEVT_TEXT_ENTER, spinDac1->GetId(), wxCommandEventHandler(pnlX3::OnDacChange), 0, this);
    spinDac1->Disconnect(wxEVT_SPINCTRL, spinDac1->GetId(), wxCommandEventHandler(pnlX3::OnDacChange), 0, this);
    spinDac1->Disconnect(wxEVT_SPIN, spinDac1->GetId(), wxCommandEventHandler(pnlX3::OnDacChange), 0, this);

    spinDac2->Disconnect(wxEVT_TEXT_ENTER, spinDac2->GetId(), wxCommandEventHandler(pnlX3::OnDacChange), 0, this);
    spinDac2->Disconnect(wxEVT_SPINCTRL, spinDac2->GetId(), wxCommandEventHandler(pnlX3::OnDacChange), 0, this);
    spinDac2->Disconnect(wxEVT_SPIN, spinDac2->GetId(), wxCommandEventHandler(pnlX3::OnDacChange), 0, this);

    // LMS2
    cmbLms2Trx1TPath->Disconnect(wxEVT_CHOICE, cmbLms2Trx1TPath->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms2Trx1Path->Disconnect(wxEVT_CHOICE, cmbLms2Trx1Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms2Rx1CPath->Disconnect(wxEVT_CHOICE, cmbLms2Rx1CPath->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms2Rx1InPath->Disconnect(wxEVT_CHOICE, cmbLms2Rx1InPath->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);

    cmbLms2Trx2TPath->Disconnect(wxEVT_CHOICE, cmbLms2Trx2TPath->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms2Trx2Path->Disconnect(wxEVT_CHOICE, cmbLms2Trx2Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms2Rx2CPath->Disconnect(wxEVT_CHOICE, cmbLms2Rx2CPath->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms2Rx2InPath->Disconnect(wxEVT_CHOICE, cmbLms2Rx2InPath->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);

    // LMS3
    cmbLms3Rx1Path->Disconnect(wxEVT_CHOICE, cmbLms3Rx1Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
    cmbLms3Rx1Path->Disconnect(wxEVT_CHOICE, cmbLms3Rx2Path->GetId(), wxCommandEventHandler(pnlX3::OnInputChange), 0, this);
}

void pnlX3::OnInputChange(wxCommandEvent& event)
{
    uint16_t addr = 0x00D1;
    uint16_t value = 0;

    value |= cmbLms3Rx1Path->GetSelection() << 0;
    value |= cmbLms3Rx2Path->GetSelection() << 1;
    value |= cmbLms2Rx1CPath->GetSelection() << 2;
    value |= cmbLms2Rx1InPath->GetSelection() << 3;
    value |= cmbLms2Rx2CPath->GetSelection() << 4;
    value |= cmbLms2Rx2InPath->GetSelection() << 5;
    value |= cmbLms2Trx1Path->GetSelection() << 6;
    value |= cmbLms2Trx1TPath->GetSelection() << 7;
    value |= cmbLms2Trx2Path->GetSelection() << 8;
    value |= cmbLms2Trx2TPath->GetSelection() << 9;
    value |= cmbLms1Rx2Path->GetSelection() << 10;
    value |= cmbLms1Rx1Path->GetSelection() << 11;
    value |= cmbLms1Tx2Path->GetSelection() << 12;
    value |= cmbLms1Tx1Path->GetSelection() << 13;

    if (LMS_WriteFPGAReg(device, addr, value))
        wxMessageBox(_("Failed to write FPGA registers"), _("Error"), wxICON_ERROR | wxOK);

    addr = 0x00D2;
    value = 0;

    value |= (!Lms2Lna2En->GetValue()) << 0;
    value |= (!Lms2Lna1En->GetValue()) << 1;
    value |= Lms2tx2En->GetValue() << 2;
    value |= Lms2tx1En->GetValue() << 3;
    value |= Lms1tx2En->GetValue() << 4;
    value |= Lms1tx1En->GetValue() << 5;

    if (LMS_WriteFPGAReg(device, addr, value))
        wxMessageBox(_("Failed to write FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
}

void pnlX3::OnDacChange(wxCommandEvent& event)
{
    LMS_WriteCustomBoardParam(device, 2, spinDac1->GetValue(), "");
    LMS_WriteCustomBoardParam(device, 3, spinDac2->GetValue(), "");

    return;
}

void pnlX3::UpdatePanel()
{
    uint16_t addr = 0x00D1;
    uint16_t value = 0;
    if (LMS_ReadFPGAReg(device, addr, &value))
    {
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    cmbLms3Rx1Path->SetSelection((value >> 0) & 1);
    cmbLms3Rx2Path->SetSelection((value >> 1) & 1);
    cmbLms2Rx1CPath->SetSelection((value >> 2) & 1);
    cmbLms2Rx1InPath->SetSelection((value >> 3) & 1);
    cmbLms2Rx2CPath->SetSelection((value >> 4) & 1);
    cmbLms2Rx2InPath->SetSelection((value >> 5) & 1);
    cmbLms2Trx1Path->SetSelection((value >> 6) & 1);
    cmbLms2Trx1TPath->SetSelection((value >> 7) & 1);
    cmbLms2Trx2Path->SetSelection((value >> 8) & 1);
    cmbLms2Trx2TPath->SetSelection((value >> 9) & 1);
    cmbLms1Rx2Path->SetSelection((value >> 10) & 1);
    cmbLms1Rx1Path->SetSelection((value >> 11) & 1);
    cmbLms1Tx2Path->SetSelection((value >> 12) & 1);
    cmbLms1Tx1Path->SetSelection((value >> 13) & 1);

    addr = 0x00D2;
    value = 0;
    if (LMS_ReadFPGAReg(device, addr, &value))
    {
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    Lms2Lna2En->SetValue(!((value >> 0) & 1));
    Lms2Lna1En->SetValue(!((value >> 1) & 1));
    Lms2tx2En->SetValue((value >> 2) & 1);
    Lms2tx1En->SetValue((value >> 3) & 1);
    Lms1tx2En->SetValue((value >> 4) & 1);
    Lms1tx1En->SetValue((value >> 5) & 1);

    double dacVal = 0;
    std::string empty = "";
    LMS_ReadCustomBoardParam(device, 2, &dacVal, empty);
    spinDac1->SetValue((int)dacVal);
    LMS_ReadCustomBoardParam(device, 3, &dacVal, empty);
    spinDac2->SetValue((int)dacVal);
}

void pnlX3::OnReadAll(wxCommandEvent& event)
{
    UpdatePanel();
}

void pnlX3::OnWriteAll(wxCommandEvent& event)
{
    OnInputChange(event);
    OnDacChange(event);
}

// B.J. start

pnlX3::Register::Register()
    : address(0)
    , msb(0)
    , lsb(0)
    , defaultValue(0)
{
}

pnlX3::Register::Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue)
    : address(address)
    , msb(msb)
    , lsb(lsb)
    , defaultValue(defaultValue)
{
}

void pnlX3::SetRegValue(lime::SDRDevice* pLmsControl, Register reg, uint16_t value)
{

    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue = 0;
    LMS_ReadFPGAReg(pLmsControl, reg.address, &regValue); // read bit content
    regValue &= ~mask;
    regValue |= (value << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(pLmsControl, reg.address, regValue) != 0)
        ; // register write
}

void pnlX3::OnLMS1LowFreq(wxCommandEvent& event)
{
    rbLMS1LowFreq->SetValue(true);
    rbLMS1HighFreq->SetValue(false);
    m_bLMS1LowFreq = 1;
}

void pnlX3::OnLMS1HighFreq(wxCommandEvent& event)
{
    rbLMS1LowFreq->SetValue(false);
    rbLMS1HighFreq->SetValue(true);
    m_bLMS1LowFreq = 0;
}

void pnlX3::OnLMS1DownLink(wxCommandEvent& event)
{
    rbLMS1DownLink->SetValue(true);
    rbLMS1UpLink->SetValue(false);
    m_bLMS1DownLink = 1;
}

void pnlX3::OnLMS1UpLink(wxCommandEvent& event)
{
    rbLMS1DownLink->SetValue(false);
    rbLMS1UpLink->SetValue(true);
    m_bLMS1DownLink = 0;
}

void pnlX3::OnLMS1Configure(wxCommandEvent& event)
{
    int m_bLMS1ChA, m_bLMS1ChB = 0;
    Register chkLms1Tx1, cmbLms1Tx1Path, cmbLms1Rx1Path, chkLms1Tx2, cmbLms1Tx2Path, cmbLms1Rx2Path;

    m_bLMS1ChA = (int)cbLMS1ChA->GetValue();
    m_bLMS1ChB = (int)cbLMS1ChB->GetValue();

    // LMS#1 channel A
    chkLms1Tx1 = Register(0x00D2, 5, 5, 0); // positive logic
    cmbLms1Tx1Path = Register(0x00D1, 13, 13, 0);
    //("TX1_2 -> TX1 (J8)");
    //("TX1_1 -> TX1 (J8)");
    cmbLms1Rx1Path = Register(0x00D1, 11, 11, 0);
    //("RX1_L <- RX1 (J1)");
    //("RX1_H <- RX1 (J1)");

    // LMS#1 channel B
    chkLms1Tx2 = Register(0x00D2, 4, 4, 0); // positive logic
    cmbLms1Tx2Path = Register(0x00D1, 12, 12, 0);
    //("TX2_2 -> TX2 (J9)");
    //("TX2_1 -> TX2 (J9)");
    cmbLms1Rx2Path = Register(0x00D1, 10, 10, 0);
    //("RX2_L <- RX2 (J2)");
    //("RX2_H <- RX2 (J2)");

    if (m_bLMS1ChA == 1) // channel A
    {

        if (m_bLMS1DownLink == 1)
        {
            SetRegValue(device, chkLms1Tx1, 1); // Enabled
            if (m_bLMS1LowFreq == 1)
            {
                SetRegValue(device, cmbLms1Tx1Path, 0); //("TX1_2 -> TX1 (J8)");
                SetRegValue(device, cmbLms1Rx1Path, 1); //("RX1_H <- RX1 (J1)");
            }
            else
            {
                SetRegValue(device, cmbLms1Tx1Path, 1); //("TX1_1 -> TX1 (J8)");
                SetRegValue(device, cmbLms1Rx1Path, 0); //("RX1_L <- RX1 (J1)");
            }
        }
        else // Uplink
        {
            SetRegValue(device, chkLms1Tx1, 0); // Disabled
            if (m_bLMS1LowFreq == 0)
            {
                SetRegValue(device, cmbLms1Tx1Path, 0); //("TX1_2 -> TX1 (J8)");
                SetRegValue(device, cmbLms1Rx1Path, 1); //("RX1_H <- RX1 (J1)");
            }
            else
            {
                SetRegValue(device, cmbLms1Tx1Path, 1); //("TX1_1 -> TX1 (J8)");
                SetRegValue(device, cmbLms1Rx1Path, 0); //("RX1_L <- RX1 (J1)");
            }
        }
    }

    if (m_bLMS1ChB == 1) // channel B
    {

        if (m_bLMS1DownLink == 1)
        {
            SetRegValue(device, chkLms1Tx2, 1); // Enabled
            if (m_bLMS1LowFreq == 1)
            {
                SetRegValue(device, cmbLms1Tx2Path, 0); //("TX2_2 -> TX2 (J9)");
                SetRegValue(device, cmbLms1Rx2Path, 1); //("RX2_H <- RX2 (J2)");
            }
            else
            {
                SetRegValue(device, cmbLms1Tx2Path, 1); //("TX2_1 -> TX2 (J9)");
                SetRegValue(device, cmbLms1Rx2Path, 0); //("RX2_L <- RX2 (J2)");
            }
        }
        else // Uplink
        {
            SetRegValue(device, chkLms1Tx2, 0); // Disabled
            if (m_bLMS1LowFreq == 0)
            {
                SetRegValue(device, cmbLms1Tx2Path, 0); //("TX2_2 -> TX2 (J9)");
                SetRegValue(device, cmbLms1Rx2Path, 1); //("RX2_H <- RX2 (J2)");
            }
            else
            {
                SetRegValue(device, cmbLms1Tx2Path, 1); //("TX2_1 -> TX2 (J9)");
                SetRegValue(device, cmbLms1Rx2Path, 0); //("RX2_L <- RX2 (J2)");
            }
        }
    }

    UpdatePanel();
}

void pnlX3::OnLMS2DownLink(wxCommandEvent& event)
{
    rbLMS2DownLink->SetValue(true);
    rbLMS2UpLink->SetValue(false);
    m_bLMS2DownLink = 1;
}
void pnlX3::OnLMS2UpLink(wxCommandEvent& event)
{
    rbLMS2DownLink->SetValue(false);
    rbLMS2UpLink->SetValue(true);
    m_bLMS2DownLink = 0;
}

void pnlX3::OnLMS2Configure(wxCommandEvent& event)
{
    int m_bLMS2ChA, m_bLMS2ChB = 0;
    Register cmbLms2Trx1T, cmbLms2Trx1, cmbLms2Rx1In, cmbLms2Rx1C, cmbLms3Rx1, chkLms2Tx1, chkLms2Lna1;
    Register cmbLms2Trx2T, cmbLms2Trx2, cmbLms2Rx2In, cmbLms2Rx2C, chkLms2Tx2, chkLms2Lna2;

    m_bLMS2ChA = (int)cbLMS2ChA->GetValue();
    m_bLMS2ChB = (int)cbLMS2ChB->GetValue();

    // RF2 (log. 0), RF1 (log. 1)
    // LMS#2 channel A
    chkLms2Tx1 = Register(0x00D2, 3, 3, 0); // positive logic
    cmbLms2Trx1T = Register(0x00D1, 7, 7, 0);
    //  ("TX1_1(LNA)->RFSW_TX1OUT");  // RF2 (log. 0)
    //  ("TX1_1(LNA)->Ground"); // RF1 (log. 1)
    cmbLms2Trx1 = Register(0x00D1, 6, 6, 0);
    // ("RFSW_RX1IN<-TRX1(J8)"); // RF2 (log. 0)
    // ("RFSW_TX1OUT->TRX1(J8)"); // RF1 (log. 1)
    chkLms2Lna1 = Register(0x00D2, 1, 1, 0); // negative logic
    cmbLms2Rx1C = Register(0x00D1, 2, 2, 0);
    // ("RX1_H<-RFSW_RX1(LNA)");  // RF2 (log. 0)
    // ("RX1_H<-TX1_1(LMS3)"); // RF1 (log. 1)
    cmbLms2Rx1In = Register(0x00D1, 3, 3, 0);
    // ("RFSW_RX1<-RX1(J9)"); // RF2 (log. 0)
    // ("RFSW_RX1<-RFSW_RX1IN"); // RF1 (log. 1)

    if (m_bLMS2ChA == 1)
    {
        if (m_bLMS2DownLink == 1)
        {
            SetRegValue(device, chkLms2Tx1, 1); // Enabled
            SetRegValue(device, cmbLms2Trx1T, 0); // TX1_1(LNA)->RFSW_TX1OUT
            SetRegValue(device, cmbLms2Trx1, 1); // RFSW_TX1OUT->TRX1(J8)
            SetRegValue(device, chkLms2Lna1, 1); // Disabled
            SetRegValue(device, cmbLms2Rx1C, 0); // RX1_H<-RFSW_RX1(LNA)
            SetRegValue(device, cmbLms2Rx1In, 1); // RFSW_RX1<-RFSW_RX1IN
        }
        else
        {
            SetRegValue(device, chkLms2Tx1, 0); // Disabled
            SetRegValue(device, cmbLms2Trx1T, 1); // TX1_1(LNA)->Ground
            SetRegValue(device, cmbLms2Trx1, 0); // RFSW_RX1IN<-TRX1(J8)
            SetRegValue(device, chkLms2Lna1, 0); // Enabled
            SetRegValue(device, cmbLms2Rx1C, 0); // RX1_H<-RFSW_RX1(LNA)
            SetRegValue(device, cmbLms2Rx1In, 1); // RFSW_RX1<-RFSW_RX1IN
        }
    }

    // LMS#2 channelB
    chkLms2Tx2 = Register(0x00D2, 2, 2, 0); // positive logic
    cmbLms2Trx2T = Register(0x00D1, 9, 9, 0);
    // ("TX2_1(LNA)->Ground"); // RF2 (log. 0)
    // ("TX2_1(LNA)->RFSW_TX2OUT"); // RF1 (log. 1)
    cmbLms2Trx2 = Register(0x00D1, 8, 8, 0);
    // ("RFSW_RX2IN<-TRX2(J10)"); // RF2 (log. 0)
    // ("RFSW_TX2OUT->TRX2(J10)"); // RF1 (log. 1)
    chkLms2Lna2 = Register(0x00D2, 0, 0, 0); // negative logic
    cmbLms2Rx2C = Register(0x00D1, 4, 4, 0);
    // ("RX2_H<-RFSW_RX2(LNA)"); // RF2 (log. 0)
    // ("RX2_H<-TX2_1(LMS3)"); // RF1 (log. 1)
    cmbLms2Rx2In = Register(0x00D1, 5, 5, 0);
    // ("RFSW_RX2<-RX2(J11)"); // RF2 (log. 0)
    // ("RFSW_RX2<-RFSW_RX2IN"); // RF1 (log. 1)

    if (m_bLMS2ChB == 1)
    {
        if (m_bLMS2DownLink == 1)
        {
            // Downlink
            SetRegValue(device, chkLms2Tx2, 1); // Enabled
            SetRegValue(device, cmbLms2Trx2T, 1); // TX2_1(LNA)->RFSW_TX2OUT
            SetRegValue(device, cmbLms2Trx2, 1); // RFSW_TX2OUT->TRX2(J10)
            SetRegValue(device, chkLms2Lna2, 1); // Disabled
            SetRegValue(device, cmbLms2Rx2C, 0); // RX2_H<-RFSW_RX2(LNA)
            SetRegValue(device, cmbLms2Rx2In, 1); // RFSW_RX2<-RFSW_RX2IN
        }
        else // Uplink
        {
            SetRegValue(device, chkLms2Tx2, 0); // Disabled
            SetRegValue(device, cmbLms2Trx2T, 0); // TX2_1(LNA)->Ground
            SetRegValue(device, cmbLms2Trx2, 0); // RFSW_RX2IN<-TRX2(J10)
            SetRegValue(device, chkLms2Lna2, 0); // Enabled
            SetRegValue(device, cmbLms2Rx2C, 0); // RX2_H<-RFSW_RX2(LNA)
            SetRegValue(device, cmbLms2Rx2In, 1); // RFSW_RX2<-RFSW_RX2IN
        }
    }

    UpdatePanel();
}
// end B.J.
