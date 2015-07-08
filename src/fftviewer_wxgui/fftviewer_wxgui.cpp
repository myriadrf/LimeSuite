///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "mathplot.h"

#include "fftviewer_wxgui.h"

///////////////////////////////////////////////////////////////////////////

frFFTviewer::frFFTviewer( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer7->AddGrowableCol( 0 );
	fgSizer7->AddGrowableRow( 0 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	mPlotsSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	mPlotsSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( frFFTviewer::mPlotsSplitterOnIdle ), NULL, this );
	
	mTimeConstellationPanel = new wxPanel( mPlotsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer9->AddGrowableCol( 0 );
	fgSizer9->AddGrowableRow( 0 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_splitter3 = new wxSplitterWindow( mTimeConstellationPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_splitter3->Connect( wxEVT_IDLE, wxIdleEventHandler( frFFTviewer::m_splitter3OnIdle ), NULL, this );
	
	mTimeDomainPanel = new mpWindow( m_splitter3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	mConstelationPanel = new mpWindow( m_splitter3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_splitter3->SplitVertically( mTimeDomainPanel, mConstelationPanel, 0 );
	fgSizer9->Add( m_splitter3, 1, wxEXPAND, 5 );
	
	
	mTimeConstellationPanel->SetSizer( fgSizer9 );
	mTimeConstellationPanel->Layout();
	fgSizer9->Fit( mTimeConstellationPanel );
	mFFTpanel = new mpWindow( mPlotsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	mPlotsSplitter->SplitHorizontally( mTimeConstellationPanel, mFFTpanel, 0 );
	fgSizer7->Add( mPlotsSplitter, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("FFT parameters") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText6 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Nyquist freq (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer11->Add( m_staticText6, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtNyquistFreqMHz = new wxTextCtrl( sbSizer1->GetStaticBox(), wxID_ANY, wxT("15.36"), wxDefaultPosition, wxDefaultSize, 0 );
	txtNyquistFreqMHz->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer11->Add( txtNyquistFreqMHz, 0, wxEXPAND, 5 );
	
	m_staticText7 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Samples count"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	fgSizer11->Add( m_staticText7, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	spinFFTsize = new wxSpinCtrl( sbSizer1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 64, 65536, 16384 );
	spinFFTsize->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer11->Add( spinFFTsize, 0, wxEXPAND, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sbSizer1->Add( fgSizer11, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( sbSizer1, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Data reading") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer91;
	fgSizer91 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer91->SetFlexibleDirection( wxBOTH );
	fgSizer91->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString cmbStreamTypeChoices[] = { wxT("IQ samples"), wxT("LTE packets") };
	int cmbStreamTypeNChoices = sizeof( cmbStreamTypeChoices ) / sizeof( wxString );
	cmbStreamType = new wxChoice( sbSizer2->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cmbStreamTypeNChoices, cmbStreamTypeChoices, 0 );
	cmbStreamType->SetSelection( 0 );
	fgSizer91->Add( cmbStreamType, 0, 0, 5 );
	
	btnStartStop = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("START"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer91->Add( btnStartStop, 1, wxEXPAND, 5 );
	
	
	fgSizer12->Add( fgSizer91, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer13;
	fgSizer13 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer13->SetFlexibleDirection( wxBOTH );
	fgSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText8 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Rx rate:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	fgSizer13->Add( m_staticText8, 0, 0, 5 );
	
	lblRxDataRate = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("0 MB/s"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRxDataRate->Wrap( -1 );
	fgSizer13->Add( lblRxDataRate, 0, 0, 5 );
	
	m_staticText18 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Tx rate:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	fgSizer13->Add( m_staticText18, 0, 0, 5 );
	
	lblTxDataRate = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("0 MB/s"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTxDataRate->Wrap( -1 );
	fgSizer13->Add( lblTxDataRate, 0, 0, 5 );
	
	m_staticText10 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("FFT/s"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer13->Add( m_staticText10, 0, 0, 5 );
	
	lblFFTrate = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFFTrate->Wrap( -1 );
	fgSizer13->Add( lblFFTrate, 0, 0, 5 );
	
	
	fgSizer12->Add( fgSizer13, 1, wxEXPAND, 5 );
	
	
	sbSizer2->Add( fgSizer12, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Graphs") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer14->SetFlexibleDirection( wxBOTH );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkShowTimeDomain = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Time domain"), wxDefaultPosition, wxDefaultSize, 0 );
	chkShowTimeDomain->Hide();
	chkShowTimeDomain->SetToolTip( wxT("Shows time domain plot") );
	
	fgSizer14->Add( chkShowTimeDomain, 0, 0, 5 );
	
	chkFreezeTimeDomain = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Freeze time"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFreezeTimeDomain->SetToolTip( wxT("Freezes time domain plot") );
	
	fgSizer14->Add( chkFreezeTimeDomain, 0, 0, 5 );
	
	chkShowConstellation = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("I versus Q"), wxDefaultPosition, wxDefaultSize, 0 );
	chkShowConstellation->Hide();
	
	fgSizer14->Add( chkShowConstellation, 0, 0, 5 );
	
	chkFreezeConstellation = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Freeze constellation"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFreezeConstellation->SetToolTip( wxT("Freezes constellation plot") );
	
	fgSizer14->Add( chkFreezeConstellation, 0, 0, 5 );
	
	chkShowFFT = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("FFT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkShowFFT->Hide();
	chkShowFFT->SetToolTip( wxT("Shows FFT plot") );
	
	fgSizer14->Add( chkShowFFT, 0, 0, 5 );
	
	chkFreezeFFT = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Freeze FFT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFreezeFFT->SetToolTip( wxT("Freezes FFT plot") );
	
	fgSizer14->Add( chkFreezeFFT, 0, 0, 5 );
	
	
	sbSizer3->Add( fgSizer14, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( sbSizer3, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Buffers status") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer19;
	fgSizer19 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer19->AddGrowableCol( 1 );
	fgSizer19->SetFlexibleDirection( wxBOTH );
	fgSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText15 = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, wxT("Rx:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	fgSizer19->Add( m_staticText15, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	gaugeRxBuffer = new wxGauge( sbSizer5->GetStaticBox(), wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	gaugeRxBuffer->SetValue( 0 ); 
	fgSizer19->Add( gaugeRxBuffer, 1, wxEXPAND, 5 );
	
	m_staticText16 = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, wxT("Tx:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer19->Add( m_staticText16, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	gaugeTxBuffer = new wxGauge( sbSizer5->GetStaticBox(), wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	gaugeTxBuffer->SetValue( 0 ); 
	fgSizer19->Add( gaugeTxBuffer, 1, wxEXPAND, 5 );
	
	
	sbSizer5->Add( fgSizer19, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( sbSizer5, 1, wxEXPAND, 5 );
	
	
	fgSizer7->Add( fgSizer10, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer7 );
	this->Layout();
	fgSizer7->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	spinFFTsize->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( frFFTviewer::OnFFTsamplesCountChanged ), NULL, this );
	btnStartStop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frFFTviewer::OnbtnStartStop ), NULL, this );
}

frFFTviewer::~frFFTviewer()
{
	// Disconnect Events
	spinFFTsize->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( frFFTviewer::OnFFTsamplesCountChanged ), NULL, this );
	btnStartStop->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frFFTviewer::OnbtnStartStop ), NULL, this );
	
}
