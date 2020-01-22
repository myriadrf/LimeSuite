///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "limeRFE_gui.h"

///////////////////////////////////////////////////////////////////////////

limeRFE_view::limeRFE_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	mbar = new wxMenuBar( 0 );
	mFile = new wxMenu();
	wxMenuItem* miQuit;
	miQuit = new wxMenuItem( mFile, wxID_ANY, wxString( wxT("&Quit") ) + wxT('\t') + wxT("Alt+F4"), wxEmptyString, wxITEM_NORMAL );
	mFile->Append( miQuit );
	
	mbar->Append( mFile, wxT("&File") ); 
	
	mView = new wxMenu();
	miPowerMeter = new wxMenuItem( mView, wxID_ANY, wxString( wxT("&Power Meter") ) , wxEmptyString, wxITEM_CHECK );
	mView->Append( miPowerMeter );
	
	mbar->Append( mView, wxT("&View") ); 
	
	this->SetMenuBar( mbar );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	pnlMain = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxFlexGridSizer* fgSizer302;
	fgSizer302 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer302->SetFlexibleDirection( wxBOTH );
	fgSizer302->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxStaticBoxSizer* sbSizer31;
	sbSizer31 = new wxStaticBoxSizer( new wxStaticBox( pnlMain, wxID_ANY, wxT("Communication") ), wxHORIZONTAL );
	
	pnlComm = new wxPanel( sbSizer31->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	rbI2C = new wxRadioButton( pnlComm, wxID_ANY, wxT("SDR"), wxDefaultPosition, wxSize( 100,-1 ), wxRB_GROUP );
	rbI2C->SetValue( true ); 
	bSizer21->Add( rbI2C, 0, wxALL, 5 );
	
	rbUSB = new wxRadioButton( pnlComm, wxID_ANY, wxT("Direct (USB)"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	bSizer21->Add( rbUSB, 0, wxALL, 5 );
	
	
	pnlComm->SetSizer( bSizer21 );
	pnlComm->Layout();
	bSizer21->Fit( pnlComm );
	sbSizer31->Add( pnlComm, 1, wxEXPAND | wxALL, 0 );
	
	
	fgSizer302->Add( sbSizer31, 1, wxEXPAND, 5 );
	
	
	fgSizer2->Add( fgSizer302, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer30;
	fgSizer30 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer30->SetFlexibleDirection( wxBOTH );
	fgSizer30->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( pnlMain, wxID_ANY, wxT("USB") ), wxHORIZONTAL );
	
	cmbbPorts = new wxComboBox( sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 ); 
	sbSizer3->Add( cmbbPorts, 0, wxALL, 5 );
	
	btnRefreshPorts = new wxButton( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer3->Add( btnRefreshPorts, 0, wxALL, 5 );
	
	
	sbSizer3->Add( 10, 0, 1, wxEXPAND, 5 );
	
	btnOpenPort = new wxButton( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Open Port"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer3->Add( btnOpenPort, 0, wxALL, 5 );
	
	btnClosePort = new wxButton( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Close Port"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer3->Add( btnClosePort, 0, wxALL, 5 );
	
	
	fgSizer30->Add( sbSizer3, 1, wxEXPAND, 5 );
	
	
	fgSizer2->Add( fgSizer30, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer21;
	sbSizer21 = new wxStaticBoxSizer( new wxStaticBox( pnlMain, wxID_ANY, wxT("Configuration") ), wxHORIZONTAL );
	
	btnOpen = new wxButton( sbSizer21->GetStaticBox(), wxID_ANY, wxT("Open"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer21->Add( btnOpen, 0, wxALL, 5 );
	
	btnSave = new wxButton( sbSizer21->GetStaticBox(), wxID_ANY, wxT("Save"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer21->Add( btnSave, 0, wxALL, 5 );
	
	
	fgSizer11->Add( sbSizer21, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( pnlMain, wxID_ANY, wxT("Synchronize") ), wxHORIZONTAL );
	
	btnReset = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Reset"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer2->Add( btnReset, 0, wxALL, 5 );
	
	btnBoard2GUI = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Board->GUI"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer2->Add( btnBoard2GUI, 0, wxALL, 5 );
	
	
	fgSizer11->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	
	fgSizer2->Add( fgSizer11, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 0, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	pnlConfiguration = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer18;
	fgSizer18 = new wxFlexGridSizer( 0, 0, 0, 0 );
	fgSizer18->SetFlexibleDirection( wxBOTH );
	fgSizer18->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );
	
	m_panel17 = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( m_panel17, wxID_ANY, wxT("RX Channel") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer26;
	fgSizer26 = new wxFlexGridSizer( 3, 2, 0, 0 );
	fgSizer26->SetFlexibleDirection( wxBOTH );
	fgSizer26->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString cTypeRXChoices[] = { wxT("Wideband"), wxT("HAM"), wxT("Cellular") };
	int cTypeRXNChoices = sizeof( cTypeRXChoices ) / sizeof( wxString );
	cTypeRX = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cTypeRXNChoices, cTypeRXChoices, 0 );
	cTypeRX->SetSelection( 0 );
	fgSizer26->Add( cTypeRX, 1, wxALIGN_CENTER|wxALL|wxEXPAND|wxFIXED_MINSIZE, 5 );
	
	wxString cChannelRXChoices[] = { wxT("1 - 1000 MHz") };
	int cChannelRXNChoices = sizeof( cChannelRXChoices ) / sizeof( wxString );
	cChannelRX = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cChannelRXNChoices, cChannelRXChoices, 0 );
	cChannelRX->SetSelection( 0 );
	fgSizer26->Add( cChannelRX, 1, wxALL, 5 );
	
	m_staticText261 = new wxStaticText( sbSizer9->GetStaticBox(), wxID_ANY, wxT("RX Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText261->Wrap( -1 );
	fgSizer26->Add( m_staticText261, 0, wxALL, 9 );
	
	wxString cPortRXChoices[] = { wxT("asdf"), wxT("asdf") };
	int cPortRXNChoices = sizeof( cPortRXChoices ) / sizeof( wxString );
	cPortRX = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cPortRXNChoices, cPortRXChoices, 0 );
	cPortRX->SetSelection( 0 );
	fgSizer26->Add( cPortRX, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer29;
	fgSizer29 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer29->AddGrowableCol( 1 );
	fgSizer29->SetFlexibleDirection( wxBOTH );
	fgSizer29->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText9 = new wxStaticText( sbSizer9->GetStaticBox(), wxID_ANY, wxT("Attenuation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	fgSizer29->Add( m_staticText9, 1, wxALL, 9 );
	
	wxString cAttenuationChoices[] = { wxT("asdf") };
	int cAttenuationNChoices = sizeof( cAttenuationChoices ) / sizeof( wxString );
	cAttenuation = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cAttenuationNChoices, cAttenuationChoices, 0 );
	cAttenuation->SetSelection( 0 );
	fgSizer29->Add( cAttenuation, 1, wxALIGN_RIGHT|wxALL, 5 );
	
	
	fgSizer26->Add( fgSizer29, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	cbNotch = new wxCheckBox( sbSizer9->GetStaticBox(), wxID_ANY, wxT("AM/FM Notch Filter"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( cbNotch, 0, wxTOP, 5 );
	
	
	fgSizer26->Add( bSizer6, 1, wxALL, 5 );
	
	
	sbSizer9->Add( fgSizer26, 1, wxEXPAND, 5 );
	
	
	m_panel17->SetSizer( sbSizer9 );
	m_panel17->Layout();
	sbSizer9->Fit( m_panel17 );
	bSizer51->Add( m_panel17, 0, wxEXPAND | wxALL, 5 );
	
	m_panel171 = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer91;
	sbSizer91 = new wxStaticBoxSizer( new wxStaticBox( m_panel171, wxID_ANY, wxT("TX Channel") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer261;
	fgSizer261 = new wxFlexGridSizer( 3, 2, 0, 0 );
	fgSizer261->AddGrowableRow( 0 );
	fgSizer261->AddGrowableRow( 1 );
	fgSizer261->AddGrowableRow( 2 );
	fgSizer261->SetFlexibleDirection( wxBOTH );
	fgSizer261->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cbTXasRX = new wxCheckBox( sbSizer91->GetStaticBox(), wxID_ANY, wxT("Same as RX"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer261->Add( cbTXasRX, 0, wxALL, 5 );
	
	
	fgSizer261->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString cTypeTXChoices[] = { wxT("Wideband"), wxT("HAM"), wxT("Cellular") };
	int cTypeTXNChoices = sizeof( cTypeTXChoices ) / sizeof( wxString );
	cTypeTX = new wxChoice( sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cTypeTXNChoices, cTypeTXChoices, 0 );
	cTypeTX->SetSelection( 0 );
	fgSizer261->Add( cTypeTX, 0, wxALIGN_CENTER|wxALL|wxEXPAND|wxFIXED_MINSIZE, 5 );
	
	wxString cChannelTXChoices[] = { wxT("1 - 1000 MHz") };
	int cChannelTXNChoices = sizeof( cChannelTXChoices ) / sizeof( wxString );
	cChannelTX = new wxChoice( sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cChannelTXNChoices, cChannelTXChoices, 0 );
	cChannelTX->SetSelection( 0 );
	fgSizer261->Add( cChannelTX, 0, wxALL, 5 );
	
	m_staticText26 = new wxStaticText( sbSizer91->GetStaticBox(), wxID_ANY, wxT("TX Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	fgSizer261->Add( m_staticText26, 0, wxALL, 9 );
	
	wxString cPortTXChoices[] = { wxT("asdf"), wxT("asdf") };
	int cPortTXNChoices = sizeof( cPortTXChoices ) / sizeof( wxString );
	cPortTX = new wxChoice( sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cPortTXNChoices, cPortTXChoices, 0 );
	cPortTX->SetSelection( 0 );
	fgSizer261->Add( cPortTX, 0, wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer91->Add( fgSizer261, 1, wxEXPAND, 5 );
	
	
	m_panel171->SetSizer( sbSizer91 );
	m_panel171->Layout();
	sbSizer91->Fit( m_panel171 );
	bSizer51->Add( m_panel171, 1, wxEXPAND | wxALL, 5 );
	
	pnlSWR = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( pnlSWR, wxID_ANY, wxT("SWR") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer22;
	fgSizer22 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer22->SetFlexibleDirection( wxBOTH );
	fgSizer22->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cbEnableSWR = new wxCheckBox( sbSizer12->GetStaticBox(), wxID_ANY, wxT("Enable SWR"), wxDefaultPosition, wxDefaultSize, 0 );
	cbEnableSWR->SetMinSize( wxSize( 140,-1 ) );
	
	fgSizer22->Add( cbEnableSWR, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString rbSWRsourceChoices[] = { wxT("External"), wxT("Cellular") };
	int rbSWRsourceNChoices = sizeof( rbSWRsourceChoices ) / sizeof( wxString );
	rbSWRsource = new wxRadioBox( sbSizer12->GetStaticBox(), wxID_ANY, wxT("Source"), wxDefaultPosition, wxDefaultSize, rbSWRsourceNChoices, rbSWRsourceChoices, 2, wxRA_SPECIFY_COLS );
	rbSWRsource->SetSelection( 0 );
	fgSizer22->Add( rbSWRsource, 1, wxLEFT|wxRIGHT, 5 );
	
	
	sbSizer12->Add( fgSizer22, 1, wxEXPAND, 5 );
	
	
	pnlSWR->SetSizer( sbSizer12 );
	pnlSWR->Layout();
	sbSizer12->Fit( pnlSWR );
	bSizer51->Add( pnlSWR, 0, wxEXPAND | wxALL, 5 );
	
	
	fgSizer12->Add( bSizer51, 1, wxEXPAND, 5 );
	
	pnlTXRX = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( pnlTXRX, wxID_ANY, wxT("Mode:") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer262;
	fgSizer262 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer262->AddGrowableRow( 0 );
	fgSizer262->SetFlexibleDirection( wxBOTH );
	fgSizer262->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	tbtnTXRX = new wxToggleButton( sbSizer11->GetStaticBox(), wxID_ANY, wxT("TX/RX"), wxDefaultPosition, wxSize( 140,80 ), 0 );
	tbtnTXRX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	fgSizer262->Add( tbtnTXRX, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer11->Add( fgSizer262, 1, wxEXPAND, 5 );
	
	
	pnlTXRX->SetSizer( sbSizer11 );
	pnlTXRX->Layout();
	sbSizer11->Fit( pnlTXRX );
	fgSizer12->Add( pnlTXRX, 0, wxEXPAND | wxALL, 5 );
	
	pnlTXRXEN = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer111;
	sbSizer111 = new wxStaticBoxSizer( new wxStaticBox( pnlTXRXEN, wxID_ANY, wxT("Mode:") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2621;
	fgSizer2621 = new wxFlexGridSizer( 3, 0, 0, 0 );
	fgSizer2621->AddGrowableRow( 0 );
	fgSizer2621->AddGrowableRow( 2 );
	fgSizer2621->SetFlexibleDirection( wxBOTH );
	fgSizer2621->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer2621->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer291;
	fgSizer291 = new wxFlexGridSizer( 2, 0, 0, 0 );
	fgSizer291->SetFlexibleDirection( wxBOTH );
	fgSizer291->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	tbtnRX = new wxToggleButton( sbSizer111->GetStaticBox(), wxID_ANY, wxT("RX"), wxDefaultPosition, wxSize( 140,35 ), 0 );
	tbtnRX->SetValue( true ); 
	tbtnRX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	fgSizer291->Add( tbtnRX, 0, wxALL, 5 );
	
	tbtnTX = new wxToggleButton( sbSizer111->GetStaticBox(), wxID_ANY, wxT("TX"), wxDefaultPosition, wxSize( 140,35 ), 0 );
	tbtnTX->SetValue( true ); 
	tbtnTX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	fgSizer291->Add( tbtnTX, 0, wxALL, 5 );
	
	
	fgSizer2621->Add( fgSizer291, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer2621->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sbSizer111->Add( fgSizer2621, 1, wxEXPAND, 5 );
	
	
	pnlTXRXEN->SetSizer( sbSizer111 );
	pnlTXRXEN->Layout();
	sbSizer111->Fit( pnlTXRXEN );
	fgSizer12->Add( pnlTXRXEN, 0, wxEXPAND | wxALL, 5 );
	
	btnConfigure = new wxButton( pnlConfiguration, wxID_ANY, wxT("Configure"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	btnConfigure->SetMinSize( wxSize( -1,60 ) );
	
	fgSizer12->Add( btnConfigure, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );
	
	pnlTXRXMode = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbsMode;
	sbsMode = new wxStaticBoxSizer( new wxStaticBox( pnlTXRXMode, wxID_ANY, wxT("Mode:") ), wxVERTICAL );
	
	txtTXRX = new wxStaticText( sbsMode->GetStaticBox(), wxID_ANY, wxT("RX"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTXRX->Wrap( -1 );
	txtTXRX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	sbsMode->Add( txtTXRX, 0, wxALL, 5 );
	
	
	pnlTXRXMode->SetSizer( sbsMode );
	pnlTXRXMode->Layout();
	sbsMode->Fit( pnlTXRXMode );
	fgSizer12->Add( pnlTXRXMode, 1, wxEXPAND | wxALL, 5 );
	
	pnlTXRXModeEN = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbsMode1;
	sbsMode1 = new wxStaticBoxSizer( new wxStaticBox( pnlTXRXModeEN, wxID_ANY, wxT("Mode:") ), wxVERTICAL );
	
	txtTXRXEN = new wxStaticText( sbsMode1->GetStaticBox(), wxID_ANY, wxT("RX"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTXRXEN->Wrap( -1 );
	txtTXRXEN->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	sbsMode1->Add( txtTXRXEN, 0, wxALL, 5 );
	
	
	pnlTXRXModeEN->SetSizer( sbsMode1 );
	pnlTXRXModeEN->Layout();
	sbsMode1->Fit( pnlTXRXModeEN );
	fgSizer12->Add( pnlTXRXModeEN, 1, wxEXPAND | wxALL, 5 );
	
	
	fgSizer18->Add( fgSizer12, 0, wxEXPAND, 5 );
	
	
	pnlConfiguration->SetSizer( fgSizer18 );
	pnlConfiguration->Layout();
	fgSizer18->Fit( pnlConfiguration );
	fgSizer1->Add( pnlConfiguration, 1, wxALL|wxEXPAND, 5 );
	
	pnlPowerMeter = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( pnlPowerMeter, wxID_ANY, wxT("Power Meter") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer31->SetFlexibleDirection( wxBOTH );
	fgSizer31->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_panel16 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer303;
	fgSizer303 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer303->AddGrowableCol( 0 );
	fgSizer303->SetFlexibleDirection( wxBOTH );
	fgSizer303->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	btnReadADC = new wxButton( m_panel16, wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer303->Add( btnReadADC, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	btnCalibrate = new wxButton( m_panel16, wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer303->Add( btnCalibrate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	m_panel16->SetSizer( fgSizer303 );
	m_panel16->Layout();
	fgSizer303->Fit( m_panel16 );
	fgSizer31->Add( m_panel16, 1, wxEXPAND | wxALL, 5 );
	
	pnlADC1 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer33;
	fgSizer33 = new wxFlexGridSizer( 3, 3, 0, 0 );
	fgSizer33->SetFlexibleDirection( wxBOTH );
	fgSizer33->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText13 = new wxStaticText( pnlADC1, wxID_ANY, wxT("ADC:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	fgSizer33->Add( m_staticText13, 0, wxALL, 2 );
	
	txtADC1 = new wxStaticText( pnlADC1, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtADC1->Wrap( -1 );
	txtADC1->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer33->Add( txtADC1, 0, wxALL, 2 );
	
	
	fgSizer33->Add( 0, 0, 1, wxEXPAND, 2 );
	
	m_staticText10 = new wxStaticText( pnlADC1, wxID_ANY, wxT("Power:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer33->Add( m_staticText10, 0, wxALL, 2 );
	
	txtPFWD_dBm = new wxStaticText( pnlADC1, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtPFWD_dBm->Wrap( -1 );
	txtPFWD_dBm->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer33->Add( txtPFWD_dBm, 0, wxALL, 2 );
	
	m_staticText12 = new wxStaticText( pnlADC1, wxID_ANY, wxT("dBm"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer33->Add( m_staticText12, 0, wxALL, 2 );
	
	
	fgSizer33->Add( 0, 0, 1, wxEXPAND, 2 );
	
	txtPFWD_W = new wxStaticText( pnlADC1, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtPFWD_W->Wrap( -1 );
	txtPFWD_W->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer33->Add( txtPFWD_W, 0, wxALL, 2 );
	
	m_staticText16 = new wxStaticText( pnlADC1, wxID_ANY, wxT("W"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer33->Add( m_staticText16, 0, wxALL, 2 );
	
	
	bSizer3->Add( fgSizer33, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer332;
	fgSizer332 = new wxFlexGridSizer( 2, 3, 0, 0 );
	fgSizer332->SetFlexibleDirection( wxBOTH );
	fgSizer332->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1022 = new wxStaticText( pnlADC1, wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1022->Wrap( -1 );
	fgSizer332->Add( m_staticText1022, 0, wxALL, 2 );
	
	
	fgSizer332->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer332->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText102 = new wxStaticText( pnlADC1, wxID_ANY, wxT("Power:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText102->Wrap( -1 );
	fgSizer332->Add( m_staticText102, 0, wxALL, 2 );
	
	tcPowerCalibrate = new wxTextCtrl( pnlADC1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tcPowerCalibrate->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer332->Add( tcPowerCalibrate, 0, wxALL, 2 );
	
	m_staticText122 = new wxStaticText( pnlADC1, wxID_ANY, wxT("dBm"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText122->Wrap( -1 );
	fgSizer332->Add( m_staticText122, 0, wxALL, 2 );
	
	
	bSizer3->Add( fgSizer332, 1, wxEXPAND, 5 );
	
	
	pnlADC1->SetSizer( bSizer3 );
	pnlADC1->Layout();
	bSizer3->Fit( pnlADC1 );
	fgSizer31->Add( pnlADC1, 1, wxEXPAND | wxALL, 5 );
	
	pnlADC2 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer331;
	fgSizer331 = new wxFlexGridSizer( 3, 3, 0, 0 );
	fgSizer331->SetFlexibleDirection( wxBOTH );
	fgSizer331->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText131 = new wxStaticText( pnlADC2, wxID_ANY, wxT("ADC:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText131->Wrap( -1 );
	fgSizer331->Add( m_staticText131, 0, wxALL, 2 );
	
	txtADC2 = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtADC2->Wrap( -1 );
	txtADC2->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer331->Add( txtADC2, 0, wxALL, 2 );
	
	
	fgSizer331->Add( 0, 0, 1, wxEXPAND, 2 );
	
	m_staticText101 = new wxStaticText( pnlADC2, wxID_ANY, wxT("RL:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	fgSizer331->Add( m_staticText101, 0, wxALL, 2 );
	
	txtRL_dB = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtRL_dB->Wrap( -1 );
	txtRL_dB->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer331->Add( txtRL_dB, 0, wxALL, 2 );
	
	m_staticText121 = new wxStaticText( pnlADC2, wxID_ANY, wxT("dB"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText121->Wrap( -1 );
	fgSizer331->Add( m_staticText121, 0, wxALL, 2 );
	
	m_staticText31 = new wxStaticText( pnlADC2, wxID_ANY, wxT("SWR:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText31->Wrap( -1 );
	fgSizer331->Add( m_staticText31, 0, wxALL, 2 );
	
	txtSWR = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtSWR->Wrap( -1 );
	txtSWR->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer331->Add( txtSWR, 0, wxALL, 2 );
	
	
	bSizer5->Add( fgSizer331, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3321;
	fgSizer3321 = new wxFlexGridSizer( 2, 3, 0, 0 );
	fgSizer3321->SetFlexibleDirection( wxBOTH );
	fgSizer3321->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText10221 = new wxStaticText( pnlADC2, wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10221->Wrap( -1 );
	fgSizer3321->Add( m_staticText10221, 0, wxALL, 2 );
	
	
	fgSizer3321->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer3321->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText1021 = new wxStaticText( pnlADC2, wxID_ANY, wxT("RL:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1021->Wrap( -1 );
	fgSizer3321->Add( m_staticText1021, 0, wxALL, 2 );
	
	tcRLCalibrate = new wxTextCtrl( pnlADC2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tcRLCalibrate->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer3321->Add( tcRLCalibrate, 0, wxALL, 2 );
	
	m_staticText1221 = new wxStaticText( pnlADC2, wxID_ANY, wxT("dB"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1221->Wrap( -1 );
	fgSizer3321->Add( m_staticText1221, 0, wxALL, 2 );
	
	
	bSizer5->Add( fgSizer3321, 1, wxEXPAND, 5 );
	
	
	pnlADC2->SetSizer( bSizer5 );
	pnlADC2->Layout();
	bSizer5->Fit( pnlADC2 );
	fgSizer31->Add( pnlADC2, 1, wxEXPAND | wxALL, 5 );
	
	
	sbSizer8->Add( fgSizer31, 1, wxEXPAND, 5 );
	
	
	pnlPowerMeter->SetSizer( sbSizer8 );
	pnlPowerMeter->Layout();
	sbSizer8->Fit( pnlPowerMeter );
	fgSizer1->Add( pnlPowerMeter, 1, wxALL|wxEXPAND, 5 );
	
	
	fgSizer2->Add( fgSizer1, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer301;
	fgSizer301 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer301->SetFlexibleDirection( wxBOTH );
	fgSizer301->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer2->Add( fgSizer301, 1, wxEXPAND, 5 );
	
	txtMessageField = new wxTextCtrl( pnlMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtMessageField->SetMinSize( wxSize( -1,100 ) );
	
	fgSizer2->Add( txtMessageField, 0, wxALL|wxEXPAND, 5 );
	
	
	pnlMain->SetSizer( fgSizer2 );
	pnlMain->Layout();
	fgSizer2->Fit( pnlMain );
	bSizer2->Add( pnlMain, 1, wxEXPAND | wxALL, 5 );
	
	
	this->SetSizer( bSizer2 );
	this->Layout();
	bSizer2->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( miQuit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( limeRFE_view::OnQuit ) );
	this->Connect( miPowerMeter->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( limeRFE_view::OnmiPowerMeter ) );
	rbI2C->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	rbUSB->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	btnRefreshPorts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnRefreshPorts ), NULL, this );
	btnOpenPort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpenPort ), NULL, this );
	btnClosePort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClosePort ), NULL, this );
	btnOpen->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpen ), NULL, this );
	btnSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnSave ), NULL, this );
	btnReset->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReset ), NULL, this );
	btnBoard2GUI->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnBoard2GUI ), NULL, this );
	pnlConfiguration->Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( limeRFE_view::OnEraseBackground_pnlConfiguration ), NULL, this );
	cTypeRX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeRX ), NULL, this );
	cChannelRX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelRX ), NULL, this );
	cPortRX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortRX ), NULL, this );
	cAttenuation->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncAttenuation ), NULL, this );
	cbNotch->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbNotch ), NULL, this );
	cbTXasRX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbTXasRX ), NULL, this );
	cTypeTX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeTX ), NULL, this );
	cChannelTX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelTX ), NULL, this );
	cPortTX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortTX ), NULL, this );
	cbEnableSWR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbEnableSWR ), NULL, this );
	rbSWRsource->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRsource ), NULL, this );
	tbtnTXRX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRX ), NULL, this );
	tbtnRX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnRXEN ), NULL, this );
	tbtnTX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXEN ), NULL, this );
	btnConfigure->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnConfigure ), NULL, this );
	btnReadADC->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReadADC ), NULL, this );
	btnCalibrate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnCalibrate ), NULL, this );
}

limeRFE_view::~limeRFE_view()
{
	// Disconnect Events
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( limeRFE_view::OnQuit ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( limeRFE_view::OnmiPowerMeter ) );
	rbI2C->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	rbUSB->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	btnRefreshPorts->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnRefreshPorts ), NULL, this );
	btnOpenPort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpenPort ), NULL, this );
	btnClosePort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClosePort ), NULL, this );
	btnOpen->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpen ), NULL, this );
	btnSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnSave ), NULL, this );
	btnReset->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReset ), NULL, this );
	btnBoard2GUI->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnBoard2GUI ), NULL, this );
	pnlConfiguration->Disconnect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( limeRFE_view::OnEraseBackground_pnlConfiguration ), NULL, this );
	cTypeRX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeRX ), NULL, this );
	cChannelRX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelRX ), NULL, this );
	cPortRX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortRX ), NULL, this );
	cAttenuation->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncAttenuation ), NULL, this );
	cbNotch->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbNotch ), NULL, this );
	cbTXasRX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbTXasRX ), NULL, this );
	cTypeTX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeTX ), NULL, this );
	cChannelTX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelTX ), NULL, this );
	cPortTX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortTX ), NULL, this );
	cbEnableSWR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbEnableSWR ), NULL, this );
	rbSWRsource->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRsource ), NULL, this );
	tbtnTXRX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRX ), NULL, this );
	tbtnRX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnRXEN ), NULL, this );
	tbtnTX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXEN ), NULL, this );
	btnConfigure->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnConfigure ), NULL, this );
	btnReadADC->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReadADC ), NULL, this );
	btnCalibrate->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnCalibrate ), NULL, this );
	
}
