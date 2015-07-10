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
