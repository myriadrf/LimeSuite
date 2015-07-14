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
	fgSizer211->SetFlexibleDirection( wxBOTH );
	fgSizer211->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer212;
	fgSizer212 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer212->AddGrowableCol( 0 );
	fgSizer212->AddGrowableCol( 1 );
	fgSizer212->AddGrowableRow( 1 );
	fgSizer212->SetFlexibleDirection( wxBOTH );
	fgSizer212->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText472 = new wxStaticText( this, wxID_ANY, wxT("LMS7 control:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText472->Wrap( -1 );
	fgSizer212->Add( m_staticText472, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_staticText473 = new wxStaticText( this, wxID_ANY, wxT("Stream board:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText473->Wrap( -1 );
	fgSizer212->Add( m_staticText473, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	mListLMS7ports = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,150 ), 0, NULL, 0 ); 
	fgSizer212->Add( mListLMS7ports, 0, wxALL|wxEXPAND, 5 );
	
	mListStreamports = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer212->Add( mListStreamports, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer211->Add( fgSizer212, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer213;
	fgSizer213 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer213->AddGrowableCol( 0 );
	fgSizer213->AddGrowableCol( 1 );
	fgSizer213->AddGrowableCol( 2 );
	fgSizer213->SetFlexibleDirection( wxBOTH );
	fgSizer213->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnConnect = new wxButton( this, wxID_ANY, wxT("Connect"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnConnect, 0, wxALL, 5 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnCancel, 0, wxALL, 5 );
	
	btnDisconnect = new wxButton( this, wxID_ANY, wxT("Disconnect"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnDisconnect, 0, wxALL, 5 );
	
	
	fgSizer211->Add( fgSizer213, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	
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
	fgSizer5->Add( btnClear, 0, 0, 5 );
	
	btnFullLog = new wxButton( this, wxID_ANY, wxT("Log"), wxDefaultPosition, wxDefaultSize, 0 );
	btnFullLog->Hide();
	
	fgSizer5->Add( btnFullLog, 0, 0, 5 );
	
	
	fgSizer4->Add( fgSizer5, 1, 0, 5 );
	
	
	this->SetSizer( fgSizer4 );
	this->Layout();
	fgSizer4->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( pnlMiniLog_view::OnUpdateGUI ) );
	btnClear->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnBtnClearClicked ), NULL, this );
	btnFullLog->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnShowFullLog ), NULL, this );
}

pnlMiniLog_view::~pnlMiniLog_view()
{
	// Disconnect Events
	this->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( pnlMiniLog_view::OnUpdateGUI ) );
	btnClear->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnBtnClearClicked ), NULL, this );
	btnFullLog->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMiniLog_view::OnShowFullLog ), NULL, this );
	
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
