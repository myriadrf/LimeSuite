///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep 24 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "lms7002_pnlAFE_view.h"
#include "lms7002_pnlBIAS_view.h"
#include "lms7002_pnlBIST_view.h"
#include "lms7002_pnlCDS_view.h"
#include "lms7002_pnlCLKGEN_view.h"
#include "lms7002_pnlCalibrations_view.h"
#include "lms7002_pnlGains_view.h"
#include "lms7002_pnlLDO_view.h"
#include "lms7002_pnlLimeLightPAD_view.h"
#include "lms7002_pnlRBB_view.h"
#include "lms7002_pnlRFE_view.h"
#include "lms7002_pnlRxTSP_view.h"
#include "lms7002_pnlSX_view.h"
#include "lms7002_pnlTBB_view.h"
#include "lms7002_pnlTRF_view.h"
#include "lms7002_pnlTxTSP_view.h"
#include "lms7002_pnlXBUF_view.h"
#include "numericSlider.h"

#include "lms7002_wxgui.h"

///////////////////////////////////////////////////////////////////////////

mainPanel::mainPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer298;
	fgSizer298 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer298->AddGrowableCol( 0 );
	fgSizer298->AddGrowableRow( 1 );
	fgSizer298->SetFlexibleDirection( wxBOTH );
	fgSizer298->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer299;
	fgSizer299 = new wxFlexGridSizer( 0, 7, 0, 0 );
	fgSizer299->AddGrowableCol( 3 );
	fgSizer299->SetFlexibleDirection( wxBOTH );
	fgSizer299->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_BUTTON2 = new wxButton( this, wxID_ANY, wxT("Open"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	ID_BUTTON2->SetDefault(); 
	fgSizer299->Add( ID_BUTTON2, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	ID_BUTTON3 = new wxButton( this, wxID_ANY, wxT("Save"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	ID_BUTTON3->SetDefault(); 
	fgSizer299->Add( ID_BUTTON3, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxFlexGridSizer* fgSizer300;
	fgSizer300 = new wxFlexGridSizer( 0, 5, 0, 0 );
	fgSizer300->AddGrowableRow( 0 );
	fgSizer300->SetFlexibleDirection( wxBOTH );
	fgSizer300->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer248;
	fgSizer248 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer248->SetFlexibleDirection( wxBOTH );
	fgSizer248->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer249;
	fgSizer249 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer249->SetFlexibleDirection( wxBOTH );
	fgSizer249->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cmbLmsDevice = new wxComboBox( this, ID_G_LNA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbLmsDevice->Append( wxT("LMS 1") );
	cmbLmsDevice->Append( wxT("LMS 2") );
	cmbLmsDevice->SetSelection( 0 );
	cmbLmsDevice->SetToolTip( wxT("Controls the gain of the LNA") );
	
	fgSizer249->Add( cmbLmsDevice, 0, wxALL, 0 );
	
	rbChannelA = new wxRadioButton( this, ID_BTN_CH_A, wxT("A CHANNEL"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer249->Add( rbChannelA, 0, wxEXPAND, 5 );
	
	rbChannelB = new wxRadioButton( this, ID_BTN_CH_B, wxT("B CHANNEL"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer249->Add( rbChannelB, 0, wxEXPAND, 5 );
	
	chkEnableMIMO = new wxCheckBox( this, wxID_ANY, wxT("Enable MIMO"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEnableMIMO->SetToolTip( wxT("Enables required registers for MIMO mode") );
	
	fgSizer249->Add( chkEnableMIMO, 0, 0, 5 );
	
	
	fgSizer248->Add( fgSizer249, 0, 0, 5 );
	
	
	fgSizer300->Add( fgSizer248, 0, 0, 5 );
	
	btnDownloadAll = new wxButton( this, ID_BTN_CHIP_TO_GUI, wxT("Chip-->GUI"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer300->Add( btnDownloadAll, 0, 0, 5 );
	
	btnUploadAll = new wxButton( this, wxID_ANY, wxT("GUI-->Chip"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer300->Add( btnUploadAll, 0, 0, 5 );
	
	btnResetChip = new wxButton( this, ID_BTN_RESET_CHIP, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer300->Add( btnResetChip, 0, 0, 5 );
	
	btnLoadDefault = new wxButton( this, ID_BTN_RESET_CHIP, wxT("Default"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer300->Add( btnLoadDefault, 0, 0, 5 );
	
	wxFlexGridSizer* fgSizer247;
	fgSizer247 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer247->SetFlexibleDirection( wxBOTH );
	fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer300->Add( fgSizer247, 1, wxEXPAND, 5 );
	
	
	fgSizer299->Add( fgSizer300, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 10 );
	
	txtTemperature = new wxStaticText( this, wxID_ANY, wxT("Temperature: ?????"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTemperature->Wrap( -1 );
	fgSizer299->Add( txtTemperature, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	btnReadTemperature = new wxButton( this, wxID_ANY, wxT("Read Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer299->Add( btnReadTemperature, 0, 0, 5 );
	
	
	fgSizer298->Add( fgSizer299, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND|wxBOTTOM, 10 );
	
	tabsNotebook = new wxNotebook( this, ID_TABS_NOTEBOOK, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	mTabCalibrations = new lms7002_pnlCalibrations_view( tabsNotebook, ID_TAB_CALIBRATIONS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabCalibrations, wxT("Calibrations"), true );
	mTabRFE = new lms7002_pnlRFE_view( tabsNotebook, ID_TAB_RFE, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabRFE, wxT("RFE"), false );
	mTabRBB = new lms7002_pnlRBB_view( tabsNotebook, ID_TAB_RBB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabRBB, wxT("RBB"), false );
	mTabTRF = new lms7002_pnlTRF_view( tabsNotebook, ID_TAB_TRF, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabTRF, wxT("TRF"), false );
	mTabTBB = new lms7002_pnlTBB_view( tabsNotebook, ID_TAB_TBB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabTBB, wxT("TBB"), false );
	mTabAFE = new lms7002_pnlAFE_view( tabsNotebook, ID_TAB_AFE, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabAFE, wxT("AFE"), false );
	mTabBIAS = new lms7002_pnlBIAS_view( tabsNotebook, ID_TAB_BIAS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabBIAS, wxT("BIAS"), false );
	mTabLDO = new lms7002_pnlLDO_view( tabsNotebook, ID_TAB_LDO, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabLDO, wxT("LDO"), false );
	mTabXBUF = new lms7002_pnlXBUF_view( tabsNotebook, ID_TAB_XBUF, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabXBUF, wxT("XBUF"), false );
	mTabCGEN = new lms7002_pnlCLKGEN_view( tabsNotebook, ID_TAB_CGEN, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabCGEN, wxT("CLKGEN"), false );
	mTabSXR = new lms7002_pnlSX_view( tabsNotebook, ID_TAB_SXR, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabSXR, wxT("SXR"), false );
	mTabSXT = new lms7002_pnlSX_view( tabsNotebook, ID_TAB_SXT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabSXT, wxT("SXT"), false );
	mTabLimeLight = new lms7002_pnlLimeLightPAD_view( tabsNotebook, ID_TAB_LIMELIGHT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabLimeLight, wxT("LimeLight && PAD"), false );
	mTabTxTSP = new lms7002_pnlTxTSP_view( tabsNotebook, ID_TAB_TXTSP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabTxTSP, wxT("TxTSP"), false );
	mTabRxTSP = new lms7002_pnlRxTSP_view( tabsNotebook, ID_TAB_RXTSP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabRxTSP, wxT("RxTSP"), false );
	mTabCDS = new lms7002_pnlCDS_view( tabsNotebook, ID_TAB_CDS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabCDS, wxT("CDS"), false );
	mTabBIST = new lms7002_pnlBIST_view( tabsNotebook, ID_TAB_BIST, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabBIST, wxT("BIST"), false );
	mTabTrxGain = new lms7002_pnlGains_view( tabsNotebook, ID_TAB_GAINS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	tabsNotebook->AddPage( mTabTrxGain, wxT("TRX Gain"), false );
	
	fgSizer298->Add( tabsNotebook, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	this->SetSizer( fgSizer298 );
	this->Layout();
	fgSizer298->Fit( this );
	
	// Connect Events
	ID_BUTTON2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnOpenProject ), NULL, this );
	ID_BUTTON3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnSaveProject ), NULL, this );
	cmbLmsDevice->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( mainPanel::OnLmsDeviceSelect ), NULL, this );
	rbChannelA->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( mainPanel::OnSwitchToChannelA ), NULL, this );
	rbChannelB->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( mainPanel::OnSwitchToChannelB ), NULL, this );
	chkEnableMIMO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( mainPanel::OnEnableMIMOchecked ), NULL, this );
	btnDownloadAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnDownloadAll ), NULL, this );
	btnUploadAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnUploadAll ), NULL, this );
	btnResetChip->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnResetChip ), NULL, this );
	btnLoadDefault->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnLoadDefault ), NULL, this );
	btnReadTemperature->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnReadTemperature ), NULL, this );
	tabsNotebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( mainPanel::Onnotebook_modulesPageChanged ), NULL, this );
}

mainPanel::~mainPanel()
{
	// Disconnect Events
	ID_BUTTON2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnOpenProject ), NULL, this );
	ID_BUTTON3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnSaveProject ), NULL, this );
	cmbLmsDevice->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( mainPanel::OnLmsDeviceSelect ), NULL, this );
	rbChannelA->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( mainPanel::OnSwitchToChannelA ), NULL, this );
	rbChannelB->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( mainPanel::OnSwitchToChannelB ), NULL, this );
	chkEnableMIMO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( mainPanel::OnEnableMIMOchecked ), NULL, this );
	btnDownloadAll->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnDownloadAll ), NULL, this );
	btnUploadAll->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnUploadAll ), NULL, this );
	btnResetChip->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnResetChip ), NULL, this );
	btnLoadDefault->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnLoadDefault ), NULL, this );
	btnReadTemperature->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainPanel::OnReadTemperature ), NULL, this );
	tabsNotebook->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( mainPanel::Onnotebook_modulesPageChanged ), NULL, this );
	
}

pnlRFE_view::pnlRFE_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgMainSizer;
	fgMainSizer = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgMainSizer->SetFlexibleDirection( wxBOTH );
	fgMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer190;
	fgSizer190 = new wxFlexGridSizer( 0, 1, 5, 0 );
	fgSizer190->SetFlexibleDirection( wxBOTH );
	fgSizer190->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer42;
	fgSizer42 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer42->SetFlexibleDirection( wxBOTH );
	fgSizer42->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkPD_LNA_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LNA_RFE, wxT("LNA_RFE"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LNA_RFE->SetToolTip( wxT("Power control signal for LNA_RFE") );
	
	fgSizer42->Add( chkPD_LNA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkPD_RLOOPB_1_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_RLOOPB_1_RFE, wxT("Loopback 1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_RLOOPB_1_RFE->SetToolTip( wxT("Power control signal for RXFE loopback 1") );
	
	fgSizer42->Add( chkPD_RLOOPB_1_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkPD_RLOOPB_2_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_RLOOPB_2_RFE, wxT("Loopback 2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_RLOOPB_2_RFE->SetToolTip( wxT("Power control signal for RXFE loopback 2") );
	
	fgSizer42->Add( chkPD_RLOOPB_2_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_MXLOBUF_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_MXLOBUF_RFE, wxT("Mixer LO buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_MXLOBUF_RFE->SetToolTip( wxT("Power control signal for RXFE mixer lo buffer") );
	
	fgSizer42->Add( chkPD_MXLOBUF_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkPD_QGEN_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_QGEN_RFE, wxT("Quadrature LO generator"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_QGEN_RFE->SetToolTip( wxT("Power control signal for RXFE Quadrature LO generator") );
	
	fgSizer42->Add( chkPD_QGEN_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkPD_RSSI_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_RSSI_RFE, wxT("RSSI"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_RSSI_RFE->SetToolTip( wxT("Power control signal for RXFE RSSI") );
	
	fgSizer42->Add( chkPD_RSSI_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkPD_TIA_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TIA_RFE, wxT("TIA"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_TIA_RFE->SetToolTip( wxT("Power control signal for RXFE TIA") );
	
	fgSizer42->Add( chkPD_TIA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkEN_G_RFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_RFE, wxT("Enable RFE module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_RFE->SetToolTip( wxT("Enable control for all the RFE_1 power downs") );
	
	fgSizer42->Add( chkEN_G_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	wxStaticBoxSizer* sbSizer23;
	sbSizer23 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
	
	chkEN_DIR_RFE = new wxCheckBox( sbSizer23->GetStaticBox(), ID_EN_DIR_RFE, wxT("Direct control of PDs and ENs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DIR_RFE->SetToolTip( wxT("Enables direct control of PDs and ENs for RFE module") );
	
	sbSizer23->Add( chkEN_DIR_RFE, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	
	fgSizer42->Add( sbSizer23, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	
	sbSizerPowerDowns->Add( fgSizer42, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer190->Add( sbSizerPowerDowns, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerInputShorting;
	sbSizerInputShorting = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Input shorting switches") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer44;
	fgSizer44 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer44->SetFlexibleDirection( wxBOTH );
	fgSizer44->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkEN_INSHSW_LB1_RFE = new wxCheckBox( sbSizerInputShorting->GetStaticBox(), ID_EN_INSHSW_LB1_RFE, wxT("input of loopback 1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_INSHSW_LB1_RFE->SetToolTip( wxT("Enables the input shorting switch at the input  of the loopback 1 (in parallel with LNAL mixer)") );
	
	fgSizer44->Add( chkEN_INSHSW_LB1_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkEN_INSHSW_LB2_RFE = new wxCheckBox( sbSizerInputShorting->GetStaticBox(), ID_EN_INSHSW_LB2_RFE, wxT("input of loopback 2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_INSHSW_LB2_RFE->SetToolTip( wxT("Enables the input shorting switch at the input  of the loopback 2 (in parallel with LNAW mixer)") );
	
	fgSizer44->Add( chkEN_INSHSW_LB2_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkEN_INSHSW_L_RFE = new wxCheckBox( sbSizerInputShorting->GetStaticBox(), ID_EN_INSHSW_L_RFE, wxT("input of LNAL"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_INSHSW_L_RFE->SetToolTip( wxT("Enables the input shorting switch at the input  of the LNAL") );
	
	fgSizer44->Add( chkEN_INSHSW_L_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkEN_INSHSW_W_RFE = new wxCheckBox( sbSizerInputShorting->GetStaticBox(), ID_EN_INSHSW_W_RFE, wxT("input of LNAW"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_INSHSW_W_RFE->SetToolTip( wxT("Enables the input shorting switch at the input  of the LNAW") );
	
	fgSizer44->Add( chkEN_INSHSW_W_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	
	sbSizerInputShorting->Add( fgSizer44, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer190->Add( sbSizerInputShorting, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgMainSizer->Add( fgSizer190, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer41;
	fgSizer41 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer41->SetFlexibleDirection( wxBOTH );
	fgSizer41->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer43;
	fgSizer43 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer43->SetFlexibleDirection( wxBOTH );
	fgSizer43->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("Active path to the RXFE"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer43->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbSEL_PATH_RFE = new wxComboBox( this, ID_SEL_PATH_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbSEL_PATH_RFE->Append( wxT("No path active") );
	cmbSEL_PATH_RFE->Append( wxT("LNAH") );
	cmbSEL_PATH_RFE->Append( wxT("LNAL") );
	cmbSEL_PATH_RFE->Append( wxT("LNAW") );
	cmbSEL_PATH_RFE->SetToolTip( wxT("Selects the active path of the RXFE") );
	
	fgSizer43->Add( cmbSEL_PATH_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("Decoupling cap at output of RX mixer"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer43->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCAP_RXMXO_RFE = new wxComboBox( this, ID_CAP_RXMXO_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCAP_RXMXO_RFE->SetToolTip( wxT("Control the decoupling cap at the output of the RX Mixer") );
	
	fgSizer43->Add( cmbCAP_RXMXO_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("Controls cap parallel with the LNA input"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer43->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCGSIN_LNA_RFE = new wxComboBox( this, ID_CGSIN_LNA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCGSIN_LNA_RFE->SetToolTip( wxT("Controls the cap parallel with the LNA input input NMOS CGS to control the Q of the maching circuit and provides trade off between gain/NF and IIP. The higher the frequency, the lower CGSIN_LNA_RFE should be. Also, the higher CGSIN, the lower the Q, The lower the gain, the higher the NF, and the higher the IIP3") );
	
	fgSizer43->Add( cmbCGSIN_LNA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT4 = new wxStaticText( this, wxID_ANY, wxT("Compensation resistor of TIA opamp"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer43->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRCOMP_TIA_RFE = new wxComboBox( this, ID_RCOMP_TIA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRCOMP_TIA_RFE->SetToolTip( wxT("Controls the compensation resistors of the TIA opamp") );
	
	fgSizer43->Add( cmbRCOMP_TIA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT5 = new wxStaticText( this, wxID_ANY, wxT("Sets feedback resistor to nominal value"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer43->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRFB_TIA_RFE = new wxComboBox( this, ID_RFB_TIA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRFB_TIA_RFE->SetToolTip( wxT("Sets the feedback resistor to the nominal value") );
	
	fgSizer43->Add( cmbRFB_TIA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_NEXTRX_RFE = new wxCheckBox( this, ID_EN_NEXTRX_RFE, wxT("Enable Rx MIMO mode"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_NEXTRX_RFE->SetToolTip( wxT("Enables the daisy chain LO buffer going from RXFE1  to RXFE2") );
	
	fgSizer43->Add( chkEN_NEXTRX_RFE, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer41->Add( fgSizer43, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizerCurrentControl;
	sbSizerCurrentControl = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Current control") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer47;
	fgSizer47 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer47->AddGrowableCol( 1 );
	fgSizer47->SetFlexibleDirection( wxBOTH );
	fgSizer47->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizerCurrentControl->GetStaticBox(), wxID_ANY, wxT("LNA output common mode voltage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer47->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LNACMO_RFE = new wxComboBox( sbSizerCurrentControl->GetStaticBox(), ID_ICT_LNACMO_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LNACMO_RFE->SetToolTip( wxT("Controls the current generating LNA output common mode voltage") );
	
	fgSizer47->Add( cmbICT_LNACMO_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizerCurrentControl->GetStaticBox(), wxID_ANY, wxT("LNA core"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer47->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LNA_RFE = new wxComboBox( sbSizerCurrentControl->GetStaticBox(), ID_ICT_LNA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LNA_RFE->SetToolTip( wxT("Controls the current of the LNA core") );
	
	fgSizer47->Add( cmbICT_LNA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerCurrentControl->Add( fgSizer47, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer41->Add( sbSizerCurrentControl, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerDC;
	sbSizerDC = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("DC") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer45;
	fgSizer45 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer45->AddGrowableCol( 1 );
	fgSizer45->SetFlexibleDirection( wxBOTH );
	fgSizer45->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgSizer45->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCOFFI_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFI_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
	fgSizer45->Add( cmbDCOFFI_RFE, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer45->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCOFFQ_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFQ_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
	fgSizer45->Add( cmbDCOFFQ_RFE, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT8 = new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Mixer LO signal"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer45->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LODC_RFE = new wxComboBox( sbSizerDC->GetStaticBox(), ID_ICT_LODC_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LODC_RFE->SetToolTip( wxT("Controls the DC of the mixer LO signal at the gate of the mixer switches") );
	
	fgSizer45->Add( cmbICT_LODC_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer45->Add( 0, 0, 1, wxEXPAND, 5 );
	
	chkEN_DCOFF_RXFE_RFE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_EN_DCOFF_RXFE_RFE, wxT("Enable DC offset"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DCOFF_RXFE_RFE->SetToolTip( wxT("Enables the DCOFFSET block for the RXFE") );
	
	fgSizer45->Add( chkEN_DCOFF_RXFE_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerDC->Add( fgSizer45, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer41->Add( sbSizerDC, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgMainSizer->Add( fgSizer41, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer191;
	fgSizer191 = new wxFlexGridSizer( 0, 1, 5, 0 );
	fgSizer191->SetFlexibleDirection( wxBOTH );
	fgSizer191->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerCapacitorControls;
	sbSizerCapacitorControls = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Capacitor controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer49;
	fgSizer49 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer49->AddGrowableCol( 1 );
	fgSizer49->SetFlexibleDirection( wxBOTH );
	fgSizer49->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizerCapacitorControls->GetStaticBox(), wxID_ANY, wxT("Compensation TIA"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer49->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCCOMP_TIA_RFE = new wxComboBox( sbSizerCapacitorControls->GetStaticBox(), ID_CCOMP_TIA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCCOMP_TIA_RFE->SetToolTip( wxT("Compensation capacitor for TIA") );
	
	fgSizer49->Add( cmbCCOMP_TIA_RFE, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizerCapacitorControls->GetStaticBox(), wxID_ANY, wxT("Feedback TIA"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( -1 );
	fgSizer49->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCFB_TIA_RFE = new NumericSlider( sbSizerCapacitorControls->GetStaticBox(), ID_CFB_TIA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 4095, 0 );
	fgSizer49->Add( cmbCFB_TIA_RFE, 1, wxEXPAND, 5 );
	
	
	sbSizerCapacitorControls->Add( fgSizer49, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer191->Add( sbSizerCapacitorControls, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerGainControls;
	sbSizerGainControls = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Gain controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer50;
	fgSizer50 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer50->AddGrowableCol( 0 );
	fgSizer50->SetFlexibleDirection( wxBOTH );
	fgSizer50->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizerGainControls->GetStaticBox(), wxID_ANY, wxT("LNA"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer50->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbG_LNA_RFE = new wxComboBox( sbSizerGainControls->GetStaticBox(), ID_G_LNA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbG_LNA_RFE->SetToolTip( wxT("Controls the gain of the LNA") );
	
	fgSizer50->Add( cmbG_LNA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT17 = new wxStaticText( sbSizerGainControls->GetStaticBox(), wxID_ANY, wxT("Loopback"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT17->Wrap( -1 );
	fgSizer50->Add( ID_STATICTEXT17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbG_RXLOOPB_RFE = new wxComboBox( sbSizerGainControls->GetStaticBox(), ID_G_RXLOOPB_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbG_RXLOOPB_RFE->SetToolTip( wxT("Controls RXFE loopback gain") );
	
	fgSizer50->Add( cmbG_RXLOOPB_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT18 = new wxStaticText( sbSizerGainControls->GetStaticBox(), wxID_ANY, wxT("TIA"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT18->Wrap( -1 );
	fgSizer50->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbG_TIA_RFE = new wxComboBox( sbSizerGainControls->GetStaticBox(), ID_G_TIA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbG_TIA_RFE->SetToolTip( wxT("Controls the Gain of the TIA") );
	
	fgSizer50->Add( cmbG_TIA_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerGainControls->Add( fgSizer50, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	
	fgSizer191->Add( sbSizerGainControls, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerRefCurrent;
	sbSizerRefCurrent = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Reference current") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer46;
	fgSizer46 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer46->AddGrowableCol( 1 );
	fgSizer46->SetFlexibleDirection( wxBOTH );
	fgSizer46->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizerRefCurrent->GetStaticBox(), wxID_ANY, wxT("Loopback amplifier"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer46->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LOOPB_RFE = new wxComboBox( sbSizerRefCurrent->GetStaticBox(), ID_ICT_LOOPB_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LOOPB_RFE->SetToolTip( wxT("Controls the reference current of the RXFE loopback amplifier") );
	
	fgSizer46->Add( cmbICT_LOOPB_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizerRefCurrent->GetStaticBox(), wxID_ANY, wxT("TIA 1st stage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer46->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_TIAMAIN_RFE = new NumericSlider( sbSizerRefCurrent->GetStaticBox(), ID_ICT_TIAMAIN_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
	fgSizer46->Add( cmbICT_TIAMAIN_RFE, 1, wxEXPAND, 5 );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizerRefCurrent->GetStaticBox(), wxID_ANY, wxT("TIA 2nd stage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer46->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_TIAOUT_RFE = new NumericSlider( sbSizerRefCurrent->GetStaticBox(), ID_ICT_TIAOUT_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
	fgSizer46->Add( cmbICT_TIAOUT_RFE, 1, wxEXPAND, 5 );
	
	
	sbSizerRefCurrent->Add( fgSizer46, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer191->Add( sbSizerRefCurrent, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerTrimDuty;
	sbSizerTrimDuty = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Trim duty cycle") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer51;
	fgSizer51 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer51->AddGrowableCol( 0 );
	fgSizer51->AddGrowableCol( 1 );
	fgSizer51->SetFlexibleDirection( wxBOTH );
	fgSizer51->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT19 = new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("I channel:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT19->Wrap( -1 );
	fgSizer51->Add( ID_STATICTEXT19, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbCDC_I_RFE = new wxComboBox( sbSizerTrimDuty->GetStaticBox(), ID_CDC_I_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer51->Add( cmbCDC_I_RFE, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	ID_STATICTEXT20 = new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("Q channel:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT20->Wrap( -1 );
	fgSizer51->Add( ID_STATICTEXT20, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbCDC_Q_RFE = new wxComboBox( sbSizerTrimDuty->GetStaticBox(), ID_CDC_Q_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer51->Add( cmbCDC_Q_RFE, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	sbSizerTrimDuty->Add( fgSizer51, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer191->Add( sbSizerTrimDuty, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgMainSizer->Add( fgSizer191, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgMainSizer );
	this->Layout();
	fgMainSizer->Fit( this );
	
	// Connect Events
	chkPD_LNA_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RLOOPB_1_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RLOOPB_2_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_MXLOBUF_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_QGEN_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RSSI_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_TIA_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_LB1_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_LB2_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_L_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_W_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbSEL_PATH_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCAP_RXMXO_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCGSIN_LNA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbRCOMP_TIA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbRFB_TIA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_NEXTRX_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LNACMO_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LNA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFI_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFQ_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LODC_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_DCOFF_RXFE_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCCOMP_TIA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCFB_TIA_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbG_LNA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbG_RXLOOPB_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbG_TIA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LOOPB_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_TIAMAIN_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_TIAOUT_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_I_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_Q_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
}

pnlRFE_view::~pnlRFE_view()
{
	// Disconnect Events
	chkPD_LNA_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RLOOPB_1_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RLOOPB_2_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_MXLOBUF_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_QGEN_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RSSI_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_TIA_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_LB1_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_LB2_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_L_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_INSHSW_W_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbSEL_PATH_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCAP_RXMXO_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCGSIN_LNA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbRCOMP_TIA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbRFB_TIA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_NEXTRX_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LNACMO_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LNA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFI_RFE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFQ_RFE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LODC_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_DCOFF_RXFE_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCCOMP_TIA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCFB_TIA_RFE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbG_LNA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbG_RXLOOPB_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbG_TIA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LOOPB_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_TIAMAIN_RFE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbICT_TIAOUT_RFE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_I_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_Q_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRFE_view::ParameterChangeHandler ), NULL, this );
	
}

pnlRBB_view::pnlRBB_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer52;
	fgSizer52 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer52->SetFlexibleDirection( wxBOTH );
	fgSizer52->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer55;
	fgSizer55 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer55->SetFlexibleDirection( wxBOTH );
	fgSizer55->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkPD_LPFH_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFH_RBB, wxT("LPFH block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LPFH_RBB->SetToolTip( wxT("Power down of the LPFH block") );
	
	fgSizer55->Add( chkPD_LPFH_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_LPFL_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFL_RBB, wxT("LPFL block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LPFL_RBB->SetToolTip( wxT("Power down of the LPFL block") );
	
	fgSizer55->Add( chkPD_LPFL_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_PGA_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PGA_RBB, wxT("PGA block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_PGA_RBB->SetToolTip( wxT("Power down of the PGA block") );
	
	fgSizer55->Add( chkPD_PGA_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_RBB, wxT("Enable RBB module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_RBB->SetToolTip( wxT("Enable control for all the RBB_1 power downs") );
	
	fgSizer55->Add( chkEN_G_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizer35;
	sbSizer35 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
	
	chkEN_DIR_RBB = new wxCheckBox( sbSizer35->GetStaticBox(), ID_EN_DIR_RBB, wxT("Direct control of PDs and ENs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DIR_RBB->SetToolTip( wxT("Enables direct control of PDs and ENs for RBB module") );
	
	sbSizer35->Add( chkEN_DIR_RBB, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer55->Add( sbSizer35, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizerPowerDowns->Add( fgSizer55, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer52->Add( sbSizerPowerDowns, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer259;
	fgSizer259 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer259->AddGrowableCol( 0 );
	fgSizer259->SetFlexibleDirection( wxBOTH );
	fgSizer259->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer53;
	fgSizer53 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer53->AddGrowableCol( 1 );
	fgSizer53->SetFlexibleDirection( wxBOTH );
	fgSizer53->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT14 = new wxStaticText( this, wxID_ANY, wxT("BB loopback to RXLPF"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer53->Add( ID_STATICTEXT14, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbBBLoopback = new wxComboBox( this, ID_BBLoopback, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbBBLoopback->Append( wxT("LPFH & RBB") );
	cmbBBLoopback->Append( wxT("LPFL & RBB") );
	cmbBBLoopback->Append( wxT("Disabled") );
	cmbBBLoopback->SetSelection( 0 );
	cmbBBLoopback->SetToolTip( wxT("This is the loopback enable signal that is enabled when high band LPFH_RBB or low band LPFL_RBB  is selected for the loopback path. \n0x0115 [15] EN_LB_LPFH_RBB\n0x0115 [14] EN_LB_LPFL_RBB") );
	
	fgSizer53->Add( cmbBBLoopback, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("PGA input connected to"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer53->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbINPUT_CTL_PGA_RBB = new wxComboBox( this, ID_INPUT_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	cmbINPUT_CTL_PGA_RBB->SetToolTip( wxT("There are a total of four different differential inputs to the PGA. Only one of them is active at a time") );
	
	fgSizer53->Add( cmbINPUT_CTL_PGA_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("PGA gain"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer53->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbG_PGA_RBB = new wxComboBox( this, ID_G_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbG_PGA_RBB->SetToolTip( wxT("This is the gain of the PGA") );
	
	fgSizer53->Add( cmbG_PGA_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("PGA Feedback capacitor"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer53->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbC_CTL_PGA_RBB = new NumericSlider( this, ID_C_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer53->Add( cmbC_CTL_PGA_RBB, 0, wxEXPAND, 5 );
	
	
	fgSizer259->Add( fgSizer53, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0 );
	
	wxString rgrOSW_PGA_RBBChoices[] = { wxT("ADC"), wxT("Output pads") };
	int rgrOSW_PGA_RBBNChoices = sizeof( rgrOSW_PGA_RBBChoices ) / sizeof( wxString );
	rgrOSW_PGA_RBB = new wxRadioBox( this, ID_OSW_PGA_RBB, wxT("PGA output connected to"), wxDefaultPosition, wxDefaultSize, rgrOSW_PGA_RBBNChoices, rgrOSW_PGA_RBBChoices, 2, wxRA_SPECIFY_COLS );
	rgrOSW_PGA_RBB->SetSelection( 1 );
	rgrOSW_PGA_RBB->SetToolTip( wxT("There are two instances of the PGA circuit in the design. The output of the RBB_LPF blocks are connected the input of these PGA blocks (common). The output of one of them is connected to two pads pgaoutn and pgaoutp and the output of the other PGA is connected directly to the ADC input") );
	
	fgSizer259->Add( rgrOSW_PGA_RBB, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer52->Add( fgSizer259, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerRCtimeConstants;
	sbSizerRCtimeConstants = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("RXLPF RC time constant") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer54;
	fgSizer54 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer54->SetFlexibleDirection( wxBOTH );
	fgSizer54->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("Resistance"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer54->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbR_CTL_LPF_RBB = new wxComboBox( sbSizerRCtimeConstants->GetStaticBox(), ID_R_CTL_LPF_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbR_CTL_LPF_RBB->SetToolTip( wxT("Controls the absolute value of the resistance of the RC time constant of the RBB_LPF blocks (both Low and High)") );
	
	fgSizer54->Add( cmbR_CTL_LPF_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("LPFH capacitance value"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer54->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbC_CTL_LPFH_RBB = new NumericSlider( sbSizerRCtimeConstants->GetStaticBox(), ID_C_CTL_LPFH_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbC_CTL_LPFH_RBB->SetMinSize( wxSize( 100,-1 ) );
	
	fgSizer54->Add( cmbC_CTL_LPFH_RBB, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("LPFL capacitance value"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgSizer54->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbC_CTL_LPFL_RBB = new NumericSlider( sbSizerRCtimeConstants->GetStaticBox(), ID_C_CTL_LPFL_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 2047, 0 );
	fgSizer54->Add( cmbC_CTL_LPFL_RBB, 0, wxEXPAND, 5 );
	
	
	sbSizerRCtimeConstants->Add( fgSizer54, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizerRxFilters;
	sbSizerRxFilters = new wxStaticBoxSizer( new wxStaticBox( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("Rx Filters") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer199;
	fgSizer199 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer199->SetFlexibleDirection( wxBOTH );
	fgSizer199->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText309 = new wxStaticText( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("RF bandwidth (MHz)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText309->Wrap( -1 );
	fgSizer199->Add( m_staticText309, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtLowBW_MHz = new wxTextCtrl( sbSizerRxFilters->GetStaticBox(), ID_TXT_LOWBW, wxT("10"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtLowBW_MHz->HasFlag( wxTE_MULTILINE ) )
	{
	txtLowBW_MHz->SetMaxLength( 10 );
	}
	#else
	txtLowBW_MHz->SetMaxLength( 10 );
	#endif
	fgSizer199->Add( txtLowBW_MHz, 0, 0, 5 );
	
	btnTuneFilter = new wxButton( sbSizerRxFilters->GetStaticBox(), ID_BTN_TUNE_FILTER, wxT("TUNE"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer199->Add( btnTuneFilter, 0, wxEXPAND, 5 );
	
	
	sbSizerRxFilters->Add( fgSizer199, 1, wxEXPAND, 5 );
	
	
	sbSizerRCtimeConstants->Add( sbSizerRxFilters, 1, wxEXPAND, 5 );
	
	
	fgSizer52->Add( sbSizerRCtimeConstants, 0, wxALIGN_LEFT|wxALIGN_TOP, 0 );
	
	wxStaticBoxSizer* sbSizerOpAmp;
	sbSizerOpAmp = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Operational amplifier") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer56;
	fgSizer56 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer56->AddGrowableCol( 1 );
	fgSizer56->SetFlexibleDirection( wxBOTH );
	fgSizer56->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("LPFH stability passive compensation"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer56->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRCC_CTL_LPFH_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_RCC_CTL_LPFH_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRCC_CTL_LPFH_RBB->SetToolTip( wxT("Controls the stability passive compensation of the LPFH_RBB operational amplifier") );
	
	fgSizer56->Add( cmbRCC_CTL_LPFH_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT8 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("LPFL stability passive compensation"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer56->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRCC_CTL_LPFL_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_RCC_CTL_LPFL_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRCC_CTL_LPFL_RBB->SetToolTip( wxT("Controls the stability passive compensation of the LPFL_RBB operational amplifier") );
	
	fgSizer56->Add( cmbRCC_CTL_LPFL_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Input stage reference bias current (RBB_LPF)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer56->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPF_IN_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_LPF_IN_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPF_IN_RBB->SetToolTip( wxT("Controls the reference bias current of the input stage of the operational amplifier used in RBB_LPF blocks (Low or High). ") );
	
	fgSizer56->Add( cmbICT_LPF_IN_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Output stage reference bias current (RBB_LPF)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer56->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPF_OUT_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_LPF_OUT_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPF_OUT_RBB->SetToolTip( wxT("The reference bias current of the output stage of the operational amplifier used in RBB_LPF blocks (low or High)") );
	
	fgSizer56->Add( cmbICT_LPF_OUT_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Output stage reference bias current (PGA)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer56->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_PGA_OUT_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_PGA_OUT_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_PGA_OUT_RBB->SetToolTip( wxT("Controls the output stage reference bias current of the operational amplifier used in the PGA circuit") );
	
	fgSizer56->Add( cmbICT_PGA_OUT_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Input stage reference bias current (PGA)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer56->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_PGA_IN_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_PGA_IN_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_PGA_IN_RBB->SetToolTip( wxT("Controls the input stage reference bias current of the operational amplifier used in the PGA circuit") );
	
	fgSizer56->Add( cmbICT_PGA_IN_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("PGA stability passive compensation"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer56->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRCC_CTL_PGA_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_RCC_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRCC_CTL_PGA_RBB->SetToolTip( wxT("Controls the stability passive compensation of the PGA_RBB operational amplifier") );
	
	fgSizer56->Add( cmbRCC_CTL_PGA_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerOpAmp->Add( fgSizer56, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer52->Add( sbSizerOpAmp, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	this->SetSizer( fgSizer52 );
	this->Layout();
	fgSizer52->Fit( this );
	
	// Connect Events
	chkPD_LPFH_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFL_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_PGA_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbBBLoopback->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::OncmbBBLoopbackSelected ), NULL, this );
	cmbINPUT_CTL_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbG_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	rgrOSW_PGA_RBB->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbR_CTL_LPF_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_LPFH_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_LPFL_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	btnTuneFilter->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRBB_view::OnbtnTuneFilter ), NULL, this );
	cmbRCC_CTL_LPFH_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCC_CTL_LPFL_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPF_IN_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPF_OUT_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_PGA_OUT_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_PGA_IN_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
}

pnlRBB_view::~pnlRBB_view()
{
	// Disconnect Events
	chkPD_LPFH_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFL_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_PGA_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbBBLoopback->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::OncmbBBLoopbackSelected ), NULL, this );
	cmbINPUT_CTL_PGA_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbG_PGA_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_PGA_RBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	rgrOSW_PGA_RBB->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbR_CTL_LPF_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_LPFH_RBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_LPFL_RBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	btnTuneFilter->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRBB_view::OnbtnTuneFilter ), NULL, this );
	cmbRCC_CTL_LPFH_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCC_CTL_LPFL_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPF_IN_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPF_OUT_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_PGA_OUT_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_PGA_IN_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCC_CTL_PGA_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRBB_view::ParameterChangeHandler ), NULL, this );
	
}

pnlTRF_view::pnlTRF_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer33;
	fgSizer33 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer33->SetFlexibleDirection( wxBOTH );
	fgSizer33->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer35;
	fgSizer35 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer35->SetFlexibleDirection( wxBOTH );
	fgSizer35->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer38;
	fgSizer38 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer38->SetFlexibleDirection( wxBOTH );
	fgSizer38->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkPD_PDET_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PDET_TRF, wxT("Power detector"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_PDET_TRF->SetToolTip( wxT("Powerdown signal for Power Detector") );
	
	fgSizer38->Add( chkPD_PDET_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_TLOBUF_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TLOBUF_TRF, wxT("TX LO buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_TLOBUF_TRF->SetToolTip( wxT("Powerdown signal for TX LO buffer") );
	
	fgSizer38->Add( chkPD_TLOBUF_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_TXPAD_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TXPAD_TRF, wxT("TXPAD"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_TXPAD_TRF->SetToolTip( wxT("Powerdown signal for TXPAD") );
	
	fgSizer38->Add( chkPD_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_TRF, wxT("Enable TRF module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_TRF->SetToolTip( wxT("Enable control for all the TRF_1 power downs") );
	
	fgSizer38->Add( chkEN_G_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizer20;
	sbSizer20 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
	
	chkEN_DIR_TRF = new wxCheckBox( sbSizer20->GetStaticBox(), ID_EN_DIR_TRF, wxT("Direct control of PDs and ENs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DIR_TRF->SetToolTip( wxT("Enables direct control of PDs and ENs for TRF module") );
	
	sbSizer20->Add( chkEN_DIR_TRF, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer38->Add( sbSizer20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizerPowerDowns->Add( fgSizer38, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer35->Add( sbSizerPowerDowns, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerPowerDetector;
	sbSizerPowerDetector = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power detector") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer36;
	fgSizer36 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer36->AddGrowableCol( 1 );
	fgSizer36->SetFlexibleDirection( wxBOTH );
	fgSizer36->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizerPowerDetector->GetStaticBox(), wxID_ANY, wxT("Resistive load"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer36->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOADR_PDET_TRF = new wxComboBox( sbSizerPowerDetector->GetStaticBox(), ID_LOADR_PDET_TRF, wxEmptyString, wxDefaultPosition, wxSize( 160,-1 ), 0, NULL, 0 );
	cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K||2.5K||1.25K") );
	cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K||1.25K") );
	cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K||2.5K") );
	cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K") );
	cmbLOADR_PDET_TRF->Append( wxEmptyString );
	cmbLOADR_PDET_TRF->SetToolTip( wxT("Controls the resistive load of the Power detector") );
	
	fgSizer36->Add( cmbLOADR_PDET_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerPowerDetector->Add( fgSizer36, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer35->Add( sbSizerPowerDetector, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerBiasCurrent;
	sbSizerBiasCurrent = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bias current") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer37;
	fgSizer37 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer37->AddGrowableCol( 1 );
	fgSizer37->SetFlexibleDirection( wxBOTH );
	fgSizer37->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizerBiasCurrent->GetStaticBox(), wxID_ANY, wxT("Linearization section"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer37->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LIN_TXPAD_TRF = new wxComboBox( sbSizerBiasCurrent->GetStaticBox(), ID_ICT_LIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LIN_TXPAD_TRF->SetToolTip( wxT("Control the bias current of the linearization section of the TXPAD") );
	
	fgSizer37->Add( cmbICT_LIN_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizerBiasCurrent->GetStaticBox(), wxID_ANY, wxT("Main gm section"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer37->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_MAIN_TXPAD_TRF = new wxComboBox( sbSizerBiasCurrent->GetStaticBox(), ID_ICT_MAIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_MAIN_TXPAD_TRF->SetToolTip( wxT("Control the bias current of the main gm section of the TXPAD") );
	
	fgSizer37->Add( cmbICT_MAIN_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerBiasCurrent->Add( fgSizer37, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer35->Add( sbSizerBiasCurrent, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerTrimDuty;
	sbSizerTrimDuty = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Trim duty cycle") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer39;
	fgSizer39 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer39->AddGrowableCol( 1 );
	fgSizer39->SetFlexibleDirection( wxBOTH );
	fgSizer39->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("I channel:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer39->Add( ID_STATICTEXT14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbCDC_I_TRF = new wxComboBox( sbSizerTrimDuty->GetStaticBox(), ID_CDC_I_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer39->Add( cmbCDC_I_TRF, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("Q channel:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( -1 );
	fgSizer39->Add( ID_STATICTEXT15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbCDC_Q_TRF = new wxComboBox( sbSizerTrimDuty->GetStaticBox(), ID_CDC_Q_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer39->Add( cmbCDC_Q_TRF, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	sbSizerTrimDuty->Add( fgSizer39, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	
	fgSizer35->Add( sbSizerTrimDuty, 0, wxEXPAND, 5 );
	
	wxString rgrGCAS_GNDREF_TXPAD_TRFChoices[] = { wxT("VDD"), wxT("GNDS") };
	int rgrGCAS_GNDREF_TXPAD_TRFNChoices = sizeof( rgrGCAS_GNDREF_TXPAD_TRFChoices ) / sizeof( wxString );
	rgrGCAS_GNDREF_TXPAD_TRF = new wxRadioBox( this, ID_GCAS_GNDREF_TXPAD_TRF, wxT("TXPAD cascode transistor gate bias"), wxDefaultPosition, wxDefaultSize, rgrGCAS_GNDREF_TXPAD_TRFNChoices, rgrGCAS_GNDREF_TXPAD_TRFChoices, 2, wxRA_SPECIFY_COLS );
	rgrGCAS_GNDREF_TXPAD_TRF->SetSelection( 1 );
	rgrGCAS_GNDREF_TXPAD_TRF->SetToolTip( wxT("Controls if the TXPAD cascode transistor gate bias is referred to VDD or GND") );
	
	fgSizer35->Add( rgrGCAS_GNDREF_TXPAD_TRF, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer33->Add( fgSizer35, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer34;
	fgSizer34 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer34->SetFlexibleDirection( wxBOTH );
	fgSizer34->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkEN_NEXTTX_TRF = new wxCheckBox( this, ID_EN_NEXTTX_TRF, wxT("Enable Tx MIMO mode"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer34->Add( chkEN_NEXTTX_TRF, 0, wxEXPAND, 5 );
	
	chkEN_LOOPB_TXPAD_TRF = new wxCheckBox( this, ID_EN_LOOPB_TXPAD_TRF, wxT("Enable TXPAD loopback path"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOOPB_TXPAD_TRF->SetToolTip( wxT("Enables the TXPAD loopback path") );
	
	fgSizer34->Add( chkEN_LOOPB_TXPAD_TRF, 0, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 0 );
	
	ID_STATICTEXT16 = new wxStaticText( this, wxID_ANY, wxT("TXFE output selection"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT16, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbTXFEoutput = new wxComboBox( this, ID_TXFEoutput, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbTXFEoutput->Append( wxT("Band1") );
	cmbTXFEoutput->Append( wxT("Band2") );
	cmbTXFEoutput->Append( wxT("Disable") );
	cmbTXFEoutput->SetSelection( 0 );
	cmbTXFEoutput->SetToolTip( wxT("Enable signal for TXFE, Band 1 or Band 2.\n0x0103 [11] SEL_BAND1_TRF\n0x0103 [10] SEL_BAND2_TRF") );
	
	fgSizer34->Add( cmbTXFEoutput, 0, wxEXPAND|wxALIGN_LEFT, 5 );
	
	ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("EN_LOWBWLOMX_TMX_TRF"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbEN_LOWBWLOMX_TMX_TRF = new wxComboBox( this, ID_EN_LOWBWLOMX_TMX_TRF, wxEmptyString, wxDefaultPosition, wxSize( 201,-1 ), 0, NULL, 0 );
	cmbEN_LOWBWLOMX_TMX_TRF->Append( wxT("High band - bias resistor 3K") );
	cmbEN_LOWBWLOMX_TMX_TRF->Append( wxT("Low band -bias resistor 30K") );
	cmbEN_LOWBWLOMX_TMX_TRF->SetToolTip( wxT("Controls the high pass pole frequency of the RC biasing the gate of the mixer switches") );
	
	fgSizer34->Add( cmbEN_LOWBWLOMX_TMX_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("TXPAD power detector preamplifier gain"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbEN_AMPHF_PDET_TRF = new wxComboBox( this, ID_EN_AMPHF_PDET_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbEN_AMPHF_PDET_TRF->SetToolTip( wxT("Enables the TXPAD power detector preamplifier") );
	
	fgSizer34->Add( cmbEN_AMPHF_PDET_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT4 = new wxStaticText( this, wxID_ANY, wxT("Switched capacitor at TXPAD output"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbF_TXPAD_TRF = new wxComboBox( this, ID_F_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbF_TXPAD_TRF->SetToolTip( wxT("Controls the switched capacitor at the TXPAD output. Is used for fine tuning of the TXPAD output") );
	
	fgSizer34->Add( cmbF_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT5 = new wxStaticText( this, wxID_ANY, wxT("Loss of the loopback path at the TX side"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbL_LOOPB_TXPAD_TRF = new wxComboBox( this, ID_L_LOOPB_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbL_LOOPB_TXPAD_TRF->Append( wxT("0 dB") );
	cmbL_LOOPB_TXPAD_TRF->Append( wxT("13.9 dB") );
	cmbL_LOOPB_TXPAD_TRF->Append( wxT("20.8 dB") );
	cmbL_LOOPB_TXPAD_TRF->Append( wxT("24 dB") );
	cmbL_LOOPB_TXPAD_TRF->SetToolTip( wxT("Controls the loss of the of the loopback path at the TX side") );
	
	fgSizer34->Add( cmbL_LOOPB_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT6 = new wxStaticText( this, wxID_ANY, wxT("TXPAD linearizing part gain"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOSS_LIN_TXPAD_TRF = new wxComboBox( this, ID_LOSS_LIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbLOSS_LIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain of the linearizing part of of the TXPAD") );
	
	fgSizer34->Add( cmbLOSS_LIN_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT7 = new wxStaticText( this, wxID_ANY, wxT("TXPAD gain control"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOSS_MAIN_TXPAD_TRF = new wxComboBox( this, ID_LOSS_MAIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbLOSS_MAIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain  output power of the TXPAD") );
	
	fgSizer34->Add( cmbLOSS_MAIN_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT8 = new wxStaticText( this, wxID_ANY, wxT("Bias voltage at gate of TXPAD cascade"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbVGCAS_TXPAD_TRF = new wxComboBox( this, ID_VGCAS_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbVGCAS_TXPAD_TRF->SetToolTip( wxT("Controls the bias voltage at the gate of TXPAD cascade") );
	
	fgSizer34->Add( cmbVGCAS_TXPAD_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT9 = new wxStaticText( this, wxID_ANY, wxT("Bias at gate of mixer NMOS"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOBIASN_TXM_TRF = new wxComboBox( this, ID_LOBIASN_TXM_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbLOBIASN_TXM_TRF->SetToolTip( wxT("Controls the bias at the gate of the mixer NMOS") );
	
	fgSizer34->Add( cmbLOBIASN_TXM_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT10 = new wxStaticText( this, wxID_ANY, wxT("Bias at gate of mixer PMOS"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer34->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOBIASP_TXX_TRF = new wxComboBox( this, ID_LOBIASP_TXX_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbLOBIASP_TXX_TRF->SetToolTip( wxT("Controls the bias at the gate of the mixer PMOS") );
	
	fgSizer34->Add( cmbLOBIASP_TXX_TRF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer33->Add( fgSizer34, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	this->SetSizer( fgSizer33 );
	this->Layout();
	fgSizer33->Fit( this );
	
	// Connect Events
	chkPD_PDET_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkPD_TLOBUF_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkPD_TXPAD_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOADR_PDET_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbICT_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_I_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_Q_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	rgrGCAS_GNDREF_TXPAD_TRF->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_NEXTTX_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOOPB_TXPAD_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbTXFEoutput->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::OnBandChange ), NULL, this );
	cmbEN_LOWBWLOMX_TMX_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbEN_AMPHF_PDET_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbF_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbL_LOOPB_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbVGCAS_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOBIASN_TXM_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOBIASP_TXX_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
}

pnlTRF_view::~pnlTRF_view()
{
	// Disconnect Events
	chkPD_PDET_TRF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkPD_TLOBUF_TRF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkPD_TXPAD_TRF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_TRF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_TRF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOADR_PDET_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbICT_MAIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_I_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbCDC_Q_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	rgrGCAS_GNDREF_TXPAD_TRF->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_NEXTTX_TRF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOOPB_TXPAD_TRF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbTXFEoutput->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::OnBandChange ), NULL, this );
	cmbEN_LOWBWLOMX_TMX_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbEN_AMPHF_PDET_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbF_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbL_LOOPB_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_LIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_MAIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbVGCAS_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOBIASN_TXM_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	cmbLOBIASP_TXX_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTRF_view::ParameterChangeHandler ), NULL, this );
	
}

pnlTBB_view::pnlTBB_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer57;
	fgSizer57 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer57->SetFlexibleDirection( wxBOTH );
	fgSizer57->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer59;
	fgSizer59 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer59->SetFlexibleDirection( wxBOTH );
	fgSizer59->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkPD_LPFH_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFH_TBB, wxT("LPFH_TBB biquad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LPFH_TBB->SetToolTip( wxT("This selectively powers down the LPFH_TBB biquad") );
	
	fgSizer59->Add( chkPD_LPFH_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_LPFIAMP_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFIAMP_TBB, wxT("LPFIAMP_TBB front-end current amp"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LPFIAMP_TBB->SetToolTip( wxT("selectively powers down the LPFIAMP_TBB front-end current amp of the transmitter baseband") );
	
	fgSizer59->Add( chkPD_LPFIAMP_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_LPFLAD_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFLAD_TBB, wxT("LPFLAD_TBB low pass ladder filter"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LPFLAD_TBB->SetToolTip( wxT("This selectively powers down the LPFLAD_TBB low pass ladder filter of the transmitter baseband") );
	
	fgSizer59->Add( chkPD_LPFLAD_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_LPFS5_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFS5_TBB, wxT("LPFS5_TBB low pass real-pole filter"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LPFS5_TBB->SetToolTip( wxT("This selectively powers down the LPFS5_TBB low pass real-pole filter of the transmitter baseband") );
	
	fgSizer59->Add( chkPD_LPFS5_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_TBB, wxT("Enable TBB module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_TBB->SetToolTip( wxT("Enable control for all the TBB_TOP power downs") );
	
	fgSizer59->Add( chkEN_G_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizer39;
	sbSizer39 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
	
	chkEN_DIR_TBB = new wxCheckBox( sbSizer39->GetStaticBox(), ID_EN_DIR_TBB, wxT("Direct control of PDs and ENs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DIR_TBB->SetToolTip( wxT("Enables direct control of PDs and ENs for TBB module") );
	
	sbSizer39->Add( chkEN_DIR_TBB, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer59->Add( sbSizer39, 1, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	
	sbSizerPowerDowns->Add( fgSizer59, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer57->Add( sbSizerPowerDowns, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxFlexGridSizer* fgSizer214;
	fgSizer214 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer214->AddGrowableCol( 0 );
	fgSizer214->SetFlexibleDirection( wxBOTH );
	fgSizer214->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer58;
	fgSizer58 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer58->AddGrowableCol( 1 );
	fgSizer58->SetFlexibleDirection( wxBOTH );
	fgSizer58->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkBYPLADDER_TBB = new wxCheckBox( this, ID_BYPLADDER_TBB, wxT("Bypass LPF ladder of TBB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBYPLADDER_TBB->SetToolTip( wxT("This signal bypasses the LPF ladder of TBB and directly connects the output of current amplifier to the null port of the real pole stage of TBB low pass filter") );
	
	fgSizer58->Add( chkBYPLADDER_TBB, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
	
	chkR5_LPF_BYP_TBB = new wxCheckBox( this, ID_BYPLADDER_TBB, wxT("Bypass LPFS5 filter capacitor banks"), wxDefaultPosition, wxDefaultSize, 0 );
	chkR5_LPF_BYP_TBB->SetToolTip( wxT("This signal bypasses the LPF ladder of TBB and directly connects the output of current amplifier to the null port of the real pole stage of TBB low pass filter") );
	
	fgSizer58->Add( chkR5_LPF_BYP_TBB, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 0 );
	
	
	fgSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
	
	ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("Tx BB loopback"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer58->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOOPB_TBB = new wxComboBox( this, ID_LOOPB_TBB, wxEmptyString, wxDefaultPosition, wxSize( 197,-1 ), 0, NULL, 0 ); 
	cmbLOOPB_TBB->SetToolTip( wxT("This controls which signal is connected to the loopback output pins. Note: when both the lowpass ladder and real pole are powered down, the output of the active highband biquad is routed to the loopb output") );
	
	fgSizer58->Add( cmbLOOPB_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT5 = new wxStaticText( this, wxID_ANY, wxT("Enable Tx IQ analog input"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer58->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbTSTIN_TBB = new wxComboBox( this, ID_TSTIN_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbTSTIN_TBB->SetToolTip( wxT("This control selects where the input test signal (vinp/n_aux_bbq/i) is routed to as well as disabling the route.") );
	
	fgSizer58->Add( cmbTSTIN_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer214->Add( fgSizer58, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer192;
	fgSizer192 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer192->AddGrowableCol( 1 );
	fgSizer192->SetFlexibleDirection( wxBOTH );
	fgSizer192->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("Frontend gain"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer192->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCG_IAMP_TBB = new NumericSlider( this, ID_CG_IAMP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
	fgSizer192->Add( cmbCG_IAMP_TBB, 0, wxEXPAND, 5 );
	
	
	fgSizer214->Add( fgSizer192, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer141;
	sbSizer141 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Reference bias current") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer260;
	fgSizer260 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer260->AddGrowableCol( 1 );
	fgSizer260->SetFlexibleDirection( wxBOTH );
	fgSizer260->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer141->GetStaticBox(), wxID_ANY, wxT("IAMP main bias current sources"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer260->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_IAMP_FRP_TBB = new NumericSlider( sbSizer141->GetStaticBox(), ID_ICT_IAMP_FRP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
	fgSizer260->Add( cmbICT_IAMP_FRP_TBB, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizer141->GetStaticBox(), wxID_ANY, wxT("IAMP cascade transistors gate voltage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer260->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_IAMP_GG_FRP_TBB = new NumericSlider( sbSizer141->GetStaticBox(), ID_ICT_IAMP_GG_FRP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
	fgSizer260->Add( cmbICT_IAMP_GG_FRP_TBB, 0, wxEXPAND, 5 );
	
	
	sbSizer141->Add( fgSizer260, 1, wxEXPAND, 5 );
	
	
	fgSizer214->Add( sbSizer141, 1, wxEXPAND, 5 );
	
	
	fgSizer57->Add( fgSizer214, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerOpamp;
	sbSizerOpamp = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Operational amplifier") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer60;
	fgSizer60 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer60->AddGrowableCol( 1 );
	fgSizer60->SetFlexibleDirection( wxBOTH );
	fgSizer60->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Output stage bias current low band real pole filter"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgSizer60->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPFS5_F_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFS5_F_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPFS5_F_TBB->SetToolTip( wxT("This controls the operational amplifier's output stage bias current of the low band real pole filter of the transmitter's baseband") );
	
	fgSizer60->Add( cmbICT_LPFS5_F_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Input stage bias current of low band real pole filter"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -14 );
	fgSizer60->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPFS5_PT_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFS5_PT_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPFS5_PT_TBB->SetToolTip( wxT("This controls the operational amplifier's input stage bias current of the low band real pole filter of the transmitter's baseband") );
	
	fgSizer60->Add( cmbICT_LPFS5_PT_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT8 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Input stage bias reference current of high band low pass filter"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer60->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPF_H_PT_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPF_H_PT_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPF_H_PT_TBB->SetToolTip( wxT("This controls the operational amplifiers input stage bias reference current of the high band low pass filter of the transmitter's baseband ") );
	
	fgSizer60->Add( cmbICT_LPF_H_PT_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Output stage bias reference current of high band low pass filter"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer60->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPFH_F_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFH_F_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPFH_F_TBB->SetToolTip( wxT("controls the operational amplifiers output stage bias reference current of the high band low pass filter of the transmitter's baseband (LPFH_TBB)") );
	
	fgSizer60->Add( cmbICT_LPFH_F_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Output stage bias reference of low band ladder filter"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer60->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPFLAD_F_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFLAD_F_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPFLAD_F_TBB->SetToolTip( wxT("This controls the operational amplfiers' output stages bias referene current of the low band ladder filter of the transmisster's baseband") );
	
	fgSizer60->Add( cmbICT_LPFLAD_F_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Input stage bias reference of low band ladder filter"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer60->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_LPFLAD_PT_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFLAD_PT_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbICT_LPFLAD_PT_TBB->SetToolTip( wxT("This controls the operational amplifers' input stages bias reference current of the low band ladder filter of the transmitter's baseband") );
	
	fgSizer60->Add( cmbICT_LPFLAD_PT_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerOpamp->Add( fgSizer60, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer57->Add( sbSizerOpamp, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizerResistorBanks;
	sbSizerResistorBanks = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TxLPF resistor banks") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer61;
	fgSizer61 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer61->AddGrowableCol( 1 );
	fgSizer61->SetFlexibleDirection( wxBOTH );
	fgSizer61->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("LPFH equivalent resistance stage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer61->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRCAL_LPFH_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_RCAL_LPFH_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer61->Add( cmbRCAL_LPFH_TBB, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("LPFLAD equivalent resistance stage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer61->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRCAL_LPFLAD_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_RCAL_LPFLAD_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer61->Add( cmbRCAL_LPFLAD_TBB, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("LPFS5 equivalent resistance stage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( -1 );
	fgSizer61->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRCAL_LPFS5_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_RCAL_LPFS5_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer61->Add( cmbRCAL_LPFS5_TBB, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("Common control signal for all TBB filters"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer61->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCCAL_LPFLAD_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_CCAL_LPFLAD_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
	fgSizer61->Add( cmbCCAL_LPFLAD_TBB, 0, wxEXPAND, 5 );
	
	
	sbSizerResistorBanks->Add( fgSizer61, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer57->Add( sbSizerResistorBanks, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizerRxFilters;
	sbSizerRxFilters = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Tx Filters") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer245;
	fgSizer245 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer245->SetFlexibleDirection( wxBOTH );
	fgSizer245->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblFilterInputName = new wxStaticText( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("RF bandwidth (MHz)"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFilterInputName->Wrap( -1 );
	fgSizer245->Add( lblFilterInputName, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFilterFrequency = new wxTextCtrl( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("56"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFilterFrequency->HasFlag( wxTE_MULTILINE ) )
	{
	txtFilterFrequency->SetMaxLength( 10 );
	}
	#else
	txtFilterFrequency->SetMaxLength( 10 );
	#endif
	fgSizer245->Add( txtFilterFrequency, 0, 0, 5 );
	
	btnTuneFilter = new wxButton( sbSizerRxFilters->GetStaticBox(), ID_BTN_TUNE_FILTER, wxT("TUNE"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer245->Add( btnTuneFilter, 0, wxEXPAND, 5 );
	
	btnTuneTxGain = new wxButton( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("Tune Gain"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer245->Add( btnTuneTxGain, 0, 0, 5 );
	
	
	sbSizerRxFilters->Add( fgSizer245, 1, wxEXPAND, 5 );
	
	
	fgSizer57->Add( sbSizerRxFilters, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer57 );
	this->Layout();
	fgSizer57->Fit( this );
	
	// Connect Events
	chkPD_LPFH_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFIAMP_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFLAD_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFS5_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkBYPLADDER_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkR5_LPF_BYP_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbLOOPB_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbTSTIN_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbCG_IAMP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_IAMP_FRP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_IAMP_GG_FRP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFS5_F_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFS5_PT_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPF_H_PT_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFH_F_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFLAD_F_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFLAD_PT_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCAL_LPFH_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCAL_LPFLAD_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCAL_LPFS5_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbCCAL_LPFLAD_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	btnTuneFilter->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTBB_view::OnbtnTuneFilter ), NULL, this );
	btnTuneTxGain->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTBB_view::OnbtnTuneTxGain ), NULL, this );
}

pnlTBB_view::~pnlTBB_view()
{
	// Disconnect Events
	chkPD_LPFH_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFIAMP_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFLAD_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkPD_LPFS5_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkBYPLADDER_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	chkR5_LPF_BYP_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbLOOPB_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbTSTIN_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbCG_IAMP_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_IAMP_FRP_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_IAMP_GG_FRP_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFS5_F_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFS5_PT_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPF_H_PT_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFH_F_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFLAD_F_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbICT_LPFLAD_PT_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCAL_LPFH_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCAL_LPFLAD_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbRCAL_LPFS5_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	cmbCCAL_LPFLAD_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTBB_view::ParameterChangeHandler ), NULL, this );
	btnTuneFilter->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTBB_view::OnbtnTuneFilter ), NULL, this );
	btnTuneTxGain->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTBB_view::OnbtnTuneTxGain ), NULL, this );
	
}

pnlAFE_view::pnlAFE_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer62;
	fgSizer62 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer62->SetFlexibleDirection( wxBOTH );
	fgSizer62->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer64;
	fgSizer64 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer64->SetFlexibleDirection( wxBOTH );
	fgSizer64->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkPD_AFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_AFE, wxT("AFE current mirror in BIAS_TOP"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_AFE->SetValue(true); 
	chkPD_AFE->SetToolTip( wxT("Power down control for the AFE current mirror in BIAS_TOP") );
	
	fgSizer64->Add( chkPD_AFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_RX_AFE1 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_RX_AFE1, wxT("ADC ch. 1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_RX_AFE1->SetToolTip( wxT("Power down control for the ADC of  channel 1") );
	
	fgSizer64->Add( chkPD_RX_AFE1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_RX_AFE2 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_RX_AFE2, wxT("ADC ch. 2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_RX_AFE2->SetValue(true); 
	chkPD_RX_AFE2->SetToolTip( wxT("Power down control for the ADC of channel 2") );
	
	fgSizer64->Add( chkPD_RX_AFE2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_TX_AFE1 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TX_AFE1, wxT("DAC ch. 1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_TX_AFE1->SetValue(true); 
	chkPD_TX_AFE1->SetToolTip( wxT("Power down control for the DAC of channel 1") );
	
	fgSizer64->Add( chkPD_TX_AFE1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_TX_AFE2 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TX_AFE2, wxT("DAC ch. 2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_TX_AFE2->SetValue(true); 
	chkPD_TX_AFE2->SetToolTip( wxT("Power down control for the DAC of channel 2") );
	
	fgSizer64->Add( chkPD_TX_AFE2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G_AFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_AFE, wxT("Enable AFE module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_AFE->SetToolTip( wxT("Enable control for all the AFE power downs") );
	
	fgSizer64->Add( chkEN_G_AFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerPowerDowns->Add( fgSizer64, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer62->Add( sbSizerPowerDowns, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxFlexGridSizer* fgSizer63;
	fgSizer63 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer63->SetFlexibleDirection( wxBOTH );
	fgSizer63->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("Peak current of DAC"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer63->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbISEL_DAC_AFE = new wxComboBox( this, ID_ISEL_DAC_AFE, wxEmptyString, wxDefaultPosition, wxSize( 162,-1 ), 0, NULL, 0 ); 
	cmbISEL_DAC_AFE->SetToolTip( wxT("Controls the peak current of the DAC output current") );
	
	fgSizer63->Add( cmbISEL_DAC_AFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("MUX input of ADC ch. 1"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer63->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbMUX_AFE_1 = new wxComboBox( this, ID_MUX_AFE_1, wxEmptyString, wxDefaultPosition, wxSize( 315,-1 ), 0, NULL, 0 ); 
	cmbMUX_AFE_1->SetToolTip( wxT("Controls the MUX at the input of the ADC channel 1") );
	
	fgSizer63->Add( cmbMUX_AFE_1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("MUX input of ADC ch. 2"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer63->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbMUX_AFE_2 = new wxComboBox( this, ID_MUX_AFE_2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbMUX_AFE_2->SetToolTip( wxT("Controls the MUX at the input of the ADC channel 2") );
	
	fgSizer63->Add( cmbMUX_AFE_2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer63->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString rgrMODE_INTERLEAVE_AFEChoices[] = { wxT("Two ADCs"), wxT("Interleaved") };
	int rgrMODE_INTERLEAVE_AFENChoices = sizeof( rgrMODE_INTERLEAVE_AFEChoices ) / sizeof( wxString );
	rgrMODE_INTERLEAVE_AFE = new wxRadioBox( this, ID_MODE_INTERLEAVE_AFE, wxT("Time interleave two ADCs into one ADC"), wxDefaultPosition, wxDefaultSize, rgrMODE_INTERLEAVE_AFENChoices, rgrMODE_INTERLEAVE_AFEChoices, 2, wxRA_SPECIFY_COLS );
	rgrMODE_INTERLEAVE_AFE->SetSelection( 1 );
	rgrMODE_INTERLEAVE_AFE->SetToolTip( wxT("time interleaves the two ADCs into one ADC") );
	
	fgSizer63->Add( rgrMODE_INTERLEAVE_AFE, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer62->Add( fgSizer63, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	this->SetSizer( fgSizer62 );
	this->Layout();
	fgSizer62->Fit( this );
	
	// Connect Events
	chkPD_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RX_AFE1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RX_AFE2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_TX_AFE1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_TX_AFE2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	cmbISEL_DAC_AFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	cmbMUX_AFE_1->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	cmbMUX_AFE_2->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	rgrMODE_INTERLEAVE_AFE->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
}

pnlAFE_view::~pnlAFE_view()
{
	// Disconnect Events
	chkPD_AFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RX_AFE1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_RX_AFE2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_TX_AFE1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkPD_TX_AFE2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_AFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	cmbISEL_DAC_AFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	cmbMUX_AFE_1->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	cmbMUX_AFE_2->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	rgrMODE_INTERLEAVE_AFE->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlAFE_view::ParameterChangeHandler ), NULL, this );
	
}

pnlBIAS_view::pnlBIAS_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer65;
	fgSizer65 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer65->SetFlexibleDirection( wxBOTH );
	fgSizer65->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer66;
	fgSizer66 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer66->SetFlexibleDirection( wxBOTH );
	fgSizer66->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkPD_FRP_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_FRP_BIAS, wxT("Fix/RP block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_FRP_BIAS->SetToolTip( wxT("Power down signal for Fix/RP block") );
	
	fgSizer66->Add( chkPD_FRP_BIAS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_F_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_F_BIAS, wxT("Fix"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_F_BIAS->SetToolTip( wxT("Power down signal for Fix") );
	
	fgSizer66->Add( chkPD_F_BIAS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_PTRP_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PTRP_BIAS, wxT("PTAT/RP block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_PTRP_BIAS->SetToolTip( wxT("Power down signal for PTAT/RP block") );
	
	fgSizer66->Add( chkPD_PTRP_BIAS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_PT_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PT_BIAS, wxT("PTAT block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_PT_BIAS->SetToolTip( wxT("Power down signal for PTAT block") );
	
	fgSizer66->Add( chkPD_PT_BIAS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_BIAS_MASTER = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_BIAS_MASTER, wxT("Power down all block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_BIAS_MASTER->SetToolTip( wxT("Enable signal for central bias block") );
	
	fgSizer66->Add( chkPD_BIAS_MASTER, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerPowerDowns->Add( fgSizer66, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer65->Add( sbSizerPowerDowns, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxFlexGridSizer* fgSizer67;
	fgSizer67 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer67->SetFlexibleDirection( wxBOTH );
	fgSizer67->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("BIAS_TOP test mode"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer67->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbMUX_BIAS_OUT = new wxComboBox( this, ID_MUX_BIAS_OUT, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	cmbMUX_BIAS_OUT->SetToolTip( wxT("Test mode of the BIAS_TOP") );
	
	fgSizer67->Add( cmbMUX_BIAS_OUT, 0, wxEXPAND|wxALIGN_LEFT, 0 );
	
	ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("RP_CALIB_BIAS"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer67->Add( ID_STATICTEXT2, 1, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 5 );
	
	cmbRP_CALIB_BIAS = new wxComboBox( this, ID_RP_CALIB_BIAS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRP_CALIB_BIAS->SetToolTip( wxT("Calibration code for rppolywo. This code is set by the calibration algorithm: BIAS_RPPOLY_calibration") );
	
	fgSizer67->Add( cmbRP_CALIB_BIAS, 0, wxEXPAND|wxALIGN_LEFT, 5 );
	
	btnCalibrateRP_BIAS = new wxButton( this, wxID_ANY, wxT("Calibrate RP_BIAS"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer67->Add( btnCalibrateRP_BIAS, 0, 0, 5 );
	
	
	fgSizer65->Add( fgSizer67, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	this->SetSizer( fgSizer65 );
	this->Layout();
	fgSizer65->Fit( this );
	
	// Connect Events
	chkPD_FRP_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_F_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_PTRP_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_PT_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_BIAS_MASTER->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	cmbMUX_BIAS_OUT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	cmbRP_CALIB_BIAS->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	btnCalibrateRP_BIAS->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBIAS_view::OnCalibrateRP_BIAS ), NULL, this );
}

pnlBIAS_view::~pnlBIAS_view()
{
	// Disconnect Events
	chkPD_FRP_BIAS->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_F_BIAS->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_PTRP_BIAS->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_PT_BIAS->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	chkPD_BIAS_MASTER->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	cmbMUX_BIAS_OUT->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	cmbRP_CALIB_BIAS->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlBIAS_view::ParameterChangeHandler ), NULL, this );
	btnCalibrateRP_BIAS->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBIAS_view::OnCalibrateRP_BIAS ), NULL, this );
	
}

pnlLDO_view::pnlLDO_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer68;
	fgSizer68 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer68->SetFlexibleDirection( wxBOTH );
	fgSizer68->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_NOTEBOOK1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	ID_PANEL3 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer69;
	fgSizer69 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer69->SetFlexibleDirection( wxBOTH );
	fgSizer69->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer46;
	sbSizer46 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL3, wxID_ANY, wxT("Power controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer70;
	fgSizer70 = new wxFlexGridSizer( 0, 7, 2, 5 );
	fgSizer70->SetFlexibleDirection( wxBOTH );
	fgSizer70->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkEN_G_LDOP = new wxCheckBox( sbSizer46->GetStaticBox(), ID_EN_G_LDOP, wxT("Enable LDO digital module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_LDOP->SetToolTip( wxT("Enable control for all the LDO power downs") );
	
	fgSizer70->Add( chkEN_G_LDOP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G_LDO = new wxCheckBox( sbSizer46->GetStaticBox(), ID_EN_G_LDO, wxT("Enable LDO analog module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_LDO->SetToolTip( wxT("Enable control for all the LDO power downs") );
	
	fgSizer70->Add( chkEN_G_LDO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer47;
	sbSizer47 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("CLKGEN") ), wxVERTICAL );
	
	chkEN_LDO_DIVGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_DIVGN, wxT("EN_LDO_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_DIVGN->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer47->Add( chkEN_LDO_DIVGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_DIGGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_DIGGN, wxT("EN_LDO_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_DIGGN->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer47->Add( chkEN_LDO_DIGGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_CPGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_CPGN, wxT("EN_LDO_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_CPGN->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer47->Add( chkEN_LDO_CPGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_VCOGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_VCOGN, wxT("EN_LDO_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_VCOGN->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer47->Add( chkEN_LDO_VCOGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer47, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer48;
	sbSizer48 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("RX RF") ), wxVERTICAL );
	
	chkEN_LDO_MXRFE = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_MXRFE, wxT("EN_LDO_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_MXRFE->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer48->Add( chkEN_LDO_MXRFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_LNA14 = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_LNA14, wxT("EN_LDO_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_LNA14->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer48->Add( chkEN_LDO_LNA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_LNA12 = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_LNA12, wxT("EN_LDO_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_LNA12->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer48->Add( chkEN_LDO_LNA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_TIA14 = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_TIA14, wxT("EN_LDO_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_TIA14->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer48->Add( chkEN_LDO_TIA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer48, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer49;
	sbSizer49 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("SX TX") ), wxVERTICAL );
	
	chkEN_LDO_DIVSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_DIVSXT, wxT("EN_LDO_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_DIVSXT->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer49->Add( chkEN_LDO_DIVSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_DIGSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_DIGSXT, wxT("EN_LDO_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_DIGSXT->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer49->Add( chkEN_LDO_DIGSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_CPSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_CPSXT, wxT("EN_LDO_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_CPSXT->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer49->Add( chkEN_LDO_CPSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_VCOSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_VCOSXT, wxT("EN_LDO_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_VCOSXT->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer49->Add( chkEN_LDO_VCOSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer49, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer50;
	sbSizer50 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("SX RX") ), wxVERTICAL );
	
	chkEN_LDO_DIVSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_DIVSXR, wxT("EN_LDO_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_DIVSXR->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer50->Add( chkEN_LDO_DIVSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_DIGSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_DIGSXR, wxT("EN_LDO_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_DIGSXR->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer50->Add( chkEN_LDO_DIGSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_CPSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_CPSXR, wxT("EN_LDO_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_CPSXR->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer50->Add( chkEN_LDO_CPSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_VCOSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_VCOSXR, wxT("EN_LDO_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_VCOSXR->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer50->Add( chkEN_LDO_VCOSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer50, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer51;
	sbSizer51 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Digital Blocks") ), wxVERTICAL );
	
	chkPD_LDO_DIGIp1 = new wxCheckBox( sbSizer51->GetStaticBox(), ID_PD_LDO_DIGIp1, wxT("PD_LDO_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LDO_DIGIp1->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer51->Add( chkPD_LDO_DIGIp1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_LDO_DIGIp2 = new wxCheckBox( sbSizer51->GetStaticBox(), ID_PD_LDO_DIGIp2, wxT("PD_LDO_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LDO_DIGIp2->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer51->Add( chkPD_LDO_DIGIp2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer51, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer52;
	sbSizer52 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("XBUF") ), wxVERTICAL );
	
	chkEN_LDO_TXBUF = new wxCheckBox( sbSizer52->GetStaticBox(), ID_EN_LDO_TXBUF, wxT("EN_LDO_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_TXBUF->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer52->Add( chkEN_LDO_TXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_RXBUF = new wxCheckBox( sbSizer52->GetStaticBox(), ID_EN_LDO_RXBUF, wxT("EN_LDO_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_RXBUF->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer52->Add( chkEN_LDO_RXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer52, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer58;
	sbSizer58 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Tx RF") ), wxVERTICAL );
	
	chkEN_LDO_TPAD = new wxCheckBox( sbSizer58->GetStaticBox(), ID_EN_LDO_TPAD, wxT("EN_LDO_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_TPAD->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer58->Add( chkEN_LDO_TPAD, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LDO_TLOB = new wxCheckBox( sbSizer58->GetStaticBox(), ID_EN_LDO_TLOB, wxT("EN_LDO_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_TLOB->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer58->Add( chkEN_LDO_TLOB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer58, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer54;
	sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("SPI Buffer to analog blocks") ), wxVERTICAL );
	
	chkPD_LDO_SPIBUF = new wxCheckBox( sbSizer54->GetStaticBox(), ID_PD_LDO_SPIBUF, wxT("PD_LDO_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LDO_SPIBUF->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer54->Add( chkPD_LDO_SPIBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxFlexGridSizer* fgSizer251;
	fgSizer251 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer251->SetFlexibleDirection( wxBOTH );
	fgSizer251->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText349 = new wxStaticText( sbSizer54->GetStaticBox(), wxID_ANY, wxT("ISINK_SPI_BUFF"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText349->Wrap( -1 );
	fgSizer251->Add( m_staticText349, 0, wxALL, 5 );
	
	cmbISINK_SPIBUFF = new wxComboBox( sbSizer54->GetStaticBox(), ID_RDIV_TXBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbISINK_SPIBUFF->Append( wxT("Off") );
	cmbISINK_SPIBUFF->Append( wxT("10 kOhm") );
	cmbISINK_SPIBUFF->Append( wxT("2.5 kOhm") );
	cmbISINK_SPIBUFF->Append( wxT("2 kOhm") );
	cmbISINK_SPIBUFF->Append( wxT("625 Ohm") );
	cmbISINK_SPIBUFF->Append( wxT("588 Ohm") );
	cmbISINK_SPIBUFF->Append( wxT("500 Ohm") );
	cmbISINK_SPIBUFF->Append( wxT("476 Ohm") );
	cmbISINK_SPIBUFF->Append( wxT("10 kOhm") );
	fgSizer251->Add( cmbISINK_SPIBUFF, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 0 );
	
	
	sbSizer54->Add( fgSizer251, 1, wxEXPAND, 5 );
	
	
	fgSizer70->Add( sbSizer54, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer53;
	sbSizer53 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("AFE") ), wxVERTICAL );
	
	chkEN_LDO_AFE = new wxCheckBox( sbSizer53->GetStaticBox(), ID_EN_LDO_AFE, wxT("EN_LDO_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_AFE->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer53->Add( chkEN_LDO_AFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer53, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizer55;
	sbSizer55 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Tx BB") ), wxVERTICAL );
	
	chkEN_LDO_TBB = new wxCheckBox( sbSizer55->GetStaticBox(), ID_EN_LDO_TBB, wxT("EN_LDO_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_TBB->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer55->Add( chkEN_LDO_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer55, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer56;
	sbSizer56 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Rx RBB") ), wxVERTICAL );
	
	chkEN_LDO_RBB = new wxCheckBox( sbSizer56->GetStaticBox(), ID_EN_LDO_RBB, wxT("EN_LDO_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_RBB->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer56->Add( chkEN_LDO_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer56, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer57;
	sbSizer57 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Rx RF+ Rx RBB") ), wxVERTICAL );
	
	chkEN_LDO_TIA12 = new wxCheckBox( sbSizer57->GetStaticBox(), ID_EN_LDO_TIA12, wxT("EN_LDO_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_TIA12->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer57->Add( chkEN_LDO_TIA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer57, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer59;
	sbSizer59 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Misc") ), wxVERTICAL );
	
	chkEN_LDO_DIG = new wxCheckBox( sbSizer59->GetStaticBox(), ID_EN_LDO_DIG, wxT("EN_LDO_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LDO_DIG->SetToolTip( wxT("Enables the LDO") );
	
	sbSizer59->Add( chkEN_LDO_DIG, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer70->Add( sbSizer59, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer46->Add( fgSizer70, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0 );
	
	
	fgSizer69->Add( sbSizer46, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer255;
	fgSizer255 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer255->SetFlexibleDirection( wxBOTH );
	fgSizer255->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer61;
	sbSizer61 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL3, wxID_ANY, wxT("Short noise filter resistor") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer72;
	fgSizer72 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer72->SetFlexibleDirection( wxBOTH );
	fgSizer72->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkSPDUP_LDO_TBB = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TBB, wxT("SPDUP_LDO_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_TBB->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_TIA12 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TIA12, wxT("SPDUP_LDO_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_TIA12->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_TIA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_TIA14 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TIA14, wxT("SPDUP_LDO_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_TIA14->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_TIA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_TLOB = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TLOB, wxT("SPDUP_LDO_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_TLOB->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_TLOB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_TPAD = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TPAD, wxT("SPDUP_LDO_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_TPAD->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_TPAD, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_TXBUF = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TXBUF, wxT("SPDUP_LDO_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_TXBUF->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_TXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_VCOGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_VCOGN, wxT("SPDUP_LDO_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_VCOGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_VCOGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIVSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIVSXR, wxT("SPDUP_LDO_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIVSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIVSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIVSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIVSXT, wxT("SPDUP_LDO_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIVSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIVSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_AFE = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_AFE, wxT("SPDUP_LDO_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_AFE->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_AFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_CPGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_CPGN, wxT("SPDUP_LDO_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_CPGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_CPGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_VCOSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_VCOSXR, wxT("SPDUP_LDO_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_VCOSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_VCOSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_VCOSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_VCOSXT, wxT("SPDUP_LDO_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_VCOSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_VCOSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIG = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIG, wxT("SPDUP_LDO_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIG->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIG, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIGGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGGN, wxT("SPDUP_LDO_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIGGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIGGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIGSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGSXR, wxT("SPDUP_LDO_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIGSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIGSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIGSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGSXT, wxT("SPDUP_LDO_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIGSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIGSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIVGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIVGN, wxT("SPDUP_LDO_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIVGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIVGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_CPSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_CPSXR, wxT("SPDUP_LDO_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_CPSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_CPSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_CPSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_CPSXT, wxT("SPDUP_LDO_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_CPSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_CPSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_LNA12 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_LNA12, wxT("SPDUP_LDO_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_LNA12->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_LNA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_LNA14 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_LNA14, wxT("SPDUP_LDO_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_LNA14->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_LNA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_MXRFE = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_MXRFE, wxT("SPDUP_LDO_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_MXRFE->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_MXRFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_RBB = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_RBB, wxT("SPDUP_LDO_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_RBB->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_RXBUF = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_RXBUF, wxT("SPDUP_LDO_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_RXBUF->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_RXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_SPIBUF = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_SPIBUF, wxT("SPDUP_LDO_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_SPIBUF->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_SPIBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIGIp2 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGIp2, wxT("SPDUP_LDO_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIGIp2->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIGIp2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_LDO_DIGIp1 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGIp1, wxT("SPDUP_LDO_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_LDO_DIGIp1->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
	
	fgSizer72->Add( chkSPDUP_LDO_DIGIp1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer61->Add( fgSizer72, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer255->Add( sbSizer61, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer62;
	sbSizer62 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL3, wxID_ANY, wxT("Bias") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer73;
	fgSizer73 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer73->SetFlexibleDirection( wxBOTH );
	fgSizer73->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkEN_LOADIMP_LDO_TBB = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TBB, wxT("EN_LOADIMP_LDO_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_TBB->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_TIA12 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TIA12, wxT("EN_LOADIMP_LDO_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_TIA12->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_TIA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_TIA14 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TIA14, wxT("EN_LOADIMP_LDO_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_TIA14->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_TIA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_TLOB = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TLOB, wxT("EN_LOADIMP_LDO_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_TLOB->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_TLOB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_TPAD = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TPAD, wxT("EN_LOADIMP_LDO_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_TPAD->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_TPAD, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_TXBUF = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TXBUF, wxT("EN_LOADIMP_LDO_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_TXBUF->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_TXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_VCOGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_VCOGN, wxT("EN_LOADIMP_LDO_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_VCOGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_VCOGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_VCOSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_VCOSXR, wxT("EN_LOADIMP_LDO_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_VCOSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_VCOSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_VCOSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_VCOSXT, wxT("EN_LOADIMP_LDO_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_VCOSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_VCOSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_AFE = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_AFE, wxT("EN_LOADIMP_LDO_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_AFE->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_AFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_CPGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_CPGN, wxT("EN_LOADIMP_LDO_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_CPGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_CPGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIVSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIVSXR, wxT("EN_LOADIMP_LDO_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIVSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIVSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIVSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIVSXT, wxT("EN_LOADIMP_LDO_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIVSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIVSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIG = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIG, wxT("EN_LOADIMP_LDO_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIG->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIG, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIGGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGGN, wxT("EN_LOADIMP_LDO_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIGGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIGGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIGSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGSXR, wxT("EN_LOADIMP_LDO_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIGSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIGSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIGSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGSXT, wxT("EN_LOADIMP_LDO_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIGSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIGSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIVGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIVGN, wxT("EN_LOADIMP_LDO_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIVGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIVGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_CPSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_CPSXR, wxT("EN_LOADIMP_LDO_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_CPSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_CPSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_CPSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_CPSXT, wxT("EN_LOADIMP_LDO_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_CPSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_CPSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_LNA12 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_LNA12, wxT("EN_LOADIMP_LDO_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_LNA12->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_LNA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_LNA14 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_LNA14, wxT("EN_LOADIMP_LDO_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_LNA14->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_LNA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_MXRFE = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_MXRFE, wxT("EN_LOADIMP_LDO_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_MXRFE->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_MXRFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_RBB = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_RBB, wxT("EN_LOADIMP_LDO_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_RBB->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_RXBUF = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_RXBUF, wxT("EN_LOADIMP_LDO_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_RXBUF->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_RXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_SPIBUF = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_SPIBUF, wxT("EN_LOADIMP_LDO_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_SPIBUF->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_SPIBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIGIp2 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGIp2, wxT("EN_LOADIMP_LDO_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIGIp2->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIGIp2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_LOADIMP_LDO_DIGIp1 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGIp1, wxT("EN_LOADIMP_LDO_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_LOADIMP_LDO_DIGIp1->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
	
	fgSizer73->Add( chkEN_LOADIMP_LDO_DIGIp1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer62->Add( fgSizer73, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer255->Add( sbSizer62, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer69->Add( fgSizer255, 1, wxEXPAND, 5 );
	
	
	ID_PANEL3->SetSizer( fgSizer69 );
	ID_PANEL3->Layout();
	fgSizer69->Fit( ID_PANEL3 );
	ID_NOTEBOOK1->AddPage( ID_PANEL3, wxT("Bias && Noise filter"), true );
	ID_PANEL2 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer75;
	fgSizer75 = new wxFlexGridSizer( 0, 4, 0, 5 );
	fgSizer75->SetFlexibleDirection( wxBOTH );
	fgSizer75->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_VCOSXR = new wxComboBox( ID_PANEL2, ID_RDIV_VCOSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_VCOSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_VCOSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT2 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_VCOSXT = new wxComboBox( ID_PANEL2, ID_RDIV_VCOSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_VCOSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_VCOSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_TXBUF = new wxComboBox( ID_PANEL2, ID_RDIV_TXBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_TXBUF->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_TXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT4 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_VCOGN = new wxComboBox( ID_PANEL2, ID_RDIV_VCOGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_VCOGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_VCOGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT5 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_TLOB = new wxComboBox( ID_PANEL2, ID_RDIV_TLOB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_TLOB->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_TLOB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT6 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_TPAD = new wxComboBox( ID_PANEL2, ID_RDIV_TPAD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_TPAD->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_TPAD, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT7 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_TIA12 = new wxComboBox( ID_PANEL2, ID_RDIV_TIA12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_TIA12->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_TIA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT8 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_TIA14 = new wxComboBox( ID_PANEL2, ID_RDIV_TIA14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_TIA14->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_TIA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT9 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_RXBUF = new wxComboBox( ID_PANEL2, ID_RDIV_RXBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_RXBUF->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_RXBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT10 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_TBB = new wxComboBox( ID_PANEL2, ID_RDIV_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_TBB->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_TBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT11 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_MXRFE = new wxComboBox( ID_PANEL2, ID_RDIV_MXRFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_MXRFE->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_MXRFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT12 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_RBB = new wxComboBox( ID_PANEL2, ID_RDIV_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_RBB->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_RBB, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT13 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_LNA12 = new wxComboBox( ID_PANEL2, ID_RDIV_LNA12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_LNA12->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_LNA12, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT14 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_LNA14 = new wxComboBox( ID_PANEL2, ID_RDIV_LNA14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_LNA14->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_LNA14, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT15 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIVSXR = new wxComboBox( ID_PANEL2, ID_RDIV_DIVSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIVSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIVSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT16 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIVSXT = new wxComboBox( ID_PANEL2, ID_RDIV_DIVSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIVSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIVSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT17 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT17->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIGSXT = new wxComboBox( ID_PANEL2, ID_RDIV_DIGSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIGSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIGSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT18 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT18->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIVGN = new wxComboBox( ID_PANEL2, ID_RDIV_DIVGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIVGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIVGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT19 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT19->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT19, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIGGN = new wxComboBox( ID_PANEL2, ID_RDIV_DIGGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIGGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIGGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT20 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT20->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIGSXR = new wxComboBox( ID_PANEL2, ID_RDIV_DIGSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIGSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIGSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT21 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT21->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_CPSXT = new wxComboBox( ID_PANEL2, ID_RDIV_CPSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_CPSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_CPSXT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT22 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT22->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT22, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIG = new wxComboBox( ID_PANEL2, ID_RDIV_DIG, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIG->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIG, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT23 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT23->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT23, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_CPGN = new wxComboBox( ID_PANEL2, ID_RDIV_CPGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_CPGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_CPGN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT24 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT24->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT24, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_CPSXR = new wxComboBox( ID_PANEL2, ID_RDIV_CPSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_CPSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_CPSXR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT25 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT25->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_SPIBUF = new wxComboBox( ID_PANEL2, ID_RDIV_SPIBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_SPIBUF->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_SPIBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT26 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT26->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_AFE = new wxComboBox( ID_PANEL2, ID_RDIV_AFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer75->Add( cmbRDIV_AFE, 0, 0, 5 );
	
	ID_STATICTEXT27 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT27->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIGIp2 = new wxComboBox( ID_PANEL2, ID_RDIV_DIGIp2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIGIp2->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIGIp2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT28 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT28->Wrap( 0 );
	fgSizer75->Add( ID_STATICTEXT28, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRDIV_DIGIp1 = new wxComboBox( ID_PANEL2, ID_RDIV_DIGIp1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRDIV_DIGIp1->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
	
	fgSizer75->Add( cmbRDIV_DIGIp1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	ID_PANEL2->SetSizer( fgSizer75 );
	ID_PANEL2->Layout();
	fgSizer75->Fit( ID_PANEL2 );
	ID_NOTEBOOK1->AddPage( ID_PANEL2, wxT("Output Voltage"), false );
	
	fgSizer68->Add( ID_NOTEBOOK1, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( fgSizer68 );
	this->Layout();
	
	// Connect Events
	chkEN_G_LDOP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_LDO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIVGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIGGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_CPGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_VCOGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_MXRFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_LNA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_LNA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TIA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIVSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIGSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_CPSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_VCOSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIVSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIGSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_CPSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_VCOSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkPD_LDO_DIGIp1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkPD_LDO_DIGIp2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_RXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TPAD->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TLOB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkPD_LDO_SPIBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbISINK_SPIBUFF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TIA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIG->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TIA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TIA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TLOB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TPAD->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_VCOGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIVSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIVSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_CPGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_VCOSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_VCOSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIG->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIVGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_CPSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_CPSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_LNA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_LNA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_MXRFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_RXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_SPIBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGIp2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGIp1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TIA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TIA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TLOB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TPAD->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_VCOGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_VCOSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_VCOSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_CPGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIVSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIVSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIG->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIVGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_CPSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_CPSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_LNA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_LNA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_MXRFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_RXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_SPIBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGIp2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGIp1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_VCOSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_VCOSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TXBUF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_VCOGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TLOB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TPAD->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TIA12->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TIA14->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_RXBUF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_MXRFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_LNA12->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_LNA14->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIVSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIVSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIVGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_CPSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIG->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_CPGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_CPSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_SPIBUF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_AFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGIp2->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGIp1->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
}

pnlLDO_view::~pnlLDO_view()
{
	// Disconnect Events
	chkEN_G_LDOP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_LDO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIVGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIGGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_CPGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_VCOGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_MXRFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_LNA14->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_LNA12->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TIA14->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIVSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIGSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_CPSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_VCOSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIVSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIGSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_CPSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_VCOSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkPD_LDO_DIGIp1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkPD_LDO_DIGIp2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TXBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_RXBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TPAD->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TLOB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkPD_LDO_SPIBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbISINK_SPIBUFF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_AFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_TIA12->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LDO_DIG->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TIA12->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TIA14->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TLOB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TPAD->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_TXBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_VCOGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIVSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIVSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_AFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_CPGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_VCOSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_VCOSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIG->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIVGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_CPSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_CPSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_LNA12->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_LNA14->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_MXRFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_RXBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_SPIBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGIp2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_LDO_DIGIp1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TIA12->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TIA14->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TLOB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TPAD->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_TXBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_VCOGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_VCOSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_VCOSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_AFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_CPGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIVSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIVSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIG->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIVGN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_CPSXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_CPSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_LNA12->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_LNA14->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_MXRFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_RBB->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_RXBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_SPIBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGIp2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	chkEN_LOADIMP_LDO_DIGIp1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_VCOSXR->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_VCOSXT->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TXBUF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_VCOGN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TLOB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TPAD->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TIA12->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TIA14->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_RXBUF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_TBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_MXRFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_LNA12->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_LNA14->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIVSXR->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIVSXT->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGSXT->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIVGN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGGN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGSXR->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_CPSXT->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIG->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_CPGN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_CPSXR->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_SPIBUF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_AFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGIp2->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	cmbRDIV_DIGIp1->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLDO_view::ParameterChangeHandler ), NULL, this );
	
}

pnlXBUF_view::pnlXBUF_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer76;
	fgSizer76 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer76->SetFlexibleDirection( wxBOTH );
	fgSizer76->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
	
	chkPD_XBUF_RX = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_XBUF_RX, wxT("Power down Rx"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_XBUF_RX->SetToolTip( wxT("Power down signal PD_XBUF_RX") );
	
	sbSizerPowerDowns->Add( chkPD_XBUF_RX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_XBUF_TX = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_XBUF_TX, wxT("Power down Tx"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_XBUF_TX->SetToolTip( wxT("Power down signal PD_XBUF_TX") );
	
	sbSizerPowerDowns->Add( chkPD_XBUF_TX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G_XBUF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_XBUF, wxT("Enable XBUF module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_XBUF->SetToolTip( wxT("Enable control for all the XBUF power downs") );
	
	sbSizerPowerDowns->Add( chkEN_G_XBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer76->Add( sbSizerPowerDowns, 0, wxALL|wxALIGN_LEFT, 0 );
	
	wxFlexGridSizer* fgSizer77;
	fgSizer77 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer77->SetFlexibleDirection( wxBOTH );
	fgSizer77->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkSLFB_XBUF_RX = new wxCheckBox( this, ID_SLFB_XBUF_RX, wxT("Rx Enable biasing the input's DC voltage "), wxDefaultPosition, wxDefaultSize, 0 );
	chkSLFB_XBUF_RX->SetToolTip( wxT("Self biasing digital contol SLFB_XBUF_RX") );
	
	fgSizer77->Add( chkSLFB_XBUF_RX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSLFB_XBUF_TX = new wxCheckBox( this, ID_SLFB_XBUF_TX, wxT("Tx Enable biasing the input's DC voltage "), wxDefaultPosition, wxDefaultSize, 0 );
	chkSLFB_XBUF_TX->SetToolTip( wxT("Self biasing digital contol SLFB_XBUF_TX") );
	
	fgSizer77->Add( chkSLFB_XBUF_TX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkBYP_XBUF_RX = new wxCheckBox( this, ID_BYP_XBUF_RX, wxT("Shorts the Input 3.3 V buffer in XBUF RX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBYP_XBUF_RX->SetToolTip( wxT("Shorts the Input 3.3V buffer in XBUF") );
	
	fgSizer77->Add( chkBYP_XBUF_RX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkBYP_XBUF_TX = new wxCheckBox( this, ID_BYP_XBUF_TX, wxT("Shorts the Input 3.3 V buffer in XBUF TX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBYP_XBUF_TX->SetToolTip( wxT("Shorts the Input 3.3V buffer in XBUF") );
	
	fgSizer77->Add( chkBYP_XBUF_TX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_OUT2_XBUF_TX = new wxCheckBox( this, ID_EN_OUT2_XBUF_TX, wxT("EN_OUT2_XBUF_TX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_OUT2_XBUF_TX->SetToolTip( wxT("Enables the 2nd output of TX XBUF. This 2nd buffer goes to XBUF_RX. This should be active when only 1 XBUF is to be used") );
	
	fgSizer77->Add( chkEN_OUT2_XBUF_TX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_TBUFIN_XBUF_RX = new wxCheckBox( this, ID_EN_TBUFIN_XBUF_RX, wxT("EN_TBUFIN_XBUF_RX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_TBUFIN_XBUF_RX->SetToolTip( wxT("Disables the input from the external XOSC and buffers the 2nd input signal (from TX XBUF 2nd output) to the RX. This should be active when only 1 XBUF is to be used") );
	
	fgSizer77->Add( chkEN_TBUFIN_XBUF_RX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer76->Add( fgSizer77, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( fgSizer76 );
	this->Layout();
	fgSizer76->Fit( this );
	
	// Connect Events
	chkPD_XBUF_RX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkPD_XBUF_TX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_XBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkSLFB_XBUF_RX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkSLFB_XBUF_TX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkBYP_XBUF_RX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkBYP_XBUF_TX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkEN_OUT2_XBUF_TX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkEN_TBUFIN_XBUF_RX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
}

pnlXBUF_view::~pnlXBUF_view()
{
	// Disconnect Events
	chkPD_XBUF_RX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkPD_XBUF_TX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_XBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkSLFB_XBUF_RX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkSLFB_XBUF_TX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkBYP_XBUF_RX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkBYP_XBUF_TX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkEN_OUT2_XBUF_TX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	chkEN_TBUFIN_XBUF_RX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlXBUF_view::ParameterChangeHandler ), NULL, this );
	
}

pnlCLKGEN_view::pnlCLKGEN_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer244;
	fgSizer244 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer244->SetFlexibleDirection( wxBOTH );
	fgSizer244->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer247;
	fgSizer247 = new wxFlexGridSizer( 0, 1, 0, 5 );
	fgSizer247->SetFlexibleDirection( wxBOTH );
	fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer66;
	sbSizer66 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
	
	chkPD_CP_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_CP_CGEN, wxT("Charge pump"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_CP_CGEN->SetToolTip( wxT("Power down for Charge Pump") );
	
	sbSizer66->Add( chkPD_CP_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_FDIV_FB_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_FDIV_FB_CGEN, wxT("Feedback frequency divider"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_FDIV_FB_CGEN->SetToolTip( wxT("Power down for feedback frequency divider") );
	
	sbSizer66->Add( chkPD_FDIV_FB_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_FDIV_O_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_FDIV_O_CGEN, wxT("Frequency divider"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_FDIV_O_CGEN->SetToolTip( wxT("Power down for forward frequency divider of the CGEN block") );
	
	sbSizer66->Add( chkPD_FDIV_O_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_SDM_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_SDM_CGEN, wxT("SDM"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_SDM_CGEN->SetToolTip( wxT("Power down for SDM") );
	
	sbSizer66->Add( chkPD_SDM_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_VCO_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_VCO_CGEN, wxT("VCO"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_VCO_CGEN->SetToolTip( wxT("Power down for VCO") );
	
	sbSizer66->Add( chkPD_VCO_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_VCO_COMP_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_VCO_COMP_CGEN, wxT("VCO comparator"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_VCO_COMP_CGEN->SetToolTip( wxT("Power down for VCO comparator") );
	
	sbSizer66->Add( chkPD_VCO_COMP_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_EN_G_CGEN, wxT("Enable CLKGEN module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G_CGEN->SetToolTip( wxT("Enable control for all the CGEN power downs") );
	
	sbSizer66->Add( chkEN_G_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer247->Add( sbSizer66, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer65;
	sbSizer65 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("UNGROUPED") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer81;
	fgSizer81 = new wxFlexGridSizer( 0, 1, 0, 5 );
	fgSizer81->SetFlexibleDirection( wxBOTH );
	fgSizer81->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkSPDUP_VCO_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_SPDUP_VCO_CGEN, wxT("Bypass noise filter resistor"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_VCO_CGEN->SetToolTip( wxT("Bypasses the noise filter resistor for fast setlling time. It should be connected to a 1us pulse") );
	
	fgSizer81->Add( chkSPDUP_VCO_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkRESET_N_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_RESET_N_CGEN, wxT("Pulse used in start-up to reset"), wxDefaultPosition, wxDefaultSize, 0 );
	chkRESET_N_CGEN->SetToolTip( wxT("A pulse should be used in the start-up to reset ( 1-normal operation)") );
	
	fgSizer81->Add( chkRESET_N_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_COARSE_CKLGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_EN_COARSE_CKLGEN, wxT("Enable coarse tuning block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_COARSE_CKLGEN->SetToolTip( wxT("Enable signal for coarse tuning block") );
	
	fgSizer81->Add( chkEN_COARSE_CKLGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_INTONLY_SDM_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_EN_INTONLY_SDM_CGEN, wxT("Enable INTEGER-N mode"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_INTONLY_SDM_CGEN->SetToolTip( wxT("Enables INTEGER-N mode of the SX ") );
	
	fgSizer81->Add( chkEN_INTONLY_SDM_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_SDM_CLK_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_EN_SDM_CLK_CGEN, wxT("Enable SDM clock"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_SDM_CLK_CGEN->SetToolTip( wxT("Enables/Disables SDM clock. In INT-N mode or for noise testing, SDM clock can be disabled") );
	
	fgSizer81->Add( chkEN_SDM_CLK_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkREV_SDMCLK_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REV_SDMCLK_CGEN, wxT("Reverse SDM clock"), wxDefaultPosition, wxDefaultSize, 0 );
	chkREV_SDMCLK_CGEN->SetToolTip( wxT("Reverses the SDM clock") );
	
	fgSizer81->Add( chkREV_SDMCLK_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSX_DITHER_EN_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_SX_DITHER_EN_CGEN, wxT("Enable dithering in SDM"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSX_DITHER_EN_CGEN->SetToolTip( wxT("Enabled dithering in SDM") );
	
	fgSizer81->Add( chkSX_DITHER_EN_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkREV_CLKDAC_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REV_CLKDAC_CGEN, wxT("Invert DAC F_CLKL"), wxDefaultPosition, wxDefaultSize, 0 );
	chkREV_CLKDAC_CGEN->SetToolTip( wxT("Inverts the clock F_CLKL") );
	
	fgSizer81->Add( chkREV_CLKDAC_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkREV_CLKADC_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REV_CLKADC_CGEN, wxT("Invert ADC F_CLKL"), wxDefaultPosition, wxDefaultSize, 0 );
	chkREV_CLKADC_CGEN->SetToolTip( wxT("Inverts the clock F_CLKL") );
	
	fgSizer81->Add( chkREV_CLKADC_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkREVPH_PFD_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REVPH_PFD_CGEN, wxT("REVPH_PFD_CGEN"), wxDefaultPosition, wxDefaultSize, 0 );
	chkREVPH_PFD_CGEN->SetToolTip( wxT("Reverse the pulses of PFD. It can be used to reverse the polarity of the PLL loop (positive feedback to negative feedback)") );
	
	fgSizer81->Add( chkREVPH_PFD_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCOARSE_START_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_COARSE_START_CGEN, wxT("Signal coarse tuning algorithm"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCOARSE_START_CGEN->SetToolTip( wxT("Control signal for coarse tuning algorithm (SX_SWC_calibration)") );
	
	fgSizer81->Add( chkCOARSE_START_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer65->Add( fgSizer81, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer247->Add( sbSizer65, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrSEL_SDMCLK_CGENChoices[] = { wxT("Feedback divider"), wxT("Fref") };
	int rgrSEL_SDMCLK_CGENNChoices = sizeof( rgrSEL_SDMCLK_CGENChoices ) / sizeof( wxString );
	rgrSEL_SDMCLK_CGEN = new wxRadioBox( this, ID_SEL_SDMCLK_CGEN, wxT("Output for SDM"), wxDefaultPosition, wxDefaultSize, rgrSEL_SDMCLK_CGENNChoices, rgrSEL_SDMCLK_CGENChoices, 1, wxRA_SPECIFY_COLS );
	rgrSEL_SDMCLK_CGEN->SetSelection( 0 );
	rgrSEL_SDMCLK_CGEN->SetToolTip( wxT("Selects between the feedback divider output and Fref for SDM") );
	
	fgSizer247->Add( rgrSEL_SDMCLK_CGEN, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer244->Add( fgSizer247, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer245;
	fgSizer245 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer245->SetFlexibleDirection( wxBOTH );
	fgSizer245->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer246;
	fgSizer246 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer246->AddGrowableCol( 1 );
	fgSizer246->SetFlexibleDirection( wxBOTH );
	fgSizer246->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT4 = new wxStaticText( this, wxID_ANY, wxT("Test mode of SX"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer246->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbTST_CGEN = new wxComboBox( this, ID_TST_CGEN, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	fgSizer246->Add( cmbTST_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer245->Add( fgSizer246, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer248;
	fgSizer248 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer248->AddGrowableCol( 2 );
	fgSizer248->SetFlexibleDirection( wxBOTH );
	fgSizer248->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer68;
	sbSizer68 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("PLL loop filter") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer86;
	fgSizer86 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer86->SetFlexibleDirection( wxBOTH );
	fgSizer86->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("CP2"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer86->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCP2_CGEN = new wxComboBox( sbSizer68->GetStaticBox(), ID_CP2_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCP2_CGEN->SetToolTip( wxT("Controls the value of CP2 (cap from CP output to GND) in the PLL filter") );
	
	fgSizer86->Add( cmbCP2_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("CP3"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer86->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCP3_CGEN = new wxComboBox( sbSizer68->GetStaticBox(), ID_CP3_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCP3_CGEN->SetToolTip( wxT("Controls the value of CP3 (cap from VCO Vtune input to GND) in the PLL filter") );
	
	fgSizer86->Add( cmbCP3_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("CZ"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( -1 );
	fgSizer86->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCZ_CGEN = new wxComboBox( sbSizer68->GetStaticBox(), ID_CZ_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCZ_CGEN->SetToolTip( wxT("Controls the value of CZ (Zero capacitor) in the PLL filter") );
	
	fgSizer86->Add( cmbCZ_CGEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer68->Add( fgSizer86, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	
	fgSizer248->Add( sbSizer68, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer254;
	fgSizer254 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer254->SetFlexibleDirection( wxBOTH );
	fgSizer254->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString cmbEN_ADCCLKH_CLKGNChoices[] = { wxT("ADC"), wxT("DAC") };
	int cmbEN_ADCCLKH_CLKGNNChoices = sizeof( cmbEN_ADCCLKH_CLKGNChoices ) / sizeof( wxString );
	cmbEN_ADCCLKH_CLKGN = new wxRadioBox( this, ID_EN_ADCCLKH_CLKGN, wxT("FCLKH to"), wxDefaultPosition, wxDefaultSize, cmbEN_ADCCLKH_CLKGNNChoices, cmbEN_ADCCLKH_CLKGNChoices, 2, wxRA_SPECIFY_COLS );
	cmbEN_ADCCLKH_CLKGN->SetSelection( 1 );
	fgSizer254->Add( cmbEN_ADCCLKH_CLKGN, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer140;
	sbSizer140 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TSP frequency") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer255;
	fgSizer255 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer255->SetFlexibleDirection( wxBOTH );
	fgSizer255->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText369 = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("RxTSP (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText369->Wrap( -1 );
	fgSizer255->Add( m_staticText369, 0, 0, 5 );
	
	lblRxTSPfreq = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("??????"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRxTSPfreq->Wrap( -1 );
	fgSizer255->Add( lblRxTSPfreq, 0, 0, 5 );
	
	m_staticText367 = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("TxTSP (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText367->Wrap( -1 );
	fgSizer255->Add( m_staticText367, 0, 0, 5 );
	
	lblTxTSPfreq = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("??????"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTxTSPfreq->Wrap( -1 );
	fgSizer255->Add( lblTxTSPfreq, 0, 0, 5 );
	
	
	sbSizer140->Add( fgSizer255, 1, wxEXPAND, 5 );
	
	
	fgSizer254->Add( sbSizer140, 1, wxEXPAND, 5 );
	
	
	fgSizer248->Add( fgSizer254, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer70;
	sbSizer70 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Frequency controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer89;
	fgSizer89 = new wxFlexGridSizer( 0, 3, 0, 10 );
	fgSizer89->AddGrowableCol( 0 );
	fgSizer89->AddGrowableCol( 2 );
	fgSizer89->SetFlexibleDirection( wxBOTH );
	fgSizer89->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("CLK_H (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer89->Add( ID_STATICTEXT10, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT18 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("CLKH_OV_CLKL"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT18->Wrap( -1 );
	fgSizer89->Add( ID_STATICTEXT18, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10 );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("CLK_L (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer89->Add( ID_STATICTEXT3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFrequency = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0 );
	#ifdef __WXGTK__
	if ( !txtFrequency->HasFlag( wxTE_MULTILINE ) )
	{
	txtFrequency->SetMaxLength( 10 );
	}
	#else
	txtFrequency->SetMaxLength( 10 );
	#endif
	fgSizer89->Add( txtFrequency, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	cmbCLKH_OV_CLKL_CGEN = new wxComboBox( sbSizer70->GetStaticBox(), ID_CLKH_OV_CLKL_CGEN, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0, NULL, 0 ); 
	cmbCLKH_OV_CLKL_CGEN->SetToolTip( wxT("FCLKL here is ADC clock. FCLKH is the clock to the DAC and if no division is added to the ADC as well") );
	
	fgSizer89->Add( cmbCLKH_OV_CLKL_CGEN, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFrequencyCLKL = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 48,-1 ), wxTE_READONLY );
	#ifdef __WXGTK__
	if ( !txtFrequencyCLKL->HasFlag( wxTE_MULTILINE ) )
	{
	txtFrequencyCLKL->SetMaxLength( 10 );
	}
	#else
	txtFrequencyCLKL->SetMaxLength( 10 );
	#endif
	txtFrequencyCLKL->Enable( false );
	
	fgSizer89->Add( txtFrequencyCLKL, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer70->Add( fgSizer89, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	
	ID_STATICTEXT101 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Rx phase"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT101->Wrap( -1 );
	bSizer11->Add( ID_STATICTEXT101, 0, wxALL, 5 );
	
	rxPhase = new wxSpinCtrl( sbSizer70->GetStaticBox(), ID_GFIR1_N_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 360, 120 );
	rxPhase->Enable( false );
	rxPhase->SetToolTip( wxT("LML interface phase offset for Rx") );
	rxPhase->SetMinSize( wxSize( 56,-1 ) );
	
	bSizer11->Add( rxPhase, 0, wxALL, 5 );
	
	ID_STATICTEXT102 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Tx phase"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT102->Wrap( -1 );
	bSizer11->Add( ID_STATICTEXT102, 0, wxALL, 5 );
	
	txPhase = new wxSpinCtrl( sbSizer70->GetStaticBox(), ID_GFIR1_N_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 360, 120 );
	txPhase->Enable( false );
	txPhase->SetToolTip( wxT("LML interface phase offset for Tx") );
	txPhase->SetMinSize( wxSize( 56,-1 ) );
	
	bSizer11->Add( txPhase, 0, wxALL, 5 );
	
	chkAutoPhase = new wxCheckBox( sbSizer70->GetStaticBox(), ID_AUTO_PHASE, wxT("Auto phase"), wxDefaultPosition, wxDefaultSize, 0 );
	chkAutoPhase->SetValue(true); 
	chkAutoPhase->SetToolTip( wxT("Configure FPGA PLL phase for LML interface automatically") );
	
	bSizer11->Add( chkAutoPhase, 0, wxALL, 5 );
	
	
	sbSizer70->Add( bSizer11, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	btnCalculate = new wxButton( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Calculate"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( btnCalculate, 1, wxRIGHT|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	btnTune = new wxButton( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Tune"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( btnTune, 1, wxLEFT|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	sbSizer70->Add( bSizer1, 0, wxALIGN_RIGHT|wxALIGN_TOP, 5 );
	
	
	fgSizer248->Add( sbSizer70, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	
	fgSizer245->Add( fgSizer248, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer88;
	fgSizer88 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer88->SetFlexibleDirection( wxBOTH );
	fgSizer88->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer71;
	sbSizer71 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Calculated Values for Fractional Mode") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer90;
	fgSizer90 = new wxFlexGridSizer( 5, 4, 5, 10 );
	fgSizer90->SetFlexibleDirection( wxBOTH );
	fgSizer90->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("N Integer:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer90->Add( ID_STATICTEXT7, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblINT_SDM_CGEN = new wxStaticText( sbSizer71->GetStaticBox(), ID_INT_SDM_CGEN, wxT("???"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	lblINT_SDM_CGEN->Wrap( 0 );
	lblINT_SDM_CGEN->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer90->Add( lblINT_SDM_CGEN, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT23 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Ref. clock (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT23->Wrap( -1 );
	fgSizer90->Add( ID_STATICTEXT23, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	lblRefClk_MHz = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	lblRefClk_MHz->Wrap( -1 );
	lblRefClk_MHz->SetToolTip( wxT("Reference clock is received from SXR") );
	lblRefClk_MHz->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer90->Add( lblRefClk_MHz, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT21 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("N Fractional:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT21->Wrap( -1 );
	fgSizer90->Add( ID_STATICTEXT21, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblFRAC_SDM_CGEN = new wxStaticText( sbSizer71->GetStaticBox(), ID_FRAC_SDM_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFRAC_SDM_CGEN->Wrap( 0 );
	fgSizer90->Add( lblFRAC_SDM_CGEN, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT2 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Divider:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer90->Add( ID_STATICTEXT2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblDivider = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDivider->Wrap( 0 );
	fgSizer90->Add( lblDivider, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT25 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Output Freq (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT25->Wrap( -1 );
	fgSizer90->Add( ID_STATICTEXT25, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblRealOutFrequency = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRealOutFrequency->Wrap( 0 );
	fgSizer90->Add( lblRealOutFrequency, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer71->Add( fgSizer90, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer88->Add( sbSizer71, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer76;
	sbSizer76 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCO Comparators") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer97;
	fgSizer97 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer97->AddGrowableRow( 0 );
	fgSizer97->SetFlexibleDirection( wxBOTH );
	fgSizer97->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT161 = new wxStaticText( sbSizer76->GetStaticBox(), wxID_ANY, wxT("High:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT161->Wrap( -1 );
	fgSizer97->Add( ID_STATICTEXT161, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblVCO_CMPHO_CGEN = new wxStaticText( sbSizer76->GetStaticBox(), ID_VCO_CMPHO_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVCO_CMPHO_CGEN->Wrap( -1 );
	fgSizer97->Add( lblVCO_CMPHO_CGEN, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT171 = new wxStaticText( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Low:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT171->Wrap( -1 );
	fgSizer97->Add( ID_STATICTEXT171, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10 );
	
	lblVCO_CMPLO_CGEN = new wxStaticText( sbSizer76->GetStaticBox(), ID_VCO_CMPLO_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVCO_CMPLO_CGEN->Wrap( -1 );
	fgSizer97->Add( lblVCO_CMPLO_CGEN, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer76->Add( fgSizer97, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	btnUpdateValues1 = new wxButton( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer76->Add( btnUpdateValues1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer88->Add( sbSizer76, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer139;
	sbSizer139 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Coarse comparator") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer252;
	fgSizer252 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer252->SetFlexibleDirection( wxBOTH );
	fgSizer252->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer139->GetStaticBox(), wxID_ANY, wxT("STEPDONE:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	ID_STATICTEXT5->SetToolTip( wxT("COARSE_STEPDONE_CGEN") );
	
	fgSizer252->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblCOARSE_STEPDONE_CGEN = new wxStaticText( sbSizer139->GetStaticBox(), ID_COARSE_STEPDONE_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCOARSE_STEPDONE_CGEN->Wrap( -1 );
	fgSizer252->Add( lblCOARSE_STEPDONE_CGEN, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizer139->GetStaticBox(), wxID_ANY, wxT("COMPO:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	ID_STATICTEXT6->SetToolTip( wxT("COARSEPLL_COMPO_CGEN") );
	
	fgSizer252->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblCOARSEPLL_COMPO_CGEN = new wxStaticText( sbSizer139->GetStaticBox(), ID_COARSEPLL_COMPO_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCOARSEPLL_COMPO_CGEN->Wrap( -1 );
	fgSizer252->Add( lblCOARSEPLL_COMPO_CGEN, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer139->Add( fgSizer252, 0, wxLEFT, 5 );
	
	btnUpdateCoarse = new wxButton( sbSizer139->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer139->Add( btnUpdateCoarse, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer88->Add( sbSizer139, 1, 0, 5 );
	
	
	fgSizer245->Add( fgSizer88, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer67;
	sbSizer67 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCO") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer85;
	fgSizer85 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer85->AddGrowableCol( 1 );
	fgSizer85->SetFlexibleDirection( wxBOTH );
	fgSizer85->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT20 = new wxStaticText( sbSizer67->GetStaticBox(), wxID_ANY, wxT("CSW_VCO_CGEN"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT20->Wrap( -1 );
	fgSizer85->Add( ID_STATICTEXT20, 1, wxALIGN_LEFT|wxALIGN_BOTTOM, 5 );
	
	cmbCSW_VCO_CGEN = new NumericSlider( sbSizer67->GetStaticBox(), ID_CSW_VCO_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer85->Add( cmbCSW_VCO_CGEN, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizer67->GetStaticBox(), wxID_ANY, wxT("Scales VCO bias current"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer85->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbICT_VCO_CGEN = new NumericSlider( sbSizer67->GetStaticBox(), ID_ICT_VCO_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
	fgSizer85->Add( cmbICT_VCO_CGEN, 1, wxEXPAND, 5 );
	
	btnShowVCO = new wxButton( sbSizer67->GetStaticBox(), wxID_ANY, wxT("VCO params"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer85->Add( btnShowVCO, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer67->Add( fgSizer85, 1, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	
	fgSizer245->Add( sbSizer67, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer69;
	sbSizer69 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Charge pump scales") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer87;
	fgSizer87 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer87->AddGrowableCol( 1 );
	fgSizer87->SetFlexibleDirection( wxBOTH );
	fgSizer87->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Offset current"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer87->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbIOFFSET_CP_CGEN = new NumericSlider( sbSizer69->GetStaticBox(), ID_IOFFSET_CP_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
	cmbIOFFSET_CP_CGEN->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer87->Add( cmbIOFFSET_CP_CGEN, 1, wxEXPAND, 5 );
	
	ID_STATICTEXT17 = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Pulse current"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT17->Wrap( -1 );
	fgSizer87->Add( ID_STATICTEXT17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbIPULSE_CP_CGEN = new NumericSlider( sbSizer69->GetStaticBox(), ID_IPULSE_CP_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
	cmbIPULSE_CP_CGEN->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer87->Add( cmbIPULSE_CP_CGEN, 1, wxEXPAND, 5 );
	
	
	sbSizer69->Add( fgSizer87, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer245->Add( sbSizer69, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	sizerR3 = new wxFlexGridSizer( 0, 2, 0, 0 );
	sizerR3->SetFlexibleDirection( wxBOTH );
	sizerR3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer245->Add( sizerR3, 1, wxEXPAND, 5 );
	
	
	fgSizer244->Add( fgSizer245, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer244 );
	this->Layout();
	fgSizer244->Fit( this );
	
	// Connect Events
	chkPD_CP_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_FDIV_FB_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_FDIV_O_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_SDM_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO_COMP_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_VCO_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkRESET_N_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_COARSE_CKLGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_INTONLY_SDM_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_SDM_CLK_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREV_SDMCLK_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkSX_DITHER_EN_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREV_CLKDAC_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREV_CLKADC_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREVPH_PFD_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkCOARSE_START_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	rgrSEL_SDMCLK_CGEN->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbTST_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCP2_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCP3_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCZ_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbEN_ADCCLKH_CLKGN->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCLKH_OV_CLKL_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	rxPhase->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
	txPhase->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
	chkAutoPhase->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnAutoPhase ), NULL, this );
	btnCalculate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
	btnTune->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::onbtnTuneClick ), NULL, this );
	btnUpdateValues1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnbtnReadComparators ), NULL, this );
	btnUpdateCoarse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnbtnUpdateCoarse ), NULL, this );
	cmbCSW_VCO_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbICT_VCO_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	btnShowVCO->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnShowVCOclicked ), NULL, this );
	cmbIOFFSET_CP_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbIPULSE_CP_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
}

pnlCLKGEN_view::~pnlCLKGEN_view()
{
	// Disconnect Events
	chkPD_CP_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_FDIV_FB_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_FDIV_O_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_SDM_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO_COMP_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_G_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_VCO_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkRESET_N_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_COARSE_CKLGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_INTONLY_SDM_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkEN_SDM_CLK_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREV_SDMCLK_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkSX_DITHER_EN_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREV_CLKDAC_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREV_CLKADC_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkREVPH_PFD_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	chkCOARSE_START_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	rgrSEL_SDMCLK_CGEN->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbTST_CGEN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCP2_CGEN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCP3_CGEN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCZ_CGEN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbEN_ADCCLKH_CLKGN->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbCLKH_OV_CLKL_CGEN->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	rxPhase->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
	txPhase->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
	chkAutoPhase->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnAutoPhase ), NULL, this );
	btnCalculate->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
	btnTune->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::onbtnTuneClick ), NULL, this );
	btnUpdateValues1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnbtnReadComparators ), NULL, this );
	btnUpdateCoarse->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnbtnUpdateCoarse ), NULL, this );
	cmbCSW_VCO_CGEN->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbICT_VCO_CGEN->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	btnShowVCO->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCLKGEN_view::OnShowVCOclicked ), NULL, this );
	cmbIOFFSET_CP_CGEN->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	cmbIPULSE_CP_CGEN->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
	
}

pnlSX_view::pnlSX_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer92;
	fgSizer92 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer92->SetFlexibleDirection( wxBOTH );
	fgSizer92->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer93;
	fgSizer93 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer93->SetFlexibleDirection( wxBOTH );
	fgSizer93->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerPowerDowns;
	sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer95;
	fgSizer95 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer95->SetFlexibleDirection( wxBOTH );
	fgSizer95->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkPD_FBDIV = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_FBDIV, wxT("Feedback divider block"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_FBDIV->SetToolTip( wxT("Power down the feedback divider block") );
	
	fgSizer95->Add( chkPD_FBDIV, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	chkPD_LOCH_T2RBUF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LOCH_T2RBUF, wxT("LO buffer from SXT to SXR"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_LOCH_T2RBUF->SetToolTip( wxT("Power down for LO buffer from SXT to SXR. To be active only in the TDD mode") );
	
	fgSizer95->Add( chkPD_LOCH_T2RBUF, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_CP = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_CP, wxT("Charge pump"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_CP->SetToolTip( wxT("Power down for Charge Pump") );
	
	fgSizer95->Add( chkPD_CP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_FDIV = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_FDIV, wxT("Forward frequency divider"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_FDIV->SetToolTip( wxT("Power down for feedback frequency and forward dividers") );
	
	fgSizer95->Add( chkPD_FDIV, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_SDM = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_SDM, wxT("SDM"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_SDM->SetToolTip( wxT("Power down for SDM") );
	
	fgSizer95->Add( chkPD_SDM, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_VCO_COMP = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_VCO_COMP, wxT("VCO comparator"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_VCO_COMP->SetToolTip( wxT("Power down for VCO comparator") );
	
	fgSizer95->Add( chkPD_VCO_COMP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPD_VCO = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_VCO, wxT("VCO"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPD_VCO->SetToolTip( wxT("Power down for VCO") );
	
	fgSizer95->Add( chkPD_VCO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_G = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G, wxT("Enable SXR/SXT module"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_G->SetToolTip( wxT("Enable control for all the SXT power downs") );
	
	fgSizer95->Add( chkEN_G, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizer74;
	sbSizer74 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
	
	chkEN_DIR_SXRSXT = new wxCheckBox( sbSizer74->GetStaticBox(), ID_EN_DIR_SXRSXT, wxT("Direct control of PDs and ENs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DIR_SXRSXT->SetToolTip( wxT("Enables direct control of PDs and ENs for SXR/SXT module") );
	
	sbSizer74->Add( chkEN_DIR_SXRSXT, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer95->Add( sbSizer74, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizerPowerDowns->Add( fgSizer95, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0 );
	
	
	fgSizer93->Add( sbSizerPowerDowns, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer77;
	sbSizer77 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("SXT/SXR controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer101;
	fgSizer101 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer101->SetFlexibleDirection( wxBOTH );
	fgSizer101->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkRESET_N = new wxCheckBox( sbSizer77->GetStaticBox(), ID_RESET_N, wxT("Reset SX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkRESET_N->SetToolTip( wxT("Resets SX. A pulse should be used in the start-up to reset") );
	
	fgSizer101->Add( chkRESET_N, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSPDUP_VCO = new wxCheckBox( sbSizer77->GetStaticBox(), ID_SPDUP_VCO, wxT("Bypass noise filter resistor"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSPDUP_VCO->SetToolTip( wxT("Bypasses the noise filter resistor for fast settling time. It should be connected to a 1uS pulse") );
	
	fgSizer101->Add( chkSPDUP_VCO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkBYPLDO_VCO = new wxCheckBox( sbSizer77->GetStaticBox(), ID_BYPLDO_VCO, wxT("Bypass SX LDO"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBYPLDO_VCO->SetToolTip( wxT("Controls the bypass signal for the SX LDO") );
	
	fgSizer101->Add( chkBYPLDO_VCO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_COARSEPLL = new wxCheckBox( sbSizer77->GetStaticBox(), ID_EN_COARSEPLL, wxT("Enable coarse tuning"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_COARSEPLL->SetToolTip( wxT("Enable signal for coarse tuning block") );
	
	fgSizer101->Add( chkEN_COARSEPLL, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCURLIM_VCO = new wxCheckBox( sbSizer77->GetStaticBox(), ID_CURLIM_VCO, wxT("Enable current limit"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCURLIM_VCO->SetToolTip( wxT("Enables the output current limitation in the VCO regulator") );
	
	fgSizer101->Add( chkCURLIM_VCO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkREVPH_PFD = new wxCheckBox( sbSizer77->GetStaticBox(), ID_REVPH_PFD, wxT("Reverse pulses of PFD"), wxDefaultPosition, wxDefaultSize, 0 );
	chkREVPH_PFD->SetToolTip( wxT("Reverse the pulses of PFD. It can be used to reverse the polarity of the PLL loop (positive feedback to negative feedback)") );
	
	fgSizer101->Add( chkREVPH_PFD, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_INTONLY_SDM = new wxCheckBox( sbSizer77->GetStaticBox(), ID_EN_INTONLY_SDM, wxT("Enable INTEGER_N mode"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_INTONLY_SDM->SetToolTip( wxT("Enables INTEGER-N mode of the SX ") );
	
	fgSizer101->Add( chkEN_INTONLY_SDM, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_SDM_CLK = new wxCheckBox( sbSizer77->GetStaticBox(), ID_EN_SDM_CLK, wxT("Enable SDM clock"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_SDM_CLK->SetToolTip( wxT("Enables/Disables SDM clock. In INT-N mode or for noise testing, SDM clock can be disabled") );
	
	fgSizer101->Add( chkEN_SDM_CLK, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkREV_SDMCLK = new wxCheckBox( sbSizer77->GetStaticBox(), ID_REV_SDMCLK, wxT("Reverse SDM clock"), wxDefaultPosition, wxDefaultSize, 0 );
	chkREV_SDMCLK->SetToolTip( wxT("Reverses the SDM clock") );
	
	fgSizer101->Add( chkREV_SDMCLK, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSX_DITHER_EN = new wxCheckBox( sbSizer77->GetStaticBox(), ID_SX_DITHER_EN, wxT("Enable dithering in SDM"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSX_DITHER_EN->SetToolTip( wxT("Enabled dithering in SDM") );
	
	fgSizer101->Add( chkSX_DITHER_EN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_DIV2_DIVPROGenabled = new wxCheckBox( sbSizer77->GetStaticBox(), ID_EN_DIV2_DIVPROGenabled, wxT("Enable additional DIV2 prescaler"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DIV2_DIVPROGenabled->Hide();
	
	fgSizer101->Add( chkEN_DIV2_DIVPROGenabled, 0, 0, 5 );
	
	
	sbSizer77->Add( fgSizer101, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer93->Add( sbSizer77, 0, wxEXPAND, 5 );
	
	
	fgSizer92->Add( fgSizer93, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer103;
	fgSizer103 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer103->AddGrowableCol( 0 );
	fgSizer103->SetFlexibleDirection( wxBOTH );
	fgSizer103->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer203;
	fgSizer203 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer203->AddGrowableCol( 1 );
	fgSizer203->SetFlexibleDirection( wxBOTH );
	fgSizer203->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT7 = new wxStaticText( this, wxID_ANY, wxT("Test mode of SX"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer203->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbTST_SX = new wxComboBox( this, ID_TST_SX, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	fgSizer203->Add( cmbTST_SX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer103->Add( fgSizer203, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer198;
	fgSizer198 = new wxFlexGridSizer( 0, 5, 5, 5 );
	fgSizer198->SetFlexibleDirection( wxBOTH );
	fgSizer198->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerDivisionRatio;
	sbSizerDivisionRatio = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Division ratio") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgDivisionRatio;
	fgDivisionRatio = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgDivisionRatio->AddGrowableCol( 1 );
	fgDivisionRatio->AddGrowableRow( 0 );
	fgDivisionRatio->AddGrowableRow( 1 );
	fgDivisionRatio->AddGrowableRow( 2 );
	fgDivisionRatio->SetFlexibleDirection( wxBOTH );
	fgDivisionRatio->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizerDivisionRatio->GetStaticBox(), wxID_ANY, wxT("Trim duty cycle of DIV2 LOCH"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgDivisionRatio->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ctrPW_DIV2_LOCH = new wxSpinCtrl( sbSizerDivisionRatio->GetStaticBox(), ID_PW_DIV2_LOCH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 1 );
	ctrPW_DIV2_LOCH->SetMinSize( wxSize( 48,-1 ) );
	
	fgDivisionRatio->Add( ctrPW_DIV2_LOCH, 0, 0, 5 );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizerDivisionRatio->GetStaticBox(), wxID_ANY, wxT("Trim duty cycle of DIV4 LOCH"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgDivisionRatio->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ctrPW_DIV4_LOCH = new wxSpinCtrl( sbSizerDivisionRatio->GetStaticBox(), ID_PW_DIV4_LOCH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 2 );
	ctrPW_DIV4_LOCH->SetMinSize( wxSize( 48,-1 ) );
	
	fgDivisionRatio->Add( ctrPW_DIV4_LOCH, 0, 0, 5 );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizerDivisionRatio->GetStaticBox(), wxID_ANY, wxT("LOCH_DIV division ratio"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgDivisionRatio->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ctrDIV_LOCH = new wxSpinCtrl( sbSizerDivisionRatio->GetStaticBox(), ID_DIV_LOCH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 2 );
	ctrDIV_LOCH->SetMinSize( wxSize( 48,-1 ) );
	
	fgDivisionRatio->Add( ctrDIV_LOCH, 0, 0, 5 );
	
	
	sbSizerDivisionRatio->Add( fgDivisionRatio, 1, wxALL|wxEXPAND|wxALIGN_LEFT, 0 );
	
	
	fgSizer198->Add( sbSizerDivisionRatio, 1, wxALIGN_LEFT|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer78;
	sbSizer78 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("PLL loop filter") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer102;
	fgSizer102 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer102->SetFlexibleDirection( wxBOTH );
	fgSizer102->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT18 = new wxStaticText( sbSizer78->GetStaticBox(), wxID_ANY, wxT("CP2"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT18->Wrap( -1 );
	fgSizer102->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCP2_PLL = new wxComboBox( sbSizer78->GetStaticBox(), ID_CP2_PLL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCP2_PLL->SetToolTip( wxT("Controls the value of CP2 (cap from CP output to GND) in the PLL filter") );
	
	fgSizer102->Add( cmbCP2_PLL, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT19 = new wxStaticText( sbSizer78->GetStaticBox(), wxID_ANY, wxT("CP3"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT19->Wrap( -1 );
	fgSizer102->Add( ID_STATICTEXT19, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCP3_PLL = new wxComboBox( sbSizer78->GetStaticBox(), ID_CP3_PLL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCP3_PLL->SetToolTip( wxT("Controls the value of CP3 (cap from VCO Vtune input to GND) in the PLL filter") );
	
	fgSizer102->Add( cmbCP3_PLL, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT20 = new wxStaticText( sbSizer78->GetStaticBox(), wxID_ANY, wxT("CZ"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT20->Wrap( -1 );
	fgSizer102->Add( ID_STATICTEXT20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCZ = new wxComboBox( sbSizer78->GetStaticBox(), ID_CZ, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbCZ->SetToolTip( wxT("Controls the value of CZ (Zero capacitor) in the PLL filter") );
	
	fgSizer102->Add( cmbCZ, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer78->Add( fgSizer102, 1, wxEXPAND, 5 );
	
	
	fgSizer198->Add( sbSizer78, 1, wxLEFT|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrSEL_VCOChoices[] = { wxT("VCOL"), wxT("VCOM"), wxT("VCOH") };
	int rgrSEL_VCONChoices = sizeof( rgrSEL_VCOChoices ) / sizeof( wxString );
	rgrSEL_VCO = new wxRadioBox( this, ID_SEL_VCO, wxT("Active VCO"), wxDefaultPosition, wxDefaultSize, rgrSEL_VCONChoices, rgrSEL_VCOChoices, 1, wxRA_SPECIFY_COLS );
	rgrSEL_VCO->SetSelection( 0 );
	rgrSEL_VCO->SetToolTip( wxT("Selects the active VCO. It is set by SX_SWC_calibration") );
	
	fgSizer198->Add( rgrSEL_VCO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizer79;
	sbSizer79 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Frequency, MHz") ), wxVERTICAL );
	
	txtFrequency = new wxTextCtrl( sbSizer79->GetStaticBox(), wxID_ANY, wxT("2140"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFrequency->HasFlag( wxTE_MULTILINE ) )
	{
	txtFrequency->SetMaxLength( 10 );
	}
	#else
	txtFrequency->SetMaxLength( 10 );
	#endif
	sbSizer79->Add( txtFrequency, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnCalculate = new wxButton( sbSizer79->GetStaticBox(), wxID_ANY, wxT("Calculate"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer79->Add( btnCalculate, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	btnTune = new wxButton( sbSizer79->GetStaticBox(), wxID_ANY, wxT("Tune"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer79->Add( btnTune, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	ID_BUTTON5 = new wxButton( sbSizer79->GetStaticBox(), wxID_ANY, wxT("Coarse Tune"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON5->Hide();
	
	sbSizer79->Add( ID_BUTTON5, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer198->Add( sbSizer79, 0, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	pnlRefClkSpur = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlRefClkSpur->Hide();
	
	RefClkSpurSizer = new wxStaticBoxSizer( new wxStaticBox( pnlRefClkSpur, wxID_ANY, wxT("Receiver Ref Clk    \nSpur Cancelation") ), wxVERTICAL );
	
	chkEnableRefSpurCancelation = new wxCheckBox( RefClkSpurSizer->GetStaticBox(), wxID_ANY, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	RefClkSpurSizer->Add( chkEnableRefSpurCancelation, 0, 0, 5 );
	
	m_staticText359 = new wxStaticText( RefClkSpurSizer->GetStaticBox(), wxID_ANY, wxT("RF Bandwidth (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText359->Wrap( -1 );
	RefClkSpurSizer->Add( m_staticText359, 0, 0, 5 );
	
	txtRefSpurBW = new wxTextCtrl( RefClkSpurSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtRefSpurBW->HasFlag( wxTE_MULTILINE ) )
	{
	txtRefSpurBW->SetMaxLength( 10 );
	}
	#else
	txtRefSpurBW->SetMaxLength( 10 );
	#endif
	txtRefSpurBW->Enable( false );
	
	RefClkSpurSizer->Add( txtRefSpurBW, 0, 0, 5 );
	
	
	pnlRefClkSpur->SetSizer( RefClkSpurSizer );
	pnlRefClkSpur->Layout();
	RefClkSpurSizer->Fit( pnlRefClkSpur );
	fgSizer198->Add( pnlRefClkSpur, 1, wxEXPAND | wxALL, 5 );
	
	
	fgSizer103->Add( fgSizer198, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerFrequencyControls;
	sbSizerFrequencyControls = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Frequency Controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer194;
	fgSizer194 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer194->AddGrowableCol( 0 );
	fgSizer194->SetFlexibleDirection( wxBOTH );
	fgSizer194->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer80;
	sbSizer80 = new wxStaticBoxSizer( new wxStaticBox( sbSizerFrequencyControls->GetStaticBox(), wxID_ANY, wxT("Calculated Values for Fractional Mode") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer104;
	fgSizer104 = new wxFlexGridSizer( 0, 4, 5, 10 );
	fgSizer104->AddGrowableCol( 1 );
	fgSizer104->AddGrowableCol( 3 );
	fgSizer104->SetFlexibleDirection( wxBOTH );
	fgSizer104->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT2 = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("N Integer:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer104->Add( ID_STATICTEXT2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblINT_SDM = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	lblINT_SDM->Wrap( 0 );
	fgSizer104->Add( lblINT_SDM, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT21 = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("N Fractional:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT21->Wrap( -1 );
	fgSizer104->Add( ID_STATICTEXT21, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblFRAC_SDM = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFRAC_SDM->Wrap( 0 );
	fgSizer104->Add( lblFRAC_SDM, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT25 = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("Output Freq, MHz:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT25->Wrap( -1 );
	fgSizer104->Add( ID_STATICTEXT25, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblRealOutFrequency = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRealOutFrequency->Wrap( 0 );
	fgSizer104->Add( lblRealOutFrequency, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT23 = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("Divider:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT23->Wrap( -1 );
	fgSizer104->Add( ID_STATICTEXT23, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblDivider = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDivider->Wrap( 0 );
	fgSizer104->Add( lblDivider, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT30 = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("DIV2 prescaler:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT30->Wrap( -1 );
	fgSizer104->Add( ID_STATICTEXT30, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblEN_DIV2_DIVPROG = new wxStaticText( sbSizer80->GetStaticBox(), ID_EN_DIV2_DIVPROG, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblEN_DIV2_DIVPROG->Wrap( 0 );
	fgSizer104->Add( lblEN_DIV2_DIVPROG, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnChangeRefClk = new wxButton( sbSizer80->GetStaticBox(), wxID_ANY, wxT("Reference clock (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer104->Add( btnChangeRefClk, 1, wxEXPAND, 5 );
	
	lblRefClk_MHz = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	lblRefClk_MHz->Wrap( 0 );
	fgSizer104->Add( lblRefClk_MHz, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer80->Add( fgSizer104, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer194->Add( sbSizer80, 0, wxLEFT|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer76;
	sbSizer76 = new wxStaticBoxSizer( new wxStaticBox( sbSizerFrequencyControls->GetStaticBox(), wxID_ANY, wxT("VCO Comparators") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer97;
	fgSizer97 = new wxFlexGridSizer( 0, 4, 0, 5 );
	fgSizer97->AddGrowableRow( 0 );
	fgSizer97->SetFlexibleDirection( wxBOTH );
	fgSizer97->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer76->GetStaticBox(), wxID_ANY, wxT("High:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer97->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblVCO_CMPHO = new wxStaticText( sbSizer76->GetStaticBox(), ID_VCO_CMPHO, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVCO_CMPHO->Wrap( -1 );
	fgSizer97->Add( lblVCO_CMPHO, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT17 = new wxStaticText( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Low:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT17->Wrap( -1 );
	fgSizer97->Add( ID_STATICTEXT17, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10 );
	
	lblVCO_CMPLO = new wxStaticText( sbSizer76->GetStaticBox(), ID_VCO_CMPLO, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVCO_CMPLO->Wrap( -1 );
	fgSizer97->Add( lblVCO_CMPLO, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer76->Add( fgSizer97, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnUpdateValues = new wxButton( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer76->Add( btnUpdateValues, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer194->Add( sbSizer76, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizerFrequencyControls->Add( fgSizer194, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer1961;
	fgSizer1961 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer1961->AddGrowableCol( 1 );
	fgSizer1961->SetFlexibleDirection( wxBOTH );
	fgSizer1961->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizerFrequencyControls->GetStaticBox(), wxID_ANY, wxT("CSW_VCO"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer1961->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ctrCSW_VCO = new NumericSlider( sbSizerFrequencyControls->GetStaticBox(), ID_CSW_VCO, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 128 );
	fgSizer1961->Add( ctrCSW_VCO, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizerFrequencyControls->GetStaticBox(), wxID_ANY, wxT("Scales VCO bias current"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer1961->Add( ID_STATICTEXT10, 0, wxTOP|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbICT_VCO = new NumericSlider( sbSizerFrequencyControls->GetStaticBox(), ID_ICT_VCO, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer1961->Add( cmbICT_VCO, 0, wxEXPAND, 5 );
	
	
	sbSizerFrequencyControls->Add( fgSizer1961, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer196;
	fgSizer196 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer196->SetFlexibleDirection( wxBOTH );
	fgSizer196->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer96;
	fgSizer96 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer96->SetFlexibleDirection( wxBOTH );
	fgSizer96->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnShowVCOparams = new wxButton( sbSizerFrequencyControls->GetStaticBox(), ID_BTN_SHOW_VCO, wxT("VCO params"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer96->Add( btnShowVCOparams, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer196->Add( fgSizer96, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxFlexGridSizer* fgSizer98;
	fgSizer98 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer98->SetFlexibleDirection( wxBOTH );
	fgSizer98->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizerFrequencyControls->GetStaticBox(), wxID_ANY, wxT("LDO output voltage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer98->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbVDIV_VCO = new wxComboBox( sbSizerFrequencyControls->GetStaticBox(), ID_VDIV_VCO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbVDIV_VCO->SetToolTip( wxT("Controls VCO LDO output voltage") );
	
	fgSizer98->Add( cmbVDIV_VCO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer196->Add( fgSizer98, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizerFrequencyControls->Add( fgSizer196, 0, wxEXPAND, 5 );
	
	
	fgSizer103->Add( sbSizerFrequencyControls, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer193;
	fgSizer193 = new wxFlexGridSizer( 0, 4, 0, 5 );
	fgSizer193->AddGrowableCol( 1 );
	fgSizer193->SetFlexibleDirection( wxBOTH );
	fgSizer193->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT13 = new wxStaticText( this, wxID_ANY, wxT("Scales pulse current of charge pump"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer193->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbIPULSE_CP = new wxComboBox( this, ID_IPULSE_CP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbIPULSE_CP->SetToolTip( wxT("Scales the pulse current of the charge pump") );
	
	fgSizer193->Add( cmbIPULSE_CP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT8 = new wxStaticText( this, wxID_ANY, wxT("Feedback divider for SDM"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer193->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbSEL_SDMCLK = new wxComboBox( this, ID_SEL_SDMCLK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbSEL_SDMCLK->SetToolTip( wxT("Selects between the feedback divider output and Fref for SDM") );
	
	fgSizer193->Add( cmbSEL_SDMCLK, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT11 = new wxStaticText( this, wxID_ANY, wxT("Reference voltage"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer193->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRSEL_LDO_VCO = new wxComboBox( this, ID_RSEL_LDO_VCO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbRSEL_LDO_VCO->SetToolTip( wxT("Set the reference voltage that supplies bias voltage of switch-cap array and varactor") );
	
	fgSizer193->Add( cmbRSEL_LDO_VCO, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT12 = new wxStaticText( this, wxID_ANY, wxT("Scales offset of charge pump"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer193->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbIOFFSET_CP = new wxComboBox( this, ID_IOFFSET_CP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbIOFFSET_CP->SetToolTip( wxT("Scales the offset current of the charge pump, 0-->63. This current is used in Fran-N mode to create an offset in the CP response and avoid the non-linear section") );
	
	fgSizer193->Add( cmbIOFFSET_CP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer103->Add( fgSizer193, 0, 0, 0 );
	
	sizerR3 = new wxFlexGridSizer( 0, 2, 0, 0 );
	sizerR3->SetFlexibleDirection( wxBOTH );
	sizerR3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer103->Add( sizerR3, 1, wxEXPAND, 5 );
	
	
	fgSizer92->Add( fgSizer103, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	this->SetSizer( fgSizer92 );
	this->Layout();
	fgSizer92->Fit( this );
	
	// Connect Events
	chkPD_FBDIV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_LOCH_T2RBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_CP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_FDIV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_SDM->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO_COMP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_G->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_SXRSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkRESET_N->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_VCO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkBYPLDO_VCO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_COARSEPLL->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkCURLIM_VCO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkREVPH_PFD->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_INTONLY_SDM->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_SDM_CLK->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkREV_SDMCLK->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkSX_DITHER_EN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIV2_DIVPROGenabled->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::OnDIV2PrescalerChange ), NULL, this );
	cmbTST_SX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	ctrPW_DIV2_LOCH->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	ctrPW_DIV4_LOCH->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	ctrDIV_LOCH->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbCP2_PLL->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbCP3_PLL->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbCZ->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	rgrSEL_VCO->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	btnCalculate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnCalculateClick ), NULL, this );
	btnTune->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnTuneClick ), NULL, this );
	chkEnableRefSpurCancelation->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::OnEnableRefSpurCancelation ), NULL, this );
	btnChangeRefClk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnChangeRefClkClick ), NULL, this );
	btnUpdateValues->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnReadComparators ), NULL, this );
	ctrCSW_VCO->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbICT_VCO->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	btnShowVCOparams->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnShowVCOclicked ), NULL, this );
	cmbVDIV_VCO->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbIPULSE_CP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbSEL_SDMCLK->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbRSEL_LDO_VCO->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbIOFFSET_CP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
}

pnlSX_view::~pnlSX_view()
{
	// Disconnect Events
	chkPD_FBDIV->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_LOCH_T2RBUF->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_CP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_FDIV->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_SDM->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO_COMP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkPD_VCO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_G->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIR_SXRSXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkRESET_N->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkSPDUP_VCO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkBYPLDO_VCO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_COARSEPLL->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkCURLIM_VCO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkREVPH_PFD->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_INTONLY_SDM->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_SDM_CLK->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkREV_SDMCLK->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkSX_DITHER_EN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	chkEN_DIV2_DIVPROGenabled->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::OnDIV2PrescalerChange ), NULL, this );
	cmbTST_SX->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	ctrPW_DIV2_LOCH->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	ctrPW_DIV4_LOCH->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	ctrDIV_LOCH->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbCP2_PLL->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbCP3_PLL->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbCZ->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	rgrSEL_VCO->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	btnCalculate->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnCalculateClick ), NULL, this );
	btnTune->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnTuneClick ), NULL, this );
	chkEnableRefSpurCancelation->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlSX_view::OnEnableRefSpurCancelation ), NULL, this );
	btnChangeRefClk->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnChangeRefClkClick ), NULL, this );
	btnUpdateValues->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnbtnReadComparators ), NULL, this );
	ctrCSW_VCO->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbICT_VCO->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	btnShowVCOparams->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlSX_view::OnShowVCOclicked ), NULL, this );
	cmbVDIV_VCO->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbIPULSE_CP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbSEL_SDMCLK->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbRSEL_LDO_VCO->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	cmbIOFFSET_CP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlSX_view::ParameterChangeHandler ), NULL, this );
	
}

pnlLimeLightPAD_view::pnlLimeLightPAD_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer105;
	fgSizer105 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer105->SetFlexibleDirection( wxBOTH );
	fgSizer105->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_NOTEBOOK1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	ID_PANEL1 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgMainSizer;
	fgMainSizer = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgMainSizer->SetFlexibleDirection( wxBOTH );
	fgMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer195;
	fgSizer195 = new wxFlexGridSizer( 0, 1, 5, 0 );
	fgSizer195->SetFlexibleDirection( wxBOTH );
	fgSizer195->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerEngagePullUp;
	sbSizerEngagePullUp = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Engage pull up control") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer107;
	fgSizer107 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer107->SetFlexibleDirection( wxBOTH );
	fgSizer107->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkSDA_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SDA_PE, wxT("SDA pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSDA_PE->SetToolTip( wxT("Pull up control of SDA pad") );
	
	fgSizer107->Add( chkSDA_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSCL_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SCL_PE, wxT("SCL pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSCL_PE->SetToolTip( wxT("Pull up control of SCL pad") );
	
	fgSizer107->Add( chkSCL_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkRX_CLK_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_RX_CLK_PE, wxT("RX_CLK pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkRX_CLK_PE->SetToolTip( wxT("Pull up control of RX_CLK pad") );
	
	fgSizer107->Add( chkRX_CLK_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSDIO_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SDIO_PE, wxT("SDIO pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSDIO_PE->SetToolTip( wxT("Pull up control of SDIO pad") );
	
	fgSizer107->Add( chkSDIO_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSDO_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SDO_PE, wxT("SDO pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSDO_PE->SetToolTip( wxT("Pull up control of SDO pad") );
	
	fgSizer107->Add( chkSDO_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkTX_CLK_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_TX_CLK_PE, wxT("TX_CLK pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTX_CLK_PE->SetToolTip( wxT("Pull up control of TX_CLK pad") );
	
	fgSizer107->Add( chkTX_CLK_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSEN_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SEN_PE, wxT("SEN pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSEN_PE->SetToolTip( wxT("Pull up control of SEN pad") );
	
	fgSizer107->Add( chkSEN_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkDIQ1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_DIQ1_PE, wxT("DIQ1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkDIQ1_PE->SetToolTip( wxT("Pull up control of DIQ1 pad") );
	
	fgSizer107->Add( chkDIQ1_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkTXNRX1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_TXNRX1_PE, wxT("TXNRX1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTXNRX1_PE->SetToolTip( wxT("Pull up control of TXNRX1 pad") );
	
	fgSizer107->Add( chkTXNRX1_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSCLK_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SCLK_PE, wxT("CLK pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSCLK_PE->SetToolTip( wxT("Pull up control of SCLK pad") );
	
	fgSizer107->Add( chkSCLK_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkDIQ2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_DIQ2_PE, wxT("DIQ2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkDIQ2_PE->SetToolTip( wxT("Pull up control of DIQ2 pad") );
	
	fgSizer107->Add( chkDIQ2_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkTXNRX2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_TXNRX2_PE, wxT("TXNRX2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTXNRX2_PE->SetValue(true); 
	chkTXNRX2_PE->SetToolTip( wxT("Pull up control of TXNRX2 pad") );
	
	fgSizer107->Add( chkTXNRX2_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkIQ_SEL_EN_1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_IQ_SEL_EN_1_PE, wxT("IQ_SEL_EN_1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkIQ_SEL_EN_1_PE->SetToolTip( wxT("Pull up control of IQ_SEL_EN_1 pad") );
	
	fgSizer107->Add( chkIQ_SEL_EN_1_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkFCLK1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_FCLK1_PE, wxT("FCLK1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFCLK1_PE->SetToolTip( wxT("Pull up control of FCLK1 pad") );
	
	fgSizer107->Add( chkFCLK1_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkMCLK1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_MCLK1_PE, wxT("MCLK1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMCLK1_PE->SetToolTip( wxT("Pull up control of MCLK1 pad") );
	
	fgSizer107->Add( chkMCLK1_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkIQ_SEL_EN_2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_IQ_SEL_EN_2_PE, wxT("IQ_SEL_EN_2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkIQ_SEL_EN_2_PE->SetToolTip( wxT("Pull up control of IQ_SEL_EN_2 pad") );
	
	fgSizer107->Add( chkIQ_SEL_EN_2_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkFCLK2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_FCLK2_PE, wxT("FCLK2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFCLK2_PE->SetToolTip( wxT("Pull up control of FCLK2 pad") );
	
	fgSizer107->Add( chkFCLK2_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkMCLK2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_MCLK2_PE, wxT("MCLK2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMCLK2_PE->SetToolTip( wxT("Pull up control of MCLK2 pad") );
	
	fgSizer107->Add( chkMCLK2_PE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerEngagePullUp->Add( fgSizer107, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer195->Add( sbSizerEngagePullUp, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizerResetSignals;
	sbSizerResetSignals = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Reset signals") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer196;
	fgSizer196 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer196->AddGrowableCol( 0 );
	fgSizer196->SetFlexibleDirection( wxBOTH );
	fgSizer196->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer197;
	fgSizer197 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer197->AddGrowableCol( 0 );
	fgSizer197->AddGrowableCol( 1 );
	fgSizer197->SetFlexibleDirection( wxBOTH );
	fgSizer197->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkSRST_RXFIFO = new wxCheckBox( sbSizerResetSignals->GetStaticBox(), ID_SRST_RXFIFO, wxT("Rx FIFO soft reset"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSRST_RXFIFO->SetToolTip( wxT("RX FIFO soft reset (LimeLight Interface)") );
	
	fgSizer197->Add( chkSRST_RXFIFO, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSRST_TXFIFO = new wxCheckBox( sbSizerResetSignals->GetStaticBox(), ID_SRST_TXFIFO, wxT("Tx FIFO soft reset"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSRST_TXFIFO->SetToolTip( wxT("TX FIFO soft reset (LimeLight Interface)") );
	
	fgSizer197->Add( chkSRST_TXFIFO, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer196->Add( fgSizer197, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer198;
	fgSizer198 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer198->AddGrowableCol( 0 );
	fgSizer198->AddGrowableCol( 1 );
	fgSizer198->SetFlexibleDirection( wxBOTH );
	fgSizer198->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerLogicRegistersReset;
	sbSizerLogicRegistersReset = new wxStaticBoxSizer( new wxStaticBox( sbSizerResetSignals->GetStaticBox(), wxID_ANY, wxT("Logic registers") ), wxVERTICAL );
	
	chkLRST_TX_A = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_TX_A, wxT("Tx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLRST_TX_A->SetToolTip( wxT("Resets all the logic registers to the default state for Tx MIMO channel A") );
	
	sbSizerLogicRegistersReset->Add( chkLRST_TX_A, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkLRST_TX_B = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_TX_B, wxT("Tx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLRST_TX_B->SetToolTip( wxT("Resets all the logic registers to the default state for Tx MIMO channel B") );
	
	sbSizerLogicRegistersReset->Add( chkLRST_TX_B, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkLRST_RX_A = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_RX_A, wxT("Rx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLRST_RX_A->SetToolTip( wxT("Resets all the logic registers to the default state for Rx MIMO channel A") );
	
	sbSizerLogicRegistersReset->Add( chkLRST_RX_A, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkLRST_RX_B = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_RX_B, wxT("Rx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLRST_RX_B->SetToolTip( wxT("Resets all the logic registers to the default state for Rx MIMO channel B") );
	
	sbSizerLogicRegistersReset->Add( chkLRST_RX_B, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer198->Add( sbSizerLogicRegistersReset, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerConfigurationMemoryReset;
	sbSizerConfigurationMemoryReset = new wxStaticBoxSizer( new wxStaticBox( sbSizerResetSignals->GetStaticBox(), wxID_ANY, wxT("Configuration memory") ), wxVERTICAL );
	
	chkMRST_TX_A = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_TX_A, wxT("Tx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMRST_TX_A->SetToolTip( wxT("Resets all the configuration memory to the default state for Tx MIMO channel A") );
	
	sbSizerConfigurationMemoryReset->Add( chkMRST_TX_A, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkMRST_TX_B = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_TX_B, wxT("Tx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMRST_TX_B->SetToolTip( wxT("Resets all the configuration memory to the default state for Tx MIMO channel B") );
	
	sbSizerConfigurationMemoryReset->Add( chkMRST_TX_B, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkMRST_RX_A = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_RX_A, wxT("Rx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMRST_RX_A->SetToolTip( wxT("Resets all the configuration memory to the default state for Rx MIMO channel A") );
	
	sbSizerConfigurationMemoryReset->Add( chkMRST_RX_A, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkMRST_RX_B = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_RX_B, wxT("Rx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMRST_RX_B->SetToolTip( wxT("Resets all the configuration memory to the default state for Rx MIMO channel B") );
	
	sbSizerConfigurationMemoryReset->Add( chkMRST_RX_B, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer198->Add( sbSizerConfigurationMemoryReset, 1, wxEXPAND, 5 );
	
	
	fgSizer196->Add( fgSizer198, 1, wxEXPAND, 5 );
	
	
	sbSizerResetSignals->Add( fgSizer196, 1, wxEXPAND, 5 );
	
	
	fgSizer195->Add( sbSizerResetSignals, 0, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerPowerControls;
	sbSizerPowerControls = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Power Control") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer113;
	fgSizer113 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer113->SetFlexibleDirection( wxBOTH );
	fgSizer113->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkRXEN_A = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_RXEN_A, wxT("Enable Rx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
	chkRXEN_A->SetToolTip( wxT("Power control for Rx MIMO channel A") );
	
	fgSizer113->Add( chkRXEN_A, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkTXEN_A = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_TXEN_A, wxT("Enable Tx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTXEN_A->SetToolTip( wxT("Power control for Tx MIMO channel A") );
	
	fgSizer113->Add( chkTXEN_A, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkRXEN_B = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_RXEN_B, wxT("Enable Rx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
	chkRXEN_B->SetToolTip( wxT("Power control for Rx MIMO channel B") );
	
	fgSizer113->Add( chkRXEN_B, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkTXEN_B = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_TXEN_B, wxT("Enable Tx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTXEN_B->SetToolTip( wxT("Power control for Tx MIMO channel B") );
	
	fgSizer113->Add( chkTXEN_B, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerPowerControls->Add( fgSizer113, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer195->Add( sbSizerPowerControls, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	chkMIMO_SISO = new wxCheckBox( ID_PANEL1, ID_MIMO_SISO, wxT("Disable MIMO channel B"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMIMO_SISO->SetValue(true); 
	fgSizer195->Add( chkMIMO_SISO, 0, wxLEFT, 5 );
	
	
	fgMainSizer->Add( fgSizer195, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer199;
	fgSizer199 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer199->AddGrowableCol( 0 );
	fgSizer199->SetFlexibleDirection( wxBOTH );
	fgSizer199->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerLimeLightModes;
	sbSizerLimeLightModes = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("LimeLight modes") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer108;
	fgSizer108 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer108->AddGrowableCol( 0 );
	fgSizer108->AddGrowableCol( 1 );
	fgSizer108->SetFlexibleDirection( wxBOTH );
	fgSizer108->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkMOD_EN = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_MOD_EN, wxT("Enable LimeLight interface"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMOD_EN->SetToolTip( wxT("LimeLight interface enable") );
	
	fgSizer108->Add( chkMOD_EN, 0, wxEXPAND|wxALIGN_LEFT|wxLEFT, 5 );
	
	
	fgSizer108->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString rgrLML_FIDM1Choices[] = { wxT("0"), wxT("1") };
	int rgrLML_FIDM1NChoices = sizeof( rgrLML_FIDM1Choices ) / sizeof( wxString );
	rgrLML_FIDM1 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_FIDM1, wxT("Frame start for Port 1"), wxDefaultPosition, wxDefaultSize, rgrLML_FIDM1NChoices, rgrLML_FIDM1Choices, 2, wxRA_SPECIFY_COLS );
	rgrLML_FIDM1->SetSelection( 0 );
	rgrLML_FIDM1->SetToolTip( wxT("Frame start ID selection for Port 1 when LML_MODE1 = 0") );
	
	fgSizer108->Add( rgrLML_FIDM1, 0, wxEXPAND|wxALIGN_LEFT, 0 );
	
	wxString rgrLML_FIDM2Choices[] = { wxT("0"), wxT("1") };
	int rgrLML_FIDM2NChoices = sizeof( rgrLML_FIDM2Choices ) / sizeof( wxString );
	rgrLML_FIDM2 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_FIDM2, wxT("Frame start for Port 2"), wxDefaultPosition, wxDefaultSize, rgrLML_FIDM2NChoices, rgrLML_FIDM2Choices, 2, wxRA_SPECIFY_COLS );
	rgrLML_FIDM2->SetSelection( 1 );
	rgrLML_FIDM2->SetToolTip( wxT("Frame start ID selection for Port 2 when LML_MODE2 = 0") );
	
	fgSizer108->Add( rgrLML_FIDM2, 0, wxALIGN_LEFT|wxEXPAND, 0 );
	
	wxString rgrLML_MODE1Choices[] = { wxT("TRXIQ"), wxT("JESD207") };
	int rgrLML_MODE1NChoices = sizeof( rgrLML_MODE1Choices ) / sizeof( wxString );
	rgrLML_MODE1 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_MODE1, wxT("LimeLight port 1 mode"), wxDefaultPosition, wxDefaultSize, rgrLML_MODE1NChoices, rgrLML_MODE1Choices, 2, wxRA_SPECIFY_COLS );
	rgrLML_MODE1->SetSelection( 0 );
	rgrLML_MODE1->SetToolTip( wxT("Mode of LimeLight Port 1") );
	
	fgSizer108->Add( rgrLML_MODE1, 0, wxEXPAND|wxALIGN_LEFT, 0 );
	
	wxString rgrLML_MODE2Choices[] = { wxT("TRXIQ"), wxT("JESD207") };
	int rgrLML_MODE2NChoices = sizeof( rgrLML_MODE2Choices ) / sizeof( wxString );
	rgrLML_MODE2 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_MODE2, wxT("LimeLight port 2 mode"), wxDefaultPosition, wxDefaultSize, rgrLML_MODE2NChoices, rgrLML_MODE2Choices, 2, wxRA_SPECIFY_COLS );
	rgrLML_MODE2->SetSelection( 0 );
	rgrLML_MODE2->SetToolTip( wxT("Mode of LimeLight Port 2") );
	
	fgSizer108->Add( rgrLML_MODE2, 0, wxEXPAND|wxALIGN_LEFT, 0 );
	
	wxString rgrLML_TXNRXIQ1Choices[] = { wxT("RXIQ"), wxT("TXIQ") };
	int rgrLML_TXNRXIQ1NChoices = sizeof( rgrLML_TXNRXIQ1Choices ) / sizeof( wxString );
	rgrLML_TXNRXIQ1 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_TXNRXIQ1, wxT("Port 1 mode selection"), wxDefaultPosition, wxDefaultSize, rgrLML_TXNRXIQ1NChoices, rgrLML_TXNRXIQ1Choices, 2, wxRA_SPECIFY_COLS );
	rgrLML_TXNRXIQ1->SetSelection( 0 );
	rgrLML_TXNRXIQ1->SetToolTip( wxT("TXIQ/RXIQ mode selection for Port 1 when LML_MODE1 = 0") );
	
	fgSizer108->Add( rgrLML_TXNRXIQ1, 0, wxEXPAND|wxALIGN_LEFT, 0 );
	
	wxString rgrLML_TXNRXIQ2Choices[] = { wxT("RXIQ"), wxT("TXIQ") };
	int rgrLML_TXNRXIQ2NChoices = sizeof( rgrLML_TXNRXIQ2Choices ) / sizeof( wxString );
	rgrLML_TXNRXIQ2 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_TXNRXIQ2, wxT("Port 2 mode selection"), wxDefaultPosition, wxDefaultSize, rgrLML_TXNRXIQ2NChoices, rgrLML_TXNRXIQ2Choices, 2, wxRA_SPECIFY_COLS );
	rgrLML_TXNRXIQ2->SetSelection( 0 );
	rgrLML_TXNRXIQ2->SetToolTip( wxT("TXIQ/RXIQ mode selection for Port 2 when LML_MODE2 = 0") );
	
	fgSizer108->Add( rgrLML_TXNRXIQ2, 0, wxEXPAND|wxALIGN_LEFT, 0 );
	
	chkLML1_TRXIQPULSE = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML1 TRXIQPULSE mode"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLML1_TRXIQPULSE->SetToolTip( wxT("Power control for Tx MIMO channel B") );
	
	fgSizer108->Add( chkLML1_TRXIQPULSE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	chkLML2_TRXIQPULSE = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML2 TRXIQPULSE mode"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLML2_TRXIQPULSE->SetToolTip( wxT("Power control for Tx MIMO channel B") );
	
	fgSizer108->Add( chkLML2_TRXIQPULSE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	chkLML1_SISODDR = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML1 SISODDR mode"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLML1_SISODDR->SetToolTip( wxT("Power control for Tx MIMO channel B") );
	
	fgSizer108->Add( chkLML1_SISODDR, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	chkLML2_SISODDR = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML2 SISODDR mode"), wxDefaultPosition, wxDefaultSize, 0 );
	chkLML2_SISODDR->SetToolTip( wxT("Power control for Tx MIMO channel B") );
	
	fgSizer108->Add( chkLML2_SISODDR, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	
	sbSizerLimeLightModes->Add( fgSizer108, 0, wxEXPAND|wxALIGN_LEFT, 0 );
	
	
	fgSizer199->Add( sbSizerLimeLightModes, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerDriverStrength;
	sbSizerDriverStrength = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Driver strength") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer111;
	fgSizer111 = new wxFlexGridSizer( 0, 5, 0, 0 );
	fgSizer111->SetFlexibleDirection( wxBOTH );
	fgSizer111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rgrSDA_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
	int rgrSDA_DSNChoices = sizeof( rgrSDA_DSChoices ) / sizeof( wxString );
	rgrSDA_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_SDA_DS, wxT("SDA pad"), wxDefaultPosition, wxDefaultSize, rgrSDA_DSNChoices, rgrSDA_DSChoices, 1, wxRA_SPECIFY_COLS );
	rgrSDA_DS->SetSelection( 0 );
	rgrSDA_DS->SetToolTip( wxT("Driver strength of SDA pad") );
	
	fgSizer111->Add( rgrSDA_DS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrSCL_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
	int rgrSCL_DSNChoices = sizeof( rgrSCL_DSChoices ) / sizeof( wxString );
	rgrSCL_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_SCL_DS, wxT("SCL pad"), wxDefaultPosition, wxDefaultSize, rgrSCL_DSNChoices, rgrSCL_DSChoices, 1, wxRA_SPECIFY_COLS );
	rgrSCL_DS->SetSelection( 0 );
	rgrSCL_DS->SetToolTip( wxT("Driver strength of SCL pad") );
	
	fgSizer111->Add( rgrSCL_DS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrSDIO_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
	int rgrSDIO_DSNChoices = sizeof( rgrSDIO_DSChoices ) / sizeof( wxString );
	rgrSDIO_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_SDIO_DS, wxT("SDIO pad"), wxDefaultPosition, wxDefaultSize, rgrSDIO_DSNChoices, rgrSDIO_DSChoices, 1, wxRA_SPECIFY_COLS );
	rgrSDIO_DS->SetSelection( 0 );
	rgrSDIO_DS->SetToolTip( wxT("Driver strength of SDIO pad") );
	
	fgSizer111->Add( rgrSDIO_DS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrDIQ2_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
	int rgrDIQ2_DSNChoices = sizeof( rgrDIQ2_DSChoices ) / sizeof( wxString );
	rgrDIQ2_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_DIQ2_DS, wxT("DIQ2 pad"), wxDefaultPosition, wxDefaultSize, rgrDIQ2_DSNChoices, rgrDIQ2_DSChoices, 1, wxRA_SPECIFY_COLS );
	rgrDIQ2_DS->SetSelection( 0 );
	rgrDIQ2_DS->SetToolTip( wxT("Driver strength of DIQ2 pad") );
	
	fgSizer111->Add( rgrDIQ2_DS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrDIQ1_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
	int rgrDIQ1_DSNChoices = sizeof( rgrDIQ1_DSChoices ) / sizeof( wxString );
	rgrDIQ1_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_DIQ1_DS, wxT("DIQ1 pad"), wxDefaultPosition, wxDefaultSize, rgrDIQ1_DSNChoices, rgrDIQ1_DSChoices, 1, wxRA_SPECIFY_COLS );
	rgrDIQ1_DS->SetSelection( 0 );
	rgrDIQ1_DS->SetToolTip( wxT("Pull up control of MCLK2 pad") );
	
	fgSizer111->Add( rgrDIQ1_DS, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerDriverStrength->Add( fgSizer111, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer199->Add( sbSizerDriverStrength, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgMainSizer->Add( fgSizer199, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer222;
	fgSizer222 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer222->SetFlexibleDirection( wxBOTH );
	fgSizer222->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer202;
	fgSizer202 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer202->AddGrowableCol( 0 );
	fgSizer202->SetFlexibleDirection( wxBOTH );
	fgSizer202->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rgrRX_MUXChoices[] = { wxT("RxTSP"), wxT("TxFIFO"), wxT("LFSR") };
	int rgrRX_MUXNChoices = sizeof( rgrRX_MUXChoices ) / sizeof( wxString );
	rgrRX_MUX = new wxRadioBox( ID_PANEL1, ID_RX_MUX, wxT("RxFIFO source"), wxDefaultPosition, wxDefaultSize, rgrRX_MUXNChoices, rgrRX_MUXChoices, 1, wxRA_SPECIFY_COLS );
	rgrRX_MUX->SetSelection( 0 );
	fgSizer202->Add( rgrRX_MUX, 0, wxEXPAND, 5 );
	
	wxString rgrTX_MUXChoices[] = { wxT("Port1"), wxT("Port2"), wxT("RxTSP") };
	int rgrTX_MUXNChoices = sizeof( rgrTX_MUXChoices ) / sizeof( wxString );
	rgrTX_MUX = new wxRadioBox( ID_PANEL1, ID_TX_MUX, wxT("Data transmit port"), wxDefaultPosition, wxDefaultSize, rgrTX_MUXNChoices, rgrTX_MUXChoices, 1, wxRA_SPECIFY_COLS );
	rgrTX_MUX->SetSelection( 0 );
	fgSizer202->Add( rgrTX_MUX, 0, wxEXPAND, 5 );
	
	wxString rgrSPIMODEChoices[] = { wxT("3 wire mode"), wxT("4 wire mode") };
	int rgrSPIMODENChoices = sizeof( rgrSPIMODEChoices ) / sizeof( wxString );
	rgrSPIMODE = new wxRadioBox( ID_PANEL1, ID_SPIMODE, wxT("SPI mode"), wxDefaultPosition, wxDefaultSize, rgrSPIMODENChoices, rgrSPIMODEChoices, 1, wxRA_SPECIFY_COLS );
	rgrSPIMODE->SetSelection( 0 );
	rgrSPIMODE->SetToolTip( wxT("SPI communication mode") );
	
	fgSizer202->Add( rgrSPIMODE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0 );
	
	
	fgSizer222->Add( fgSizer202, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer114;
	fgSizer114 = new wxFlexGridSizer( 0, 6, 5, 5 );
	fgSizer114->SetFlexibleDirection( wxBOTH );
	fgSizer114->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT36 = new wxStaticText( ID_PANEL1, wxID_ANY, wxT("Ver:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT36->Wrap( -1 );
	fgSizer114->Add( ID_STATICTEXT36, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblVER = new wxStaticText( ID_PANEL1, ID_VER, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVER->Wrap( -1 );
	fgSizer114->Add( lblVER, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT38 = new wxStaticText( ID_PANEL1, wxID_ANY, wxT("Rev:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT38->Wrap( -1 );
	fgSizer114->Add( ID_STATICTEXT38, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblREV = new wxStaticText( ID_PANEL1, ID_REV, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblREV->Wrap( -1 );
	fgSizer114->Add( lblREV, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT40 = new wxStaticText( ID_PANEL1, wxID_ANY, wxT("Mask:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT40->Wrap( -1 );
	fgSizer114->Add( ID_STATICTEXT40, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblMASK = new wxStaticText( ID_PANEL1, ID_MASK, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMASK->Wrap( -1 );
	fgSizer114->Add( lblMASK, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer222->Add( fgSizer114, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	btnReadVerRevMask = new wxButton( ID_PANEL1, wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer222->Add( btnReadVerRevMask, 0, 0, 5 );
	
	
	fgMainSizer->Add( fgSizer222, 1, wxEXPAND, 5 );
	
	
	ID_PANEL1->SetSizer( fgMainSizer );
	ID_PANEL1->Layout();
	fgMainSizer->Fit( ID_PANEL1 );
	ID_NOTEBOOK1->AddPage( ID_PANEL1, wxT("Modes"), true );
	ID_PANEL2 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer115;
	fgSizer115 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer115->SetFlexibleDirection( wxBOTH );
	fgSizer115->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer224;
	fgSizer224 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer224->SetFlexibleDirection( wxBOTH );
	fgSizer224->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer117;
	fgSizer117 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer117->SetFlexibleDirection( wxBOTH );
	fgSizer117->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerSampleSource;
	sbSizerSampleSource = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Sample source") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer87;
	sbSizer87 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSampleSource->GetStaticBox(), wxID_ANY, wxT("When Port 1 is RF2BB") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer118;
	fgSizer118 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer118->AddGrowableCol( 1 );
	fgSizer118->SetFlexibleDirection( wxBOTH );
	fgSizer118->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 3"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer118->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_S3S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S3S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML1_S3S->Append( wxT("AI") );
	cmbLML1_S3S->Append( wxT("AQ") );
	cmbLML1_S3S->Append( wxT("BI") );
	cmbLML1_S3S->Append( wxT("BQ") );
	cmbLML1_S3S->SetSelection( 0 );
	cmbLML1_S3S->SetToolTip( wxT("Sample source in position 3, when Port 1 is TX") );
	
	fgSizer118->Add( cmbLML1_S3S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT2 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 2"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer118->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_S2S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S2S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML1_S2S->Append( wxT("AI") );
	cmbLML1_S2S->Append( wxT("AQ") );
	cmbLML1_S2S->Append( wxT("BI") );
	cmbLML1_S2S->Append( wxT("BQ") );
	cmbLML1_S2S->SetSelection( 0 );
	cmbLML1_S2S->SetToolTip( wxT("Sample source in position 2, when Port 1 is TX") );
	
	fgSizer118->Add( cmbLML1_S2S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 1"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer118->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_S1S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S1S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML1_S1S->Append( wxT("AI") );
	cmbLML1_S1S->Append( wxT("AQ") );
	cmbLML1_S1S->Append( wxT("BI") );
	cmbLML1_S1S->Append( wxT("BQ") );
	cmbLML1_S1S->SetSelection( 0 );
	cmbLML1_S1S->SetToolTip( wxT("Sample source in position 1, when Port 1 is TX") );
	
	fgSizer118->Add( cmbLML1_S1S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 0"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer118->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_S0S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S0S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML1_S0S->Append( wxT("AI") );
	cmbLML1_S0S->Append( wxT("AQ") );
	cmbLML1_S0S->Append( wxT("BI") );
	cmbLML1_S0S->Append( wxT("BQ") );
	cmbLML1_S0S->SetSelection( 0 );
	cmbLML1_S0S->SetToolTip( wxT("Sample source in position 0, when Port 1 is TX") );
	
	fgSizer118->Add( cmbLML1_S0S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer87->Add( fgSizer118, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerSampleSource->Add( sbSizer87, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer89;
	sbSizer89 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSampleSource->GetStaticBox(), wxID_ANY, wxT("When Port 2 is RF2BB") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer120;
	fgSizer120 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer120->AddGrowableCol( 1 );
	fgSizer120->SetFlexibleDirection( wxBOTH );
	fgSizer120->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 3"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer120->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_S3S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S3S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML2_S3S->Append( wxT("AI") );
	cmbLML2_S3S->Append( wxT("AQ") );
	cmbLML2_S3S->Append( wxT("BI") );
	cmbLML2_S3S->Append( wxT("BQ") );
	cmbLML2_S3S->SetSelection( 0 );
	cmbLML2_S3S->SetToolTip( wxT("Sample source in position 3, when Port 2 is TX") );
	
	fgSizer120->Add( cmbLML2_S3S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 2"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer120->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_S2S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S2S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML2_S2S->Append( wxT("AI") );
	cmbLML2_S2S->Append( wxT("AQ") );
	cmbLML2_S2S->Append( wxT("BI") );
	cmbLML2_S2S->Append( wxT("BQ") );
	cmbLML2_S2S->SetSelection( 0 );
	cmbLML2_S2S->SetToolTip( wxT("Sample source in position 2, when Port 2 is TX") );
	
	fgSizer120->Add( cmbLML2_S2S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 1"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer120->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_S1S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S1S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML2_S1S->Append( wxT("AI") );
	cmbLML2_S1S->Append( wxT("AQ") );
	cmbLML2_S1S->Append( wxT("BI") );
	cmbLML2_S1S->Append( wxT("BQ") );
	cmbLML2_S1S->SetSelection( 0 );
	cmbLML2_S1S->SetToolTip( wxT("Sample source in position 1, when Port 2 is TX") );
	
	fgSizer120->Add( cmbLML2_S1S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 0"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer120->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_S0S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S0S, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0, NULL, 0 );
	cmbLML2_S0S->Append( wxT("AI") );
	cmbLML2_S0S->Append( wxT("AQ") );
	cmbLML2_S0S->Append( wxT("BI") );
	cmbLML2_S0S->Append( wxT("BQ") );
	cmbLML2_S0S->SetSelection( 0 );
	cmbLML2_S0S->SetToolTip( wxT("Sample source in position 0, when Port 2 is TX") );
	
	fgSizer120->Add( cmbLML2_S0S, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer89->Add( fgSizer120, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerSampleSource->Add( sbSizer89, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer117->Add( sbSizerSampleSource, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerSamplePositions;
	sbSizerSamplePositions = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Sample positions") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer88;
	sbSizer88 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSamplePositions->GetStaticBox(), wxID_ANY, wxT("When Port 1 is BB2RF") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer119;
	fgSizer119 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer119->SetFlexibleDirection( wxBOTH );
	fgSizer119->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("BQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer119->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_BQP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_BQP, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 );
	cmbLML1_BQP->Append( wxT("position 0") );
	cmbLML1_BQP->Append( wxT("position 1") );
	cmbLML1_BQP->Append( wxT("position 2") );
	cmbLML1_BQP->Append( wxT("position 3") );
	cmbLML1_BQP->SetToolTip( wxT("BQ sample position in frame, when Port 1 is RX") );
	
	fgSizer119->Add( cmbLML1_BQP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("BI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgSizer119->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_BIP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_BIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
	cmbLML1_BIP->Append( wxT("position 0") );
	cmbLML1_BIP->Append( wxT("position 1") );
	cmbLML1_BIP->Append( wxT("position 2") );
	cmbLML1_BIP->Append( wxT("position 3") );
	cmbLML1_BIP->SetToolTip( wxT("BI sample position in frame, when Port 1 is RX") );
	
	fgSizer119->Add( cmbLML1_BIP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("AQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer119->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_AQP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_AQP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
	cmbLML1_AQP->Append( wxT("position 0") );
	cmbLML1_AQP->Append( wxT("position 1") );
	cmbLML1_AQP->Append( wxT("position 2") );
	cmbLML1_AQP->Append( wxT("position 3") );
	cmbLML1_AQP->SetToolTip( wxT("AQ sample position in frame, when Port 1 is RX") );
	
	fgSizer119->Add( cmbLML1_AQP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT8 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("AI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer119->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_AIP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_AIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
	cmbLML1_AIP->Append( wxT("position 0") );
	cmbLML1_AIP->Append( wxT("position 1") );
	cmbLML1_AIP->Append( wxT("position 2") );
	cmbLML1_AIP->Append( wxT("position 3") );
	cmbLML1_AIP->SetToolTip( wxT("AI sample position in frame, when Port 1 is RX") );
	
	fgSizer119->Add( cmbLML1_AIP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer88->Add( fgSizer119, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerSamplePositions->Add( sbSizer88, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer90;
	sbSizer90 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSamplePositions->GetStaticBox(), wxID_ANY, wxT("When Port 2 is BB2RF") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer121;
	fgSizer121 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer121->SetFlexibleDirection( wxBOTH );
	fgSizer121->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("BQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer121->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_BQP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_BQP, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 );
	cmbLML2_BQP->Append( wxT("position 0") );
	cmbLML2_BQP->Append( wxT("position 1") );
	cmbLML2_BQP->Append( wxT("position 2") );
	cmbLML2_BQP->Append( wxT("position 3") );
	cmbLML2_BQP->SetToolTip( wxT("BQ sample position in frame, when Port 2 is RX") );
	
	fgSizer121->Add( cmbLML2_BQP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("BI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer121->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_BIP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_BIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
	cmbLML2_BIP->Append( wxT("position 0") );
	cmbLML2_BIP->Append( wxT("position 1") );
	cmbLML2_BIP->Append( wxT("position 2") );
	cmbLML2_BIP->Append( wxT("position 3") );
	cmbLML2_BIP->SetToolTip( wxT("BI sample position in frame, when Port 2is RX") );
	
	fgSizer121->Add( cmbLML2_BIP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("AQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( -1 );
	fgSizer121->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_AQP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_AQP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
	cmbLML2_AQP->Append( wxT("position 0") );
	cmbLML2_AQP->Append( wxT("position 1") );
	cmbLML2_AQP->Append( wxT("position 2") );
	cmbLML2_AQP->Append( wxT("position 3") );
	cmbLML2_AQP->SetToolTip( wxT("AQ sample position in frame, when Port 2 is RX") );
	
	fgSizer121->Add( cmbLML2_AQP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("AI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer121->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_AIP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_AIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
	cmbLML2_AIP->Append( wxT("position 0") );
	cmbLML2_AIP->Append( wxT("position 1") );
	cmbLML2_AIP->Append( wxT("position 2") );
	cmbLML2_AIP->Append( wxT("position 3") );
	cmbLML2_AIP->SetToolTip( wxT("AI sample position in frame, when Port 2 is RX") );
	
	fgSizer121->Add( cmbLML2_AIP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer90->Add( fgSizer121, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizerSamplePositions->Add( sbSizer90, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer117->Add( sbSizerSamplePositions, 1, wxEXPAND, 5 );
	
	
	fgSizer224->Add( fgSizer117, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerClockControls;
	sbSizerClockControls = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Clock controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer122;
	fgSizer122 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer122->SetFlexibleDirection( wxBOTH );
	fgSizer122->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT25 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("TX FIFO read clock source"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT25->Wrap( -1 );
	fgSizer122->Add( ID_STATICTEXT25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbTXRDCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_TXRDCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbTXRDCLK_MUX->Append( wxT("FCLK1") );
	cmbTXRDCLK_MUX->Append( wxT("FCLK2") );
	cmbTXRDCLK_MUX->Append( wxT("TxTSPCLK") );
	cmbTXRDCLK_MUX->SetToolTip( wxT("TX FIFO read clock selection") );
	
	fgSizer122->Add( cmbTXRDCLK_MUX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT26 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("Tx FIFO write clock source"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT26->Wrap( -1 );
	fgSizer122->Add( ID_STATICTEXT26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbTXWRCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_TXWRCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbTXWRCLK_MUX->Append( wxT("FCLK1") );
	cmbTXWRCLK_MUX->Append( wxT("FCLK2") );
	cmbTXWRCLK_MUX->Append( wxT("RxTSPCLK") );
	cmbTXWRCLK_MUX->SetToolTip( wxT("TX FIFO write clock selection") );
	
	fgSizer122->Add( cmbTXWRCLK_MUX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT27 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("Rx FIFO read clock source"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT27->Wrap( -1 );
	fgSizer122->Add( ID_STATICTEXT27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRXRDCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_RXRDCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbRXRDCLK_MUX->Append( wxT("MCLK1") );
	cmbRXRDCLK_MUX->Append( wxT("MCLK2") );
	cmbRXRDCLK_MUX->Append( wxT("FCLK1") );
	cmbRXRDCLK_MUX->Append( wxT("FCLK2") );
	cmbRXRDCLK_MUX->SetToolTip( wxT("RX FIFO read clock selection") );
	
	fgSizer122->Add( cmbRXRDCLK_MUX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT28 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("Rx FIFO write clock source"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT28->Wrap( -1 );
	fgSizer122->Add( ID_STATICTEXT28, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRXWRCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_RXWRCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbRXWRCLK_MUX->Append( wxT("FCLK1") );
	cmbRXWRCLK_MUX->Append( wxT("FCLK2") );
	cmbRXWRCLK_MUX->Append( wxT("RxTSPCLK") );
	cmbRXWRCLK_MUX->SetToolTip( wxT("RX FIFO write clock selection") );
	
	fgSizer122->Add( cmbRXWRCLK_MUX, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT29 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK2 clock source"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT29->Wrap( -1 );
	fgSizer122->Add( ID_STATICTEXT29, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbMCLK2SRC = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK2SRC, wxEmptyString, wxDefaultPosition, wxSize( 154,-1 ), 0, NULL, 0 );
	cmbMCLK2SRC->Append( wxT("TxTSPCLKA after divider") );
	cmbMCLK2SRC->Append( wxT("RxTSPCLKA after divider") );
	cmbMCLK2SRC->Append( wxT("TxTSPCLKA") );
	cmbMCLK2SRC->Append( wxT("RxTSPCLKA") );
	cmbMCLK2SRC->SetToolTip( wxT("MCLK2 clock source") );
	
	fgSizer122->Add( cmbMCLK2SRC, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT30 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK1 clock source"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT30->Wrap( -1 );
	fgSizer122->Add( ID_STATICTEXT30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbMCLK1SRC = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK1SRC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbMCLK1SRC->Append( wxT("TxTSPCLKA after divider") );
	cmbMCLK1SRC->Append( wxT("RxTSPCLKA after divider") );
	cmbMCLK1SRC->Append( wxT("TxTSPCLKA") );
	cmbMCLK1SRC->Append( wxT("RxTSPCLKA") );
	cmbMCLK1SRC->SetToolTip( wxT("MCLK1 clock source") );
	
	fgSizer122->Add( cmbMCLK1SRC, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkTXDIVEN = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_TXDIVEN, wxT("Enable Tx clock divider"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTXDIVEN->SetToolTip( wxT("TX clock divider enable") );
	
	fgSizer122->Add( chkTXDIVEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbTXTSPCLKA_DIV = new wxSpinCtrl( sbSizerClockControls->GetStaticBox(), ID_TXTSPCLKA_DIV, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer122->Add( cmbTXTSPCLKA_DIV, 0, wxEXPAND, 5 );
	
	chkRXDIVEN = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_RXDIVEN, wxT("Enable Rx clock divider"), wxDefaultPosition, wxDefaultSize, 0 );
	chkRXDIVEN->SetToolTip( wxT("RX clock divider enable") );
	
	fgSizer122->Add( chkRXDIVEN, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbRXTSPCLKA_DIV = new wxSpinCtrl( sbSizerClockControls->GetStaticBox(), ID_RXTSPCLKA_DIV, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer122->Add( cmbRXTSPCLKA_DIV, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT37 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK1DLY"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT37->Wrap( -1 );
	ID_STATICTEXT37->Hide();
	
	fgSizer122->Add( ID_STATICTEXT37, 1, wxALIGN_LEFT|wxALIGN_BOTTOM, 5 );
	
	cmbMCLK1DLY = new wxSpinCtrl( sbSizerClockControls->GetStaticBox(), ID_MCLK1DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbMCLK1DLY->Hide();
	
	fgSizer122->Add( cmbMCLK1DLY, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT39 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK2DLY"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT39->Wrap( -1 );
	ID_STATICTEXT39->Hide();
	
	fgSizer122->Add( ID_STATICTEXT39, 1, wxALIGN_LEFT|wxALIGN_BOTTOM, 5 );
	
	cmbMCLK2DLY = new wxSpinCtrl( sbSizerClockControls->GetStaticBox(), ID_MCLK2DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbMCLK2DLY->Hide();
	
	fgSizer122->Add( cmbMCLK2DLY, 0, wxEXPAND, 5 );
	
	chkFCLK1_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK1_INV, wxT("FCLK1 invert"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFCLK1_INV->SetToolTip( wxT("FCLK1 clock inversion") );
	
	fgSizer122->Add( chkFCLK1_INV, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	chkFCLK2_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2_INV, wxT("FCLK2 invert"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFCLK2_INV->SetToolTip( wxT("FCLK2 clock inversion") );
	
	fgSizer122->Add( chkFCLK2_INV, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	chkMCLK1_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2_INV, wxT("MCLK1 invert"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMCLK1_INV->SetToolTip( wxT("FCLK2 clock inversion") );
	
	fgSizer122->Add( chkMCLK1_INV, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
	
	chkMCLK2_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2_INV, wxT("MCLK2 invert"), wxDefaultPosition, wxDefaultSize, 0 );
	chkMCLK2_INV->SetToolTip( wxT("FCLK2 clock inversion") );
	
	fgSizer122->Add( chkMCLK2_INV, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
	
	
	sbSizerClockControls->Add( fgSizer122, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxFlexGridSizer* fgSizer251;
	fgSizer251 = new wxFlexGridSizer( 0, 4, 0, 5 );
	fgSizer251->SetFlexibleDirection( wxBOTH );
	fgSizer251->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT301 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("FCLK1 delay"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT301->Wrap( -1 );
	fgSizer251->Add( ID_STATICTEXT301, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
	
	cmbFCLK1_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_FCLK1DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbFCLK1_DLY->Append( wxT("No delay") );
	cmbFCLK1_DLY->Append( wxT("1x delay") );
	cmbFCLK1_DLY->Append( wxT("2x delay") );
	cmbFCLK1_DLY->Append( wxT("3x delay") );
	cmbFCLK1_DLY->SetToolTip( wxT("MCLK1 clock source") );
	
	fgSizer251->Add( cmbFCLK1_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	ID_STATICTEXT302 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("FCLK2 delay"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT302->Wrap( -1 );
	fgSizer251->Add( ID_STATICTEXT302, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
	
	cmbFCLK2_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbFCLK2_DLY->Append( wxT("No delay") );
	cmbFCLK2_DLY->Append( wxT("1x delay") );
	cmbFCLK2_DLY->Append( wxT("2x delay") );
	cmbFCLK2_DLY->Append( wxT("3x delay") );
	cmbFCLK2_DLY->SetToolTip( wxT("MCLK1 clock source") );
	
	fgSizer251->Add( cmbFCLK2_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	ID_STATICTEXT303 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK1 delay"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT303->Wrap( -1 );
	fgSizer251->Add( ID_STATICTEXT303, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
	
	cmbMCLK1_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK1DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbMCLK1_DLY->Append( wxT("No delay") );
	cmbMCLK1_DLY->Append( wxT("1x delay") );
	cmbMCLK1_DLY->Append( wxT("2x delay") );
	cmbMCLK1_DLY->Append( wxT("3x delay") );
	cmbMCLK1_DLY->SetToolTip( wxT("MCLK1 clock source") );
	
	fgSizer251->Add( cmbMCLK1_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	ID_STATICTEXT304 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK2 delay"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT304->Wrap( -1 );
	fgSizer251->Add( ID_STATICTEXT304, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
	
	cmbMCLK2_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK2DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbMCLK2_DLY->Append( wxT("No delay") );
	cmbMCLK2_DLY->Append( wxT("1x delay") );
	cmbMCLK2_DLY->Append( wxT("2x delay") );
	cmbMCLK2_DLY->Append( wxT("3x delay") );
	cmbMCLK2_DLY->SetToolTip( wxT("MCLK1 clock source") );
	
	fgSizer251->Add( cmbMCLK2_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	
	sbSizerClockControls->Add( fgSizer251, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	
	fgSizer224->Add( sbSizerClockControls, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 0 );
	
	wxFlexGridSizer* fgSizer124;
	fgSizer124 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer124->AddGrowableRow( 0 );
	fgSizer124->AddGrowableRow( 1 );
	fgSizer124->SetFlexibleDirection( wxBOTH );
	fgSizer124->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerClockCyclesToWaitLML1;
	sbSizerClockCyclesToWaitLML1 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("LML1 Clock cycles to wait before") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer125;
	fgSizer125 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer125->AddGrowableRow( 0 );
	fgSizer125->AddGrowableRow( 1 );
	fgSizer125->AddGrowableRow( 2 );
	fgSizer125->AddGrowableRow( 3 );
	fgSizer125->SetFlexibleDirection( wxBOTH );
	fgSizer125->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT17 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data drive stop"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT17->Wrap( -1 );
	fgSizer125->Add( ID_STATICTEXT17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_TX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_TX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML1_TX_PST->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer125->Add( cmbLML1_TX_PST, 0, 0, 5 );
	
	ID_STATICTEXT18 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data drive start"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT18->Wrap( -1 );
	fgSizer125->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_TX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_TX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML1_TX_PRE->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer125->Add( cmbLML1_TX_PRE, 0, 0, 5 );
	
	ID_STATICTEXT19 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data capture stop"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT19->Wrap( -1 );
	fgSizer125->Add( ID_STATICTEXT19, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_RX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_RX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML1_RX_PST->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer125->Add( cmbLML1_RX_PST, 0, 0, 5 );
	
	ID_STATICTEXT20 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data capture start"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT20->Wrap( -1 );
	fgSizer125->Add( ID_STATICTEXT20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML1_RX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_RX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML1_RX_PRE->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer125->Add( cmbLML1_RX_PRE, 0, 0, 5 );
	
	
	sbSizerClockCyclesToWaitLML1->Add( fgSizer125, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer124->Add( sbSizerClockCyclesToWaitLML1, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerClockCyclesToWaitLML2;
	sbSizerClockCyclesToWaitLML2 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("LML2 Clock cycles to wait before") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer126;
	fgSizer126 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer126->AddGrowableRow( 0 );
	fgSizer126->AddGrowableRow( 1 );
	fgSizer126->AddGrowableRow( 2 );
	fgSizer126->AddGrowableRow( 3 );
	fgSizer126->SetFlexibleDirection( wxBOTH );
	fgSizer126->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT21 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data drive stop"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT21->Wrap( -1 );
	fgSizer126->Add( ID_STATICTEXT21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_TX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_TX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML2_TX_PST->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer126->Add( cmbLML2_TX_PST, 0, 0, 5 );
	
	ID_STATICTEXT22 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data drive start"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT22->Wrap( -1 );
	fgSizer126->Add( ID_STATICTEXT22, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_TX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_TX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML2_TX_PRE->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer126->Add( cmbLML2_TX_PRE, 0, 0, 5 );
	
	ID_STATICTEXT23 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data capture stop"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT23->Wrap( -1 );
	fgSizer126->Add( ID_STATICTEXT23, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_RX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_RX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML2_RX_PST->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer126->Add( cmbLML2_RX_PST, 0, 0, 5 );
	
	ID_STATICTEXT24 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data capture start"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT24->Wrap( -1 );
	fgSizer126->Add( ID_STATICTEXT24, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLML2_RX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_RX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	cmbLML2_RX_PRE->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer126->Add( cmbLML2_RX_PRE, 0, 0, 5 );
	
	
	sbSizerClockCyclesToWaitLML2->Add( fgSizer126, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer124->Add( sbSizerClockCyclesToWaitLML2, 1, wxEXPAND, 5 );
	
	
	fgSizer224->Add( fgSizer124, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 15 );
	
	
	fgSizer115->Add( fgSizer224, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerDirectionControls;
	sbSizerDirectionControls = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Direction controls") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer123;
	fgSizer123 = new wxFlexGridSizer( 0, 8, 0, 5 );
	fgSizer123->SetFlexibleDirection( wxBOTH );
	fgSizer123->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rgrDIQDIRCTR1Choices[] = { wxT("Automatic"), wxT("Manual") };
	int rgrDIQDIRCTR1NChoices = sizeof( rgrDIQDIRCTR1Choices ) / sizeof( wxString );
	rgrDIQDIRCTR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIRCTR1, wxT("DIQ1 mode"), wxDefaultPosition, wxDefaultSize, rgrDIQDIRCTR1NChoices, rgrDIQDIRCTR1Choices, 1, wxRA_SPECIFY_COLS );
	rgrDIQDIRCTR1->SetSelection( 0 );
	rgrDIQDIRCTR1->SetToolTip( wxT("DIQ1 direction control mode") );
	
	fgSizer123->Add( rgrDIQDIRCTR1, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrDIQDIR1Choices[] = { wxT("Output"), wxT("Input") };
	int rgrDIQDIR1NChoices = sizeof( rgrDIQDIR1Choices ) / sizeof( wxString );
	rgrDIQDIR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIR1, wxT("DIQ1 direction"), wxDefaultPosition, wxDefaultSize, rgrDIQDIR1NChoices, rgrDIQDIR1Choices, 1, wxRA_SPECIFY_COLS );
	rgrDIQDIR1->SetSelection( 0 );
	rgrDIQDIR1->SetToolTip( wxT("DIQ1 direction") );
	
	fgSizer123->Add( rgrDIQDIR1, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrENABLEDIRCTR1Choices[] = { wxT("Automatic"), wxT("Manual") };
	int rgrENABLEDIRCTR1NChoices = sizeof( rgrENABLEDIRCTR1Choices ) / sizeof( wxString );
	rgrENABLEDIRCTR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIRCTR1, wxT("ENABLE1 mode"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIRCTR1NChoices, rgrENABLEDIRCTR1Choices, 1, wxRA_SPECIFY_COLS );
	rgrENABLEDIRCTR1->SetSelection( 0 );
	rgrENABLEDIRCTR1->SetToolTip( wxT("ENABLE1 direction control mode") );
	
	fgSizer123->Add( rgrENABLEDIRCTR1, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrENABLEDIR1Choices[] = { wxT("Output"), wxT("Input") };
	int rgrENABLEDIR1NChoices = sizeof( rgrENABLEDIR1Choices ) / sizeof( wxString );
	rgrENABLEDIR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIR1, wxT("ENABLE1 direction"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIR1NChoices, rgrENABLEDIR1Choices, 1, wxRA_SPECIFY_COLS );
	rgrENABLEDIR1->SetSelection( 0 );
	rgrENABLEDIR1->SetToolTip( wxT("ENABLE1 direction.") );
	
	fgSizer123->Add( rgrENABLEDIR1, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrDIQDIRCTR2Choices[] = { wxT("Automatic"), wxT("Manual") };
	int rgrDIQDIRCTR2NChoices = sizeof( rgrDIQDIRCTR2Choices ) / sizeof( wxString );
	rgrDIQDIRCTR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIRCTR2, wxT("DIQ2 mode"), wxDefaultPosition, wxDefaultSize, rgrDIQDIRCTR2NChoices, rgrDIQDIRCTR2Choices, 1, wxRA_SPECIFY_COLS );
	rgrDIQDIRCTR2->SetSelection( 0 );
	rgrDIQDIRCTR2->SetToolTip( wxT("DIQ2 direction control mode") );
	
	fgSizer123->Add( rgrDIQDIRCTR2, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrDIQDIR2Choices[] = { wxT("Output"), wxT("Input") };
	int rgrDIQDIR2NChoices = sizeof( rgrDIQDIR2Choices ) / sizeof( wxString );
	rgrDIQDIR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIR2, wxT("DIQ2 direction"), wxDefaultPosition, wxDefaultSize, rgrDIQDIR2NChoices, rgrDIQDIR2Choices, 1, wxRA_SPECIFY_COLS );
	rgrDIQDIR2->SetSelection( 0 );
	rgrDIQDIR2->SetToolTip( wxT("DIQ2 direction") );
	
	fgSizer123->Add( rgrDIQDIR2, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrENABLEDIRCTR2Choices[] = { wxT("Automatic"), wxT("Manual") };
	int rgrENABLEDIRCTR2NChoices = sizeof( rgrENABLEDIRCTR2Choices ) / sizeof( wxString );
	rgrENABLEDIRCTR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIRCTR2, wxT("ENABLE2 mode"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIRCTR2NChoices, rgrENABLEDIRCTR2Choices, 1, wxRA_SPECIFY_COLS );
	rgrENABLEDIRCTR2->SetSelection( 0 );
	rgrENABLEDIRCTR2->SetToolTip( wxT("ENABLE2 direction control mode") );
	
	fgSizer123->Add( rgrENABLEDIRCTR2, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrENABLEDIR2Choices[] = { wxT("Output"), wxT("Input") };
	int rgrENABLEDIR2NChoices = sizeof( rgrENABLEDIR2Choices ) / sizeof( wxString );
	rgrENABLEDIR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIR2, wxT("ENABLE2 direction"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIR2NChoices, rgrENABLEDIR2Choices, 1, wxRA_SPECIFY_COLS );
	rgrENABLEDIR2->SetSelection( 1 );
	rgrENABLEDIR2->SetToolTip( wxT("ENABLE2 direction") );
	
	fgSizer123->Add( rgrENABLEDIR2, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizerDirectionControls->Add( fgSizer123, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer115->Add( sbSizerDirectionControls, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	ID_PANEL2->SetSizer( fgSizer115 );
	ID_PANEL2->Layout();
	fgSizer115->Fit( ID_PANEL2 );
	ID_NOTEBOOK1->AddPage( ID_PANEL2, wxT("Sample position && Clock"), false );
	
	fgSizer105->Add( ID_NOTEBOOK1, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( fgSizer105 );
	this->Layout();
	fgSizer105->Fit( this );
	
	// Connect Events
	chkSDA_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSCL_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRX_CLK_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSDIO_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSDO_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTX_CLK_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSEN_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkDIQ1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXNRX1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSCLK_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkDIQ2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXNRX2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkIQ_SEL_EN_1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkIQ_SEL_EN_2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSRST_RXFIFO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSRST_TXFIFO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_TX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_TX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_RX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_RX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_TX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_TX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_RX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_RX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRXEN_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXEN_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRXEN_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXEN_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMIMO_SISO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMOD_EN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_FIDM1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_FIDM2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_MODE1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_MODE2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_TXNRXIQ1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_TXNRXIQ2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML1_TRXIQPULSE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML2_TRXIQPULSE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML1_SISODDR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML2_SISODDR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSDA_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSCL_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSDIO_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQ2_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQ1_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrRX_MUX->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrTX_MUX->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSPIMODE->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	btnReadVerRevMask->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::onbtnReadVerRevMask ), NULL, this );
	cmbLML1_S3S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_S2S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_S1S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_S0S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S3S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S2S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S1S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S0S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_BQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_BIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_AQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_AIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_BQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_BIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_AQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_AIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbTXRDCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbTXWRCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbRXRDCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbRXWRCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK2SRC->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1SRC->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXDIVEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbTXTSPCLKA_DIV->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRXDIVEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbRXTSPCLKA_DIV->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1DLY->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK2DLY->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK1_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK2_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK1_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK2_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbFCLK1_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbFCLK2_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK2_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_TX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_TX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_RX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_RX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_TX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_TX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_RX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_RX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIRCTR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIRCTR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIRCTR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIRCTR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
}

pnlLimeLightPAD_view::~pnlLimeLightPAD_view()
{
	// Disconnect Events
	chkSDA_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSCL_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRX_CLK_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSDIO_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSDO_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTX_CLK_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSEN_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkDIQ1_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXNRX1_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSCLK_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkDIQ2_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXNRX2_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkIQ_SEL_EN_1_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK1_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK1_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkIQ_SEL_EN_2_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK2_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK2_PE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSRST_RXFIFO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkSRST_TXFIFO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_TX_A->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_TX_B->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_RX_A->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLRST_RX_B->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_TX_A->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_TX_B->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_RX_A->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMRST_RX_B->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRXEN_A->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXEN_A->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRXEN_B->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXEN_B->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMIMO_SISO->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMOD_EN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_FIDM1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_FIDM2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_MODE1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_MODE2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_TXNRXIQ1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrLML_TXNRXIQ2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML1_TRXIQPULSE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML2_TRXIQPULSE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML1_SISODDR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkLML2_SISODDR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSDA_DS->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSCL_DS->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSDIO_DS->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQ2_DS->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQ1_DS->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrRX_MUX->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrTX_MUX->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrSPIMODE->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	btnReadVerRevMask->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::onbtnReadVerRevMask ), NULL, this );
	cmbLML1_S3S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_S2S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_S1S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_S0S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S3S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S2S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S1S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_S0S->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_BQP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_BIP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_AQP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_AIP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_BQP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_BIP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_AQP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_AIP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbTXRDCLK_MUX->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbTXWRCLK_MUX->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbRXRDCLK_MUX->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbRXWRCLK_MUX->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK2SRC->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1SRC->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkTXDIVEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbTXTSPCLKA_DIV->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkRXDIVEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbRXTSPCLKA_DIV->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1DLY->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK2DLY->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK1_INV->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkFCLK2_INV->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK1_INV->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	chkMCLK2_INV->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbFCLK1_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbFCLK2_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK2_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_TX_PST->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_TX_PRE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_RX_PST->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML1_RX_PRE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_TX_PST->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_TX_PRE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_RX_PST->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	cmbLML2_RX_PRE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIRCTR1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIR1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIRCTR1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIR1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIRCTR2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrDIQDIR2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIRCTR2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	rgrENABLEDIR2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
	
}

pnlTxTSP_view::pnlTxTSP_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer215;
	fgSizer215 = new wxFlexGridSizer( 0, 4, 5, 5 );
	fgSizer215->SetFlexibleDirection( wxBOTH );
	fgSizer215->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer216;
	fgSizer216 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer216->SetFlexibleDirection( wxBOTH );
	fgSizer216->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkEN_TXTSP = new wxCheckBox( this, ID_EN_TXTSP, wxT("Enable TxTSP"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_TXTSP->SetToolTip( wxT("TxTSP modules enable") );
	chkEN_TXTSP->SetMinSize( wxSize( 135,-1 ) );
	
	fgSizer216->Add( chkEN_TXTSP, 1, wxALIGN_LEFT|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer103;
	sbSizer103 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bypass") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer145;
	fgSizer145 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer145->SetFlexibleDirection( wxBOTH );
	fgSizer145->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkDC_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_DC_BYP_TXTSP, wxT("DC corrector"), wxDefaultPosition, wxDefaultSize, 0 );
	chkDC_BYP_TXTSP->SetToolTip( wxT("DC corrector bypass") );
	
	fgSizer145->Add( chkDC_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkGC_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GC_BYP_TXTSP, wxT("Gain corrector"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGC_BYP_TXTSP->SetToolTip( wxT("Gain corrector bypass") );
	
	fgSizer145->Add( chkGC_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPH_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_PH_BYP_TXTSP, wxT("Phase corrector"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPH_BYP_TXTSP->SetToolTip( wxT("Phase corrector bypass") );
	
	fgSizer145->Add( chkPH_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCMIX_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_CMIX_BYP_TXTSP, wxT("CMIX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCMIX_BYP_TXTSP->SetValue(true); 
	chkCMIX_BYP_TXTSP->SetToolTip( wxT("CMIX bypass") );
	
	fgSizer145->Add( chkCMIX_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkISINC_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_ISINC_BYP_TXTSP, wxT("ISINC "), wxDefaultPosition, wxDefaultSize, 0 );
	chkISINC_BYP_TXTSP->SetValue(true); 
	chkISINC_BYP_TXTSP->SetToolTip( wxT("ISINC bypass") );
	
	fgSizer145->Add( chkISINC_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkGFIR1_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GFIR1_BYP_TXTSP, wxT("GFIR1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGFIR1_BYP_TXTSP->SetToolTip( wxT("GFIR1 bypass") );
	
	fgSizer145->Add( chkGFIR1_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkGFIR2_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GFIR2_BYP_TXTSP, wxT("GFIR2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGFIR2_BYP_TXTSP->SetValue(true); 
	chkGFIR2_BYP_TXTSP->SetToolTip( wxT("GFIR2 bypass") );
	
	fgSizer145->Add( chkGFIR2_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkGFIR3_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GFIR3_BYP_TXTSP, wxT("GFIR3"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGFIR3_BYP_TXTSP->SetValue(true); 
	chkGFIR3_BYP_TXTSP->SetToolTip( wxT("GFIR3 bypass") );
	
	fgSizer145->Add( chkGFIR3_BYP_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer103->Add( fgSizer145, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer216->Add( sbSizer103, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer104;
	sbSizer104 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("BIST") ), wxVERTICAL );
	
	chkBSTART_TXTSP = new wxCheckBox( sbSizer104->GetStaticBox(), ID_BSTART_TXTSP, wxT("Start BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBSTART_TXTSP->SetValue(true); 
	chkBSTART_TXTSP->SetToolTip( wxT("Starts delta sigma built in self test. Keep it at 1 one at least three clock cycles") );
	
	sbSizer104->Add( chkBSTART_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxFlexGridSizer* fgSizer146;
	fgSizer146 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer146->SetFlexibleDirection( wxBOTH );
	fgSizer146->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT2 = new wxStaticText( sbSizer104->GetStaticBox(), wxID_ANY, wxT("State"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer146->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblBSTATE_TXTSP = new wxStaticText( sbSizer104->GetStaticBox(), ID_BSTATE_TXTSP, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSTATE_TXTSP->Wrap( 0 );
	fgSizer146->Add( lblBSTATE_TXTSP, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizer104->GetStaticBox(), wxID_ANY, wxT("Signature ch. I"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer146->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblBSIGI_TXTSP = new wxStaticText( sbSizer104->GetStaticBox(), ID_BSIGI_TXTSP, wxT("?????????"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSIGI_TXTSP->Wrap( 0 );
	fgSizer146->Add( lblBSIGI_TXTSP, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer104->GetStaticBox(), wxID_ANY, wxT("Signature ch. Q"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer146->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblBSIGQ_TXTSP = new wxStaticText( sbSizer104->GetStaticBox(), ID_BSIGQ_TXTSP, wxT("?????????"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSIGQ_TXTSP->Wrap( 0 );
	fgSizer146->Add( lblBSIGQ_TXTSP, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer104->Add( fgSizer146, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnReadBIST = new wxButton( sbSizer104->GetStaticBox(), wxID_ANY, wxT("Read BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	btnReadBIST->SetDefault(); 
	sbSizer104->Add( btnReadBIST, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer216->Add( sbSizer104, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer215->Add( fgSizer216, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer142;
	fgSizer142 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer142->SetFlexibleDirection( wxBOTH );
	fgSizer142->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer106;
	sbSizer106 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("NCO") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer152;
	fgSizer152 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer152->SetFlexibleDirection( wxBOTH );
	fgSizer152->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	sizerNCOgrid = new wxFlexGridSizer( 0, 1, 0, 5 );
	sizerNCOgrid->SetFlexibleDirection( wxBOTH );
	sizerNCOgrid->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer221;
	fgSizer221 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer221->SetFlexibleDirection( wxBOTH );
	fgSizer221->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer221->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tableTitleCol1 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("FCW (MHz)"), wxDefaultPosition, wxDefaultSize, 0 );
	tableTitleCol1->Wrap( -1 );
	fgSizer221->Add( tableTitleCol1, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	tableTitleCol2 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Angle (Deg)"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	tableTitleCol2->Wrap( -1 );
	fgSizer221->Add( tableTitleCol2, 1, wxLEFT|wxALIGN_RIGHT, 5 );
	
	rgrSEL0 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer221->Add( rgrSEL0, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO0 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO0->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO0->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO0->SetMaxLength( 10 );
	#endif
	fgSizer221->Add( txtFCWPHO0, 0, 0, 5 );
	
	txtAnglePHO0 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.00000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO0->Wrap( -1 );
	fgSizer221->Add( txtAnglePHO0, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer221, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2211;
	fgSizer2211 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2211->SetFlexibleDirection( wxBOTH );
	fgSizer2211->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL01 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2211->Add( rgrSEL01, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO01 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO01->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO01->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO01->SetMaxLength( 10 );
	#endif
	fgSizer2211->Add( txtFCWPHO01, 0, 0, 5 );
	
	txtAnglePHO01 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO01->Wrap( -1 );
	fgSizer2211->Add( txtAnglePHO01, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2211, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2212;
	fgSizer2212 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2212->SetFlexibleDirection( wxBOTH );
	fgSizer2212->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL02 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2212->Add( rgrSEL02, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO02 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO02->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO02->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO02->SetMaxLength( 10 );
	#endif
	fgSizer2212->Add( txtFCWPHO02, 0, 0, 5 );
	
	txtAnglePHO02 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO02->Wrap( -1 );
	fgSizer2212->Add( txtAnglePHO02, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2212, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2213;
	fgSizer2213 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2213->SetFlexibleDirection( wxBOTH );
	fgSizer2213->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL03 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2213->Add( rgrSEL03, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO03 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO03->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO03->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO03->SetMaxLength( 10 );
	#endif
	fgSizer2213->Add( txtFCWPHO03, 0, 0, 5 );
	
	txtAnglePHO03 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO03->Wrap( -1 );
	fgSizer2213->Add( txtAnglePHO03, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2213, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2214;
	fgSizer2214 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2214->SetFlexibleDirection( wxBOTH );
	fgSizer2214->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL04 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2214->Add( rgrSEL04, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO04 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO04->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO04->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO04->SetMaxLength( 10 );
	#endif
	fgSizer2214->Add( txtFCWPHO04, 0, 0, 5 );
	
	txtAnglePHO04 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO04->Wrap( -1 );
	fgSizer2214->Add( txtAnglePHO04, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2214, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2215;
	fgSizer2215 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2215->SetFlexibleDirection( wxBOTH );
	fgSizer2215->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL05 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2215->Add( rgrSEL05, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO05 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO05->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO05->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO05->SetMaxLength( 10 );
	#endif
	fgSizer2215->Add( txtFCWPHO05, 0, 0, 5 );
	
	txtAnglePHO05 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO05->Wrap( -1 );
	fgSizer2215->Add( txtAnglePHO05, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2215, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2216;
	fgSizer2216 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2216->SetFlexibleDirection( wxBOTH );
	fgSizer2216->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL06 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2216->Add( rgrSEL06, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO06 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO06->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO06->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO06->SetMaxLength( 10 );
	#endif
	fgSizer2216->Add( txtFCWPHO06, 0, 0, 5 );
	
	txtAnglePHO06 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO06->Wrap( -1 );
	fgSizer2216->Add( txtAnglePHO06, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2216, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2217;
	fgSizer2217 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2217->SetFlexibleDirection( wxBOTH );
	fgSizer2217->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL07 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2217->Add( rgrSEL07, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO07 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO07->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO07->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO07->SetMaxLength( 10 );
	#endif
	fgSizer2217->Add( txtFCWPHO07, 0, 0, 5 );
	
	txtAnglePHO07 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO07->Wrap( -1 );
	fgSizer2217->Add( txtAnglePHO07, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2217, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2218;
	fgSizer2218 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2218->SetFlexibleDirection( wxBOTH );
	fgSizer2218->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL08 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2218->Add( rgrSEL08, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO08 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO08->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO08->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO08->SetMaxLength( 10 );
	#endif
	fgSizer2218->Add( txtFCWPHO08, 0, 0, 5 );
	
	txtAnglePHO08 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO08->Wrap( -1 );
	fgSizer2218->Add( txtAnglePHO08, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2218, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2219;
	fgSizer2219 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2219->SetFlexibleDirection( wxBOTH );
	fgSizer2219->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL09 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2219->Add( rgrSEL09, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO09 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO09->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO09->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO09->SetMaxLength( 10 );
	#endif
	fgSizer2219->Add( txtFCWPHO09, 0, 0, 5 );
	
	txtAnglePHO09 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO09->Wrap( -1 );
	fgSizer2219->Add( txtAnglePHO09, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2219, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22110;
	fgSizer22110 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22110->SetFlexibleDirection( wxBOTH );
	fgSizer22110->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL10 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22110->Add( rgrSEL10, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO10 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO10->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO10->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO10->SetMaxLength( 10 );
	#endif
	fgSizer22110->Add( txtFCWPHO10, 0, 0, 5 );
	
	txtAnglePHO10 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO10->Wrap( -1 );
	fgSizer22110->Add( txtAnglePHO10, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22110, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22111;
	fgSizer22111 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22111->SetFlexibleDirection( wxBOTH );
	fgSizer22111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL11 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22111->Add( rgrSEL11, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO11 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO11->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO11->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO11->SetMaxLength( 10 );
	#endif
	fgSizer22111->Add( txtFCWPHO11, 0, 0, 5 );
	
	txtAnglePHO11 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO11->Wrap( -1 );
	fgSizer22111->Add( txtAnglePHO11, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22111, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22112;
	fgSizer22112 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22112->SetFlexibleDirection( wxBOTH );
	fgSizer22112->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL12 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22112->Add( rgrSEL12, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO12 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO12->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO12->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO12->SetMaxLength( 10 );
	#endif
	fgSizer22112->Add( txtFCWPHO12, 0, 0, 5 );
	
	txtAnglePHO12 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO12->Wrap( -1 );
	fgSizer22112->Add( txtAnglePHO12, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22112, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22113;
	fgSizer22113 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22113->SetFlexibleDirection( wxBOTH );
	fgSizer22113->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL13 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22113->Add( rgrSEL13, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO13 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO13->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO13->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO13->SetMaxLength( 10 );
	#endif
	fgSizer22113->Add( txtFCWPHO13, 0, 0, 5 );
	
	txtAnglePHO13 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO13->Wrap( -1 );
	fgSizer22113->Add( txtAnglePHO13, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22113, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22114;
	fgSizer22114 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22114->SetFlexibleDirection( wxBOTH );
	fgSizer22114->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL14 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22114->Add( rgrSEL14, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO14 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO14->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO14->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO14->SetMaxLength( 10 );
	#endif
	fgSizer22114->Add( txtFCWPHO14, 0, 0, 5 );
	
	txtAnglePHO14 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO14->Wrap( -1 );
	fgSizer22114->Add( txtAnglePHO14, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22114, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22115;
	fgSizer22115 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22115->SetFlexibleDirection( wxBOTH );
	fgSizer22115->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL15 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22115->Add( rgrSEL15, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO15 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO15->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO15->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO15->SetMaxLength( 10 );
	#endif
	fgSizer22115->Add( txtFCWPHO15, 0, 0, 5 );
	
	txtAnglePHO15 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO15->Wrap( -1 );
	fgSizer22115->Add( txtAnglePHO15, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22115, 1, wxEXPAND, 5 );
	
	
	fgSizer152->Add( sizerNCOgrid, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer228;
	fgSizer228 = new wxFlexGridSizer( 0, 1, 5, 0 );
	fgSizer228->SetFlexibleDirection( wxBOTH );
	fgSizer228->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer154;
	fgSizer154 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer154->SetFlexibleDirection( wxBOTH );
	fgSizer154->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT21 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("RefClk (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT21->Wrap( -1 );
	fgSizer154->Add( ID_STATICTEXT21, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	lblRefClk = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("30.72"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRefClk->Wrap( -1 );
	fgSizer154->Add( lblRefClk, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer228->Add( fgSizer154, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	btnUploadNCO = new wxButton( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Upload NCO"), wxDefaultPosition, wxDefaultSize, 0 );
	btnUploadNCO->SetDefault(); 
	fgSizer228->Add( btnUploadNCO, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrMODE_TXChoices[] = { wxT("FCW"), wxT("PHO") };
	int rgrMODE_TXNChoices = sizeof( rgrMODE_TXChoices ) / sizeof( wxString );
	rgrMODE_TX = new wxRadioBox( sbSizer106->GetStaticBox(), ID_MODE_TX, wxT("Mode"), wxDefaultPosition, wxDefaultSize, rgrMODE_TXNChoices, rgrMODE_TXChoices, 2, wxRA_SPECIFY_COLS );
	rgrMODE_TX->SetSelection( 0 );
	rgrMODE_TX->SetToolTip( wxT("Memory table mode") );
	
	fgSizer228->Add( rgrMODE_TX, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer229;
	fgSizer229 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer229->AddGrowableCol( 1 );
	fgSizer229->SetFlexibleDirection( wxBOTH );
	fgSizer229->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblFCWPHOmodeName = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("FCW (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFCWPHOmodeName->Wrap( -1 );
	fgSizer229->Add( lblFCWPHOmodeName, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHOmodeAdditional = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHOmodeAdditional->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHOmodeAdditional->SetMaxLength( 10 );
	}
	#else
	txtFCWPHOmodeAdditional->SetMaxLength( 10 );
	#endif
	fgSizer229->Add( txtFCWPHOmodeAdditional, 1, wxEXPAND, 5 );
	
	
	fgSizer228->Add( fgSizer229, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer153;
	fgSizer153 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer153->AddGrowableCol( 1 );
	fgSizer153->SetFlexibleDirection( wxBOTH );
	fgSizer153->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT25 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Bits to dither:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT25->Wrap( -1 );
	fgSizer153->Add( ID_STATICTEXT25, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbDTHBIT_TX = new wxComboBox( sbSizer106->GetStaticBox(), ID_DTHBIT_TX, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	cmbDTHBIT_TX->SetToolTip( wxT("NCO bits to dither") );
	
	fgSizer153->Add( cmbDTHBIT_TX, 1, wxEXPAND, 5 );
	
	
	fgSizer228->Add( fgSizer153, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer105;
	sbSizer105 = new wxStaticBoxSizer( new wxStaticBox( sbSizer106->GetStaticBox(), wxID_ANY, wxT("TSG") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer147;
	fgSizer147 = new wxFlexGridSizer( 0, 1, 5, 0 );
	fgSizer147->SetFlexibleDirection( wxBOTH );
	fgSizer147->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkTSGSWAPIQ_TXTSP = new wxCheckBox( sbSizer105->GetStaticBox(), ID_TSGSWAPIQ_TXTSP, wxT("Swap I and Q\n signal sources from TSG"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTSGSWAPIQ_TXTSP->SetValue(true); 
	chkTSGSWAPIQ_TXTSP->SetToolTip( wxT("Swap signals at test signal generator's output") );
	
	fgSizer147->Add( chkTSGSWAPIQ_TXTSP, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer148;
	fgSizer148 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer148->SetFlexibleDirection( wxBOTH );
	fgSizer148->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rgrTSGFCW_TXTSPChoices[] = { wxT("TSP clk/8"), wxT("TSP clk/4") };
	int rgrTSGFCW_TXTSPNChoices = sizeof( rgrTSGFCW_TXTSPChoices ) / sizeof( wxString );
	rgrTSGFCW_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGFCW_TXTSP, wxT("TSGFCW"), wxDefaultPosition, wxDefaultSize, rgrTSGFCW_TXTSPNChoices, rgrTSGFCW_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrTSGFCW_TXTSP->SetSelection( 0 );
	rgrTSGFCW_TXTSP->SetToolTip( wxT("Set frequency of TSG's NCO") );
	
	fgSizer148->Add( rgrTSGFCW_TXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrTSGMODE_TXTSPChoices[] = { wxT("NCO"), wxT("DC source") };
	int rgrTSGMODE_TXTSPNChoices = sizeof( rgrTSGMODE_TXTSPChoices ) / sizeof( wxString );
	rgrTSGMODE_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGMODE_TXTSP, wxT("TSGMODE"), wxDefaultPosition, wxDefaultSize, rgrTSGMODE_TXTSPNChoices, rgrTSGMODE_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrTSGMODE_TXTSP->SetSelection( 0 );
	rgrTSGMODE_TXTSP->SetToolTip( wxT("Test signal generator mode") );
	
	fgSizer148->Add( rgrTSGMODE_TXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrINSEL_TXTSPChoices[] = { wxT("LML output"), wxT("Test signal") };
	int rgrINSEL_TXTSPNChoices = sizeof( rgrINSEL_TXTSPChoices ) / sizeof( wxString );
	rgrINSEL_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_INSEL_TXTSP, wxT("Input source"), wxDefaultPosition, wxDefaultSize, rgrINSEL_TXTSPNChoices, rgrINSEL_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrINSEL_TXTSP->SetSelection( 0 );
	rgrINSEL_TXTSP->SetToolTip( wxT("Input source of TxTSP") );
	
	fgSizer148->Add( rgrINSEL_TXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrTSGFC_TXTSPChoices[] = { wxT("-6 dB"), wxT("Full scale") };
	int rgrTSGFC_TXTSPNChoices = sizeof( rgrTSGFC_TXTSPChoices ) / sizeof( wxString );
	rgrTSGFC_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGFC_TXTSP, wxT("TSGFC"), wxDefaultPosition, wxDefaultSize, rgrTSGFC_TXTSPNChoices, rgrTSGFC_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrTSGFC_TXTSP->SetSelection( 0 );
	rgrTSGFC_TXTSP->SetToolTip( wxT("TSG full scale control") );
	
	fgSizer148->Add( rgrTSGFC_TXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer147->Add( fgSizer148, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizer105->Add( fgSizer147, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer228->Add( sbSizer105, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer152->Add( fgSizer228, 1, wxEXPAND, 5 );
	
	
	sbSizer106->Add( fgSizer152, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer142->Add( sbSizer106, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer215->Add( fgSizer142, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer218;
	fgSizer218 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer218->SetFlexibleDirection( wxBOTH );
	fgSizer218->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer220;
	fgSizer220 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer220->AddGrowableCol( 1 );
	fgSizer220->SetFlexibleDirection( wxBOTH );
	fgSizer220->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer149;
	fgSizer149 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer149->SetFlexibleDirection( wxBOTH );
	fgSizer149->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer150;
	fgSizer150 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer150->SetFlexibleDirection( wxBOTH );
	fgSizer150->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT23 = new wxStaticText( this, wxID_ANY, wxT("DC_REG:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT23->Wrap( -1 );
	fgSizer150->Add( ID_STATICTEXT23, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtDC_REG_TXTSP = new wxTextCtrl( this, ID_DC_REG_TXTSP, wxT("ffff"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	fgSizer150->Add( txtDC_REG_TXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer149->Add( fgSizer150, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLoadDCI = new wxButton( this, wxID_ANY, wxT("Load to DC I"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLoadDCI->SetDefault(); 
	fgSizer149->Add( btnLoadDCI, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	btnLoadDCQ = new wxButton( this, wxID_ANY, wxT("Load to DC Q"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLoadDCQ->SetDefault(); 
	fgSizer149->Add( btnLoadDCQ, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer220->Add( fgSizer149, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer98;
	sbSizer98 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("CMIX") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer132;
	fgSizer132 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer132->AddGrowableCol( 0 );
	fgSizer132->SetFlexibleDirection( wxBOTH );
	fgSizer132->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cmbCMIX_SC_TXTSP = new wxComboBox( sbSizer98->GetStaticBox(), ID_CMIX_SC_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 );
	cmbCMIX_SC_TXTSP->Append( wxT("Upconvert") );
	cmbCMIX_SC_TXTSP->Append( wxT("Downconvert") );
	fgSizer132->Add( cmbCMIX_SC_TXTSP, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer133;
	fgSizer133 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer133->AddGrowableCol( 1 );
	fgSizer133->SetFlexibleDirection( wxBOTH );
	fgSizer133->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT24 = new wxStaticText( sbSizer98->GetStaticBox(), wxID_ANY, wxT("Gain:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT24->Wrap( -1 );
	fgSizer133->Add( ID_STATICTEXT24, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbCMIX_GAIN_TXTSP = new wxComboBox( sbSizer98->GetStaticBox(), ID_CMIX_GAIN_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbCMIX_GAIN_TXTSP->Append( wxT("-6 dB") );
	cmbCMIX_GAIN_TXTSP->Append( wxT("-3 dB") );
	cmbCMIX_GAIN_TXTSP->Append( wxT("0 dB") );
	cmbCMIX_GAIN_TXTSP->Append( wxT("+3 dB") );
	cmbCMIX_GAIN_TXTSP->Append( wxT("+6 dB") );
	cmbCMIX_GAIN_TXTSP->SetToolTip( wxT("Gain of CMIX output") );
	
	fgSizer133->Add( cmbCMIX_GAIN_TXTSP, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	
	fgSizer132->Add( fgSizer133, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer98->Add( fgSizer132, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer220->Add( sbSizer98, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer99;
	sbSizer99 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Interpolation") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer134;
	fgSizer134 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer134->AddGrowableCol( 0 );
	fgSizer134->SetFlexibleDirection( wxBOTH );
	fgSizer134->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizer99->GetStaticBox(), wxID_ANY, wxT("HBI ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer134->Add( ID_STATICTEXT7, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbHBI_OVR_TXTSP = new wxComboBox( sbSizer99->GetStaticBox(), ID_HBI_OVR_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( 64,-1 ), 0, NULL, 0 ); 
	cmbHBI_OVR_TXTSP->SetToolTip( wxT("HBI interpolation ratio") );
	
	fgSizer134->Add( cmbHBI_OVR_TXTSP, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer99->Add( fgSizer134, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer220->Add( sbSizer99, 1, wxALIGN_LEFT|wxEXPAND, 5 );
	
	
	fgSizer218->Add( fgSizer220, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer138;
	sbSizer138 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("GFIR") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer259;
	fgSizer259 = new wxFlexGridSizer( 2, 0, 0, 0 );
	fgSizer259->SetFlexibleDirection( wxBOTH );
	fgSizer259->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer135;
	fgSizer135 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer135->SetFlexibleDirection( wxBOTH );
	fgSizer135->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer100;
	sbSizer100 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("GFIR1") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer137;
	fgSizer137 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer137->AddGrowableCol( 1 );
	fgSizer137->SetFlexibleDirection( wxBOTH );
	fgSizer137->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer137->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR1_L_TXTSP = new wxComboBox( sbSizer100->GetStaticBox(), ID_GFIR1_L_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0, NULL, 0 ); 
	cmbGFIR1_L_TXTSP->SetToolTip( wxT("Parameter l of GFIR1 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
	
	fgSizer137->Add( cmbGFIR1_L_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer137->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR1_N_TXTSP = new wxSpinCtrl( sbSizer100->GetStaticBox(), ID_GFIR1_N_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 255, 255 );
	cmbGFIR1_N_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer137->Add( cmbGFIR1_N_TXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer100->Add( fgSizer137, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnGFIR1Coef = new wxButton( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
	btnGFIR1Coef->SetDefault(); 
	sbSizer100->Add( btnGFIR1Coef, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	fgSizer135->Add( sbSizer100, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer101;
	sbSizer101 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("GFIR2") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer139;
	fgSizer139 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer139->AddGrowableCol( 1 );
	fgSizer139->SetFlexibleDirection( wxBOTH );
	fgSizer139->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer139->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR2_L_TXTSP = new wxComboBox( sbSizer101->GetStaticBox(), ID_GFIR2_L_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0, NULL, 0 ); 
	cmbGFIR2_L_TXTSP->SetToolTip( wxT("Parameter l of GFIR2 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
	
	fgSizer139->Add( cmbGFIR2_L_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer139->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR2_N_TXTSP = new wxSpinCtrl( sbSizer101->GetStaticBox(), ID_GFIR2_N_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 255, 255 );
	cmbGFIR2_N_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer139->Add( cmbGFIR2_N_TXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer101->Add( fgSizer139, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnGFIR2Coef = new wxButton( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
	btnGFIR2Coef->SetDefault(); 
	sbSizer101->Add( btnGFIR2Coef, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer135->Add( sbSizer101, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer102;
	sbSizer102 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("GFIR3") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer141;
	fgSizer141 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer141->AddGrowableCol( 1 );
	fgSizer141->SetFlexibleDirection( wxBOTH );
	fgSizer141->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer141->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR3_L_TXTSP = new wxComboBox( sbSizer102->GetStaticBox(), ID_GFIR3_L_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0, NULL, 0 ); 
	cmbGFIR3_L_TXTSP->SetToolTip( wxT("Parameter l of GFIR3 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
	
	fgSizer141->Add( cmbGFIR3_L_TXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( -1 );
	fgSizer141->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR3_N_TXTSP = new wxSpinCtrl( sbSizer102->GetStaticBox(), ID_GFIR3_N_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 255, 255 );
	cmbGFIR3_N_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer141->Add( cmbGFIR3_N_TXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer102->Add( fgSizer141, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnGFIR3Coef = new wxButton( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
	btnGFIR3Coef->SetDefault(); 
	sbSizer102->Add( btnGFIR3Coef, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer135->Add( sbSizer102, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer259->Add( fgSizer135, 1, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer145;
	sbSizer145 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("Configure GFIRs as LPF") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1351;
	fgSizer1351 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer1351->SetFlexibleDirection( wxBOTH );
	fgSizer1351->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	txtBW = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Bandwidth (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	txtBW->Wrap( -1 );
	fgSizer1351->Add( txtBW, 0, wxALIGN_CENTER|wxALL, 0 );
	
	txtLPFBW = new wxTextCtrl( sbSizer145->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtLPFBW->HasFlag( wxTE_MULTILINE ) )
	{
	txtLPFBW->SetMaxLength( 8 );
	}
	#else
	txtLPFBW->SetMaxLength( 8 );
	#endif
	fgSizer1351->Add( txtLPFBW, 0, wxALL, 0 );
	
	btnSetLPF = new wxButton( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	btnSetLPF->SetDefault(); 
	fgSizer1351->Add( btnSetLPF, 0, wxALL, 0 );
	
	txtRATE = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Sample rate:"), wxDefaultPosition, wxDefaultSize, 0 );
	txtRATE->Wrap( -1 );
	fgSizer1351->Add( txtRATE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 0 );
	
	txtRATEVAL = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("0 MHz"), wxDefaultPosition, wxDefaultSize, 0 );
	txtRATEVAL->Wrap( -1 );
	fgSizer1351->Add( txtRATEVAL, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5 );
	
	
	sbSizer145->Add( fgSizer1351, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_RIGHT|wxEXPAND, 5 );
	
	
	fgSizer259->Add( sbSizer145, 1, wxEXPAND, 5 );
	
	
	sbSizer138->Add( fgSizer259, 1, wxEXPAND, 5 );
	
	
	fgSizer218->Add( sbSizer138, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer219;
	fgSizer219 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer219->AddGrowableCol( 0 );
	fgSizer219->SetFlexibleDirection( wxBOTH );
	fgSizer219->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer96;
	sbSizer96 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
	
	cmbIQCORR_TXTSP = new NumericSlider( sbSizer96->GetStaticBox(), ID_IQCORR_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
	cmbIQCORR_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	sbSizer96->Add( cmbIQCORR_TXTSP, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer130;
	fgSizer130 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer130->SetFlexibleDirection( wxBOTH );
	fgSizer130->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer130->Add( ID_STATICTEXT16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtPhaseAlpha = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txtPhaseAlpha->Wrap( -1 );
	fgSizer130->Add( txtPhaseAlpha, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer96->Add( fgSizer130, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer219->Add( sbSizer96, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer97;
	sbSizer97 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer131;
	fgSizer131 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer131->AddGrowableCol( 1 );
	fgSizer131->SetFlexibleDirection( wxBOTH );
	fgSizer131->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer131->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRI_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer131->Add( cmbGCORRI_TXTSP, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer131->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRQ_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer131->Add( cmbGCORRQ_TXTSP, 0, wxEXPAND, 5 );
	
	ID_BUTTON10 = new wxButton( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON10->SetDefault(); 
	ID_BUTTON10->Hide();
	
	fgSizer131->Add( ID_BUTTON10, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer97->Add( fgSizer131, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer219->Add( sbSizer97, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer95;
	sbSizer95 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("DC Corrector") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer129;
	fgSizer129 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer129->AddGrowableCol( 1 );
	fgSizer129->SetFlexibleDirection( wxBOTH );
	fgSizer129->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT8 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer129->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCCORRI_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
	cmbDCCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer129->Add( cmbDCCORRI_TXTSP, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer129->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCCORRQ_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
	cmbDCCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer129->Add( cmbDCCORRQ_TXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer95->Add( fgSizer129, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer219->Add( sbSizer95, 1, wxEXPAND, 5 );
	
	
	fgSizer218->Add( fgSizer219, 1, wxEXPAND, 5 );
	
	
	fgSizer215->Add( fgSizer218, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer215 );
	this->Layout();
	fgSizer215->Fit( this );
	
	// Connect Events
	chkEN_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkDC_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGC_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkPH_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkCMIX_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkISINC_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR1_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR2_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR3_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkBSTART_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnReadBIST->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnReadBISTSignature ), NULL, this );
	rgrSEL0->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO0->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL01->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO01->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL02->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO02->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL03->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO03->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL04->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO04->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL05->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO05->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL06->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO06->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL07->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO07->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL08->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO08->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL09->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO09->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL10->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO10->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL11->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO11->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL12->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO12->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL13->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO13->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL14->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO14->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL15->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO15->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	btnUploadNCO->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnUploadNCOClick ), NULL, this );
	rgrMODE_TX->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	txtFCWPHOmodeAdditional->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	txtFCWPHOmodeAdditional->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	cmbDTHBIT_TX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkTSGSWAPIQ_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFCW_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGMODE_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrINSEL_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFC_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnLoadDCI->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnLoadDCIClick ), NULL, this );
	btnLoadDCQ->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnLoadDCQClick ), NULL, this );
	cmbCMIX_SC_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbCMIX_GAIN_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbHBI_OVR_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_L_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_N_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR1Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnGFIR1Coef ), NULL, this );
	cmbGFIR2_L_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR2_N_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR2Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnGFIR2Coef ), NULL, this );
	cmbGFIR3_L_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR3_N_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR3Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnGFIR3Coef ), NULL, this );
	txtLPFBW->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	btnSetLPF->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnSetLPFClick ), NULL, this );
	cmbIQCORR_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
}

pnlTxTSP_view::~pnlTxTSP_view()
{
	// Disconnect Events
	chkEN_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkDC_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGC_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkPH_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkCMIX_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkISINC_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR1_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR2_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR3_BYP_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkBSTART_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnReadBIST->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnReadBISTSignature ), NULL, this );
	rgrSEL0->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO0->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL01->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO01->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL02->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO02->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL03->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO03->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL04->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO04->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL05->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO05->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL06->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO06->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL07->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO07->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL08->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO08->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL09->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO09->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL10->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO10->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL11->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO11->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL12->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO12->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL13->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO13->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL14->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO14->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL15->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO15->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	btnUploadNCO->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnUploadNCOClick ), NULL, this );
	rgrMODE_TX->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	txtFCWPHOmodeAdditional->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	txtFCWPHOmodeAdditional->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlTxTSP_view::PHOinputChanged ), NULL, this );
	cmbDTHBIT_TX->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	chkTSGSWAPIQ_TXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFCW_TXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGMODE_TXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrINSEL_TXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFC_TXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnLoadDCI->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnLoadDCIClick ), NULL, this );
	btnLoadDCQ->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnLoadDCQClick ), NULL, this );
	cmbCMIX_SC_TXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbCMIX_GAIN_TXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbHBI_OVR_TXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_L_TXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_N_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR1Coef->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnGFIR1Coef ), NULL, this );
	cmbGFIR2_L_TXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR2_N_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR2Coef->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnGFIR2Coef ), NULL, this );
	cmbGFIR3_L_TXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR3_N_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR3Coef->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::onbtnGFIR3Coef ), NULL, this );
	txtLPFBW->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	btnSetLPF->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlTxTSP_view::OnbtnSetLPFClick ), NULL, this );
	cmbIQCORR_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRI_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRI_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRQ_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
	
}

pnlRxTSP_view::pnlRxTSP_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer223;
	fgSizer223 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer223->SetFlexibleDirection( wxBOTH );
	fgSizer223->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer176;
	fgSizer176 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer176->SetFlexibleDirection( wxBOTH );
	fgSizer176->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkEN_RXTSP = new wxCheckBox( this, ID_EN_RXTSP, wxT("Enable RxTSP"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_RXTSP->SetValue(true); 
	chkEN_RXTSP->SetToolTip( wxT("RxTSP modules enable") );
	chkEN_RXTSP->SetMinSize( wxSize( 135,-1 ) );
	
	fgSizer176->Add( chkEN_RXTSP, 1, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer117;
	sbSizer117 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bypass") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer177;
	fgSizer177 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer177->SetFlexibleDirection( wxBOTH );
	fgSizer177->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkDC_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_DC_BYP_RXTSP, wxT("DC corrector"), wxDefaultPosition, wxDefaultSize, 0 );
	chkDC_BYP_RXTSP->SetValue(true); 
	chkDC_BYP_RXTSP->SetToolTip( wxT("DC corrector bypass") );
	
	fgSizer177->Add( chkDC_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkDC_LOOP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_DC_BYP_RXTSP, wxT("DC tracking loop"), wxDefaultPosition, wxDefaultSize, 0 );
	chkDC_LOOP_RXTSP->SetToolTip( wxT("DC corrector bypass") );
	
	fgSizer177->Add( chkDC_LOOP_RXTSP, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	chkGC_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_GC_BYP_RXTSP, wxT("Gain corrector"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGC_BYP_RXTSP->SetValue(true); 
	chkGC_BYP_RXTSP->SetToolTip( wxT("Gain corrector bypass") );
	
	fgSizer177->Add( chkGC_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkPH_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_PH_BYP_RXTSP, wxT("Phase corrector"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPH_BYP_RXTSP->SetValue(true); 
	chkPH_BYP_RXTSP->SetToolTip( wxT("Phase corrector bypass") );
	
	fgSizer177->Add( chkPH_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCMIX_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_CMIX_BYP_RXTSP, wxT("CMIX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCMIX_BYP_RXTSP->SetValue(true); 
	chkCMIX_BYP_RXTSP->SetToolTip( wxT("CMIX bypass") );
	
	fgSizer177->Add( chkCMIX_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkAGC_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_AGC_BYP_RXTSP, wxT("AGC"), wxDefaultPosition, wxDefaultSize, 0 );
	chkAGC_BYP_RXTSP->SetValue(true); 
	chkAGC_BYP_RXTSP->SetToolTip( wxT("AGC bypass") );
	
	fgSizer177->Add( chkAGC_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkGFIR1_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_GFIR1_BYP_RXTSP, wxT("GFIR1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGFIR1_BYP_RXTSP->SetToolTip( wxT("GFIR1 bypass") );
	
	fgSizer177->Add( chkGFIR1_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkGFIR2_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_GFIR2_BYP_RXTSP, wxT("GFIR2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGFIR2_BYP_RXTSP->SetValue(true); 
	chkGFIR2_BYP_RXTSP->SetToolTip( wxT("GFIR2 bypass") );
	
	fgSizer177->Add( chkGFIR2_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkGFIR3_BYP_RXTSP = new wxCheckBox( sbSizer117->GetStaticBox(), ID_GFIR3_BYP_RXTSP, wxT("GFIR3"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGFIR3_BYP_RXTSP->SetValue(true); 
	chkGFIR3_BYP_RXTSP->SetToolTip( wxT("GFIR3 bypass") );
	
	fgSizer177->Add( chkGFIR3_BYP_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer117->Add( fgSizer177, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer176->Add( sbSizer117, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer118;
	sbSizer118 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("BIST") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer178;
	fgSizer178 = new wxFlexGridSizer( 0, 2, 2, 5 );
	fgSizer178->AddGrowableCol( 1 );
	fgSizer178->SetFlexibleDirection( wxBOTH );
	fgSizer178->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT29 = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("BISTI:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT29->Wrap( -1 );
	fgSizer178->Add( ID_STATICTEXT29, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBISTI = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	lblBISTI->Wrap( -1 );
	fgSizer178->Add( lblBISTI, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 5 );
	
	ID_STATICTEXT30 = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("BSTATE_I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT30->Wrap( -1 );
	fgSizer178->Add( ID_STATICTEXT30, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBSTATE_I = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSTATE_I->Wrap( -14 );
	fgSizer178->Add( lblBSTATE_I, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 5 );
	
	ID_STATICTEXT31 = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("BISTQ:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT31->Wrap( -1 );
	fgSizer178->Add( ID_STATICTEXT31, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBISTQ = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	lblBISTQ->Wrap( -1 );
	fgSizer178->Add( lblBISTQ, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 5 );
	
	ID_STATICTEXT32 = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("BSTATE_Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT32->Wrap( -1 );
	fgSizer178->Add( ID_STATICTEXT32, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBSTATE_Q = new wxStaticText( sbSizer118->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSTATE_Q->Wrap( -1 );
	fgSizer178->Add( lblBSTATE_Q, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 5 );
	
	chkBSTART_RXTSP = new wxCheckBox( sbSizer118->GetStaticBox(), ID_BSTART_RXTSP, wxT("Start BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBSTART_RXTSP->SetValue(true); 
	chkBSTART_RXTSP->SetToolTip( wxT("Starts delta sigma built in self test. Keep it at 1 one at least three clock cycles") );
	
	fgSizer178->Add( chkBSTART_RXTSP, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnReadBIST = new wxButton( sbSizer118->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer178->Add( btnReadBIST, 0, wxEXPAND, 5 );
	
	
	sbSizer118->Add( fgSizer178, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer176->Add( sbSizer118, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer115;
	sbSizer115 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("RSSI") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer172;
	fgSizer172 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer172->AddGrowableCol( 1 );
	fgSizer172->SetFlexibleDirection( wxBOTH );
	fgSizer172->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT71 = new wxStaticText( sbSizer115->GetStaticBox(), wxID_ANY, wxT("ADCI:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT71->Wrap( -1 );
	fgSizer172->Add( ID_STATICTEXT71, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblADCI = new wxStaticText( sbSizer115->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblADCI->Wrap( -1 );
	fgSizer172->Add( lblADCI, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 5 );
	
	ID_STATICTEXT28 = new wxStaticText( sbSizer115->GetStaticBox(), wxID_ANY, wxT("ADCQ:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT28->Wrap( -1 );
	fgSizer172->Add( ID_STATICTEXT28, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblADCQ = new wxStaticText( sbSizer115->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 48,-1 ), 0 );
	lblADCQ->Wrap( -1 );
	fgSizer172->Add( lblADCQ, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 5 );
	
	ID_STATICTEXT51 = new wxStaticText( sbSizer115->GetStaticBox(), wxID_ANY, wxT("RSSI:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT51->Wrap( -1 );
	fgSizer172->Add( ID_STATICTEXT51, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblRSSI = new wxStaticText( sbSizer115->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 48,-1 ), 0 );
	lblRSSI->Wrap( -1 );
	fgSizer172->Add( lblRSSI, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 5 );
	
	chkCAPSEL_ADC_RXTSP = new wxCheckBox( sbSizer115->GetStaticBox(), ID_BSTART_RXTSP, wxT("CAPSEL_ADC"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer172->Add( chkCAPSEL_ADC_RXTSP, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_BUTTON1 = new wxButton( sbSizer115->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	ID_BUTTON1->SetDefault(); 
	ID_BUTTON1->SetMinSize( wxSize( 56,-1 ) );
	
	fgSizer172->Add( ID_BUTTON1, 0, wxEXPAND, 5 );
	
	
	sbSizer115->Add( fgSizer172, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer176->Add( sbSizer115, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer223->Add( fgSizer176, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer142;
	fgSizer142 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer142->SetFlexibleDirection( wxBOTH );
	fgSizer142->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer106;
	sbSizer106 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("NCO") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer152;
	fgSizer152 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer152->SetFlexibleDirection( wxBOTH );
	fgSizer152->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	sizerNCOgrid = new wxFlexGridSizer( 0, 1, 0, 5 );
	sizerNCOgrid->SetFlexibleDirection( wxBOTH );
	sizerNCOgrid->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer225;
	fgSizer225 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer225->AddGrowableCol( 0 );
	fgSizer225->AddGrowableCol( 1 );
	fgSizer225->SetFlexibleDirection( wxBOTH );
	fgSizer225->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	sizerNCOgrid->Add( fgSizer225, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer221;
	fgSizer221 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer221->SetFlexibleDirection( wxBOTH );
	fgSizer221->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer221->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tableTitleCol1 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("FCW (MHz)"), wxDefaultPosition, wxDefaultSize, 0 );
	tableTitleCol1->Wrap( -1 );
	fgSizer221->Add( tableTitleCol1, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	tableTitleCol2 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Angle (Deg)"), wxPoint( -1,-1 ), wxSize( -1,-1 ), 0 );
	tableTitleCol2->Wrap( -1 );
	fgSizer221->Add( tableTitleCol2, 1, wxLEFT|wxALIGN_RIGHT, 5 );
	
	rgrSEL0 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer221->Add( rgrSEL0, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO0 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO0->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO0->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO0->SetMaxLength( 10 );
	#endif
	fgSizer221->Add( txtFCWPHO0, 0, 0, 5 );
	
	txtAnglePHO0 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO0->Wrap( -1 );
	fgSizer221->Add( txtAnglePHO0, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer221, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2211;
	fgSizer2211 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2211->SetFlexibleDirection( wxBOTH );
	fgSizer2211->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL01 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2211->Add( rgrSEL01, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO01 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO01->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO01->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO01->SetMaxLength( 10 );
	#endif
	fgSizer2211->Add( txtFCWPHO01, 0, 0, 5 );
	
	txtAnglePHO01 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO01->Wrap( -1 );
	fgSizer2211->Add( txtAnglePHO01, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2211, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2212;
	fgSizer2212 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2212->SetFlexibleDirection( wxBOTH );
	fgSizer2212->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL02 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2212->Add( rgrSEL02, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO02 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO02->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO02->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO02->SetMaxLength( 10 );
	#endif
	fgSizer2212->Add( txtFCWPHO02, 0, 0, 5 );
	
	txtAnglePHO02 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO02->Wrap( -1 );
	fgSizer2212->Add( txtAnglePHO02, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2212, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2213;
	fgSizer2213 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2213->SetFlexibleDirection( wxBOTH );
	fgSizer2213->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL03 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2213->Add( rgrSEL03, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO03 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO03->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO03->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO03->SetMaxLength( 10 );
	#endif
	fgSizer2213->Add( txtFCWPHO03, 0, 0, 5 );
	
	txtAnglePHO03 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO03->Wrap( -1 );
	fgSizer2213->Add( txtAnglePHO03, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2213, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2214;
	fgSizer2214 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2214->SetFlexibleDirection( wxBOTH );
	fgSizer2214->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL04 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2214->Add( rgrSEL04, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO04 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO04->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO04->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO04->SetMaxLength( 10 );
	#endif
	fgSizer2214->Add( txtFCWPHO04, 0, 0, 5 );
	
	txtAnglePHO04 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO04->Wrap( -1 );
	fgSizer2214->Add( txtAnglePHO04, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2214, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2215;
	fgSizer2215 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2215->SetFlexibleDirection( wxBOTH );
	fgSizer2215->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL05 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2215->Add( rgrSEL05, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO05 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO05->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO05->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO05->SetMaxLength( 10 );
	#endif
	fgSizer2215->Add( txtFCWPHO05, 0, 0, 5 );
	
	txtAnglePHO05 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO05->Wrap( -1 );
	fgSizer2215->Add( txtAnglePHO05, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2215, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2216;
	fgSizer2216 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2216->SetFlexibleDirection( wxBOTH );
	fgSizer2216->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL06 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2216->Add( rgrSEL06, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO06 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO06->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO06->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO06->SetMaxLength( 10 );
	#endif
	fgSizer2216->Add( txtFCWPHO06, 0, 0, 5 );
	
	txtAnglePHO06 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO06->Wrap( -1 );
	fgSizer2216->Add( txtAnglePHO06, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2216, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2217;
	fgSizer2217 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2217->SetFlexibleDirection( wxBOTH );
	fgSizer2217->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL07 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2217->Add( rgrSEL07, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO07 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO07->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO07->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO07->SetMaxLength( 10 );
	#endif
	fgSizer2217->Add( txtFCWPHO07, 0, 0, 5 );
	
	txtAnglePHO07 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO07->Wrap( -1 );
	fgSizer2217->Add( txtAnglePHO07, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2217, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2218;
	fgSizer2218 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2218->SetFlexibleDirection( wxBOTH );
	fgSizer2218->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL08 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2218->Add( rgrSEL08, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO08 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO08->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO08->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO08->SetMaxLength( 10 );
	#endif
	fgSizer2218->Add( txtFCWPHO08, 0, 0, 5 );
	
	txtAnglePHO08 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO08->Wrap( -1 );
	fgSizer2218->Add( txtAnglePHO08, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2218, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2219;
	fgSizer2219 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2219->SetFlexibleDirection( wxBOTH );
	fgSizer2219->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL09 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2219->Add( rgrSEL09, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO09 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO09->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO09->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO09->SetMaxLength( 10 );
	#endif
	fgSizer2219->Add( txtFCWPHO09, 0, 0, 5 );
	
	txtAnglePHO09 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO09->Wrap( -1 );
	fgSizer2219->Add( txtAnglePHO09, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer2219, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22110;
	fgSizer22110 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22110->SetFlexibleDirection( wxBOTH );
	fgSizer22110->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL10 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22110->Add( rgrSEL10, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO10 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO10->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO10->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO10->SetMaxLength( 10 );
	#endif
	fgSizer22110->Add( txtFCWPHO10, 0, 0, 5 );
	
	txtAnglePHO10 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO10->Wrap( -1 );
	fgSizer22110->Add( txtAnglePHO10, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22110, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22111;
	fgSizer22111 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22111->SetFlexibleDirection( wxBOTH );
	fgSizer22111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL11 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22111->Add( rgrSEL11, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO11 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO11->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO11->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO11->SetMaxLength( 10 );
	#endif
	fgSizer22111->Add( txtFCWPHO11, 0, 0, 5 );
	
	txtAnglePHO11 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO11->Wrap( -1 );
	fgSizer22111->Add( txtAnglePHO11, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22111, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22112;
	fgSizer22112 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22112->SetFlexibleDirection( wxBOTH );
	fgSizer22112->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL12 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22112->Add( rgrSEL12, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO12 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO12->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO12->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO12->SetMaxLength( 10 );
	#endif
	fgSizer22112->Add( txtFCWPHO12, 0, 0, 5 );
	
	txtAnglePHO12 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO12->Wrap( -1 );
	fgSizer22112->Add( txtAnglePHO12, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22112, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22113;
	fgSizer22113 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22113->SetFlexibleDirection( wxBOTH );
	fgSizer22113->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL13 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22113->Add( rgrSEL13, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO13 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO13->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO13->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO13->SetMaxLength( 10 );
	#endif
	fgSizer22113->Add( txtFCWPHO13, 0, 0, 5 );
	
	txtAnglePHO13 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO13->Wrap( -1 );
	fgSizer22113->Add( txtAnglePHO13, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22113, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22114;
	fgSizer22114 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22114->SetFlexibleDirection( wxBOTH );
	fgSizer22114->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL14 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22114->Add( rgrSEL14, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO14 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO14->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO14->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO14->SetMaxLength( 10 );
	#endif
	fgSizer22114->Add( txtFCWPHO14, 0, 0, 5 );
	
	txtAnglePHO14 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO14->Wrap( -1 );
	fgSizer22114->Add( txtAnglePHO14, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22114, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22115;
	fgSizer22115 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer22115->SetFlexibleDirection( wxBOTH );
	fgSizer22115->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rgrSEL15 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer22115->Add( rgrSEL15, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHO15 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHO15->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHO15->SetMaxLength( 10 );
	}
	#else
	txtFCWPHO15->SetMaxLength( 10 );
	#endif
	fgSizer22115->Add( txtFCWPHO15, 0, 0, 5 );
	
	txtAnglePHO15 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
	txtAnglePHO15->Wrap( -1 );
	fgSizer22115->Add( txtAnglePHO15, 0, wxALL, 5 );
	
	
	sizerNCOgrid->Add( fgSizer22115, 1, wxEXPAND, 5 );
	
	
	fgSizer152->Add( sizerNCOgrid, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer228;
	fgSizer228 = new wxFlexGridSizer( 0, 1, 5, 0 );
	fgSizer228->SetFlexibleDirection( wxBOTH );
	fgSizer228->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer154;
	fgSizer154 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer154->SetFlexibleDirection( wxBOTH );
	fgSizer154->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT21 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("RefClk (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT21->Wrap( -1 );
	fgSizer154->Add( ID_STATICTEXT21, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	lblRefClk = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("30.72"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRefClk->Wrap( -1 );
	fgSizer154->Add( lblRefClk, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer228->Add( fgSizer154, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	btnUploadNCO = new wxButton( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Upload NCO"), wxDefaultPosition, wxDefaultSize, 0 );
	btnUploadNCO->SetDefault(); 
	fgSizer228->Add( btnUploadNCO, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrMODE_RXChoices[] = { wxT("FCW"), wxT("PHO") };
	int rgrMODE_RXNChoices = sizeof( rgrMODE_RXChoices ) / sizeof( wxString );
	rgrMODE_RX = new wxRadioBox( sbSizer106->GetStaticBox(), ID_MODE_RX, wxT("Mode"), wxDefaultPosition, wxDefaultSize, rgrMODE_RXNChoices, rgrMODE_RXChoices, 2, wxRA_SPECIFY_COLS );
	rgrMODE_RX->SetSelection( 1 );
	rgrMODE_RX->SetToolTip( wxT("Memory table mode") );
	
	fgSizer228->Add( rgrMODE_RX, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer229;
	fgSizer229 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer229->AddGrowableCol( 1 );
	fgSizer229->SetFlexibleDirection( wxBOTH );
	fgSizer229->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblFCWPHOmodeName = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("FCW (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFCWPHOmodeName->Wrap( -1 );
	fgSizer229->Add( lblFCWPHOmodeName, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtFCWPHOmodeAdditional = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtFCWPHOmodeAdditional->HasFlag( wxTE_MULTILINE ) )
	{
	txtFCWPHOmodeAdditional->SetMaxLength( 10 );
	}
	#else
	txtFCWPHOmodeAdditional->SetMaxLength( 10 );
	#endif
	fgSizer229->Add( txtFCWPHOmodeAdditional, 1, wxEXPAND, 5 );
	
	
	fgSizer228->Add( fgSizer229, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer153;
	fgSizer153 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer153->AddGrowableCol( 1 );
	fgSizer153->SetFlexibleDirection( wxBOTH );
	fgSizer153->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT25 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Bits to dither:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT25->Wrap( -1 );
	fgSizer153->Add( ID_STATICTEXT25, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbDTHBIT_RX = new wxComboBox( sbSizer106->GetStaticBox(), ID_DTHBIT_RX, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	cmbDTHBIT_RX->SetToolTip( wxT("NCO bits to dither") );
	
	fgSizer153->Add( cmbDTHBIT_RX, 1, wxEXPAND, 5 );
	
	
	fgSizer228->Add( fgSizer153, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer105;
	sbSizer105 = new wxStaticBoxSizer( new wxStaticBox( sbSizer106->GetStaticBox(), wxID_ANY, wxT("TSG") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer147;
	fgSizer147 = new wxFlexGridSizer( 0, 1, 5, 0 );
	fgSizer147->SetFlexibleDirection( wxBOTH );
	fgSizer147->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkTSGSWAPIQ_RXTSP = new wxCheckBox( sbSizer105->GetStaticBox(), ID_TSGSWAPIQ_RXTSP, wxT("Swap I and Q\n signal sources from TSG"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTSGSWAPIQ_RXTSP->SetValue(true); 
	chkTSGSWAPIQ_RXTSP->SetToolTip( wxT("Swap signals at test signal generator's output") );
	
	fgSizer147->Add( chkTSGSWAPIQ_RXTSP, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer148;
	fgSizer148 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer148->SetFlexibleDirection( wxBOTH );
	fgSizer148->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rgrTSGFCW_RXTSPChoices[] = { wxT("TSP clk/8"), wxT("TSP clk/4") };
	int rgrTSGFCW_RXTSPNChoices = sizeof( rgrTSGFCW_RXTSPChoices ) / sizeof( wxString );
	rgrTSGFCW_RXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGFCW_RXTSP, wxT("TSGFCW"), wxDefaultPosition, wxDefaultSize, rgrTSGFCW_RXTSPNChoices, rgrTSGFCW_RXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrTSGFCW_RXTSP->SetSelection( 0 );
	rgrTSGFCW_RXTSP->SetToolTip( wxT("Set frequency of TSG's NCO") );
	
	fgSizer148->Add( rgrTSGFCW_RXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrTSGMODE_RXTSPChoices[] = { wxT("NCO"), wxT("DC source") };
	int rgrTSGMODE_RXTSPNChoices = sizeof( rgrTSGMODE_RXTSPChoices ) / sizeof( wxString );
	rgrTSGMODE_RXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGMODE_RXTSP, wxT("TSGMODE"), wxDefaultPosition, wxDefaultSize, rgrTSGMODE_RXTSPNChoices, rgrTSGMODE_RXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrTSGMODE_RXTSP->SetSelection( 0 );
	rgrTSGMODE_RXTSP->SetToolTip( wxT("Test signal generator mode") );
	
	fgSizer148->Add( rgrTSGMODE_RXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrINSEL_RXTSPChoices[] = { wxT("ADC"), wxT("Test signal") };
	int rgrINSEL_RXTSPNChoices = sizeof( rgrINSEL_RXTSPChoices ) / sizeof( wxString );
	rgrINSEL_RXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_INSEL_RXTSP, wxT("Input source"), wxDefaultPosition, wxDefaultSize, rgrINSEL_RXTSPNChoices, rgrINSEL_RXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrINSEL_RXTSP->SetSelection( 0 );
	rgrINSEL_RXTSP->SetToolTip( wxT("Input source of TxTSP") );
	
	fgSizer148->Add( rgrINSEL_RXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxString rgrTSGFC_RXTSPChoices[] = { wxT("-6 dB"), wxT("Full scale") };
	int rgrTSGFC_RXTSPNChoices = sizeof( rgrTSGFC_RXTSPChoices ) / sizeof( wxString );
	rgrTSGFC_RXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGFC_RXTSP, wxT("TSGFC"), wxDefaultPosition, wxDefaultSize, rgrTSGFC_RXTSPNChoices, rgrTSGFC_RXTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrTSGFC_RXTSP->SetSelection( 1 );
	rgrTSGFC_RXTSP->SetToolTip( wxT("TSG full scale control") );
	
	fgSizer148->Add( rgrTSGFC_RXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer147->Add( fgSizer148, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizer105->Add( fgSizer147, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer228->Add( sbSizer105, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxStaticBoxSizer* sbSizer134;
	sbSizer134 = new wxStaticBoxSizer( new wxStaticBox( sbSizer106->GetStaticBox(), wxID_ANY, wxT("DC avg. window size:") ), wxVERTICAL );
	
	cmbDCCORR_AVG = new wxComboBox( sbSizer134->GetStaticBox(), ID_DCCORR_AVG, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	sbSizer134->Add( cmbDCCORR_AVG, 0, wxEXPAND, 5 );
	
	
	fgSizer228->Add( sbSizer134, 1, wxEXPAND, 5 );
	
	
	fgSizer152->Add( fgSizer228, 1, wxEXPAND, 5 );
	
	
	sbSizer106->Add( fgSizer152, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer142->Add( sbSizer106, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer223->Add( fgSizer142, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer218;
	fgSizer218 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer218->SetFlexibleDirection( wxBOTH );
	fgSizer218->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer220;
	fgSizer220 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer220->AddGrowableCol( 1 );
	fgSizer220->SetFlexibleDirection( wxBOTH );
	fgSizer220->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer149;
	fgSizer149 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer149->SetFlexibleDirection( wxBOTH );
	fgSizer149->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer150;
	fgSizer150 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer150->SetFlexibleDirection( wxBOTH );
	fgSizer150->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT23 = new wxStaticText( this, wxID_ANY, wxT("DC_REG:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT23->Wrap( -1 );
	fgSizer150->Add( ID_STATICTEXT23, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtDC_REG_RXTSP = new wxTextCtrl( this, ID_DC_REG_RXTSP, wxT("ffff"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	fgSizer150->Add( txtDC_REG_RXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer149->Add( fgSizer150, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLoadDCI = new wxButton( this, wxID_ANY, wxT("Load to DC I"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLoadDCI->SetDefault(); 
	fgSizer149->Add( btnLoadDCI, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	btnLoadDCQ = new wxButton( this, wxID_ANY, wxT("Load to DC Q"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLoadDCQ->SetDefault(); 
	fgSizer149->Add( btnLoadDCQ, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer220->Add( fgSizer149, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer98;
	sbSizer98 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("CMIX") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer132;
	fgSizer132 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer132->AddGrowableCol( 0 );
	fgSizer132->SetFlexibleDirection( wxBOTH );
	fgSizer132->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cmbCMIX_SC_RXTSP = new wxComboBox( sbSizer98->GetStaticBox(), ID_CMIX_SC_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 );
	cmbCMIX_SC_RXTSP->Append( wxT("Upconvert") );
	cmbCMIX_SC_RXTSP->Append( wxT("Downconvert") );
	fgSizer132->Add( cmbCMIX_SC_RXTSP, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer133;
	fgSizer133 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer133->AddGrowableCol( 1 );
	fgSizer133->SetFlexibleDirection( wxBOTH );
	fgSizer133->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT24 = new wxStaticText( sbSizer98->GetStaticBox(), wxID_ANY, wxT("Gain:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT24->Wrap( -1 );
	fgSizer133->Add( ID_STATICTEXT24, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbCMIX_GAIN_RXTSP = new wxComboBox( sbSizer98->GetStaticBox(), ID_CMIX_GAIN_RXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbCMIX_GAIN_RXTSP->Append( wxT("-6 dB") );
	cmbCMIX_GAIN_RXTSP->Append( wxT("-3 dB") );
	cmbCMIX_GAIN_RXTSP->Append( wxT("0 dB") );
	cmbCMIX_GAIN_RXTSP->Append( wxT("+3 dB") );
	cmbCMIX_GAIN_RXTSP->Append( wxT("+6 dB") );
	cmbCMIX_GAIN_RXTSP->SetToolTip( wxT("Gain of CMIX output") );
	
	fgSizer133->Add( cmbCMIX_GAIN_RXTSP, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	
	fgSizer132->Add( fgSizer133, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer98->Add( fgSizer132, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer220->Add( sbSizer98, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer134;
	fgSizer134 = new wxFlexGridSizer( 2, 1, 10, 0 );
	fgSizer134->AddGrowableCol( 0 );
	fgSizer134->SetFlexibleDirection( wxBOTH );
	fgSizer134->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer99;
	sbSizer99 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Decimation") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer253;
	fgSizer253 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer253->AddGrowableCol( 1 );
	fgSizer253->SetFlexibleDirection( wxBOTH );
	fgSizer253->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizer99->GetStaticBox(), wxID_ANY, wxT("HBD ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	ID_STATICTEXT7->SetMinSize( wxSize( 80,-1 ) );
	
	fgSizer253->Add( ID_STATICTEXT7, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbHBD_OVR_RXTSP = new wxComboBox( sbSizer99->GetStaticBox(), ID_HBD_OVR_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	cmbHBD_OVR_RXTSP->SetToolTip( wxT("HBI interpolation ratio") );
	
	fgSizer253->Add( cmbHBD_OVR_RXTSP, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 0 );
	
	
	sbSizer99->Add( fgSizer253, 1, wxEXPAND, 5 );
	
	
	fgSizer134->Add( sbSizer99, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 0 );
	
	wxFlexGridSizer* fgSizer252;
	fgSizer252 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer252->AddGrowableCol( 1 );
	fgSizer252->SetFlexibleDirection( wxBOTH );
	fgSizer252->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT72 = new wxStaticText( this, wxID_ANY, wxT("Delay line:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT72->Wrap( -1 );
	ID_STATICTEXT72->SetMinSize( wxSize( 80,-1 ) );
	
	fgSizer252->Add( ID_STATICTEXT72, 0, wxALL, 5 );
	
	cmbHBD_DLY = new wxComboBox( this, ID_HBD_OVR_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 );
	cmbHBD_DLY->Append( wxT("No delay") );
	cmbHBD_DLY->Append( wxT("1 sample") );
	cmbHBD_DLY->Append( wxT("2 samples") );
	cmbHBD_DLY->Append( wxT("3 samples") );
	cmbHBD_DLY->Append( wxT("4 samples") );
	fgSizer252->Add( cmbHBD_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxEXPAND, 0 );
	
	
	fgSizer134->Add( fgSizer252, 1, wxEXPAND, 5 );
	
	
	fgSizer220->Add( fgSizer134, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer218->Add( fgSizer220, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer139;
	sbSizer139 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("GFIR") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer260;
	fgSizer260 = new wxFlexGridSizer( 2, 0, 0, 0 );
	fgSizer260->SetFlexibleDirection( wxBOTH );
	fgSizer260->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer135;
	fgSizer135 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer135->SetFlexibleDirection( wxBOTH );
	fgSizer135->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer100;
	sbSizer100 = new wxStaticBoxSizer( new wxStaticBox( sbSizer139->GetStaticBox(), wxID_ANY, wxT("GFIR1") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer137;
	fgSizer137 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer137->AddGrowableCol( 1 );
	fgSizer137->SetFlexibleDirection( wxBOTH );
	fgSizer137->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer137->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR1_L_RXTSP = new wxComboBox( sbSizer100->GetStaticBox(), ID_GFIR1_L_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0, NULL, 0 ); 
	cmbGFIR1_L_RXTSP->SetToolTip( wxT("Parameter l of GFIR1 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
	
	fgSizer137->Add( cmbGFIR1_L_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer137->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR1_N_RXTSP = new wxSpinCtrl( sbSizer100->GetStaticBox(), ID_GFIR1_N_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 255, 255 );
	cmbGFIR1_N_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer137->Add( cmbGFIR1_N_RXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer100->Add( fgSizer137, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnGFIR1Coef = new wxButton( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
	btnGFIR1Coef->SetDefault(); 
	sbSizer100->Add( btnGFIR1Coef, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	fgSizer135->Add( sbSizer100, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer101;
	sbSizer101 = new wxStaticBoxSizer( new wxStaticBox( sbSizer139->GetStaticBox(), wxID_ANY, wxT("GFIR2") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer139;
	fgSizer139 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer139->AddGrowableCol( 1 );
	fgSizer139->SetFlexibleDirection( wxBOTH );
	fgSizer139->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( -1 );
	fgSizer139->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR2_L_RXTSP = new wxComboBox( sbSizer101->GetStaticBox(), ID_GFIR2_L_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0, NULL, 0 ); 
	cmbGFIR2_L_RXTSP->SetToolTip( wxT("Parameter l of GFIR2 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
	
	fgSizer139->Add( cmbGFIR2_L_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( -1 );
	fgSizer139->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR2_N_RXTSP = new wxSpinCtrl( sbSizer101->GetStaticBox(), ID_GFIR2_N_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 255, 255 );
	cmbGFIR2_N_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer139->Add( cmbGFIR2_N_RXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer101->Add( fgSizer139, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnGFIR2Coef = new wxButton( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
	btnGFIR2Coef->SetDefault(); 
	sbSizer101->Add( btnGFIR2Coef, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer135->Add( sbSizer101, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer102;
	sbSizer102 = new wxStaticBoxSizer( new wxStaticBox( sbSizer139->GetStaticBox(), wxID_ANY, wxT("GFIR3") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer141;
	fgSizer141 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer141->AddGrowableCol( 1 );
	fgSizer141->SetFlexibleDirection( wxBOTH );
	fgSizer141->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( -1 );
	fgSizer141->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR3_L_RXTSP = new wxComboBox( sbSizer102->GetStaticBox(), ID_GFIR3_L_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0, NULL, 0 ); 
	cmbGFIR3_L_RXTSP->SetToolTip( wxT("Parameter l of GFIR3 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
	
	fgSizer141->Add( cmbGFIR3_L_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( -1 );
	fgSizer141->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGFIR3_N_RXTSP = new wxSpinCtrl( sbSizer102->GetStaticBox(), ID_GFIR3_N_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 255, 255 );
	cmbGFIR3_N_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer141->Add( cmbGFIR3_N_RXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer102->Add( fgSizer141, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnGFIR3Coef = new wxButton( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
	btnGFIR3Coef->SetDefault(); 
	sbSizer102->Add( btnGFIR3Coef, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer135->Add( sbSizer102, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer260->Add( fgSizer135, 1, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer145;
	sbSizer145 = new wxStaticBoxSizer( new wxStaticBox( sbSizer139->GetStaticBox(), wxID_ANY, wxT("Configure GFIRs as LPF") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1351;
	fgSizer1351 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer1351->SetFlexibleDirection( wxBOTH );
	fgSizer1351->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	txtBW = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Bandwidth (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	txtBW->Wrap( -1 );
	fgSizer1351->Add( txtBW, 0, wxALIGN_CENTER|wxALL, 0 );
	
	txtLPFBW = new wxTextCtrl( sbSizer145->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtLPFBW->HasFlag( wxTE_MULTILINE ) )
	{
	txtLPFBW->SetMaxLength( 8 );
	}
	#else
	txtLPFBW->SetMaxLength( 8 );
	#endif
	fgSizer1351->Add( txtLPFBW, 0, wxALL, 0 );
	
	btnSetLPF = new wxButton( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	btnSetLPF->SetDefault(); 
	fgSizer1351->Add( btnSetLPF, 0, wxALL, 0 );
	
	txtRate = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Sample rate"), wxDefaultPosition, wxDefaultSize, 0 );
	txtRate->Wrap( -1 );
	fgSizer1351->Add( txtRate, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 0 );
	
	txtRateVal = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("0 MHz"), wxDefaultPosition, wxDefaultSize, 0 );
	txtRateVal->Wrap( -1 );
	fgSizer1351->Add( txtRateVal, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5 );
	
	
	sbSizer145->Add( fgSizer1351, 1, wxEXPAND, 5 );
	
	
	fgSizer260->Add( sbSizer145, 1, wxEXPAND, 5 );
	
	
	sbSizer139->Add( fgSizer260, 1, wxEXPAND, 5 );
	
	
	fgSizer218->Add( sbSizer139, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer219;
	fgSizer219 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer219->AddGrowableCol( 0 );
	fgSizer219->SetFlexibleDirection( wxBOTH );
	fgSizer219->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer96;
	sbSizer96 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
	
	cmbIQCORR_RXTSP = new NumericSlider( sbSizer96->GetStaticBox(), ID_IQCORR_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
	cmbIQCORR_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	sbSizer96->Add( cmbIQCORR_RXTSP, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer130;
	fgSizer130 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer130->SetFlexibleDirection( wxBOTH );
	fgSizer130->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer130->Add( ID_STATICTEXT16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtPhaseAlpha = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txtPhaseAlpha->Wrap( -1 );
	fgSizer130->Add( txtPhaseAlpha, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer96->Add( fgSizer130, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	fgSizer219->Add( sbSizer96, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer97;
	sbSizer97 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer131;
	fgSizer131 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer131->AddGrowableCol( 1 );
	fgSizer131->SetFlexibleDirection( wxBOTH );
	fgSizer131->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer131->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRI_RXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRI_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRI_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer131->Add( cmbGCORRI_RXTSP, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer131->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRQ_RXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRQ_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRQ_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer131->Add( cmbGCORRQ_RXTSP, 0, wxEXPAND, 5 );
	
	ID_BUTTON10 = new wxButton( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON10->SetDefault(); 
	ID_BUTTON10->Hide();
	
	fgSizer131->Add( ID_BUTTON10, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer97->Add( fgSizer131, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer219->Add( sbSizer97, 1, wxEXPAND, 5 );
	
	
	fgSizer218->Add( fgSizer219, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer116;
	sbSizer116 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("AGC") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer173;
	fgSizer173 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer173->AddGrowableCol( 1 );
	fgSizer173->SetFlexibleDirection( wxBOTH );
	fgSizer173->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT91 = new wxStaticText( sbSizer116->GetStaticBox(), wxID_ANY, wxT("Mode:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT91->Wrap( -1 );
	fgSizer173->Add( ID_STATICTEXT91, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	cmbAGC_MODE_RXTSP = new wxComboBox( sbSizer116->GetStaticBox(), ID_AGC_MODE_RXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmbAGC_MODE_RXTSP->Append( wxT("AGC") );
	cmbAGC_MODE_RXTSP->Append( wxT("RSSI") );
	cmbAGC_MODE_RXTSP->Append( wxT("Bypass") );
	fgSizer173->Add( cmbAGC_MODE_RXTSP, 1, wxALIGN_LEFT|wxEXPAND, 5 );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer116->GetStaticBox(), wxID_ANY, wxT("Window size:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer173->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbAGC_AVG_RXTSP = new wxComboBox( sbSizer116->GetStaticBox(), ID_AGC_AVG_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( 64,-1 ), 0, NULL, 0 );
	cmbAGC_AVG_RXTSP->Append( wxT("2^7") );
	cmbAGC_AVG_RXTSP->Append( wxT("2^8") );
	cmbAGC_AVG_RXTSP->Append( wxT("2^9") );
	cmbAGC_AVG_RXTSP->Append( wxT("2^10") );
	cmbAGC_AVG_RXTSP->Append( wxT("2^11") );
	cmbAGC_AVG_RXTSP->Append( wxT("2^12") );
	cmbAGC_AVG_RXTSP->Append( wxT("2^13") );
	cmbAGC_AVG_RXTSP->Append( wxT("2^14") );
	cmbAGC_AVG_RXTSP->SetToolTip( wxT("AGC Averaging window size") );
	
	fgSizer173->Add( cmbAGC_AVG_RXTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizer116->GetStaticBox(), wxID_ANY, wxT("Loop gain:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer173->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	spinAGC_K_RXTSP = new NumericSlider( sbSizer116->GetStaticBox(), ID_AGC_K_RXTSP, wxT("1"), wxDefaultPosition, wxSize( 64,-1 ), wxSP_ARROW_KEYS, 0, 262143, 1 );
	spinAGC_K_RXTSP->SetToolTip( wxT("AGC loop gain") );
	
	fgSizer173->Add( spinAGC_K_RXTSP, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT2 = new wxStaticText( sbSizer116->GetStaticBox(), wxID_ANY, wxT("Output level:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer173->Add( ID_STATICTEXT2, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbAGC_ADESIRED_RXTSP = new NumericSlider( sbSizer116->GetStaticBox(), ID_AGC_ADESIRED_RXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 4096, 0 );
	fgSizer173->Add( cmbAGC_ADESIRED_RXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer116->Add( fgSizer173, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	
	fgSizer218->Add( sbSizer116, 1, wxEXPAND, 5 );
	
	
	fgSizer223->Add( fgSizer218, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer223 );
	this->Layout();
	
	// Connect Events
	chkEN_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkDC_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkDC_LOOP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGC_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkPH_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkCMIX_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkAGC_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR1_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR2_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR3_BYP_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkBSTART_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnReadBIST->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnReadBISTSignature ), NULL, this );
	chkCAPSEL_ADC_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	ID_BUTTON1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnReadRSSI ), NULL, this );
	rgrSEL0->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO0->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL01->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO01->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL02->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO02->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL03->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO03->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL04->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO04->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL05->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO05->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL06->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO06->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL07->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO07->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL08->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO08->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL09->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO09->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL10->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO10->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL11->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO11->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL12->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO12->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL13->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO13->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL14->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO14->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL15->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO15->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	btnUploadNCO->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnUploadNCOClick ), NULL, this );
	rgrMODE_RX->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	txtFCWPHOmodeAdditional->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlRxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	txtFCWPHOmodeAdditional->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	cmbDTHBIT_RX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkTSGSWAPIQ_RXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFCW_RXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGMODE_RXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrINSEL_RXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFC_RXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORR_AVG->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnLoadDCI->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnLoadDCIClick ), NULL, this );
	btnLoadDCQ->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnLoadDCQClick ), NULL, this );
	cmbCMIX_SC_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbCMIX_GAIN_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbHBD_OVR_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbHBD_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_L_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_N_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR1Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::onbtnGFIR1Coef ), NULL, this );
	cmbGFIR2_L_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR2_N_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR2Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::onbtnGFIR2Coef ), NULL, this );
	cmbGFIR3_L_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR3_N_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR3Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::onbtnGFIR3Coef ), NULL, this );
	txtLPFBW->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlRxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	btnSetLPF->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnSetLPFClick ), NULL, this );
	cmbIQCORR_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRI_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbAGC_MODE_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbAGC_AVG_RXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	spinAGC_K_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbAGC_ADESIRED_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
}

pnlRxTSP_view::~pnlRxTSP_view()
{
	// Disconnect Events
	chkEN_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkDC_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkDC_LOOP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGC_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkPH_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkCMIX_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkAGC_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR1_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR2_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkGFIR3_BYP_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkBSTART_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnReadBIST->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnReadBISTSignature ), NULL, this );
	chkCAPSEL_ADC_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	ID_BUTTON1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnReadRSSI ), NULL, this );
	rgrSEL0->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO0->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL01->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO01->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL02->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO02->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL03->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO03->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL04->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO04->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL05->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO05->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL06->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO06->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL07->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO07->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL08->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO08->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL09->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO09->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL10->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO10->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL11->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO11->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL12->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO12->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL13->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO13->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL14->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO14->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	rgrSEL15->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlRxTSP_view::OnNCOSelectionChange ), NULL, this );
	txtFCWPHO15->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	btnUploadNCO->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnUploadNCOClick ), NULL, this );
	rgrMODE_RX->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	txtFCWPHOmodeAdditional->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlRxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	txtFCWPHOmodeAdditional->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( pnlRxTSP_view::PHOinputChanged ), NULL, this );
	cmbDTHBIT_RX->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	chkTSGSWAPIQ_RXTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFCW_RXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGMODE_RXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrINSEL_RXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	rgrTSGFC_RXTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORR_AVG->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnLoadDCI->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnLoadDCIClick ), NULL, this );
	btnLoadDCQ->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnLoadDCQClick ), NULL, this );
	cmbCMIX_SC_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbCMIX_GAIN_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbHBD_OVR_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbHBD_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_L_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR1_N_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR1Coef->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::onbtnGFIR1Coef ), NULL, this );
	cmbGFIR2_L_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR2_N_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR2Coef->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::onbtnGFIR2Coef ), NULL, this );
	cmbGFIR3_L_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGFIR3_N_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	btnGFIR3Coef->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::onbtnGFIR3Coef ), NULL, this );
	txtLPFBW->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( pnlRxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
	btnSetLPF->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlRxTSP_view::OnbtnSetLPFClick ), NULL, this );
	cmbIQCORR_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRI_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbAGC_MODE_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbAGC_AVG_RXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	spinAGC_K_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	cmbAGC_ADESIRED_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlRxTSP_view::ParameterChangeHandler ), NULL, this );
	
}

pnlCDS_view::pnlCDS_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer190;
	fgSizer190 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer190->SetFlexibleDirection( wxBOTH );
	fgSizer190->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer121;
	sbSizer121 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Clock inversion") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer191;
	fgSizer191 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer191->SetFlexibleDirection( wxBOTH );
	fgSizer191->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkCDSN_TXBTSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXBTSP, wxT("TX TSPB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_TXBTSP->SetToolTip( wxT("TX TSPB clock inversion control") );
	
	fgSizer191->Add( chkCDSN_TXBTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_TXATSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXATSP, wxT("TX TSPA"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_TXATSP->SetToolTip( wxT("TX TSPA clock inversion control") );
	
	fgSizer191->Add( chkCDSN_TXATSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_RXBTSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXBTSP, wxT("RX TSPB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_RXBTSP->SetToolTip( wxT("RX TSPB clock inversion control") );
	
	fgSizer191->Add( chkCDSN_RXBTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_RXATSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXATSP, wxT("RX TSPA"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_RXATSP->SetToolTip( wxT("RX TSPA clock inversion control") );
	
	fgSizer191->Add( chkCDSN_RXATSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_TXBLML = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXBLML, wxT("TX LMLB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_TXBLML->SetToolTip( wxT("TX LMLB clock inversion control") );
	
	fgSizer191->Add( chkCDSN_TXBLML, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_TXALML = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXALML, wxT("TX LMLA"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_TXALML->SetToolTip( wxT("TX LMLA clock inversion control") );
	
	fgSizer191->Add( chkCDSN_TXALML, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_RXBLML = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXBLML, wxT("RX LMLB"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_RXBLML->SetToolTip( wxT("RX LMLB clock inversion control") );
	
	fgSizer191->Add( chkCDSN_RXBLML, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_RXALML  = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXALML , wxT("RX LMLA"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_RXALML ->SetToolTip( wxT("RX LMLA clock inversion control") );
	
	fgSizer191->Add( chkCDSN_RXALML , 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_MCLK2 = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_MCLK2, wxT("MCLK2"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_MCLK2->SetToolTip( wxT("MCLK2 clock inversion control") );
	
	fgSizer191->Add( chkCDSN_MCLK2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkCDSN_MCLK1 = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_MCLK1, wxT("MCLK1"), wxDefaultPosition, wxDefaultSize, 0 );
	chkCDSN_MCLK1->SetToolTip( wxT("MCLK1 clock inversion control") );
	
	fgSizer191->Add( chkCDSN_MCLK1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer121->Add( fgSizer191, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer190->Add( sbSizer121, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxStaticBoxSizer* sbSizer122;
	sbSizer122 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Clock delay") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer192;
	fgSizer192 = new wxFlexGridSizer( 0, 5, 5, 5 );
	fgSizer192->SetFlexibleDirection( wxBOTH );
	fgSizer192->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rgrCDS_MCLK2Choices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
	int rgrCDS_MCLK2NChoices = sizeof( rgrCDS_MCLK2Choices ) / sizeof( wxString );
	rgrCDS_MCLK2 = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_MCLK2, wxT("MCLK2"), wxDefaultPosition, wxDefaultSize, rgrCDS_MCLK2NChoices, rgrCDS_MCLK2Choices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_MCLK2->SetSelection( 0 );
	rgrCDS_MCLK2->SetToolTip( wxT("MCLK2 clock delay") );
	
	fgSizer192->Add( rgrCDS_MCLK2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_MCLK1Choices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
	int rgrCDS_MCLK1NChoices = sizeof( rgrCDS_MCLK1Choices ) / sizeof( wxString );
	rgrCDS_MCLK1 = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_MCLK1, wxT("MCLK1"), wxDefaultPosition, wxDefaultSize, rgrCDS_MCLK1NChoices, rgrCDS_MCLK1Choices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_MCLK1->SetSelection( 0 );
	rgrCDS_MCLK1->SetToolTip( wxT("MCLK1 clock delay") );
	
	fgSizer192->Add( rgrCDS_MCLK1, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_TXBTSPChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
	int rgrCDS_TXBTSPNChoices = sizeof( rgrCDS_TXBTSPChoices ) / sizeof( wxString );
	rgrCDS_TXBTSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXBTSP, wxT("TX TSP B"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXBTSPNChoices, rgrCDS_TXBTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_TXBTSP->SetSelection( 0 );
	rgrCDS_TXBTSP->SetToolTip( wxT("TX TSP B clock delay") );
	
	fgSizer192->Add( rgrCDS_TXBTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_TXATSPChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
	int rgrCDS_TXATSPNChoices = sizeof( rgrCDS_TXATSPChoices ) / sizeof( wxString );
	rgrCDS_TXATSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXATSP, wxT("TX TSP A"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXATSPNChoices, rgrCDS_TXATSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_TXATSP->SetSelection( 0 );
	rgrCDS_TXATSP->SetToolTip( wxT("TX TSP A clock delay") );
	
	fgSizer192->Add( rgrCDS_TXATSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_RXBTSPChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
	int rgrCDS_RXBTSPNChoices = sizeof( rgrCDS_RXBTSPChoices ) / sizeof( wxString );
	rgrCDS_RXBTSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXBTSP, wxT("RX TSP B"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXBTSPNChoices, rgrCDS_RXBTSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_RXBTSP->SetSelection( 0 );
	rgrCDS_RXBTSP->SetToolTip( wxT("RX TSP B clock delay") );
	
	fgSizer192->Add( rgrCDS_RXBTSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_RXATSPChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
	int rgrCDS_RXATSPNChoices = sizeof( rgrCDS_RXATSPChoices ) / sizeof( wxString );
	rgrCDS_RXATSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXATSP, wxT("RX TSP A"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXATSPNChoices, rgrCDS_RXATSPChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_RXATSP->SetSelection( 0 );
	rgrCDS_RXATSP->SetToolTip( wxT("RX TSP A clock delay") );
	
	fgSizer192->Add( rgrCDS_RXATSP, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_TXBLMLChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
	int rgrCDS_TXBLMLNChoices = sizeof( rgrCDS_TXBLMLChoices ) / sizeof( wxString );
	rgrCDS_TXBLML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXBLML, wxT("TX LML B"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXBLMLNChoices, rgrCDS_TXBLMLChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_TXBLML->SetSelection( 0 );
	rgrCDS_TXBLML->SetToolTip( wxT("TX LML B clock delay") );
	
	fgSizer192->Add( rgrCDS_TXBLML, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_TXALMLChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
	int rgrCDS_TXALMLNChoices = sizeof( rgrCDS_TXALMLChoices ) / sizeof( wxString );
	rgrCDS_TXALML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXALML, wxT("TX LML A"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXALMLNChoices, rgrCDS_TXALMLChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_TXALML->SetSelection( 0 );
	rgrCDS_TXALML->SetToolTip( wxT("TX LML A clock delay") );
	
	fgSizer192->Add( rgrCDS_TXALML, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_RXBLMLChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
	int rgrCDS_RXBLMLNChoices = sizeof( rgrCDS_RXBLMLChoices ) / sizeof( wxString );
	rgrCDS_RXBLML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXBLML, wxT("RX LML B"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXBLMLNChoices, rgrCDS_RXBLMLChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_RXBLML->SetSelection( 0 );
	rgrCDS_RXBLML->SetToolTip( wxT("RX LML B clock delay") );
	
	fgSizer192->Add( rgrCDS_RXBLML, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxString rgrCDS_RXALMLChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
	int rgrCDS_RXALMLNChoices = sizeof( rgrCDS_RXALMLChoices ) / sizeof( wxString );
	rgrCDS_RXALML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXALML, wxT("RX LML A"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXALMLNChoices, rgrCDS_RXALMLChoices, 1, wxRA_SPECIFY_COLS );
	rgrCDS_RXALML->SetSelection( 0 );
	rgrCDS_RXALML->SetToolTip( wxT("RX LML A clock delay") );
	
	fgSizer192->Add( rgrCDS_RXALML, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer122->Add( fgSizer192, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer190->Add( sbSizer122, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	this->SetSizer( fgSizer190 );
	this->Layout();
	fgSizer190->Fit( this );
	
	// Connect Events
	chkCDSN_TXBTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_TXATSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXBTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXATSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_TXBLML->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_TXALML->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXBLML->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXALML ->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_MCLK2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_MCLK1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_MCLK2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_MCLK1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXBTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXATSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXBTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXATSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXBLML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXALML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXBLML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXALML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
}

pnlCDS_view::~pnlCDS_view()
{
	// Disconnect Events
	chkCDSN_TXBTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_TXATSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXBTSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXATSP->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_TXBLML->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_TXALML->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXBLML->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_RXALML ->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_MCLK2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	chkCDSN_MCLK1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_MCLK2->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_MCLK1->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXBTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXATSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXBTSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXATSP->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXBLML->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_TXALML->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXBLML->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	rgrCDS_RXALML->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCDS_view::ParameterChangeHandler ), NULL, this );
	
}

pnlBIST_view::pnlBIST_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer193;
	fgSizer193 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer193->SetFlexibleDirection( wxBOTH );
	fgSizer193->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer123;
	sbSizer123 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("BIST") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer194;
	fgSizer194 = new wxFlexGridSizer( 0, 2, 2, 10 );
	fgSizer194->SetFlexibleDirection( wxBOTH );
	fgSizer194->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkBENC = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BENC, wxT("Enable CGEN BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBENC->SetToolTip( wxT("enables CGEN BIST") );
	
	fgSizer194->Add( chkBENC, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSDM_TSTO_CGEN = new wxCheckBox( sbSizer123->GetStaticBox(), ID_SDM_TSTO_CGEN, wxT("Enable SDM_TSTO_CGEN outputs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSDM_TSTO_CGEN->SetValue(true); 
	fgSizer194->Add( chkSDM_TSTO_CGEN, 0, 0, 5 );
	
	chkBENR = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BENR, wxT("Enable SXR BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBENR->SetToolTip( wxT("enables receiver BIST") );
	
	fgSizer194->Add( chkBENR, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSDM_TSTO_SXR = new wxCheckBox( sbSizer123->GetStaticBox(), ID_SDM_TSTO_SXR, wxT("Enable SDM_TSTO_SXR outputs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSDM_TSTO_SXR->SetValue(true); 
	fgSizer194->Add( chkSDM_TSTO_SXR, 0, 0, 5 );
	
	chkBENT = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BENT, wxT("Enable SXT BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBENT->SetToolTip( wxT("enables transmitter  BIST") );
	
	fgSizer194->Add( chkBENT, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkSDM_TSTO_SXT = new wxCheckBox( sbSizer123->GetStaticBox(), ID_SDM_TSTO_SXT, wxT("Enable SDM_TSTO_SXT outputs"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSDM_TSTO_SXT->SetValue(true); 
	fgSizer194->Add( chkSDM_TSTO_SXT, 0, 0, 5 );
	
	chkBSTART = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BSTART, wxT("Start SDM BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	chkBSTART->SetToolTip( wxT("Starts delta sigma built in self test. Keep it at 1 one at least three clock cycles") );
	
	fgSizer194->Add( chkBSTART, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer194->Add( 0, 0, 1, wxEXPAND, 5 );
	
	ID_STATICTEXT2 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST state"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer194->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblBSTATE = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSTATE, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSTATE->Wrap( 0 );
	fgSizer194->Add( lblBSTATE, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST signature (Transmitter)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( -1 );
	fgSizer194->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblBSIGT = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSIGT, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSIGT->Wrap( 0 );
	fgSizer194->Add( lblBSIGT, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST signature (Receiver)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer194->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblBSIGR = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSIGR, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSIGR->Wrap( 0 );
	fgSizer194->Add( lblBSIGR, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST signature (CGEN)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer194->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	lblBSIGC = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSIGC, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBSIGC->Wrap( 0 );
	fgSizer194->Add( lblBSIGC, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	btnReadSignature = new wxButton( sbSizer123->GetStaticBox(), ID_BTN_READ_SIGNATURE, wxT("Read BIST"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer194->Add( btnReadSignature, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer123->Add( fgSizer194, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	fgSizer193->Add( sbSizer123, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	this->SetSizer( fgSizer193 );
	this->Layout();
	fgSizer193->Fit( this );
	
	// Connect Events
	chkBENC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkSDM_TSTO_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkBENR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkSDM_TSTO_SXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkBENT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkSDM_TSTO_SXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkBSTART->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	btnReadSignature->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBIST_view::onbtnReadSignature ), NULL, this );
}

pnlBIST_view::~pnlBIST_view()
{
	// Disconnect Events
	chkBENC->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkSDM_TSTO_CGEN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkBENR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkSDM_TSTO_SXR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkBENT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkSDM_TSTO_SXT->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	chkBSTART->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBIST_view::ParameterChangeHandler ), NULL, this );
	btnReadSignature->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlBIST_view::onbtnReadSignature ), NULL, this );
	
}

pnlMCU_BD_view::pnlMCU_BD_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer195;
	fgSizer195 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer195->AddGrowableCol( 0 );
	fgSizer195->AddGrowableRow( 0 );
	fgSizer195->SetFlexibleDirection( wxBOTH );
	fgSizer195->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer196;
	fgSizer196 = new wxFlexGridSizer( 0, 3, 0, 5 );
	fgSizer196->SetFlexibleDirection( wxBOTH );
	fgSizer196->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer197;
	fgSizer197 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer197->SetFlexibleDirection( wxBOTH );
	fgSizer197->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer124;
	sbSizer124 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Load .hex file:") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizer124->GetStaticBox(), wxID_ANY, wxT("Select the file:"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	ID_STATICTEXT1->Wrap( -1 );
	bSizer2->Add( ID_STATICTEXT1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	Button_LOADHEX = new wxButton( sbSizer124->GetStaticBox(), wxID_ANY, wxT("Load .hex"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( Button_LOADHEX, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer124->Add( bSizer2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer197->Add( sbSizer124, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer125;
	sbSizer125 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Programming options") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer198;
	fgSizer198 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer198->SetFlexibleDirection( wxBOTH );
	fgSizer198->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkReset = new wxCheckBox( sbSizer125->GetStaticBox(), ID_CHECKBOX_RESETMCU, wxT("Reset MCU"), wxDefaultPosition, wxDefaultSize, 0 );
	chkReset->SetValue(true); 
	fgSizer198->Add( chkReset, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString rgrModeChoices[] = { wxT("Send program to SRAM and EEPROM"), wxT("Send program to SRAM"), wxT("Boot MCU from EEPROM") };
	int rgrModeNChoices = sizeof( rgrModeChoices ) / sizeof( wxString );
	rgrMode = new wxRadioBox( sbSizer125->GetStaticBox(), wxID_ANY, wxT("MCU's programming mode:"), wxDefaultPosition, wxDefaultSize, rgrModeNChoices, rgrModeChoices, 1, wxRA_SPECIFY_COLS );
	rgrMode->SetSelection( 0 );
	rgrMode->Enable( false );
	
	fgSizer198->Add( rgrMode, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnStartProgramming = new wxButton( sbSizer125->GetStaticBox(), wxID_ANY, wxT("Send command"), wxDefaultPosition, wxDefaultSize, 0 );
	btnStartProgramming->Enable( false );
	
	fgSizer198->Add( btnStartProgramming, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_S_PROGFINISHED = new wxStaticText( sbSizer125->GetStaticBox(), wxID_ANY, wxT("Programming finished"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_S_PROGFINISHED->Wrap( -1 );
	fgSizer198->Add( ID_S_PROGFINISHED, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer125->Add( fgSizer198, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer197->Add( sbSizer125, 1, wxEXPAND|wxSHAPED|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer126;
	sbSizer126 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Testing options") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer199;
	fgSizer199 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer199->SetFlexibleDirection( wxBOTH );
	fgSizer199->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT3 = new wxStaticText( sbSizer126->GetStaticBox(), wxID_ANY, wxT("Debug test (1-15):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer199->Add( ID_STATICTEXT3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnRunTest = new wxButton( sbSizer126->GetStaticBox(), wxID_ANY, wxT("Run one test"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer199->Add( btnRunTest, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizer126->GetStaticBox(), wxID_ANY, wxT("Select test no. (1-15):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer199->Add( ID_STATICTEXT4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_sTestNo = new wxTextCtrl( sbSizer126->GetStaticBox(), ID_TESTNO, wxT("1"), wxDefaultPosition, wxSize( 73,-1 ), 0 );
	#ifdef __WXGTK__
	if ( !m_sTestNo->HasFlag( wxTE_MULTILINE ) )
	{
	m_sTestNo->SetMaxLength( 10 );
	}
	#else
	m_sTestNo->SetMaxLength( 10 );
	#endif
	fgSizer199->Add( m_sTestNo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnRunProductionTest = new wxButton( sbSizer126->GetStaticBox(), wxID_ANY, wxT("Run production test"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer199->Add( btnRunProductionTest, 0, wxALL, 5 );
	
	
	sbSizer126->Add( fgSizer199, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer197->Add( sbSizer126, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer196->Add( fgSizer197, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer127;
	sbSizer127 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("MCU's Debug mode options") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer200;
	fgSizer200 = new wxFlexGridSizer( 7, 1, 0, 0 );
	fgSizer200->SetFlexibleDirection( wxBOTH );
	fgSizer200->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	DebugMode = new wxCheckBox( sbSizer127->GetStaticBox(), ID_DEBUGMODE, wxT("Select Debug mode"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer200->Add( DebugMode, 0, wxALIGN_LEFT, 5 );
	
	wxStaticBoxSizer* sbSizer128;
	sbSizer128 = new wxStaticBoxSizer( new wxStaticBox( sbSizer127->GetStaticBox(), wxID_ANY, wxT("Execution control") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer201;
	fgSizer201 = new wxFlexGridSizer( 2, 3, 0, 0 );
	fgSizer201->SetFlexibleDirection( wxBOTH );
	fgSizer201->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	RunInstr = new wxButton( sbSizer128->GetStaticBox(), ID_RUNINSTR, wxT("Run instr."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer201->Add( RunInstr, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizer128->GetStaticBox(), wxID_ANY, wxT("No. (1-100)"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgSizer201->Add( ID_STATICTEXT6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	InstrNo = new wxTextCtrl( sbSizer128->GetStaticBox(), ID_INSTRNO, wxT("1"), wxDefaultPosition, wxSize( 68,-1 ), 0 );
	fgSizer201->Add( InstrNo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ResetPC = new wxButton( sbSizer128->GetStaticBox(), ID_RESETPC, wxT("Reset PC"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer201->Add( ResetPC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizer128->GetStaticBox(), wxID_ANY, wxT("PC value:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer201->Add( ID_STATICTEXT7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	PCValue = new wxStaticText( sbSizer128->GetStaticBox(), wxID_ANY, wxT("PCVAL"), wxDefaultPosition, wxDefaultSize, 0 );
	PCValue->Wrap( -1 );
	fgSizer201->Add( PCValue, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer128->Add( fgSizer201, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer200->Add( sbSizer128, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer129;
	sbSizer129 = new wxStaticBoxSizer( new wxStaticBox( sbSizer127->GetStaticBox(), wxID_ANY, wxT("SFR and IRAM options ") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer202;
	fgSizer202 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer202->SetFlexibleDirection( wxBOTH );
	fgSizer202->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ViewSFRs = new wxButton( sbSizer129->GetStaticBox(), wxID_ANY, wxT("View SFRs"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer202->Add( ViewSFRs, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ViewIRAM = new wxButton( sbSizer129->GetStaticBox(), wxID_ANY, wxT("View IRAM"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer202->Add( ViewIRAM, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	EraseIRAM = new wxButton( sbSizer129->GetStaticBox(), wxID_ANY, wxT("Erase IRAM"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer202->Add( EraseIRAM, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer129->Add( fgSizer202, 1, wxALL|wxEXPAND|wxSHAPED|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer200->Add( sbSizer129, 1, wxEXPAND|wxALIGN_LEFT, 5 );
	
	wxStaticBoxSizer* sbSizer130;
	sbSizer130 = new wxStaticBoxSizer( new wxStaticBox( sbSizer127->GetStaticBox(), wxID_ANY, wxT("MCU's clock divider") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer203;
	fgSizer203 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer203->SetFlexibleDirection( wxBOTH );
	fgSizer203->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT10 = new wxStaticText( sbSizer130->GetStaticBox(), wxID_ANY, wxT("Select MCU's clock divider value:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT10->Wrap( -1 );
	fgSizer203->Add( ID_STATICTEXT10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString SelDivChoices[] = { wxT("1"), wxT("2"), wxT("4"), wxT("8"), wxT("16"), wxT("32") };
	int SelDivNChoices = sizeof( SelDivChoices ) / sizeof( wxString );
	SelDiv = new wxChoice( sbSizer130->GetStaticBox(), ID_SELDIV, wxDefaultPosition, wxDefaultSize, SelDivNChoices, SelDivChoices, 0 );
	SelDiv->SetSelection( 0 );
	fgSizer203->Add( SelDiv, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer130->Add( fgSizer203, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer200->Add( sbSizer130, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	sbSizer127->Add( fgSizer200, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	
	fgSizer196->Add( sbSizer127, 1, wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxFlexGridSizer* fgSizer204;
	fgSizer204 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer204->SetFlexibleDirection( wxBOTH );
	fgSizer204->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer131;
	sbSizer131 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("SPI switch tranceiver control") ), wxVERTICAL );
	
	m_cCtrlBaseband = new wxRadioButton( sbSizer131->GetStaticBox(), ID_RADIOBUTTON4, wxT("Transceiver controlled by Baseband"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	sbSizer131->Add( m_cCtrlBaseband, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_cCtrlMCU_BD = new wxRadioButton( sbSizer131->GetStaticBox(), ID_RADIOBUTTON5, wxT("Transceiver controlled by MCU_BD"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer131->Add( m_cCtrlMCU_BD, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer204->Add( sbSizer131, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer132;
	sbSizer132 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Read/write registers REG0-REG6") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer205;
	fgSizer205 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer205->SetFlexibleDirection( wxBOTH );
	fgSizer205->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer132->GetStaticBox(), wxID_ANY, wxT("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( 0 );
	fgSizer205->Add( ID_STATICTEXT5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString cmbRegAddrChoices[] = { wxT("REG0"), wxT("REG1"), wxT("REG2"), wxT("REG3"), wxT("REG4"), wxT("REG5"), wxT("REG6") };
	int cmbRegAddrNChoices = sizeof( cmbRegAddrChoices ) / sizeof( wxString );
	cmbRegAddr = new wxChoice( sbSizer132->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cmbRegAddrNChoices, cmbRegAddrChoices, 0 );
	cmbRegAddr->SetSelection( 0 );
	fgSizer205->Add( cmbRegAddr, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer132->Add( fgSizer205, 0, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxFlexGridSizer* fgSizer206;
	fgSizer206 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer206->SetFlexibleDirection( wxBOTH );
	fgSizer206->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	rbtnRegWrite = new wxRadioButton( sbSizer132->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	fgSizer206->Add( rbtnRegWrite, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizer132->GetStaticBox(), wxID_ANY, wxT("Data (0-255):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( -1 );
	fgSizer206->Add( ID_STATICTEXT11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtRegValueWr = new wxTextCtrl( sbSizer132->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 65,-1 ), 0 );
	fgSizer206->Add( txtRegValueWr, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	rbtnRegRead = new wxRadioButton( sbSizer132->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer206->Add( rbtnRegRead, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	ReadResult = new wxStaticText( sbSizer132->GetStaticBox(), wxID_ANY, wxT("Result is:"), wxDefaultPosition, wxDefaultSize, 0 );
	ReadResult->Wrap( -1 );
	fgSizer206->Add( ReadResult, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer132->Add( fgSizer206, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnRdWr = new wxButton( sbSizer132->GetStaticBox(), wxID_ANY, wxT("Read/Write"), wxDefaultPosition, wxSize( 105,36 ), 0 );
	sbSizer132->Add( btnRdWr, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer204->Add( sbSizer132, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer196->Add( fgSizer204, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	fgSizer195->Add( fgSizer196, 1, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer209;
	fgSizer209 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer209->AddGrowableCol( 1 );
	fgSizer209->SetFlexibleDirection( wxBOTH );
	fgSizer209->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("Progress:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	fgSizer209->Add( ID_STATICTEXT2, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	progressBar = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	progressBar->SetValue( 0 ); 
	fgSizer209->Add( progressBar, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer195->Add( fgSizer209, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 1, 0, 0 );
	
	lblProgCodeFile = new wxStaticText( this, wxID_ANY, wxT("Program code file:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblProgCodeFile->Wrap( -1 );
	gSizer1->Add( lblProgCodeFile, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblTestResultsFile = new wxStaticText( this, wxID_ANY, wxT("Test results file:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTestResultsFile->Wrap( -1 );
	gSizer1->Add( lblTestResultsFile, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer195->Add( gSizer1, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	this->SetSizer( fgSizer195 );
	this->Layout();
	fgSizer195->Fit( this );
	
	// Connect Events
	Button_LOADHEX->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnButton_LOADHexClick ), NULL, this );
	chkReset->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnchkResetClick ), NULL, this );
	btnStartProgramming->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnbtnStartProgrammingClick ), NULL, this );
	btnRunTest->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnbtnRunTestClick ), NULL, this );
	btnRunProductionTest->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnbtnRunProductionTestClicked ), NULL, this );
	DebugMode->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnDebugModeClick ), NULL, this );
	RunInstr->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnRunInstruction ), NULL, this );
	ResetPC->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnResetPCClick ), NULL, this );
	ViewSFRs->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnViewSFRsClick ), NULL, this );
	ViewIRAM->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnViewIRAMClick ), NULL, this );
	EraseIRAM->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnEraseIRAMClick ), NULL, this );
	SelDiv->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( pnlMCU_BD_view::OnSelDivSelect ), NULL, this );
	m_cCtrlBaseband->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlMCU_BD_view::Onm_cCtrlBasebandSelect ), NULL, this );
	m_cCtrlMCU_BD->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlMCU_BD_view::Onm_cCtrlMCU_BDSelect ), NULL, this );
	btnRdWr->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnRegWriteRead ), NULL, this );
}

pnlMCU_BD_view::~pnlMCU_BD_view()
{
	// Disconnect Events
	Button_LOADHEX->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnButton_LOADHexClick ), NULL, this );
	chkReset->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnchkResetClick ), NULL, this );
	btnStartProgramming->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnbtnStartProgrammingClick ), NULL, this );
	btnRunTest->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnbtnRunTestClick ), NULL, this );
	btnRunProductionTest->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnbtnRunProductionTestClicked ), NULL, this );
	DebugMode->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnDebugModeClick ), NULL, this );
	RunInstr->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnRunInstruction ), NULL, this );
	ResetPC->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnResetPCClick ), NULL, this );
	ViewSFRs->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnViewSFRsClick ), NULL, this );
	ViewIRAM->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnViewIRAMClick ), NULL, this );
	EraseIRAM->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnEraseIRAMClick ), NULL, this );
	SelDiv->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( pnlMCU_BD_view::OnSelDivSelect ), NULL, this );
	m_cCtrlBaseband->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlMCU_BD_view::Onm_cCtrlBasebandSelect ), NULL, this );
	m_cCtrlMCU_BD->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( pnlMCU_BD_view::Onm_cCtrlMCU_BDSelect ), NULL, this );
	btnRdWr->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlMCU_BD_view::OnRegWriteRead ), NULL, this );
	
}

pnlCalibrations_view::pnlCalibrations_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer309;
	fgSizer309 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer309->AddGrowableCol( 0 );
	fgSizer309->AddGrowableCol( 1 );
	fgSizer309->SetFlexibleDirection( wxBOTH );
	fgSizer309->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer159;
	sbSizer159 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Receiver") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer971;
	sbSizer971 = new wxStaticBoxSizer( new wxStaticBox( sbSizer159->GetStaticBox(), wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1311;
	fgSizer1311 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer1311->AddGrowableCol( 1 );
	fgSizer1311->SetFlexibleDirection( wxBOTH );
	fgSizer1311->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT51 = new wxStaticText( sbSizer971->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT51->Wrap( -1 );
	fgSizer1311->Add( ID_STATICTEXT51, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRI_RXTSP = new NumericSlider( sbSizer971->GetStaticBox(), ID_GCORRI_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRI_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer1311->Add( cmbGCORRI_RXTSP, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT41 = new wxStaticText( sbSizer971->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT41->Wrap( -1 );
	fgSizer1311->Add( ID_STATICTEXT41, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRQ_RXTSP = new NumericSlider( sbSizer971->GetStaticBox(), ID_GCORRQ_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRQ_RXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer1311->Add( cmbGCORRQ_RXTSP, 0, wxEXPAND, 5 );
	
	ID_BUTTON101 = new wxButton( sbSizer971->GetStaticBox(), wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON101->SetDefault(); 
	ID_BUTTON101->Hide();
	
	fgSizer1311->Add( ID_BUTTON101, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer971->Add( fgSizer1311, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer159->Add( sbSizer971, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer961;
	sbSizer961 = new wxStaticBoxSizer( new wxStaticBox( sbSizer159->GetStaticBox(), wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
	
	cmbIQCORR_RXTSP = new NumericSlider( sbSizer961->GetStaticBox(), ID_IQCORR_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
	cmbIQCORR_RXTSP->SetMinSize( wxSize( 200,-1 ) );
	
	sbSizer961->Add( cmbIQCORR_RXTSP, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer1301;
	fgSizer1301 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer1301->SetFlexibleDirection( wxBOTH );
	fgSizer1301->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT161 = new wxStaticText( sbSizer961->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT161->Wrap( -1 );
	fgSizer1301->Add( ID_STATICTEXT161, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtPhaseAlpha1 = new wxStaticText( sbSizer961->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txtPhaseAlpha1->Wrap( -1 );
	fgSizer1301->Add( txtPhaseAlpha1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer961->Add( fgSizer1301, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizer159->Add( sbSizer961, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerDC;
	sbSizerDC = new wxStaticBoxSizer( new wxStaticBox( sbSizer159->GetStaticBox(), wxID_ANY, wxT("DC") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer45;
	fgSizer45 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer45->AddGrowableCol( 1 );
	fgSizer45->SetFlexibleDirection( wxBOTH );
	fgSizer45->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT6 = new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	fgSizer45->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCOFFI_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFI_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
	fgSizer45->Add( cmbDCOFFI_RFE, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT7 = new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	fgSizer45->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCOFFQ_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFQ_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
	fgSizer45->Add( cmbDCOFFQ_RFE, 0, wxEXPAND, 5 );
	
	
	sbSizerDC->Add( fgSizer45, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkEN_DCOFF_RXFE_RFE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_EN_DCOFF_RXFE_RFE, wxT("Enable DC offset"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEN_DCOFF_RXFE_RFE->SetToolTip( wxT("Enables the DCOFFSET block for the RXFE") );
	
	sbSizerDC->Add( chkEN_DCOFF_RXFE_RFE, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	chkDCMODE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_DCMODE, wxT("Automatic DC calibration mode"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerDC->Add( chkDCMODE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	
	sbSizer159->Add( sbSizerDC, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer247;
	fgSizer247 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer247->SetFlexibleDirection( wxBOTH );
	fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnCalibrateRx = new wxButton( sbSizer159->GetStaticBox(), wxID_ANY, wxT("Calibrate RX"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer247->Add( btnCalibrateRx, 0, wxALL, 5 );
	
	
	sbSizer159->Add( fgSizer247, 1, wxEXPAND, 5 );
	
	
	fgSizer309->Add( sbSizer159, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer148;
	sbSizer148 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Transmitter") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer97;
	sbSizer97 = new wxStaticBoxSizer( new wxStaticBox( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer131;
	fgSizer131 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer131->AddGrowableCol( 1 );
	fgSizer131->SetFlexibleDirection( wxBOTH );
	fgSizer131->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( -1 );
	fgSizer131->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRI_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer131->Add( cmbGCORRI_TXTSP, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT4 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT4->Wrap( -1 );
	fgSizer131->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbGCORRQ_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
	cmbGCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer131->Add( cmbGCORRQ_TXTSP, 0, wxEXPAND, 5 );
	
	ID_BUTTON10 = new wxButton( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON10->SetDefault(); 
	ID_BUTTON10->Hide();
	
	fgSizer131->Add( ID_BUTTON10, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer97->Add( fgSizer131, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer148->Add( sbSizer97, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer96;
	sbSizer96 = new wxStaticBoxSizer( new wxStaticBox( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
	
	cmbIQCORR_TXTSP = new NumericSlider( sbSizer96->GetStaticBox(), ID_IQCORR_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
	cmbIQCORR_TXTSP->SetMinSize( wxSize( 200,-1 ) );
	
	sbSizer96->Add( cmbIQCORR_TXTSP, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer130;
	fgSizer130 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer130->SetFlexibleDirection( wxBOTH );
	fgSizer130->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	fgSizer130->Add( ID_STATICTEXT16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtPhaseAlpha = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txtPhaseAlpha->Wrap( -1 );
	fgSizer130->Add( txtPhaseAlpha, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer96->Add( fgSizer130, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	
	sbSizer148->Add( sbSizer96, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer95;
	sbSizer95 = new wxStaticBoxSizer( new wxStaticBox( sbSizer148->GetStaticBox(), wxID_ANY, wxT("DC Corrector") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer129;
	fgSizer129 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer129->AddGrowableCol( 1 );
	fgSizer129->SetFlexibleDirection( wxBOTH );
	fgSizer129->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT8 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT8->Wrap( -1 );
	fgSizer129->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCCORRI_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
	cmbDCCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer129->Add( cmbDCCORRI_TXTSP, 0, wxEXPAND, 5 );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( -1 );
	fgSizer129->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbDCCORRQ_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
	cmbDCCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer129->Add( cmbDCCORRQ_TXTSP, 0, wxEXPAND, 5 );
	
	
	sbSizer95->Add( fgSizer129, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	
	sbSizer148->Add( sbSizer95, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer248;
	fgSizer248 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer248->SetFlexibleDirection( wxBOTH );
	fgSizer248->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnCalibrateTx = new wxButton( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Calibrate TX"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer248->Add( btnCalibrateTx, 0, wxALL, 5 );
	
	
	sbSizer148->Add( fgSizer248, 1, wxEXPAND, 5 );
	
	
	fgSizer309->Add( sbSizer148, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer165;
	sbSizer165 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Full calibration") ), wxVERTICAL );
	
	btnCalibrateAll = new wxButton( sbSizer165->GetStaticBox(), wxID_ANY, wxT("Calibrate All"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer165->Add( btnCalibrateAll, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer328;
	fgSizer328 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer328->SetFlexibleDirection( wxBOTH );
	fgSizer328->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText431 = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxT("CGEN Ref. Clk (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText431->Wrap( -1 );
	fgSizer328->Add( m_staticText431, 0, wxALL, 5 );
	
	lblCGENrefClk = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCGENrefClk->Wrap( -1 );
	fgSizer328->Add( lblCGENrefClk, 0, wxALL, 5 );
	
	m_staticText372 = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxT("Calibration bandwidth (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText372->Wrap( -1 );
	fgSizer328->Add( m_staticText372, 0, wxALL, 5 );
	
	txtCalibrationBW = new wxTextCtrl( sbSizer165->GetStaticBox(), wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtCalibrationBW->HasFlag( wxTE_MULTILINE ) )
	{
	txtCalibrationBW->SetMaxLength( 10 );
	}
	#else
	txtCalibrationBW->SetMaxLength( 10 );
	#endif
	txtCalibrationBW->SetMinSize( wxSize( 50,-1 ) );
	
	fgSizer328->Add( txtCalibrationBW, 0, 0, 5 );
	
	
	sbSizer165->Add( fgSizer328, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer246;
	fgSizer246 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer246->SetFlexibleDirection( wxVERTICAL );
	fgSizer246->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString rgrCalibrationMethodChoices[] = { wxT("Chip internal"), wxT("On board external") };
	int rgrCalibrationMethodNChoices = sizeof( rgrCalibrationMethodChoices ) / sizeof( wxString );
	rgrCalibrationMethod = new wxRadioBox( sbSizer165->GetStaticBox(), wxID_ANY, wxT("Calibration loopback:"), wxDefaultPosition, wxDefaultSize, rgrCalibrationMethodNChoices, rgrCalibrationMethodChoices, 1, wxRA_SPECIFY_COLS );
	rgrCalibrationMethod->SetSelection( 0 );
	fgSizer246->Add( rgrCalibrationMethod, 0, wxALL, 5 );
	
	lblCalibrationNote = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	lblCalibrationNote->Wrap( -1 );
	fgSizer246->Add( lblCalibrationNote, 0, wxALL, 5 );
	
	
	sbSizer165->Add( fgSizer246, 0, 0, 5 );
	
	
	fgSizer309->Add( sbSizer165, 0, 0, 5 );
	
	
	this->SetSizer( fgSizer309 );
	this->Layout();
	fgSizer309->Fit( this );
	
	// Connect Events
	cmbGCORRI_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbIQCORR_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFI_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFQ_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	chkEN_DCOFF_RXFE_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	chkDCMODE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	btnCalibrateRx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCalibrations_view::OnbtnCalibrateRx ), NULL, this );
	cmbGCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbIQCORR_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	btnCalibrateTx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCalibrations_view::OnbtnCalibrateTx ), NULL, this );
	btnCalibrateAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCalibrations_view::OnbtnCalibrateAll ), NULL, this );
	rgrCalibrationMethod->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCalibrations_view::OnCalibrationMethodChange ), NULL, this );
}

pnlCalibrations_view::~pnlCalibrations_view()
{
	// Disconnect Events
	cmbGCORRI_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbIQCORR_RXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFI_RFE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCOFFQ_RFE->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	chkEN_DCOFF_RXFE_RFE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	chkDCMODE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	btnCalibrateRx->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCalibrations_view::OnbtnCalibrateRx ), NULL, this );
	cmbGCORRI_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbGCORRQ_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbIQCORR_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRI_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	cmbDCCORRQ_TXTSP->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
	btnCalibrateTx->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCalibrations_view::OnbtnCalibrateTx ), NULL, this );
	btnCalibrateAll->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlCalibrations_view::OnbtnCalibrateAll ), NULL, this );
	rgrCalibrationMethod->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( pnlCalibrations_view::OnCalibrationMethodChange ), NULL, this );
	
}

pnlGains_view::pnlGains_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer309;
	fgSizer309 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer309->AddGrowableCol( 2 );
	fgSizer309->SetFlexibleDirection( wxBOTH );
	fgSizer309->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkTRX_GAIN_SRC = new wxCheckBox( this, ID_EN_NEXTRX_RFE, wxT("Alternative TRX gain source"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTRX_GAIN_SRC->SetToolTip( wxT("Enables the daisy chain LO buffer going from RXFE1  to RXFE2") );
	
	fgSizer309->Add( chkTRX_GAIN_SRC, 0, wxALL, 5 );
	
	
	fgSizer309->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer309->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer159;
	sbSizer159 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Receiver") ), wxHORIZONTAL );
	
	wxFlexGridSizer* rxSizer;
	rxSizer = new wxFlexGridSizer( 0, 2, 0, 5 );
	rxSizer->AddGrowableCol( 0 );
	rxSizer->SetFlexibleDirection( wxBOTH );
	rxSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxStaticText* ID_STATICTEXT16;
	ID_STATICTEXT16 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("LNA"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( -1 );
	rxSizer->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbG_LNA_RFE = new wxComboBox( sbSizer159->GetStaticBox(), ID_G_LNA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbG_LNA_RFE->SetToolTip( wxT("Controls the gain of the LNA") );
	
	rxSizer->Add( cmbG_LNA_RFE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	wxStaticText* ID_STATICTEXT18;
	ID_STATICTEXT18 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("TIA"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT18->Wrap( -1 );
	rxSizer->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbG_TIA_RFE = new wxComboBox( sbSizer159->GetStaticBox(), ID_G_TIA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbG_TIA_RFE->SetToolTip( wxT("Controls the Gain of the TIA") );
	
	rxSizer->Add( cmbG_TIA_RFE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	wxStaticText* ID_STATICTEXT2;
	ID_STATICTEXT2 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("PGA gain"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT2->Wrap( -1 );
	rxSizer->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbG_PGA_RBB = new wxComboBox( sbSizer159->GetStaticBox(), ID_G_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbG_PGA_RBB->SetToolTip( wxT("This is the gain of the PGA") );
	
	rxSizer->Add( cmbG_PGA_RBB, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	wxStaticText* ID_STATICTEXT3;
	ID_STATICTEXT3 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("PGA Feedback capacitor"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	rxSizer->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbC_CTL_PGA_RBB = new NumericSlider( sbSizer159->GetStaticBox(), ID_C_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 3, 0 );
	rxSizer->Add( cmbC_CTL_PGA_RBB, 0, wxALIGN_LEFT, 5 );
	
	
	sbSizer159->Add( rxSizer, 1, 0, 5 );
	
	
	fgSizer309->Add( sbSizer159, 1, 0, 5 );
	
	wxStaticBoxSizer* sbSizer148;
	sbSizer148 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Transmitter") ), wxHORIZONTAL );
	
	wxFlexGridSizer* txSizer;
	txSizer = new wxFlexGridSizer( 0, 2, 0, 5 );
	txSizer->SetFlexibleDirection( wxBOTH );
	txSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxStaticText* ID_STATICTEXT6;
	ID_STATICTEXT6 = new wxStaticText( sbSizer148->GetStaticBox(), wxID_ANY, wxT("TXPAD linearizing part gain"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT6->Wrap( -1 );
	txSizer->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOSS_LIN_TXPAD_TRF = new wxComboBox( sbSizer148->GetStaticBox(), ID_LOSS_LIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbLOSS_LIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain of the linearizing part of of the TXPAD") );
	
	txSizer->Add( cmbLOSS_LIN_TXPAD_TRF, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	wxStaticText* ID_STATICTEXT7;
	ID_STATICTEXT7 = new wxStaticText( sbSizer148->GetStaticBox(), wxID_ANY, wxT("TXPAD gain control"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT7->Wrap( -1 );
	txSizer->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbLOSS_MAIN_TXPAD_TRF = new wxComboBox( sbSizer148->GetStaticBox(), ID_LOSS_MAIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	cmbLOSS_MAIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain  output power of the TXPAD") );
	
	txSizer->Add( cmbLOSS_MAIN_TXPAD_TRF, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
	
	wxStaticText* ID_STATICTEXT21;
	ID_STATICTEXT21 = new wxStaticText( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Frontend gain"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT21->Wrap( -1 );
	txSizer->Add( ID_STATICTEXT21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
	
	cmbCG_IAMP_TBB = new NumericSlider( sbSizer148->GetStaticBox(), ID_CG_IAMP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
	txSizer->Add( cmbCG_IAMP_TBB, 0, wxALIGN_LEFT|wxEXPAND, 5 );
	
	
	sbSizer148->Add( txSizer, 1, 0, 5 );
	
	
	fgSizer309->Add( sbSizer148, 1, 0, 5 );
	
	
	fgSizer309->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer135;
	sbSizer135 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("MCU AGC") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer256;
	fgSizer256 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer256->SetFlexibleDirection( wxBOTH );
	fgSizer256->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkAGC = new wxCheckBox( sbSizer135->GetStaticBox(), wxID_ANY, wxT("Enable AGC"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer256->Add( chkAGC, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText360 = new wxStaticText( sbSizer135->GetStaticBox(), wxID_ANY, wxT("IN crest factor"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText360->Wrap( -1 );
	fgSizer256->Add( m_staticText360, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	txtCrestFactor = new wxTextCtrl( sbSizer135->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer256->Add( txtCrestFactor, 0, wxALL, 5 );
	
	
	sbSizer135->Add( fgSizer256, 1, wxEXPAND, 5 );
	
	
	fgSizer309->Add( sbSizer135, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer309 );
	this->Layout();
	fgSizer309->Fit( this );
	
	// Connect Events
	chkTRX_GAIN_SRC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbG_LNA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbG_TIA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbG_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbCG_IAMP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	chkAGC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlGains_view::OnAGCStateChange ), NULL, this );
}

pnlGains_view::~pnlGains_view()
{
	// Disconnect Events
	chkTRX_GAIN_SRC->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbG_LNA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbG_TIA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbG_PGA_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbC_CTL_PGA_RBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_LIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbLOSS_MAIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	cmbCG_IAMP_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( pnlGains_view::ParameterChangeHandler ), NULL, this );
	chkAGC->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlGains_view::OnAGCStateChange ), NULL, this );
	
}

dlgGFIR_Coefficients::dlgGFIR_Coefficients( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer235;
	fgSizer235 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer235->SetFlexibleDirection( wxBOTH );
	fgSizer235->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer237;
	fgSizer237 = new wxFlexGridSizer( 0, 5, 0, 0 );
	fgSizer237->SetFlexibleDirection( wxBOTH );
	fgSizer237->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnLoadFromFile = new wxButton( this, wxID_ANY, wxT("Load from file"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLoadFromFile->SetDefault(); 
	fgSizer237->Add( btnLoadFromFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnSaveToFile = new wxButton( this, wxID_ANY, wxT("Save to file"), wxDefaultPosition, wxDefaultSize, 0 );
	btnSaveToFile->SetDefault(); 
	fgSizer237->Add( btnSaveToFile, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnClearTable = new wxButton( this, wxID_ANY, wxT("Clear table"), wxDefaultPosition, wxDefaultSize, 0 );
	btnClearTable->SetDefault(); 
	fgSizer237->Add( btnClearTable, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("Coefficients count:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT3->Wrap( -1 );
	fgSizer237->Add( ID_STATICTEXT3, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	spinCoefCount = new wxSpinCtrl( this, wxID_ANY, wxT("40"), wxDefaultPosition, wxSize( 64,-1 ), wxSP_ARROW_KEYS, 0, 120, 40 );
	fgSizer237->Add( spinCoefCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer235->Add( fgSizer237, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer238;
	fgSizer238 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer238->AddGrowableCol( 0 );
	fgSizer238->AddGrowableRow( 0 );
	fgSizer238->SetFlexibleDirection( wxBOTH );
	fgSizer238->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	gridCoef = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	
	// Grid
	gridCoef->CreateGrid( 5, 1 );
	gridCoef->EnableEditing( true );
	gridCoef->EnableGridLines( true );
	gridCoef->EnableDragGridSize( false );
	gridCoef->SetMargins( 0, 0 );
	
	// Columns
	gridCoef->EnableDragColMove( false );
	gridCoef->EnableDragColSize( true );
	gridCoef->SetColLabelSize( 30 );
	gridCoef->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	gridCoef->EnableDragRowSize( true );
	gridCoef->SetRowLabelSize( 80 );
	gridCoef->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	gridCoef->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	gridCoef->SetMinSize( wxSize( 100,200 ) );
	
	fgSizer238->Add( gridCoef, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer235->Add( fgSizer238, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
	
	wxFlexGridSizer* fgSizer239;
	fgSizer239 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer239->SetFlexibleDirection( wxBOTH );
	fgSizer239->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnOk = new wxButton( this, wxID_ANY, wxT("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	btnOk->SetDefault(); 
	fgSizer239->Add( btnOk, 1, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	btnCancel->SetDefault(); 
	fgSizer239->Add( btnCancel, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer235->Add( fgSizer239, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( fgSizer235 );
	this->Layout();
	fgSizer235->Fit( this );
	
	// Connect Events
	btnLoadFromFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnLoadFromFile ), NULL, this );
	btnSaveToFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnSaveToFile ), NULL, this );
	btnClearTable->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnClearTable ), NULL, this );
	spinCoefCount->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( dlgGFIR_Coefficients::OnspinCoefCountChange ), NULL, this );
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnBtnOkClick ), NULL, this );
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnBtnCancelClick ), NULL, this );
}

dlgGFIR_Coefficients::~dlgGFIR_Coefficients()
{
	// Disconnect Events
	btnLoadFromFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnLoadFromFile ), NULL, this );
	btnSaveToFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnSaveToFile ), NULL, this );
	btnClearTable->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnClearTable ), NULL, this );
	spinCoefCount->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( dlgGFIR_Coefficients::OnspinCoefCountChange ), NULL, this );
	btnOk->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnBtnOkClick ), NULL, this );
	btnCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgGFIR_Coefficients::OnBtnCancelClick ), NULL, this );
	
}

dlgVCOfrequencies::dlgVCOfrequencies( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer247;
	fgSizer247 = new wxFlexGridSizer( 0, 1, 5, 5 );
	fgSizer247->SetFlexibleDirection( wxBOTH );
	fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer240;
	fgSizer240 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer240->SetFlexibleDirection( wxBOTH );
	fgSizer240->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer129;
	sbSizer129 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCOH SXR/SXT") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer241;
	fgSizer241 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer241->SetFlexibleDirection( wxBOTH );
	fgSizer241->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText341 = new wxStaticText( sbSizer129->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText341->Wrap( -1 );
	fgSizer241->Add( m_staticText341, 0, wxALL, 5 );
	
	txtVCOH_low = new wxTextCtrl( sbSizer129->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOH_low->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOH_low->SetMaxLength( 10 );
	}
	#else
	txtVCOH_low->SetMaxLength( 10 );
	#endif
	txtVCOH_low->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer241->Add( txtVCOH_low, 0, 0, 5 );
	
	m_staticText342 = new wxStaticText( sbSizer129->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText342->Wrap( -1 );
	fgSizer241->Add( m_staticText342, 0, wxALL, 5 );
	
	txtVCOH_high = new wxTextCtrl( sbSizer129->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOH_high->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOH_high->SetMaxLength( 10 );
	}
	#else
	txtVCOH_high->SetMaxLength( 10 );
	#endif
	txtVCOH_high->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer241->Add( txtVCOH_high, 0, 0, 5 );
	
	
	sbSizer129->Add( fgSizer241, 1, wxEXPAND, 5 );
	
	
	fgSizer240->Add( sbSizer129, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer1291;
	sbSizer1291 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCOM SXR/SXT") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2411;
	fgSizer2411 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer2411->SetFlexibleDirection( wxBOTH );
	fgSizer2411->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText3411 = new wxStaticText( sbSizer1291->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3411->Wrap( -1 );
	fgSizer2411->Add( m_staticText3411, 0, wxALL, 5 );
	
	txtVCOM_low = new wxTextCtrl( sbSizer1291->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOM_low->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOM_low->SetMaxLength( 10 );
	}
	#else
	txtVCOM_low->SetMaxLength( 10 );
	#endif
	txtVCOM_low->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer2411->Add( txtVCOM_low, 0, 0, 5 );
	
	m_staticText3421 = new wxStaticText( sbSizer1291->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3421->Wrap( -1 );
	fgSizer2411->Add( m_staticText3421, 0, wxALL, 5 );
	
	txtVCOM_high = new wxTextCtrl( sbSizer1291->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOM_high->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOM_high->SetMaxLength( 10 );
	}
	#else
	txtVCOM_high->SetMaxLength( 10 );
	#endif
	txtVCOM_high->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer2411->Add( txtVCOM_high, 0, 0, 5 );
	
	
	sbSizer1291->Add( fgSizer2411, 1, wxEXPAND, 5 );
	
	
	fgSizer240->Add( sbSizer1291, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer1292;
	sbSizer1292 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCOL SXR/SXT") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2412;
	fgSizer2412 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer2412->SetFlexibleDirection( wxBOTH );
	fgSizer2412->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText3412 = new wxStaticText( sbSizer1292->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3412->Wrap( -1 );
	fgSizer2412->Add( m_staticText3412, 0, wxALL, 5 );
	
	txtVCOL_low = new wxTextCtrl( sbSizer1292->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOL_low->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOL_low->SetMaxLength( 10 );
	}
	#else
	txtVCOL_low->SetMaxLength( 10 );
	#endif
	txtVCOL_low->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer2412->Add( txtVCOL_low, 0, 0, 5 );
	
	m_staticText3422 = new wxStaticText( sbSizer1292->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3422->Wrap( -1 );
	fgSizer2412->Add( m_staticText3422, 0, wxALL, 5 );
	
	txtVCOL_high = new wxTextCtrl( sbSizer1292->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOL_high->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOL_high->SetMaxLength( 10 );
	}
	#else
	txtVCOL_high->SetMaxLength( 10 );
	#endif
	txtVCOL_high->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer2412->Add( txtVCOL_high, 0, 0, 5 );
	
	
	sbSizer1292->Add( fgSizer2412, 1, wxEXPAND, 5 );
	
	
	fgSizer240->Add( sbSizer1292, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer1293;
	sbSizer1293 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCO CGEN") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2413;
	fgSizer2413 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer2413->SetFlexibleDirection( wxBOTH );
	fgSizer2413->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText3413 = new wxStaticText( sbSizer1293->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3413->Wrap( -1 );
	fgSizer2413->Add( m_staticText3413, 0, wxALL, 5 );
	
	txtVCOCGEN_low = new wxTextCtrl( sbSizer1293->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOCGEN_low->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOCGEN_low->SetMaxLength( 10 );
	}
	#else
	txtVCOCGEN_low->SetMaxLength( 10 );
	#endif
	txtVCOCGEN_low->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer2413->Add( txtVCOCGEN_low, 0, 0, 5 );
	
	m_staticText3423 = new wxStaticText( sbSizer1293->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3423->Wrap( -1 );
	fgSizer2413->Add( m_staticText3423, 0, wxALL, 5 );
	
	txtVCOCGEN_high = new wxTextCtrl( sbSizer1293->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtVCOCGEN_high->HasFlag( wxTE_MULTILINE ) )
	{
	txtVCOCGEN_high->SetMaxLength( 10 );
	}
	#else
	txtVCOCGEN_high->SetMaxLength( 10 );
	#endif
	txtVCOCGEN_high->SetMinSize( wxSize( 64,-1 ) );
	
	fgSizer2413->Add( txtVCOCGEN_high, 0, 0, 5 );
	
	
	sbSizer1293->Add( fgSizer2413, 1, wxEXPAND, 5 );
	
	
	fgSizer240->Add( sbSizer1293, 1, wxEXPAND, 5 );
	
	
	fgSizer247->Add( fgSizer240, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer249;
	fgSizer249 = new wxFlexGridSizer( 0, 4, 5, 5 );
	fgSizer249->SetFlexibleDirection( wxBOTH );
	fgSizer249->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnOk = new wxButton( this, wxID_ANY, wxT("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer249->Add( btnOk, 0, 0, 5 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer249->Add( btnCancel, 0, 0, 5 );
	
	btnLoadFile = new wxButton( this, wxID_ANY, wxT("Load file"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer249->Add( btnLoadFile, 0, 0, 5 );
	
	btnSaveFile = new wxButton( this, wxID_ANY, wxT("Save to file"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer249->Add( btnSaveFile, 0, 0, 5 );
	
	
	fgSizer247->Add( fgSizer249, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	this->SetSizer( fgSizer247 );
	this->Layout();
	fgSizer247->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnBtnOkClick ), NULL, this );
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnBtnCancelClick ), NULL, this );
	btnLoadFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnLoadFile ), NULL, this );
	btnSaveFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnSaveFile ), NULL, this );
}

dlgVCOfrequencies::~dlgVCOfrequencies()
{
	// Disconnect Events
	btnOk->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnBtnOkClick ), NULL, this );
	btnCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnBtnCancelClick ), NULL, this );
	btnLoadFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnLoadFile ), NULL, this );
	btnSaveFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgVCOfrequencies::OnSaveFile ), NULL, this );
	
}
