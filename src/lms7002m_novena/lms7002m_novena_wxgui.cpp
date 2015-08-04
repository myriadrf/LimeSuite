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
    mainSizer->Add(lms_reset, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_reset->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    lms_rxen = new wxCheckBox(this, wxNewId(), _("LMS_Rx_EN"));
    mainSizer->Add(lms_rxen, 1, wxALL | wxALIGN_LEFT | wxALIGN_TOP, 0);
    Connect(lms_rxen->GetId(), wxEVT_CHECKBOX, (wxObjectEventFunction)&LMS7002M_Novena_wxgui::ParameterChangeHandler);

    lms_txen = new wxCheckBox(this, wxNewId(), _("LMS_Tx_EN"));
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
        
    SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
}

LMS7002M_Novena_wxgui::~LMS7002M_Novena_wxgui()
{

}

void LMS7002M_Novena_wxgui::Initialize(LMScomms* serPort)
{
    assert(serPort != nullptr);
    mSerPort = serPort;
}

void LMS7002M_Novena_wxgui::UpdatePanel()
{

}

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
    pkt.outBuffer.push_back(value);
    if (mSerPort->TransferPacket(pkt) != LMScomms::TRANSFER_SUCCESS)
    {
        wxMessageBox(_("Failed to write SPI"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    if (pkt.status != STATUS_COMPLETED_CMD)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"), wxICON_WARNING | wxOK);
}