///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "utilities_gui.h"

///////////////////////////////////////////////////////////////////////////

dlgConnectionSettings_view::dlgConnectionSettings_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxFlexGridSizer *fgSizer211;
    fgSizer211 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer211->AddGrowableCol( 0 );
	fgSizer211->AddGrowableRow( 0 );
	fgSizer211->SetFlexibleDirection( wxBOTH );
	fgSizer211->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxFlexGridSizer *fgSizer212;
    fgSizer212 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer212->AddGrowableCol( 0 );
	fgSizer212->AddGrowableRow( 1 );
	fgSizer212->SetFlexibleDirection( wxBOTH );
	fgSizer212->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_staticText472 = new wxStaticText(this, wxID_ANY, wxT("Board connections:"), wxDefaultPosition,
                                       wxDefaultSize, 0);
    m_staticText472->Wrap( -1 );
	fgSizer212->Add( m_staticText472, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    mListLMS7ports = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), 0, NULL, 0);
    mListLMS7ports->SetMinSize(wxSize(400, 150));

    fgSizer212->Add(mListLMS7ports, 0, wxALL | wxEXPAND, 5);

    fgSizer211->Add(fgSizer212, 1, wxEXPAND, 5);

    wxFlexGridSizer *fgSizer213;
    fgSizer213 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer213->AddGrowableCol( 0 );
	fgSizer213->AddGrowableCol( 1 );
	fgSizer213->AddGrowableCol( 2 );
	fgSizer213->SetFlexibleDirection( wxBOTH );
	fgSizer213->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    btnConnect = new wxButton(this, wxID_ANY, wxT("Connect"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizer213->Add( btnConnect, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

    btnCancel = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizer213->Add( btnCancel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

    btnDisconnect =
        new wxButton(this, wxID_ANY, wxT("Disconnect"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizer213->Add( btnDisconnect, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

    fgSizer211->Add(fgSizer213, 1, wxALIGN_CENTER_HORIZONTAL, 5);

    this->SetSizer(fgSizer211);
    this->Layout();
	fgSizer211->Fit( this );

    this->Centre(wxBOTH);

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

dlgFullMessageLog_view::dlgFullMessageLog_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 500,200 ), wxDefaultSize );

    wxFlexGridSizer *fgSizer19;
    fgSizer19 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer19->AddGrowableCol( 0 );
	fgSizer19->AddGrowableRow( 0 );
	fgSizer19->SetFlexibleDirection( wxBOTH );
	fgSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    txtMessageField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                     wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
#ifdef __WXGTK__
	if ( !txtMessageField->HasFlag( wxTE_MULTILINE ) )
	{
	txtMessageField->SetMaxLength( 100 );
	}
	#else
	txtMessageField->SetMaxLength( 100 );
	#endif
	txtMessageField->SetMinSize( wxSize( 300,100 ) );

    fgSizer19->Add(txtMessageField, 1, wxEXPAND, 5);

    this->SetSizer(fgSizer19);
    this->Layout();
	fgSizer19->Fit( this );

    this->Centre(wxBOTH);
}

dlgFullMessageLog_view::~dlgFullMessageLog_view()
{
}

dlgDeviceInfo_view::dlgDeviceInfo_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxFlexGridSizer *fgSizer12;
    fgSizer12 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    btnGetInfo = new wxButton(this, wxID_ANY, wxT("GET INFO"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizer12->Add( btnGetInfo, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    wxFlexGridSizer *fgSizer8;
    fgSizer8 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_staticText6 =
        new wxStaticText(this, wxID_ANY, wxT("Device:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText6->Wrap( -1 );
	fgSizer8->Add( m_staticText6, 0, wxALL, 5 );

    lblDeviceCtr =
        new wxStaticText(this, wxID_ANY, wxT("?"), wxDefaultPosition, wxSize(100, -1), 0);
    lblDeviceCtr->Wrap( -1 );
	fgSizer8->Add( lblDeviceCtr, 0, wxALL, 5 );

    m_staticText8 =
        new wxStaticText(this, wxID_ANY, wxT("Expansion:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText8->Wrap( -1 );
	fgSizer8->Add( m_staticText8, 0, wxALL, 5 );

    lblExpansionCtr =
        new wxStaticText(this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0);
    lblExpansionCtr->Wrap( -1 );
	fgSizer8->Add( lblExpansionCtr, 0, wxALL, 5 );

    m_staticText10 =
        new wxStaticText(this, wxID_ANY, wxT("Firmware:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText10->Wrap( -1 );
	fgSizer8->Add( m_staticText10, 0, wxALL, 5 );

    lblFirmwareCtr =
        new wxStaticText(this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0);
    lblFirmwareCtr->Wrap( -1 );
	fgSizer8->Add( lblFirmwareCtr, 0, wxALL, 5 );

    m_staticText12 =
        new wxStaticText(this, wxID_ANY, wxT("Hardware:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText12->Wrap( -1 );
	fgSizer8->Add( m_staticText12, 0, wxALL, 5 );

    lblHardwareCtr =
        new wxStaticText(this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0);
    lblHardwareCtr->Wrap( -1 );
	fgSizer8->Add( lblHardwareCtr, 0, wxALL, 5 );

    m_staticText14 =
        new wxStaticText(this, wxID_ANY, wxT("Protocol:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText14->Wrap( -1 );
	fgSizer8->Add( m_staticText14, 0, wxALL, 5 );

    lblProtocolCtr =
        new wxStaticText(this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0);
    lblProtocolCtr->Wrap( -1 );
	fgSizer8->Add( lblProtocolCtr, 0, wxALL, 5 );

    m_staticText41 =
        new wxStaticText(this, wxID_ANY, wxT("Gateware:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText41->Wrap( -1 );
	fgSizer8->Add( m_staticText41, 0, wxALL, 5 );

    lblGatewareCtr =
        new wxStaticText(this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0);
    lblGatewareCtr->Wrap( -1 );
	fgSizer8->Add( lblGatewareCtr, 0, wxALL, 5 );

    m_staticText45 = new wxStaticText(this, wxID_ANY, wxT("Gateware target:"), wxDefaultPosition,
                                      wxDefaultSize, 0);
    m_staticText45->Wrap( -1 );
	fgSizer8->Add( m_staticText45, 0, wxALL, 5 );

    lblGatewareTargetCtr =
        new wxStaticText(this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0);
    lblGatewareTargetCtr->Wrap( -1 );
	fgSizer8->Add( lblGatewareTargetCtr, 0, wxALL, 5 );

    fgSizer12->Add(fgSizer8, 1, wxEXPAND, 5);

    this->SetSizer(fgSizer12);
    this->Layout();
	fgSizer12->Fit( this );

    this->Centre(wxBOTH);

    // Connect Events
    btnGetInfo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgDeviceInfo_view::OnGetInfo ), NULL, this );
}

dlgDeviceInfo_view::~dlgDeviceInfo_view()
{
	// Disconnect Events
	btnGetInfo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgDeviceInfo_view::OnGetInfo ), NULL, this );

}
