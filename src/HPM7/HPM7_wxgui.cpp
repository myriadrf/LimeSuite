/**
@file 	HPM7_wxgui.cpp
@author Lime Microsystems
@brief 	panel for interacting with HPM7 board
*/

#include "HPM7_wxgui.h"
#include "lms7suiteEvents.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <vector>
#include "LMS64CCommands.h"


BEGIN_EVENT_TABLE(HPM7_wxgui, wxFrame)

END_EVENT_TABLE()

HPM7_wxgui::HPM7_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
{
    lmsControl = nullptr;
    Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, _T("id"));
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 3, 5, 5);
    wxFlexGridSizer* tunersSizer = new wxFlexGridSizer(0, 3, 5, 5);
    wxStaticBoxSizer* tunerGroup;
    wxStaticText* stext;
    wxString tunerNames[] = {
        _("TUNER_A_IN"),
        _("TUNER_A_MID"),
        _("TUNER_A_OUT"),
        _("TUNER_B_IN"),
        _("TUNER_B_MID"),
        _("TUNER_B_OUT")
    };

    wxArrayString ssc1_choices;
    for (int i = 0; i < pow(2.0, 5); ++i)
        ssc1_choices.push_back(wxString::Format("%i", i));
    wxArrayString ssc2_choices;
    for (int i = 0; i < pow(2.0, 4); ++i)
        ssc2_choices.push_back(wxString::Format("%i", i));

    for (int i = 0; i < 6; ++i)
    {
        tunerIds.push_back(wxNewId());
        cmbSSC1ids.push_back(wxNewId());
        tunerGroup = new wxStaticBoxSizer(wxVERTICAL, this, tunerNames[i]);
        chkEB.push_back(new wxCheckBox(this, tunerIds[i], _("Ext. branch (F11)")));
        Connect(tunerIds[i], wxEVT_CHECKBOX, (wxObjectEventFunction)&HPM7_wxgui::OnTunerSSC2change);
        tunerGroup->Add(chkEB[i], 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        chkTP.push_back(new wxCheckBox(this, tunerIds[i], _("Through path (F10)")));
        Connect(tunerIds[i], wxEVT_CHECKBOX, (wxObjectEventFunction)&HPM7_wxgui::OnTunerSSC2change);
        tunerGroup->Add(chkTP[i], 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

        stext = new wxStaticText(this, wxNewId(), _("SSC2 (F9-F6)"));
        tunerGroup->Add(stext);
        cmbSSC2.push_back(new wxComboBox(this, tunerIds[i], ssc2_choices[0], wxDefaultPosition, wxDefaultSize, ssc2_choices));
        Connect(tunerIds[i], wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnTunerSSC2change);
        cmbSSC2[i]->SetSelection(0);
        tunerGroup->Add(cmbSSC2[i], 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);

        stext = new wxStaticText(this, wxNewId(), _("SSC1 (F5-F1)"));
        tunerGroup->Add(stext);
        cmbSSC1.push_back(new wxComboBox(this, cmbSSC1ids[i], ssc1_choices[0], wxDefaultPosition, wxDefaultSize, ssc1_choices));
        Connect(cmbSSC1ids[i], wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnTunerSSC1change);
        cmbSSC1[i]->SetSelection(0);
        tunerGroup->Add(cmbSSC1[i], 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
        tunersSizer->Add(tunerGroup);
    }

    wxFlexGridSizer* leftCollumn = new wxFlexGridSizer(0, 1, 5, 5);
    btnUpdateAll = new wxButton(this, wxNewId(), _("Read All"));
    Connect(btnUpdateAll->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&HPM7_wxgui::DownloadAll);
    leftCollumn->Add(btnUpdateAll, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    wxFlexGridSizer* gpioControls = new wxFlexGridSizer(0, 1, 5, 5);
    const wxString activePathChoices[] = { _("No path active"), _("LNAH"), _("LNAL"), _("LNAW") };
    // GPIO1 GPIO0
    //   0     0  not used
    //   0     1  LNAW_A&B (Wide band)
    //   1     0  LNAH_A&B (High band)
    //   1     1  LNAL_A&B (Low band)
    cmbActivePath = new wxComboBox(this, wxNewId(), activePathChoices[1], wxDefaultPosition, wxDefaultSize, 4, activePathChoices);
    Connect(cmbActivePath->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnGPIOchange);
    gpioControls->Add(new wxStaticText(this, wxID_ANY, _("Active path to RXFE:")), 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    gpioControls->Add(cmbActivePath, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);

    gpioControls->Add(new wxStaticText(this, wxID_ANY, _("TxFE output selection")), 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    const wxString bandChoices[] = { _("Band1"), _("Band2") };
    // GPIO2
    //   0  TXA_2 output
    //   1  TXA_1 output amplified
    cmbBand = new wxComboBox(this, wxNewId(), bandChoices[0], wxDefaultPosition, wxDefaultSize, 2, bandChoices);
    Connect(cmbBand->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnGPIOchange);
    gpioControls->Add(cmbBand, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);

    gpioControls->Add(new wxStaticText(this, wxID_ANY, _("Enable external LNA:")), 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    const wxString lnaChoices[] = { _("Bypass LNA"), _("Enable LNA")};
    //GPIO3
    //  0  Bypass LNA
    //  1  Enable LNA
    cmbLNA = new wxComboBox(this, wxNewId(), lnaChoices[1], wxDefaultPosition, wxDefaultSize, 2, lnaChoices);
    Connect(cmbLNA->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnGPIOchange);
    gpioControls->Add(cmbLNA, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);

    gpioControls->Add(new wxStaticText(this, wxID_ANY, _("PAs Vd Driver:")), 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    const wxString paChoices[] = { _("5V"), _("2V") };
    //GPIO4
    //  0  5V
    //  1  2V
    cmbPAdriver = new wxComboBox(this, wxNewId(), paChoices[0], wxDefaultPosition, wxDefaultSize, 2, paChoices);
    Connect(cmbPAdriver->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnGPIOchange);
    gpioControls->Add(cmbPAdriver, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);
    leftCollumn->Add(gpioControls, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);

    //DACs
    wxFlexGridSizer* dacSizer = new wxFlexGridSizer(0, 2, 5, 5);
    wxArrayString dac_choices;
    for (int i = 0; i < 256; ++i)
        dac_choices.push_back(wxString::Format("%.2f V", i*3.3/256));
    cmbDAC_A = new wxComboBox(this, wxNewId(), "0", wxDefaultPosition, wxSize(64, -1), dac_choices);
    dacSizer->Add(new wxStaticText(this, wxNewId(), "DAC_A: "), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    dacSizer->Add(cmbDAC_A, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);

    cmbDAC_B = new wxComboBox(this, wxNewId(), "0", wxDefaultPosition, wxSize(64, -1), dac_choices);
    dacSizer->Add(new wxStaticText(this, wxNewId(), "DAC_B: "), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    dacSizer->Add(cmbDAC_B, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    Connect(cmbDAC_A->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnDACchange);
    Connect(cmbDAC_B->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&HPM7_wxgui::OnDACchange);
    wxStaticBoxSizer* dacGroup = new wxStaticBoxSizer(wxVERTICAL, this, _("DAC"));
    dacGroup->Add(dacSizer);

    leftCollumn->Add(dacGroup);
    mainSizer->Add(leftCollumn);
    mainSizer->Add(tunersSizer);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();
}

void HPM7_wxgui::Initialize(lms_device_t* serPort)
{
    lmsControl = serPort;
}

HPM7_wxgui::~HPM7_wxgui()
{

}

void HPM7_wxgui::OnTunerSSC1change(wxCommandEvent& event)
{
    size_t tunerIndex = 0;
    for (tunerIndex = 0; tunerIndex < cmbSSC1.size(); ++tunerIndex)
        if (event.GetId() == cmbSSC1[tunerIndex]->GetId())
            break;
    if (tunerIndex >= cmbSSC1.size())
        return;

    uint8_t data[64];
    data[0] = 0x20 + tunerIndex * 2;
    data[1] = event.GetInt();
    size_t len = 2;

    if (LMS_TransferLMS64C(lmsControl, lime::CMD_MYRIAD_WR, data, &len)!=0)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
}

void HPM7_wxgui::OnTunerSSC2change(wxCommandEvent& event)
{
    size_t tunerIndex = 0;
    for (tunerIndex = 0; tunerIndex < tunerIds.size(); ++tunerIndex)
        if (event.GetId() == tunerIds[tunerIndex])
            break;

    if (tunerIndex >= tunerIds.size())
        return;

    uint8_t data[64];
    data[0] = 0x21 + tunerIndex * 2;

    unsigned char value = chkEB[tunerIndex]->GetValue() << 5;
    value |= chkTP[tunerIndex]->GetValue() << 4;
    value |= (cmbSSC2[tunerIndex]->GetSelection() & 0xF);
    data[1] = value;

    size_t len = 2;

    if (LMS_TransferLMS64C(lmsControl, lime::CMD_MYRIAD_WR, data, &len)!=0)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
}

void HPM7_wxgui::OnGPIOchange(wxCommandEvent& event)
{
    if (UploadGPIO() == false)
        return;

    wxCommandEvent evt;
    evt.SetEventObject(this);
    if (event.GetId() == cmbActivePath->GetId())
    {
        evt.SetEventType(LMS7_RXPATH_CHANGED);
        evt.SetInt(event.GetInt());
        wxPostEvent(this, evt);
    }
    else if (event.GetId() == cmbBand->GetId())
    {
        evt.SetEventType(LMS7_TXBAND_CHANGED);
        evt.SetInt(event.GetInt());
        wxPostEvent(this, evt);
    }
}

void HPM7_wxgui::DownloadAll(wxCommandEvent& event)
{

    uint8_t data[64];
    size_t len = 0;
    int i=0;
    data[len++] = 0x10;
    while (len++ <= 12)
        data[len]=0x20+i++;
    data[len++] = 0x30;
    data[len++]=0x31;

    if (LMS_TransferLMS64C(lmsControl, lime::CMD_MYRIAD_RD, data, &len)!=0)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
        return;
    }

    cmbActivePath->SetSelection(data[1] & 0x3);
    cmbBand->SetSelection((data[1] >> 2) & 0x1);
    cmbLNA->SetSelection((data[1] >> 3) & 0x1);
    cmbPAdriver->SetSelection((data[1] >> 4) & 0x1);

    int index = 3;
    for (size_t i = 0; i < chkEB.size(); ++i)
    {
        cmbSSC1[i]->SetSelection(data[index] & 0x1F);
        index+=2;
        chkEB[i]->SetValue((data[index] >> 5) & 1);
        chkTP[i]->SetValue((data[index] >> 4) & 1);
        cmbSSC2[i]->SetSelection(data[index] & 0xF);
        index += 2;
    }

    cmbDAC_A->SetSelection(data[index]);
    index += 2;
    cmbDAC_B->SetSelection(data[index]);
}

void HPM7_wxgui::OnDACchange(wxCommandEvent& event)
{

    uint8_t data[64];

    if (event.GetEventObject() == cmbDAC_A)
    {
        data[0] = 0x30;
        data[1] = cmbDAC_A->GetSelection();
    }
    else if (event.GetEventObject() == cmbDAC_B)
    {
        data[0] = 0x31;
        data[1] = cmbDAC_B->GetSelection();
    }

    size_t len = 2;

    if (LMS_TransferLMS64C(lmsControl, lime::CMD_MYRIAD_WR, data, &len)!=0)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
}

void HPM7_wxgui::SelectBand(unsigned int i)
{
    if (cmbBand)
        cmbBand->SetSelection(i & 0x1);
    UploadGPIO();
}

void HPM7_wxgui::SelectRxPath(unsigned int i)
{
    if (cmbActivePath)
        cmbActivePath->SetSelection(i & 0x3);
    UploadGPIO();
}

bool HPM7_wxgui::UploadGPIO()
{


    /*LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_MYRIAD_WR;
    pkt.outBuffer.push_back(0x10);
    unsigned char value = 0;
    int activePath = cmbActivePath->GetSelection();
    switch (activePath)
    {
    case 0: value |= 0; break;
    case 1: value |= 2; break;
    case 2: value |= 3; break;
    case 3: value |= 1; break;
    }
    value |= (cmbBand->GetSelection() & 0x1) << 2;
    value |= (cmbLNA->GetSelection() & 0x1) << 3;
    value |= (cmbPAdriver->GetSelection() & 0x1) << 4;
    pkt.outBuffer.push_back(value);
    if (m_serPort->TransferPacket(pkt) != 0)
    {
        wxMessageBox(_("Uploading HPM7 GPIO, board response: ") + wxString::From8BitData(GetLastErrorMessage()), _("Warning"));
        return false;
    }*/
    return true;

}
