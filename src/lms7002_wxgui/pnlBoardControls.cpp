#include "pnlBoardControls.h"
#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include <assert.h>
#include "lmsComms.h"
#include <wx/spinctrl.h>

static wxString power2unitsString(char powerx3)
{
    switch (powerx3)
    {
    case -8:
        return "y";
    case -7:
        return "z";
    case -6:
        return "a";
    case -5:
        return "f";
    case -4:
        return "p";
    case -3:
        return "n";
    case -2:
        return "u";
    case -1:
        return "m";
    case 0:
        return "";
    case 1:
        return "k";
    case 2:
        return "M";
    case 3:
        return "G";
    case 4:
        return "T";
    case 5:
        return "P";
    case 6:
        return "E";
    case 7:
        return "Y";
    default:
        return "";
    }
}

pnlBoardControls::pnlBoardControls(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style), serPort(nullptr)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	wxFlexGridSizer* fgSizer247;
	fgSizer247 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer247->AddGrowableCol( 0 );
	fgSizer247->AddGrowableRow( 1 );
	fgSizer247->SetFlexibleDirection( wxBOTH );
	fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer248;
	fgSizer248 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer248->SetFlexibleDirection( wxBOTH );
	fgSizer248->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnReadAll = new wxButton( this, wxID_ANY, wxT("Read all"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer248->Add( btnReadAll, 0, wxALL, 5 );
	
	btnWriteAll = new wxButton( this, wxID_ANY, wxT("Write all"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer248->Add( btnWriteAll, 0, wxALL, 5 );
	
	m_staticText349 = new wxStaticText( this, wxID_ANY, wxT("Labels:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText349->Wrap( -1 );
	fgSizer248->Add( m_staticText349, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxArrayString cmbBoardSelectionChoices;
	cmbBoardSelection = new wxChoice( this, wxNewId(), wxDefaultPosition, wxDefaultSize, cmbBoardSelectionChoices, 0 );
	cmbBoardSelection->SetSelection( 0 );
	fgSizer248->Add( cmbBoardSelection, 0, wxALL, 5 );	

    for (int i = 0; i < LMS_DEV_COUNT; ++i)
        cmbBoardSelection->AppendString(wxString::From8BitData(GetDeviceName((eLMS_DEV)i)));
	
	fgSizer247->Add( fgSizer248, 1, wxEXPAND, 5 );
	
    pnlCustomControls = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _("Custom controls"));
    wxFlexGridSizer* sizerCustomControls = new wxFlexGridSizer(0, 5, 5, 5);

    sizerCustomControls->Add(new wxStaticText(pnlCustomControls, wxID_ANY, _("Ch.")));
    sizerCustomControls->Add(new wxStaticText(pnlCustomControls, wxID_ANY, _("Value")));
    sizerCustomControls->Add(new wxStaticText(pnlCustomControls, wxID_ANY, _("Power")));
    sizerCustomControls->Add(new wxStaticText(pnlCustomControls, wxID_ANY, _("Units")));
    sizerCustomControls->Add(new wxStaticText(pnlCustomControls, wxID_ANY, _("")));
    
    //reading
    spinCustomChannelRd = new wxSpinCtrl(pnlCustomControls, wxNewId(), _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0);
    sizerCustomControls->Add(spinCustomChannelRd, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);
    txtCustomValueRd = new wxStaticText(pnlCustomControls, wxID_ANY, _("0"));
    sizerCustomControls->Add(txtCustomValueRd, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);
    txtCustomPowerOf10Rd = new wxStaticText(pnlCustomControls, wxID_ANY, _(""));
    sizerCustomControls->Add(txtCustomPowerOf10Rd, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);
    txtCustomUnitsRd = new wxStaticText(pnlCustomControls, wxID_ANY, _(""));
    sizerCustomControls->Add(txtCustomUnitsRd, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);
    btnCustomRd = new wxButton(pnlCustomControls, wxNewId(), _("Read"));
    btnCustomRd->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(pnlBoardControls::OnCustomRead), NULL, this);
    sizerCustomControls->Add(btnCustomRd, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);

    //writing
    spinCustomChannelWr = new wxSpinCtrl(pnlCustomControls, wxNewId(), _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 255, 0);
    sizerCustomControls->Add(spinCustomChannelWr, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);
    spinCustomValueWr = new wxSpinCtrl(pnlCustomControls, wxNewId(), _(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 65535, 0);
    sizerCustomControls->Add(spinCustomValueWr, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);
    spinCustomValueWr->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(pnlBoardControls::OnSetDACvalues), NULL, this);

    wxArrayString powerChoices;
    for (int i = -8; i <= 7; ++i)
        powerChoices.push_back(wxString::From8BitData(power2unitsString(i)));
    cmbCustomPowerOf10Wr = new wxChoice(pnlCustomControls, wxNewId(), wxDefaultPosition, wxDefaultSize, powerChoices, 0);
    cmbCustomPowerOf10Wr->SetSelection(0);
    sizerCustomControls->Add(cmbCustomPowerOf10Wr, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);

    wxArrayString unitChoices;
    for (int i = 0; i < ADC_UNITS_COUNT; ++i) //add all defined units
        unitChoices.push_back(wxString::From8BitData(adcUnits2string(i)));
    for (int i = ADC_UNITS_COUNT; i < ADC_UNITS_COUNT + 4; ++i) //add some options to use undefined units
        unitChoices.push_back(wxString::Format(_("%i"), i));
    cmbCustomUnitsWr = new wxChoice(pnlCustomControls, wxNewId(), wxDefaultPosition, wxDefaultSize, unitChoices, 0);
    cmbCustomUnitsWr->SetSelection(0);
    sizerCustomControls->Add(cmbCustomUnitsWr, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);

    btnCustomWr = new wxButton(pnlCustomControls, wxNewId(), _("Write"));
    btnCustomWr->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(pnlBoardControls::OnCustomWrite), NULL, this);
    sizerCustomControls->Add(btnCustomWr, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0);

    pnlCustomControls->SetSizer(sizerCustomControls);
    pnlCustomControls->Fit();

    fgSizer247->Add(pnlCustomControls, 1, wxEXPAND, 5);

	wxFlexGridSizer* fgSizer249;
	fgSizer249 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer249->AddGrowableCol( 0 );
	fgSizer249->AddGrowableCol( 1 );
	fgSizer249->SetFlexibleDirection( wxBOTH );
	fgSizer249->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer133;
	sbSizer133 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Read") ), wxVERTICAL );
	
	sizerAnalogRd = new wxFlexGridSizer( 0, 4, 2, 2 );
	sizerAnalogRd->SetFlexibleDirection( wxBOTH );
	sizerAnalogRd->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    sizerAnalogRd->Add(new wxStaticText(this, wxID_ANY, _("Ch.")), 1, wxALL, 5);
    sizerAnalogRd->Add(new wxStaticText(this, wxID_ANY, _("Name")), 1, wxALL, 5);
    sizerAnalogRd->Add(new wxStaticText(this, wxID_ANY, _("Value")), 1, wxALL, 5);
    sizerAnalogRd->Add(new wxStaticText(this, wxID_ANY, _("Units")), 1, wxALL, 5);
	
	
	sbSizer133->Add( sizerAnalogRd, 1, wxEXPAND, 5 );
	
	
	fgSizer249->Add( sbSizer133, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer134;
	sbSizer134 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Write") ), wxVERTICAL );
	
	sizerAnalogWr = new wxFlexGridSizer( 0, 4, 2, 2 );
	sizerAnalogWr->SetFlexibleDirection( wxBOTH );
	sizerAnalogWr->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	sbSizer134->Add( sizerAnalogWr, 1, wxEXPAND, 5 );
    sizerAnalogWr->Add(new wxStaticText(this, wxID_ANY, _("Ch.")), 1, wxALL, 5);
    sizerAnalogWr->Add(new wxStaticText(this, wxID_ANY, _("Name")), 1, wxALL, 5);
    sizerAnalogWr->Add(new wxStaticText(this, wxID_ANY, _("Value")), 1, wxALL, 5);
    sizerAnalogWr->Add(new wxStaticText(this, wxID_ANY, _("Units")), 1, wxALL, 5);
	
	
	fgSizer249->Add( sbSizer134, 1, wxEXPAND, 5 );
	
	
	fgSizer247->Add( fgSizer249, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer247 );
	this->Layout();
	fgSizer247->Fit( this );
	
	// Connect Events
    cmbBoardSelection->Connect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(pnlBoardControls::OnUserChangedBoardType), NULL, this);
	btnReadAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBoardControls::OnReadAll ), NULL, this );
	btnWriteAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBoardControls::OnWriteAll ), NULL, this );

    SetupControls(LMS_DEV_UNKNOWN);
}

pnlBoardControls::~pnlBoardControls()
{
	// Disconnect Events
	btnReadAll->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBoardControls::OnReadAll ), NULL, this );
	btnWriteAll->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBoardControls::OnWriteAll ), NULL, this );
	cmbBoardSelection->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( pnlBoardControls::OnUserChangedBoardType), NULL, this );	
}

void pnlBoardControls::OnReadAll( wxCommandEvent& event )
{   
    if (serPort == nullptr || serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_RD;
    
    for (const auto& param : mADCparameters)
        pkt.outBuffer.push_back(param.channel);
    
    LMScomms::TransferStatus status = serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
        return;
    }

    assert(pkt.inBuffer.size() >= 4 * mADCparameters.size());

    for (int i = 0; i < mADCparameters.size(); ++i)
    {
        mADCparameters[i].channel = pkt.inBuffer[i * 4];
        mADCparameters[i].units = (pkt.inBuffer[i * 4 + 1] & 0xF0) >> 4;
        mADCparameters[i].powerOf10 = pkt.inBuffer[i * 4 + 1] & 0x0F;
        mADCparameters[i].value = pkt.inBuffer[i * 4 + 2] << 8 | pkt.inBuffer[i * 4 + 3];
    }
    for (int i = 0; i < mDACparameters.size(); ++i)
    {
        for (int j = 0; j < mADCparameters.size(); ++j)
        {
            if (mDACparameters[i].channel == mADCparameters[j].channel)
            {
                mDACparameters[i].powerOf10 = mADCparameters[j].powerOf10;
                mDACparameters[i].value = mADCparameters[j].value;
                mDACparameters[i].units = mADCparameters[j].units;
                mDAC_GUI_widgets[i]->value->SetValue(mDACparameters[i].value);
                break;
            }
        }
    }
    UpdatePanel();
}

void pnlBoardControls::OnWriteAll( wxCommandEvent& event )
{       
    if (serPort == nullptr || serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_WR;

    for (int i = 0; i < mDACparameters.size(); ++i)
    {
        pkt.outBuffer.push_back(mDACparameters[i].channel);
        pkt.outBuffer.push_back(mDACparameters[i].units << 4 | mDACparameters[i].powerOf10);
        pkt.outBuffer.push_back(mDACparameters[i].value >> 8);
        pkt.outBuffer.push_back(mDACparameters[i].value & 0xFF);
    }
    assert(serPort != nullptr);
    LMScomms::TransferStatus status = serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
        return;
    }
}
    
void pnlBoardControls::Initialize(LMScomms* controlPort)
{
    assert(controlPort != nullptr);
    serPort = controlPort;
    SetupControls(serPort->GetInfo().device);
    if (serPort->IsOpen())
    {
        wxCommandEvent evt;
        OnReadAll(evt);
    }
}

void pnlBoardControls::UpdatePanel()
{
    assert(mADCparameters.size() == mADC_GUI_widgets.size());
    for (int i = 0; i < mADCparameters.size(); ++i)
    {
        mADC_GUI_widgets[i]->channel->SetLabel(wxString::Format(_("%i"), mADCparameters[i].channel));
        mADC_GUI_widgets[i]->title->SetLabel(wxString(mADCparameters[i].name));
        if (mADCparameters[i].units == TEMPERATURE)
            mADC_GUI_widgets[i]->value->SetLabel(wxString::Format(_("%.1f"), mADCparameters[i].value/10.0));
        else
            mADC_GUI_widgets[i]->value->SetLabel(wxString::Format(_("%i"), mADCparameters[i].value));
        mADC_GUI_widgets[i]->units->SetLabelText(wxString::Format("%s%s", power2unitsString(mADCparameters[i].powerOf10), adcUnits2string(mADCparameters[i].units)));
    }
}

std::vector<pnlBoardControls::ADC_DAC> pnlBoardControls::getBoardADCs(eLMS_DEV boardID)
{
    std::vector<ADC_DAC> paramList;
    if (boardID == LMS_DEV_SODERA)
    {
        ADC_DAC dacVoltage;
        dacVoltage.name = "DAC";
        dacVoltage.channel = 0;
        dacVoltage.powerOf10 = 0;
        dacVoltage.units = VOLTAGE;
        dacVoltage.value = 0;
        paramList.push_back(dacVoltage);

        ADC_DAC temperature;
        temperature.name = "Temperature";
        temperature.channel = 1;
        temperature.powerOf10 = 0;
        temperature.units = TEMPERATURE;
        temperature.value = 0;
        paramList.push_back(temperature);
    }
    return paramList;
}

std::vector<pnlBoardControls::ADC_DAC> pnlBoardControls::getBoardDACs(eLMS_DEV boardID)
{
    std::vector<ADC_DAC> paramList;
    if (boardID == LMS_DEV_SODERA)
    {
        ADC_DAC dacVoltage;
        dacVoltage.name = "DAC";
        dacVoltage.channel = 0;
        dacVoltage.powerOf10 = 0;
        dacVoltage.units = RAW;
        dacVoltage.value = 0;
        dacVoltage.minValue = 0;
        dacVoltage.maxValue = 255;

        paramList.push_back(dacVoltage);
    }
    return paramList;
}

void pnlBoardControls::SetupControls(eLMS_DEV boardID)
{
    if (boardID == LMS_DEV_UNKNOWN)
        pnlCustomControls->Show();
    else
        pnlCustomControls->Hide();
    cmbBoardSelection->SetSelection(boardID);
    for (auto &widget : mADC_GUI_widgets)
        delete widget;
    mADC_GUI_widgets.clear(); //delete previously existing controls
    mADCparameters = getBoardADCs(boardID); //update controls list by board type

    for (auto &widget : mDAC_GUI_widgets)
        delete widget;
    mDAC_GUI_widgets.clear(); //delete previously existing controls
    mDACparameters = getBoardDACs(boardID); //update controls list by board type

    if (boardID != LMS_DEV_UNKNOWN)
    {   
        for (int i = 0; i < mADCparameters.size(); ++i)
        {
            ADC_GUI *gui = new ADC_GUI();
            gui->channel = new wxStaticText(this, wxID_ANY, wxString::Format(_("%i"), mADCparameters[i].channel));
            gui->title = new wxStaticText(this, wxID_ANY, wxString(mADCparameters[i].name));
            gui->value = new wxStaticText(this, wxID_ANY, _(""));
            gui->units = new wxStaticText(this, wxID_ANY, wxString::Format("%s%s", power2unitsString(mADCparameters[i].powerOf10), adcUnits2string(mADCparameters[i].units)));
            mADC_GUI_widgets.push_back(gui);
            sizerAnalogRd->Add(gui->channel, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
            sizerAnalogRd->Add(gui->title, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
            sizerAnalogRd->Add(gui->value, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
            sizerAnalogRd->Add(gui->units, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
        }

        for (int i = 0; i < mDACparameters.size(); ++i)
        {
            DAC_GUI *gui = new DAC_GUI();
            gui->channel = new wxStaticText(this, wxID_ANY, wxString::Format(_("%i"), mDACparameters[i].channel));
            gui->title = new wxStaticText(this, wxID_ANY, wxString(mDACparameters[i].name));
            gui->value = new wxSpinCtrl(this, wxNewId(), _(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, mDACparameters[i].minValue, mDACparameters[i].maxValue, mDACparameters[i].minValue);
            gui->value->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(pnlBoardControls::OnSetDACvalues), NULL, this);
            gui->value->Connect(wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlBoardControls::OnSetDACvaluesENTER), NULL, this);
            gui->units = new wxStaticText(this, wxID_ANY, wxString::Format("%s%s", power2unitsString(mDACparameters[i].powerOf10), adcUnits2string(mDACparameters[i].units)));
            mDAC_GUI_widgets.push_back(gui);
            sizerAnalogWr->Add(gui->channel, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
            sizerAnalogWr->Add(gui->title, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
            sizerAnalogWr->Add(gui->value, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
            sizerAnalogWr->Add(gui->units, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
        }
    }
    sizerAnalogRd->Layout();
    sizerAnalogWr->Layout();
    Fit();
}

void pnlBoardControls::OnSetDACvaluesENTER(wxCommandEvent &event)
{   
    wxSpinEvent evt;
    evt.SetEventObject(event.GetEventObject());
    OnSetDACvalues(evt);
}

void pnlBoardControls::OnSetDACvalues(wxSpinEvent &event)
{
    for (int i = 0; i < mDAC_GUI_widgets.size(); ++i)
    {
        if (event.GetEventObject() == mDAC_GUI_widgets[i]->value)
        {
            mDACparameters[i].value = mDAC_GUI_widgets[i]->value->GetValue();
            //write to chip
            LMScomms::GenericPacket pkt;
            pkt.cmd = CMD_ANALOG_VAL_WR;
            pkt.outBuffer.push_back(mDACparameters[i].channel);
            pkt.outBuffer.push_back(mDACparameters[i].units << 4 | mDACparameters[i].powerOf10);
            pkt.outBuffer.push_back(mDACparameters[i].value >> 8);
            pkt.outBuffer.push_back(mDACparameters[i].value & 0xFF);

            assert(serPort != nullptr);
            if (serPort->IsOpen() == false)
                return;

            LMScomms::TransferStatus status = serPort->TransferPacket(pkt);
            if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
                wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
            return;
        }
    }
}

void pnlBoardControls::OnUserChangedBoardType(wxCommandEvent& event)
{
    SetupControls((eLMS_DEV)cmbBoardSelection->GetSelection());
}

void pnlBoardControls::OnCustomRead(wxCommandEvent& event)
{
    if (serPort == nullptr || serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_RD;

    pkt.outBuffer.push_back(spinCustomChannelRd->GetValue());

    LMScomms::TransferStatus status = serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
        return;
    }

    assert(pkt.inBuffer.size() >= 4 * mADCparameters.size());

    spinCustomChannelRd->SetValue(pkt.inBuffer[0]);
    int units = (pkt.inBuffer[1] & 0xF0) >> 4;
    txtCustomUnitsRd->SetLabel(wxString::From8BitData(adcUnits2string(units)));
    txtCustomPowerOf10Rd->SetLabel(wxString::From8BitData(power2unitsString(pkt.inBuffer[1] & 0x0F)));
    int16_t value = ((int16_t)pkt.inBuffer[2]) << 8 | pkt.inBuffer[3];
    if (units == TEMPERATURE)
        txtCustomValueRd->SetLabel(wxString::Format("%.1f", value/10.0));
    else
        txtCustomValueRd->SetLabel(wxString::Format("%i", value));
}

void pnlBoardControls::OnCustomWrite(wxCommandEvent& event)
{
    if (serPort == nullptr || serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_WR;
    pkt.outBuffer.push_back(spinCustomChannelWr->GetValue());
    pkt.outBuffer.push_back(cmbCustomUnitsWr->GetSelection() << 4 | cmbCustomPowerOf10Wr->GetSelection());
    int16_t value = spinCustomValueWr->GetValue();
    pkt.outBuffer.push_back(value >> 8);
    pkt.outBuffer.push_back(value & 0xFF);
    
    assert(serPort != nullptr);
    LMScomms::TransferStatus status = serPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS || pkt.status != STATUS_COMPLETED_CMD)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(status2string(pkt.status)), _("Warning"));
        return;
    }
}