///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "utilities_gui.h"

///////////////////////////////////////////////////////////////////////////

dlgConnectionSettings_view::dlgConnectionSettings_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer211;
	fgSizer211 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer211->AddGrowableCol( 0 );
	fgSizer211->AddGrowableRow( 0 );
	fgSizer211->SetFlexibleDirection( wxBOTH );
	fgSizer211->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer212;
	fgSizer212 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer212->AddGrowableCol( 0 );
	fgSizer212->AddGrowableRow( 1 );
	fgSizer212->SetFlexibleDirection( wxBOTH );
	fgSizer212->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText472 = new wxStaticText( this, wxID_ANY, wxT("Board connections:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText472->Wrap( -1 );
	fgSizer212->Add( m_staticText472, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	mListLMS7ports = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	mListLMS7ports->SetMinSize( wxSize( 400,150 ) );
	
	fgSizer212->Add( mListLMS7ports, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer211->Add( fgSizer212, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer213;
	fgSizer213 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer213->AddGrowableCol( 0 );
	fgSizer213->AddGrowableCol( 1 );
	fgSizer213->AddGrowableCol( 2 );
	fgSizer213->SetFlexibleDirection( wxBOTH );
	fgSizer213->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnConnect = new wxButton( this, wxID_ANY, wxT("Connect"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnConnect, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnCancel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	btnDisconnect = new wxButton( this, wxID_ANY, wxT("Disconnect"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnDisconnect, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	fgSizer211->Add( fgSizer213, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	this->SetSizer( fgSizer211 );
	this->Layout();
	fgSizer211->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( dlgConnectionSettings_view::GetDeviceList ) );
	btnConnect->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnConnect ), NULL, this );
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnCancel ), NULL, this );
	btnDisconnect->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnDisconnect ), NULL, this );
}

dlgConnectionSettings_view::~dlgConnectionSettings_view()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( dlgConnectionSettings_view::GetDeviceList ) );
	btnConnect->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnConnect ), NULL, this );
	btnCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnCancel ), NULL, this );
	btnDisconnect->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnDisconnect ), NULL, this );
	
}

pnlMiniLog_view::pnlMiniLog_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer4->AddGrowableCol( 0 );
	fgSizer4->AddGrowableRow( 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	txtMessageField = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2 );
	txtMessageField->SetMinSize( wxSize( 300,48 ) );
	
	fgSizer4->Add( txtMessageField, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnClear = new wxButton( this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
	btnClear->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer5->Add( btnClear, 0, wxEXPAND, 5 );
	
	btnFullLog = new wxButton( this, wxID_ANY, wxT("Show Log"), wxDefaultPosition, wxDefaultSize, 0 );
	btnFullLog->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer5->Add( btnFullLog, 0, wxEXPAND, 5 );
	
	chkLogData = new wxCheckBox( this, wxID_ANY, wxT("Log data"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( chkLogData, 0, 0, 5 );
	
	
	fgSizer4->Add( fgSizer5, 1, 0, 5 );
	
	
	this->SetSizer( fgSizer4 );
	this->Layout();
	fgSizer4->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( pnlMiniLog_view::OnUpdateGUI ) );
	btnClear->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnBtnClearClicked ), NULL, this );
	btnFullLog->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnShowFullLog ), NULL, this );
	chkLogData->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnLogDataClicked ), NULL, this );
}

pnlMiniLog_view::~pnlMiniLog_view()
{
	// Disconnect Events
	this->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( pnlMiniLog_view::OnUpdateGUI ) );
	btnClear->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnBtnClearClicked ), NULL, this );
	btnFullLog->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnShowFullLog ), NULL, this );
	chkLogData->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnLogDataClicked ), NULL, this );
	
}

dlgFullMessageLog_view::dlgFullMessageLog_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 500,200 ), wxDefaultSize );
	
	wxFlexGridSizer* fgSizer19;
	fgSizer19 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer19->AddGrowableCol( 0 );
	fgSizer19->AddGrowableRow( 0 );
	fgSizer19->SetFlexibleDirection( wxBOTH );
	fgSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	txtMessageField = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2 );
	txtMessageField->SetMinSize( wxSize( 300,100 ) );
	
	fgSizer19->Add( txtMessageField, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer19 );
	this->Layout();
	fgSizer19->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgFullMessageLog_view::~dlgFullMessageLog_view()
{
}

dlgDeviceInfo_view::dlgDeviceInfo_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnGetInfo = new wxButton( this, wxID_ANY, wxT("GET INFO"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer12->Add( btnGetInfo, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerControlPort;
	sbSizerControlPort = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Control port") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText6 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Device:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer8->Add( m_staticText6, 0, wxALL, 5 );
	
	lblDeviceCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblDeviceCtr->Wrap( -1 );
	fgSizer8->Add( lblDeviceCtr, 0, wxALL, 5 );
	
	m_staticText8 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Expansion:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	fgSizer8->Add( m_staticText8, 0, wxALL, 5 );
	
	lblExpansionCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblExpansionCtr->Wrap( -1 );
	fgSizer8->Add( lblExpansionCtr, 0, wxALL, 5 );
	
	m_staticText10 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Firmware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer8->Add( m_staticText10, 0, wxALL, 5 );
	
	lblFirmwareCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFirmwareCtr->Wrap( -1 );
	fgSizer8->Add( lblFirmwareCtr, 0, wxALL, 5 );
	
	m_staticText12 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Hardware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer8->Add( m_staticText12, 0, wxALL, 5 );
	
	lblHardwareCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHardwareCtr->Wrap( -1 );
	fgSizer8->Add( lblHardwareCtr, 0, wxALL, 5 );
	
	m_staticText14 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	fgSizer8->Add( m_staticText14, 0, wxALL, 5 );
	
	lblProtocolCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblProtocolCtr->Wrap( -1 );
	fgSizer8->Add( lblProtocolCtr, 0, wxALL, 5 );
	
	m_staticText41 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Gateware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	fgSizer8->Add( m_staticText41, 0, wxALL, 5 );
	
	lblGatewareCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareCtr->Wrap( -1 );
	fgSizer8->Add( lblGatewareCtr, 0, wxALL, 5 );
	
	m_staticText43 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Gateware rev:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText43->Wrap( -1 );
	fgSizer8->Add( m_staticText43, 0, wxALL, 5 );
	
	lblGatewareRevCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareRevCtr->Wrap( -1 );
	fgSizer8->Add( lblGatewareRevCtr, 0, wxALL, 5 );
	
	m_staticText45 = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("Gateware target:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText45->Wrap( -1 );
	fgSizer8->Add( m_staticText45, 0, wxALL, 5 );
	
	lblGatewareTargetCtr = new wxStaticText( sbSizerControlPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareTargetCtr->Wrap( -1 );
	fgSizer8->Add( lblGatewareTargetCtr, 0, wxALL, 5 );
	
	
	sbSizerControlPort->Add( fgSizer8, 1, wxEXPAND, 5 );
	
	
	fgSizer7->Add( sbSizerControlPort, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerDataPort;
	sbSizerDataPort = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Data port") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer81;
	fgSizer81 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer81->SetFlexibleDirection( wxBOTH );
	fgSizer81->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText61 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Device:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText61->Wrap( -1 );
	fgSizer81->Add( m_staticText61, 0, wxALL, 5 );
	
	lblDeviceData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblDeviceData->Wrap( -1 );
	fgSizer81->Add( lblDeviceData, 0, wxALL, 5 );
	
	m_staticText81 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Expansion:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	fgSizer81->Add( m_staticText81, 0, wxALL, 5 );
	
	lblExpansionData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblExpansionData->Wrap( -1 );
	fgSizer81->Add( lblExpansionData, 0, wxALL, 5 );
	
	m_staticText101 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Firmware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	fgSizer81->Add( m_staticText101, 0, wxALL, 5 );
	
	lblFirmwareData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFirmwareData->Wrap( -1 );
	fgSizer81->Add( lblFirmwareData, 0, wxALL, 5 );
	
	m_staticText121 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Hardware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText121->Wrap( -1 );
	fgSizer81->Add( m_staticText121, 0, wxALL, 5 );
	
	lblHardwareData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHardwareData->Wrap( -1 );
	fgSizer81->Add( lblHardwareData, 0, wxALL, 5 );
	
	m_staticText141 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText141->Wrap( -1 );
	fgSizer81->Add( m_staticText141, 0, wxALL, 5 );
	
	lblProtocolData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblProtocolData->Wrap( -1 );
	fgSizer81->Add( lblProtocolData, 0, wxALL, 5 );
	
	m_staticText47 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Gateware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText47->Wrap( -1 );
	fgSizer81->Add( m_staticText47, 0, wxALL, 5 );
	
	lblGatewareData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareData->Wrap( -1 );
	fgSizer81->Add( lblGatewareData, 0, wxALL, 5 );
	
	m_staticText49 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Gateware rev:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText49->Wrap( -1 );
	fgSizer81->Add( m_staticText49, 0, wxALL, 5 );
	
	lblGatewareRevData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareRevData->Wrap( -1 );
	fgSizer81->Add( lblGatewareRevData, 0, wxALL, 5 );
	
	m_staticText51 = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("Gateware target:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText51->Wrap( -1 );
	fgSizer81->Add( m_staticText51, 0, wxALL, 5 );
	
	lblGatewareTargetData = new wxStaticText( sbSizerDataPort->GetStaticBox(), wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareTargetData->Wrap( -1 );
	fgSizer81->Add( lblGatewareTargetData, 0, wxALL, 5 );
	
	
	sbSizerDataPort->Add( fgSizer81, 1, wxEXPAND, 5 );
	
	
	fgSizer7->Add( sbSizerDataPort, 1, wxEXPAND, 5 );
	
	
	fgSizer12->Add( fgSizer7, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer12 );
	this->Layout();
	fgSizer12->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	btnGetInfo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgDeviceInfo_view::OnGetInfo ), NULL, this );
}

dlgDeviceInfo_view::~dlgDeviceInfo_view()
{
	// Disconnect Events
	btnGetInfo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgDeviceInfo_view::OnGetInfo ), NULL, this );
	
}

SPI_view::SPI_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxFlexGridSizer* fgSizer305;
	fgSizer305 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer305->SetFlexibleDirection( wxBOTH );
	fgSizer305->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer160;
	sbSizer160 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LMS Write ") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer306;
	fgSizer306 = new wxFlexGridSizer( 0, 7, 5, 5 );
	fgSizer306->SetFlexibleDirection( wxBOTH );
	fgSizer306->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( 0 );
	fgSizer306->Add( ID_STATICTEXT1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtLMSwriteAddr = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr->SetMaxLength( 0 ); 
	txtLMSwriteAddr->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer306->Add( txtLMSwriteAddr, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( 0 );
	fgSizer306->Add( ID_STATICTEXT9, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtLMSwriteValue = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue->SetMaxLength( 0 ); 
	txtLMSwriteValue->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer306->Add( txtLMSwriteValue, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("CMD Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite->SetDefault(); 
	fgSizer306->Add( btnLMSwrite, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( 0 );
	fgSizer306->Add( ID_STATICTEXT5, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus->Wrap( 0 );
	fgSizer306->Add( lblLMSwriteStatus, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer306, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer305->Add( sbSizer160, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer161;
	sbSizer161 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LMS Read") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer307;
	fgSizer307 = new wxFlexGridSizer( 0, 7, 5, 5 );
	fgSizer307->SetFlexibleDirection( wxBOTH );
	fgSizer307->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT2 = new wxStaticText( sbSizer161->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( 0 );
	fgSizer307->Add( ID_STATICTEXT2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtLMSreadAddr = new wxTextCtrl( sbSizer161->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSreadAddr->SetMaxLength( 0 ); 
	txtLMSreadAddr->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer307->Add( txtLMSreadAddr, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread = new wxButton( sbSizer161->GetStaticBox(), wxID_ANY, wxT("CMD Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread->SetDefault(); 
	fgSizer307->Add( btnLMSread, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer308;
	fgSizer308 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer308->SetFlexibleDirection( wxBOTH );
	fgSizer308->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer161->GetStaticBox(), wxID_ANY, wxT("Read Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer308->Add( ID_STATICTEXT3, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSreadValue = new wxStaticText( sbSizer161->GetStaticBox(), wxID_ANY, wxT("????"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLMSreadValue->Wrap( 0 );
	fgSizer308->Add( lblLMSreadValue, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizer161->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer308->Add( ID_STATICTEXT7, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSreadStatus = new wxStaticText( sbSizer161->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 143,13 ), 0 );
	lblLMSreadStatus->Wrap( 0 );
	fgSizer308->Add( lblLMSreadStatus, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer307->Add( fgSizer308, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer309;
	fgSizer309 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer309->SetFlexibleDirection( wxBOTH );
	fgSizer309->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_BUTTON23 = new wxButton( sbSizer161->GetStaticBox(), wxID_ANY, wxT("PurgeCOM"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON23->SetDefault(); 
	ID_BUTTON23->Hide();
	
	fgSizer309->Add( ID_BUTTON23, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_CHECKBOX5 = new wxCheckBox( sbSizer161->GetStaticBox(), wxID_ANY, wxT("Test written data"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_CHECKBOX5->SetValue(true); 
	ID_CHECKBOX5->Hide();
	ID_CHECKBOX5->SetToolTip( wxT("After writing register, reads back the same register and checks if values match") );
	
	fgSizer309->Add( ID_CHECKBOX5, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer307->Add( fgSizer309, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizer161->Add( fgSizer307, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer305->Add( sbSizer161, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer168;
	sbSizer168 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Board SPI") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer319;
	fgSizer319 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer319->AddGrowableCol( 0 );
	fgSizer319->SetFlexibleDirection( wxBOTH );
	fgSizer319->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer169;
	sbSizer169 = new wxStaticBoxSizer( new wxStaticBox( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer320;
	fgSizer320 = new wxFlexGridSizer( 0, 7, 5, 5 );
	fgSizer320->SetFlexibleDirection( wxBOTH );
	fgSizer320->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizer169->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( 0 );
	fgSizer320->Add( ID_STATICTEXT15, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBoardwriteAddr = new wxTextCtrl( sbSizer169->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr->SetMaxLength( 0 ); 
	txtBoardwriteAddr->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer320->Add( txtBoardwriteAddr, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer169->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( 0 );
	fgSizer320->Add( ID_STATICTEXT16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBoardwriteValue = new wxTextCtrl( sbSizer169->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue->SetMaxLength( 0 ); 
	txtBoardwriteValue->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer320->Add( txtBoardwriteValue, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON24 = new wxButton( sbSizer169->GetStaticBox(), wxID_ANY, wxT("CMD Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON24->SetDefault(); 
	fgSizer320->Add( ID_BUTTON24, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT17 = new wxStaticText( sbSizer169->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT17->Wrap( 0 );
	fgSizer320->Add( ID_STATICTEXT17, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus = new wxStaticText( sbSizer169->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus->Wrap( 0 );
	fgSizer320->Add( lblBoardwriteStatus, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer169->Add( fgSizer320, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer319->Add( sbSizer169, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer170;
	sbSizer170 = new wxStaticBoxSizer( new wxStaticBox( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer321;
	fgSizer321 = new wxFlexGridSizer( 0, 7, 5, 5 );
	fgSizer321->SetFlexibleDirection( wxBOTH );
	fgSizer321->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT19 = new wxStaticText( sbSizer170->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT19->Wrap( 0 );
	fgSizer321->Add( ID_STATICTEXT19, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBoardreadAddr = new wxTextCtrl( sbSizer170->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardreadAddr->SetMaxLength( 0 ); 
	txtBoardreadAddr->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer321->Add( txtBoardreadAddr, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON25 = new wxButton( sbSizer170->GetStaticBox(), wxID_ANY, wxT("CMD Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON25->SetDefault(); 
	fgSizer321->Add( ID_BUTTON25, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer322;
	fgSizer322 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer322->SetFlexibleDirection( wxBOTH );
	fgSizer322->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT20 = new wxStaticText( sbSizer170->GetStaticBox(), wxID_ANY, wxT("Read Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT20->Wrap( -1 );
	fgSizer322->Add( ID_STATICTEXT20, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardreadValue = new wxStaticText( sbSizer170->GetStaticBox(), wxID_ANY, wxT("????"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBoardreadValue->Wrap( 0 );
	fgSizer322->Add( lblBoardreadValue, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT22 = new wxStaticText( sbSizer170->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT22->Wrap( 0 );
	fgSizer322->Add( ID_STATICTEXT22, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardreadStatus = new wxStaticText( sbSizer170->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardreadStatus->Wrap( 0 );
	fgSizer322->Add( lblBoardreadStatus, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer321->Add( fgSizer322, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer170->Add( fgSizer321, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer319->Add( sbSizer170, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizer168->Add( fgSizer319, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	bSizer3->Add( sbSizer168, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	
	fgSizer305->Add( bSizer3, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer305 );
	this->Layout();
	fgSizer305->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	btnLMSwrite->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	ID_BUTTON24->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON25->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
}

SPI_view::~SPI_view()
{
	// Disconnect Events
	btnLMSwrite->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	ID_BUTTON24->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON25->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	
}
