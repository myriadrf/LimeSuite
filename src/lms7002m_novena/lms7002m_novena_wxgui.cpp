#include "lms7002m_novena_wxgui.h"
#include "lmsComms.h"

LMS7002M_Novena_wxgui::LMS7002M_Novena_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
    :mSerPort(nullptr)
{
    Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, title);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    lms_reset = new wxCheckBox(this, wxNewId(), _("LMS_RESET"));
    lms_reset->SetValue(true);
    mainSizer->Add(lms_reset, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_reset->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    lms_rxen = new wxCheckBox(this, wxNewId(), _("LMS_Rx_EN"));
    lms_rxen->SetValue(true);
    mainSizer->Add(lms_rxen, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_rxen->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    lms_txen = new wxCheckBox(this, wxNewId(), _("LMS_Tx_EN"));
    lms_txen->SetValue(true);
    mainSizer->Add(lms_txen, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_txen->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    lms_gpio2 = new wxCheckBox(this, wxNewId(), _("LMS_GPIO2"));
    mainSizer->Add(lms_gpio2, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_gpio2->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    lms_gpio1 = new wxCheckBox(this, wxNewId(), _("LMS_GPIO1"));
    mainSizer->Add(lms_gpio1, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_gpio1->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    lms_gpio0 = new wxCheckBox(this, wxNewId(), _("LMS_GPIO0"));
    mainSizer->Add(lms_gpio0, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_gpio0->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    btnReadAll = new wxButton(this, wxNewId(), _("Read All"));
    mainSizer->Add(btnReadAll, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(btnReadAll->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::OnReadAll);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
}

LMS7002M_Novena_wxgui::~LMS7002M_Novena_wxgui()
{

}

void LMS7002M_Novena_wxgui::UpdatePanel()
{
    assert(mSerPort != nullptr);
    if (mSerPort == nullptr)
        return;

    if (mSerPort->IsOpen() == false)
    {
        wxMessageBox(_("Device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RD;
    unsigned int address = 0x0806;
    pkt.outBuffer.push_back(address >> 8);
    pkt.outBuffer.push_back(address & 0xFF);
    if (mSerPort->TransferPacket(pkt) != LMScomms::TRANSFER_SUCCESS)
    {
        wxMessageBox(_("Failed to write SPI"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    if (pkt.status != STATUS_COMPLETED_CMD)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"), wxICON_WARNING | wxOK);

    unsigned int value = (pkt.inBuffer[2] << 8) | pkt.inBuffer[3];
    lms_reset->SetValue((value >> 5)&1);
    lms_rxen->SetValue((value >> 4)&1);
    lms_txen->SetValue((value >> 3)&1);
    lms_gpio2->SetValue((value >> 2)&1);
    lms_gpio1->SetValue((value >> 1)&1);
    lms_gpio0->SetValue((value >> 0)&1);
}

void LMS7002M_Novena_wxgui::Initialize(LMScomms* serPort)
{
    assert(serPort != nullptr);
    mSerPort = serPort;
}

/**
RX active paths
lms_gpio0 | lms_gpio1      	RX_A		RX_B
    0 			0       =>  	no active path
	1   		0 		=>	LNAW_A  	LNAW_B
	0			1		=>	LNAH_A  	LNAH_B
	1			1		=>	LNAL_A 	 	LNAL_B

lms_gpio2 - tx output selection:
	0 - TX1_A and TX1_B (Band 1)
	1 - TX2_A and TX2_B (Band 2)
*/
void LMS7002M_Novena_wxgui::ParameterChangeHandler(wxCommandEvent& event)
{
    assert(mSerPort != nullptr);
    if (mSerPort == nullptr)
        return;

    if (mSerPort->IsOpen() == false)
    {
        wxMessageBox(_("Device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_WR;
    unsigned int address = 0x0806;
    pkt.outBuffer.push_back(address >> 8);
    pkt.outBuffer.push_back(address & 0xFF);
    unsigned int value = 0;
    value |= lms_reset->GetValue() << 5;
    value |= lms_rxen->GetValue() << 4;
    value |= lms_txen->GetValue() << 3;
    value |= lms_gpio2->GetValue() << 2;
    value |= lms_gpio1->GetValue() << 1;
    value |= lms_gpio0->GetValue() << 0;
    pkt.outBuffer.push_back(value >> 8);
    pkt.outBuffer.push_back(value & 0xFF);
    if (mSerPort->TransferPacket(pkt) != LMScomms::TRANSFER_SUCCESS)
    {
        wxMessageBox(_("Failed to write SPI"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    if (pkt.status != STATUS_COMPLETED_CMD)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"), wxICON_WARNING | wxOK);
}

void LMS7002M_Novena_wxgui::OnReadAll(wxCommandEvent& event)
{
    UpdatePanel();
}
