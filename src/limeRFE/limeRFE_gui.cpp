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
	this->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
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
	
	rbI2C = new wxRadioButton( pnlComm, wxID_ANY, wxT("I2C"), wxDefaultPosition, wxSize( 100,-1 ), wxRB_GROUP );
	rbI2C->SetValue( true ); 
	bSizer21->Add( rbI2C, 0, wxALL, 5 );
	
	rbUSB = new wxRadioButton( pnlComm, wxID_ANY, wxT("USB"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	bSizer21->Add( rbUSB, 0, wxALL, 5 );
	
	
	pnlComm->SetSizer( bSizer21 );
	pnlComm->Layout();
	bSizer21->Fit( pnlComm );
	sbSizer31->Add( pnlComm, 1, wxEXPAND | wxALL, 0 );
	
	
	fgSizer302->Add( sbSizer31, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( pnlMain, wxID_ANY, wxT("I2C") ), wxHORIZONTAL );
	
	m_staticText22 = new wxStaticText( sbSizer41->GetStaticBox(), wxID_ANY, wxT("Address:"), wxPoint( -1,-1 ), wxSize( 100,-1 ), 0 );
	m_staticText22->Wrap( -1 );
	sbSizer41->Add( m_staticText22, 0, wxALL, 5 );
	
	tcI2Caddress = new wxTextCtrl( sbSizer41->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer41->Add( tcI2Caddress, 0, wxALL, 5 );
	
	
	fgSizer302->Add( sbSizer41, 1, wxEXPAND, 5 );
	
	
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
	
	pnlI2Cmaster = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( pnlI2Cmaster, wxID_ANY, wxT("I2C") ), wxVERTICAL );
	
	cbI2CMaster = new wxCheckBox( sbSizer5->GetStaticBox(), wxID_ANY, wxT("I2C Master"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer5->Add( cbI2CMaster, 0, wxALL, 5 );
	
	
	pnlI2Cmaster->SetSizer( sbSizer5 );
	pnlI2Cmaster->Layout();
	sbSizer5->Fit( pnlI2Cmaster );
	fgSizer30->Add( pnlI2Cmaster, 1, wxEXPAND | wxALL, 5 );
	
	
	fgSizer2->Add( fgSizer30, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer21;
	sbSizer21 = new wxStaticBoxSizer( new wxStaticBox( pnlMain, wxID_ANY, wxT("Configuration") ), wxHORIZONTAL );
	
	btnOpen1 = new wxButton( sbSizer21->GetStaticBox(), wxID_ANY, wxT("Open"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer21->Add( btnOpen1, 0, wxALL, 5 );
	
	btnSave = new wxButton( sbSizer21->GetStaticBox(), wxID_ANY, wxT("Save"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer21->Add( btnSave, 0, wxALL, 5 );
	
	
	fgSizer11->Add( sbSizer21, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( pnlMain, wxID_ANY, wxT("Synchronize") ), wxHORIZONTAL );
	
	btnReset = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Reset"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	sbSizer2->Add( btnReset, 0, wxALL, 5 );
	
	btnBoard2GUI = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Board->GUI"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	btnBoard2GUI->Hide();
	
	sbSizer2->Add( btnBoard2GUI, 0, wxALL, 5 );
	
	
	fgSizer11->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	
	fgSizer2->Add( fgSizer11, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 0, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	pnlConfiguration = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer18;
	fgSizer18 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer18->SetFlexibleDirection( wxBOTH );
	fgSizer18->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 5, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxVERTICAL );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_panel2 = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer13;
	fgSizer13 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer13->SetFlexibleDirection( wxBOTH );
	fgSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer13->SetMinSize( wxSize( 150,-1 ) ); 
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 1, 0, 0 );
	
	gSizer1->SetMinSize( wxSize( -1,50 ) ); 
	m_staticText1 = new wxStaticText( m_panel2, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	gSizer1->Add( m_staticText1, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	
	fgSizer13->Add( gSizer1, 1, wxEXPAND, 5 );
	
	wxString cTypeChoices[] = { wxT("Wideband"), wxT("HAM"), wxT("Cellular") };
	int cTypeNChoices = sizeof( cTypeChoices ) / sizeof( wxString );
	cType = new wxChoice( m_panel2, wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cTypeNChoices, cTypeChoices, 0 );
	cType->SetSelection( 0 );
	fgSizer13->Add( cType, 0, wxALIGN_CENTER|wxALL|wxEXPAND|wxFIXED_MINSIZE, 5 );
	
	
	fgSizer13->Add( 0, 50, 1, wxBOTTOM, 5 );
	
	
	m_panel2->SetSizer( fgSizer13 );
	m_panel2->Layout();
	fgSizer13->Fit( m_panel2 );
	fgSizer12->Add( m_panel2, 1, wxEXPAND | wxALL, 5 );
	
	m_panel3 = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer14->SetFlexibleDirection( wxBOTH );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer14->SetMinSize( wxSize( 150,-1 ) ); 
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer( 0, 1, 0, 0 );
	
	gSizer2->SetMinSize( wxSize( -1,50 ) ); 
	m_staticText2 = new wxStaticText( m_panel3, wxID_ANY, wxT("Channel/Band:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	gSizer2->Add( m_staticText2, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	
	fgSizer14->Add( gSizer2, 1, wxEXPAND, 5 );
	
	wxString cChannelChoices[] = { wxT("1 - 1000 MHz") };
	int cChannelNChoices = sizeof( cChannelChoices ) / sizeof( wxString );
	cChannel = new wxChoice( m_panel3, wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cChannelNChoices, cChannelChoices, 0 );
	cChannel->SetSelection( 0 );
	fgSizer14->Add( cChannel, 0, wxALL, 5 );
	
	wxGridSizer* gSizer3;
	gSizer3 = new wxGridSizer( 2, 1, 0, 0 );
	
	gSizer3->SetMinSize( wxSize( 150,50 ) ); 
	cbNotch = new wxCheckBox( m_panel3, wxID_ANY, wxT("Notch Filter"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( cbNotch, 0, wxALL, 5 );
	
	
	fgSizer14->Add( gSizer3, 1, wxEXPAND, 5 );
	
	
	m_panel3->SetSizer( fgSizer14 );
	m_panel3->Layout();
	fgSizer14->Fit( m_panel3 );
	fgSizer12->Add( m_panel3, 1, wxEXPAND | wxALL, 5 );
	
	pnlTX2TXRX = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer131;
	fgSizer131 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer131->SetFlexibleDirection( wxBOTH );
	fgSizer131->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer131->SetMinSize( wxSize( 150,-1 ) ); 
	wxGridSizer* gSizer11;
	gSizer11 = new wxGridSizer( 0, 1, 0, 0 );
	
	gSizer11->SetMinSize( wxSize( -1,50 ) ); 
	m_staticText11 = new wxStaticText( pnlTX2TXRX, wxID_ANY, wxT("TX to:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	gSizer11->Add( m_staticText11, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	
	fgSizer131->Add( gSizer11, 1, wxEXPAND, 5 );
	
	wxString cTX2TXRXChoices[] = { wxT("asdf"), wxT("asdf") };
	int cTX2TXRXNChoices = sizeof( cTX2TXRXChoices ) / sizeof( wxString );
	cTX2TXRX = new wxChoice( pnlTX2TXRX, wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cTX2TXRXNChoices, cTX2TXRXChoices, 0 );
	cTX2TXRX->SetSelection( 0 );
	fgSizer131->Add( cTX2TXRX, 0, wxALIGN_CENTER|wxALL|wxEXPAND|wxFIXED_MINSIZE, 5 );
	
	
	fgSizer131->Add( 0, 50, 1, wxBOTTOM, 5 );
	
	
	pnlTX2TXRX->SetSizer( fgSizer131 );
	pnlTX2TXRX->Layout();
	fgSizer131->Fit( pnlTX2TXRX );
	fgSizer12->Add( pnlTX2TXRX, 1, wxEXPAND | wxALL, 5 );
	
	pnlTXRX = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer1311;
	fgSizer1311 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer1311->SetFlexibleDirection( wxBOTH );
	fgSizer1311->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1311->SetMinSize( wxSize( 150,-1 ) ); 
	wxGridSizer* gSizer111;
	gSizer111 = new wxGridSizer( 0, 1, 0, 0 );
	
	gSizer111->SetMinSize( wxSize( -1,20 ) ); 
	m_staticText111 = new wxStaticText( pnlTXRX, wxID_ANY, wxT("TX/RX:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText111->Wrap( -1 );
	gSizer111->Add( m_staticText111, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	
	fgSizer1311->Add( gSizer111, 1, wxEXPAND, 5 );
	
	tbtnTXRX = new wxToggleButton( pnlTXRX, wxID_ANY, wxT("TX/RX"), wxDefaultPosition, wxSize( 140,80 ), 0 );
	tbtnTXRX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	fgSizer1311->Add( tbtnTXRX, 0, wxALL, 5 );
	
	
	pnlTXRX->SetSizer( fgSizer1311 );
	pnlTXRX->Layout();
	fgSizer1311->Fit( pnlTXRX );
	fgSizer12->Add( pnlTXRX, 1, wxEXPAND | wxALL, 5 );
	
	pnlTXRXEN = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer13111;
	fgSizer13111 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer13111->SetFlexibleDirection( wxBOTH );
	fgSizer13111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer13111->SetMinSize( wxSize( 150,-1 ) ); 
	wxGridSizer* gSizer1111;
	gSizer1111 = new wxGridSizer( 0, 1, 0, 0 );
	
	gSizer1111->SetMinSize( wxSize( -1,20 ) ); 
	m_staticText1111 = new wxStaticText( pnlTXRXEN, wxID_ANY, wxT("TX, RX:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1111->Wrap( -1 );
	gSizer1111->Add( m_staticText1111, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	
	fgSizer13111->Add( gSizer1111, 1, wxEXPAND, 5 );
	
	tbtnRX = new wxToggleButton( pnlTXRXEN, wxID_ANY, wxT("RX"), wxDefaultPosition, wxSize( 140,35 ), 0 );
	tbtnRX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	fgSizer13111->Add( tbtnRX, 0, wxALL, 5 );
	
	tbtnTX = new wxToggleButton( pnlTXRXEN, wxID_ANY, wxT("TX"), wxDefaultPosition, wxSize( 140,35 ), 0 );
	tbtnTX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	fgSizer13111->Add( tbtnTX, 0, wxALL, 5 );
	
	
	pnlTXRXEN->SetSizer( fgSizer13111 );
	pnlTXRXEN->Layout();
	fgSizer13111->Fit( pnlTXRXEN );
	fgSizer12->Add( pnlTXRXEN, 1, wxEXPAND | wxALL, 5 );
	
	
	fgSizer18->Add( fgSizer12, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer29;
	fgSizer29 = new wxFlexGridSizer( 0, 0, 0, 0 );
	fgSizer29->SetFlexibleDirection( wxBOTH );
	fgSizer29->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText9 = new wxStaticText( pnlConfiguration, wxID_ANY, wxT("Attenuation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	fgSizer29->Add( m_staticText9, 0, wxALL, 9 );
	
	wxString cAttenuationChoices[] = { wxT("asdf") };
	int cAttenuationNChoices = sizeof( cAttenuationChoices ) / sizeof( wxString );
	cAttenuation = new wxChoice( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, cAttenuationNChoices, cAttenuationChoices, 0 );
	cAttenuation->SetSelection( 0 );
	fgSizer29->Add( cAttenuation, 0, wxALL, 5 );
	
	
	fgSizer18->Add( fgSizer29, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer19;
	fgSizer19 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer19->SetFlexibleDirection( wxBOTH );
	fgSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_panel4 = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer21;
	fgSizer21 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer21->SetFlexibleDirection( wxBOTH );
	fgSizer21->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer24;
	fgSizer24 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer24->SetFlexibleDirection( wxBOTH );
	fgSizer24->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer24->SetMinSize( wxSize( 495,-1 ) ); 
	
	fgSizer24->Add( 0, 18, 1, wxEXPAND, 5 );
	
	btnConfigure = new wxButton( m_panel4, wxID_ANY, wxT("Configure"), wxDefaultPosition, wxSize( 487,40 ), 0 );
	fgSizer24->Add( btnConfigure, 0, wxALIGN_CENTER, 5 );
	
	
	fgSizer21->Add( fgSizer24, 1, wxALIGN_CENTER|wxEXPAND, 5 );
	
	
	m_panel4->SetSizer( fgSizer21 );
	m_panel4->Layout();
	fgSizer21->Fit( m_panel4 );
	fgSizer19->Add( m_panel4, 1, wxEXPAND | wxALL, 5 );
	
	pnlTXRXMode = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbsMode;
	sbsMode = new wxStaticBoxSizer( new wxStaticBox( pnlTXRXMode, wxID_ANY, wxT("Mode:") ), wxVERTICAL );
	
	sbsMode->SetMinSize( wxSize( 150,-1 ) ); 
	txtTXRX = new wxStaticText( sbsMode->GetStaticBox(), wxID_ANY, wxT("RX"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTXRX->Wrap( -1 );
	txtTXRX->SetFont( wxFont( 16, 70, 90, 92, false, wxEmptyString ) );
	
	sbsMode->Add( txtTXRX, 0, wxALL, 5 );
	
	
	pnlTXRXMode->SetSizer( sbsMode );
	pnlTXRXMode->Layout();
	sbsMode->Fit( pnlTXRXMode );
	fgSizer19->Add( pnlTXRXMode, 1, wxEXPAND | wxALL, 5 );
	
	
	fgSizer18->Add( fgSizer19, 1, wxEXPAND, 5 );
	
	
	pnlConfiguration->SetSizer( fgSizer18 );
	pnlConfiguration->Layout();
	fgSizer18->Fit( pnlConfiguration );
	fgSizer1->Add( pnlConfiguration, 1, wxALL|wxEXPAND, 5 );
	
	
	fgSizer2->Add( fgSizer1, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer301;
	fgSizer301 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer301->SetFlexibleDirection( wxBOTH );
	fgSizer301->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	pnlPowerMeter = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( pnlPowerMeter, wxID_ANY, wxT("Power Meter") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizer31->SetFlexibleDirection( wxBOTH );
	fgSizer31->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_panel16 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer303;
	fgSizer303 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer303->SetFlexibleDirection( wxBOTH );
	fgSizer303->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnReadADC = new wxButton( m_panel16, wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer303->Add( btnReadADC, 0, wxALL, 5 );
	
	btnCalibrate = new wxButton( m_panel16, wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer303->Add( btnCalibrate, 0, wxALL, 5 );
	
	
	m_panel16->SetSizer( fgSizer303 );
	m_panel16->Layout();
	fgSizer303->Fit( m_panel16 );
	fgSizer31->Add( m_panel16, 1, wxEXPAND | wxALL, 5 );
	
	m_panel13 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer32;
	fgSizer32 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer32->SetFlexibleDirection( wxBOTH );
	fgSizer32->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rbSWRsourceChoices[] = { wxT("Cellular"), wxT("External") };
	int rbSWRsourceNChoices = sizeof( rbSWRsourceChoices ) / sizeof( wxString );
	rbSWRsource = new wxRadioBox( m_panel13, wxID_ANY, wxT("Power Meter"), wxDefaultPosition, wxDefaultSize, rbSWRsourceNChoices, rbSWRsourceChoices, 1, wxRA_SPECIFY_COLS );
	rbSWRsource->SetSelection( 0 );
	fgSizer32->Add( rbSWRsource, 0, wxALL, 5 );
	
	
	m_panel13->SetSizer( fgSizer32 );
	m_panel13->Layout();
	fgSizer32->Fit( m_panel13 );
	fgSizer31->Add( m_panel13, 1, wxEXPAND | wxALL, 5 );
	
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
	
	m_staticText101 = new wxStaticText( pnlADC2, wxID_ANY, wxT("Gamma:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	fgSizer331->Add( m_staticText101, 0, wxALL, 2 );
	
	txtGamma_dB = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtGamma_dB->Wrap( -1 );
	txtGamma_dB->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer331->Add( txtGamma_dB, 0, wxALL, 2 );
	
	m_staticText121 = new wxStaticText( pnlADC2, wxID_ANY, wxT("dB"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText121->Wrap( -1 );
	fgSizer331->Add( m_staticText121, 0, wxALL, 2 );
	
	m_staticText31 = new wxStaticText( pnlADC2, wxID_ANY, wxT("SWR"), wxDefaultPosition, wxDefaultSize, 0 );
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
	
	m_staticText1021 = new wxStaticText( pnlADC2, wxID_ANY, wxT("Gamma:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1021->Wrap( -1 );
	fgSizer3321->Add( m_staticText1021, 0, wxALL, 2 );
	
	tcGammaCalibrate = new wxTextCtrl( pnlADC2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tcGammaCalibrate->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer3321->Add( tcGammaCalibrate, 0, wxALL, 2 );
	
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
	fgSizer301->Add( pnlPowerMeter, 1, wxALL|wxEXPAND, 5 );
	
	
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
	rbI2C->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	rbUSB->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	btnRefreshPorts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnRefreshPorts ), NULL, this );
	btnOpenPort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpenPort ), NULL, this );
	btnClosePort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClosePort ), NULL, this );
	cbI2CMaster->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbI2CMaster ), NULL, this );
	btnOpen1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpen ), NULL, this );
	btnSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnSave ), NULL, this );
	btnReset->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReset ), NULL, this );
	btnBoard2GUI->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnBoard2GUI ), NULL, this );
	pnlConfiguration->Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( limeRFE_view::OnEraseBackground_pnlConfiguration ), NULL, this );
	cType->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncType ), NULL, this );
	cChannel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannel ), NULL, this );
	cbNotch->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbNotch ), NULL, this );
	cTX2TXRX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTX2TXRX ), NULL, this );
	tbtnTXRX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRX ), NULL, this );
	tbtnRX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRXEN ), NULL, this );
	tbtnTX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRXEN ), NULL, this );
	cAttenuation->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncAttenuation ), NULL, this );
	btnConfigure->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnConfigure ), NULL, this );
	btnReadADC->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReadADC ), NULL, this );
	btnCalibrate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnCalibrate ), NULL, this );
	rbSWRsource->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRsource ), NULL, this );
}

limeRFE_view::~limeRFE_view()
{
	// Disconnect Events
	rbI2C->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	rbUSB->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	btnRefreshPorts->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnRefreshPorts ), NULL, this );
	btnOpenPort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpenPort ), NULL, this );
	btnClosePort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClosePort ), NULL, this );
	cbI2CMaster->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbI2CMaster ), NULL, this );
	btnOpen1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpen ), NULL, this );
	btnSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnSave ), NULL, this );
	btnReset->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReset ), NULL, this );
	btnBoard2GUI->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnBoard2GUI ), NULL, this );
	pnlConfiguration->Disconnect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( limeRFE_view::OnEraseBackground_pnlConfiguration ), NULL, this );
	cType->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncType ), NULL, this );
	cChannel->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannel ), NULL, this );
	cbNotch->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbNotch ), NULL, this );
	cTX2TXRX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTX2TXRX ), NULL, this );
	tbtnTXRX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRX ), NULL, this );
	tbtnRX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRXEN ), NULL, this );
	tbtnTX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRXEN ), NULL, this );
	cAttenuation->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncAttenuation ), NULL, this );
	btnConfigure->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnConfigure ), NULL, this );
	btnReadADC->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReadADC ), NULL, this );
	btnCalibrate->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnCalibrate ), NULL, this );
	rbSWRsource->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRsource ), NULL, this );
	
}
