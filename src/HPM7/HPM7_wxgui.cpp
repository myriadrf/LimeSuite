/**
@file 	HPM7_wxgui.cpp
@author Lime Microsystems
@brief 	panel for interacting with HPM7 board
*/
#include "HPM7_wxgui.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <vector>
#include "lmsComms.h"

BEGIN_EVENT_TABLE(HPM7_wxgui, wxFrame)

END_EVENT_TABLE()

HPM7_wxgui::HPM7_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
{
    m_serPort = nullptr;    
    Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, _T("id"));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
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
    wxStaticBoxSizer* gpioGroup = new wxStaticBoxSizer(wxVERTICAL, this, _("GPIOS"));
    for (int i = 0; i < 5; ++i)
    {
        chkGPIO.push_back(new wxCheckBox(this, wxNewId(), wxString::Format(_("GPIO%i"), i)));
        gpioGroup->Add(chkGPIO[i], 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        Connect(chkGPIO[i]->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&HPM7_wxgui::OnGPIOchange);
    }
    leftCollumn->Add(gpioGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);

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

void HPM7_wxgui::Initialize(LMScomms* serPort)
{
	m_serPort = serPort;
	assert(m_serPort != nullptr);
}

HPM7_wxgui::~HPM7_wxgui()
{

}

void HPM7_wxgui::OnTunerSSC1change(wxCommandEvent& event)
{
    assert(m_serPort != nullptr);
    if (m_serPort == nullptr || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }
        
    int tunerIndex = 0;
    for (tunerIndex = 0; tunerIndex < cmbSSC1.size(); ++tunerIndex)
        if (event.GetId() == cmbSSC1[tunerIndex]->GetId())
            break;
    if (tunerIndex >= cmbSSC1.size())
        return;

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_MYRIAD_WR;
    pkt.outBuffer.push_back( 0x20 + tunerIndex * 2 );
    pkt.outBuffer.push_back( event.GetInt() );
    LMScomms::TransferStatus status = m_serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
}

void HPM7_wxgui::OnTunerSSC2change(wxCommandEvent& event)
{
    assert(m_serPort != nullptr);
    if (m_serPort == nullptr || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }
    int tunerIndex = 0;
    for (tunerIndex = 0; tunerIndex < tunerIds.size(); ++tunerIndex)
        if (event.GetId() == tunerIds[tunerIndex])
            break;

    if (tunerIndex >= tunerIds.size())
        return;

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_MYRIAD_WR;
    unsigned char address = (0x21 + tunerIndex * 2);
    pkt.outBuffer.push_back(0x21 + tunerIndex*2);
    unsigned char value = chkEB[tunerIndex]->GetValue() << 5;    
    value |= chkTP[tunerIndex]->GetValue() << 4;
    value |= (cmbSSC2[tunerIndex]->GetSelection() & 0xF);
    pkt.outBuffer.push_back(value);

    LMScomms::TransferStatus status = m_serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
}

void HPM7_wxgui::OnGPIOchange(wxCommandEvent& event)
{
    assert(m_serPort != nullptr);
    if (m_serPort == nullptr || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_MYRIAD_WR;
    pkt.outBuffer.push_back(0x10);    
    unsigned char value = 0;
    for (int i = 0; i < chkGPIO.size(); ++i)
        value |= (chkGPIO[i]->GetValue() << i);
    pkt.outBuffer.push_back( value );
    LMScomms::TransferStatus status = m_serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
}

void HPM7_wxgui::DownloadAll(wxCommandEvent& event)
{
    assert(m_serPort != nullptr);
    if (m_serPort == nullptr || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }
    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_MYRIAD_RD;
    pkt.outBuffer.push_back(0x10);
    for (int i = 0; i < 12; ++i)
        pkt.outBuffer.push_back(0x20 + i);
    pkt.outBuffer.push_back(0x30);
    pkt.outBuffer.push_back(0x31);

    LMScomms::TransferStatus status = m_serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
        return;
    }

    assert(pkt.inBuffer.size() >= 14);
    for (int i = 0; i < chkGPIO.size(); ++i)
        chkGPIO[i]->SetValue((pkt.inBuffer[1] >> i) & 1);

    int index = 3;
    for (int i = 0; i < chkEB.size(); ++i)
    {        
        cmbSSC1[i]->SetSelection(pkt.inBuffer[index] & 0x1F);
        index+=2;
        chkEB[i]->SetValue((pkt.inBuffer[index] >> 5) & 1);
        chkTP[i]->SetValue((pkt.inBuffer[index] >> 4) & 1);
        cmbSSC2[i]->SetSelection(pkt.inBuffer[index] & 0xF);
        index += 2;
    }

    cmbDAC_A->SetSelection(pkt.inBuffer[index]);
    index += 2;
    cmbDAC_B->SetSelection(pkt.inBuffer[index]);
}

void HPM7_wxgui::OnDACchange(wxCommandEvent& event)
{
    assert(m_serPort != nullptr);
    if (m_serPort == nullptr || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }
    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_MYRIAD_WR;

    if (event.GetEventObject() == cmbDAC_A)
    {
        pkt.outBuffer.push_back(0x30);
        pkt.outBuffer.push_back(cmbDAC_A->GetSelection());
    }
    else if (event.GetEventObject() == cmbDAC_B)
    {
        pkt.outBuffer.push_back(0x31);
        pkt.outBuffer.push_back(cmbDAC_B->GetSelection());
    }
    LMScomms::TransferStatus status = m_serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
}