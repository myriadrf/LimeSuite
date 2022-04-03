#include "lms7002_pnlCalibrations_view.h"
#include "lms7002_gui_utilities.h"
#include <wx/msgdlg.h>
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include <wx/busyinfo.h>
#include "lms7suiteAppFrame.h"
using namespace lime;

lms7002_pnlCalibrations_view::lms7002_pnlCalibrations_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
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
    
    ID_STATICTEXT51 = new wxStaticText( sbSizer971->GetStaticBox(), wxID_ANY, wxT("I:"));
    ID_STATICTEXT51->Wrap( -1 );
    fgSizer1311->Add( ID_STATICTEXT51, 0, flags, 0 );
    
    cmbGCORRI_RXTSP = new NumericSlider( sbSizer971->GetStaticBox(), ID_GCORRI_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
    cmbGCORRI_RXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer1311->Add( cmbGCORRI_RXTSP);
    
    ID_STATICTEXT41 = new wxStaticText( sbSizer971->GetStaticBox(), wxID_ANY, wxT("Q:"));
    ID_STATICTEXT41->Wrap( -1 );
    fgSizer1311->Add( ID_STATICTEXT41, 0, flags, 0 );
    
    cmbGCORRQ_RXTSP = new NumericSlider( sbSizer971->GetStaticBox(), ID_GCORRQ_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
    cmbGCORRQ_RXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer1311->Add( cmbGCORRQ_RXTSP);
    
    ID_BUTTON101 = new wxButton( sbSizer971->GetStaticBox(), wxID_ANY, wxT("Calibrate"));
    ID_BUTTON101->SetDefault(); 
    ID_BUTTON101->Hide();
    
    fgSizer1311->Add( ID_BUTTON101, 1, flags, 5 );
    
    
    sbSizer971->Add( fgSizer1311, 0, flags, 0 );
    
    
    sbSizer159->Add( sbSizer971);
    
    wxStaticBoxSizer* sbSizer961;
    sbSizer961 = new wxStaticBoxSizer( new wxStaticBox( sbSizer159->GetStaticBox(), wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
    
    cmbIQCORR_RXTSP = new NumericSlider( sbSizer961->GetStaticBox(), ID_IQCORR_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
    cmbIQCORR_RXTSP->SetMinSize( wxSize( 200,-1 ) );
    
    sbSizer961->Add( cmbIQCORR_RXTSP);
    
    wxFlexGridSizer* fgSizer1301;
    fgSizer1301 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer1301->SetFlexibleDirection( wxBOTH );
    fgSizer1301->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT161 = new wxStaticText( sbSizer961->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):"));
    ID_STATICTEXT161->Wrap( -1 );
    fgSizer1301->Add( ID_STATICTEXT161, 1, flags, 5 );
    
    txtPhaseAlpha1 = new wxStaticText( sbSizer961->GetStaticBox(), wxID_ANY, wxT("0"));
    txtPhaseAlpha1->Wrap( -1 );
    fgSizer1301->Add( txtPhaseAlpha1, 1, flags, 5 );
    
    
    sbSizer961->Add( fgSizer1301, 1, flags, 5 );
    
    
    sbSizer159->Add( sbSizer961);
    
    wxStaticBoxSizer* sbSizerDC;
    sbSizerDC = new wxStaticBoxSizer( new wxStaticBox( sbSizer159->GetStaticBox(), wxID_ANY, wxT("DC") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer45;
    fgSizer45 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer45->AddGrowableCol( 1 );
    fgSizer45->SetFlexibleDirection( wxBOTH );
    fgSizer45->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT6 = new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset I:"));
    ID_STATICTEXT6->Wrap( -1 );
    fgSizer45->Add( ID_STATICTEXT6, 0, flags, 0 );
    
    cmbDCOFFI_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFI_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
    fgSizer45->Add( cmbDCOFFI_RFE);
    
    ID_STATICTEXT7 = new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset Q:"));
    ID_STATICTEXT7->Wrap( -1 );
    fgSizer45->Add( ID_STATICTEXT7, 0, flags, 0 );
    
    cmbDCOFFQ_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFQ_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
    fgSizer45->Add( cmbDCOFFQ_RFE);
    
    
    sbSizerDC->Add( fgSizer45, 0, flags, 0 );
    
    chkEN_DCOFF_RXFE_RFE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_EN_DCOFF_RXFE_RFE, wxT("Enable DC offset"));
    chkEN_DCOFF_RXFE_RFE->SetToolTip( wxT("Enables the DCOFFSET block for the RXFE") );
    
    sbSizerDC->Add( chkEN_DCOFF_RXFE_RFE, 0, flags, 0 );
    
    chkDCMODE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_DCMODE, wxT("Automatic DC calibration mode"));
    sbSizerDC->Add( chkDCMODE, 0, flags, 0 );
    
    
    sbSizer159->Add( sbSizerDC);
    
    wxFlexGridSizer* fgSizer247;
    fgSizer247 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer247->SetFlexibleDirection( wxBOTH );
    fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    btnCalibrateRx = new wxButton( sbSizer159->GetStaticBox(), wxID_ANY, wxT("Calibrate RX"));
    fgSizer247->Add( btnCalibrateRx, 0, wxALL, 5 );
    
    
    sbSizer159->Add( fgSizer247, 1, flags, 5 );
    
    
    fgSizer309->Add( sbSizer159, 1, flags, 5 );
    
    wxStaticBoxSizer* sbSizer148;
    sbSizer148 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Transmitter") ), wxVERTICAL );
    
    wxStaticBoxSizer* sbSizer97;
    sbSizer97 = new wxStaticBoxSizer( new wxStaticBox( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer131;
    fgSizer131 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer131->AddGrowableCol( 1 );
    fgSizer131->SetFlexibleDirection( wxBOTH );
    fgSizer131->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT5 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("I:"));
    ID_STATICTEXT5->Wrap( -1 );
    fgSizer131->Add( ID_STATICTEXT5, 0, flags, 0 );
    
    cmbGCORRI_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
    cmbGCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer131->Add( cmbGCORRI_TXTSP);
    
    ID_STATICTEXT4 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Q:"));
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer131->Add( ID_STATICTEXT4, 0, flags, 0 );
    
    cmbGCORRQ_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
    cmbGCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer131->Add( cmbGCORRQ_TXTSP);
    
    ID_BUTTON10 = new wxButton( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Calibrate"));
    ID_BUTTON10->SetDefault(); 
    ID_BUTTON10->Hide();
    
    fgSizer131->Add( ID_BUTTON10, 1, flags, 5 );
    
    
    sbSizer97->Add( fgSizer131, 0, flags, 0 );
    
    
    sbSizer148->Add( sbSizer97);
    
    wxStaticBoxSizer* sbSizer96;
    sbSizer96 = new wxStaticBoxSizer( new wxStaticBox( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
    
    cmbIQCORR_TXTSP = new NumericSlider( sbSizer96->GetStaticBox(), ID_IQCORR_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
    cmbIQCORR_TXTSP->SetMinSize( wxSize( 200,-1 ) );
    
    sbSizer96->Add( cmbIQCORR_TXTSP);
    
    wxFlexGridSizer* fgSizer130;
    fgSizer130 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer130->SetFlexibleDirection( wxBOTH );
    fgSizer130->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT16 = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):"));
    ID_STATICTEXT16->Wrap( -1 );
    fgSizer130->Add( ID_STATICTEXT16, 1, flags, 5 );
    
    txtPhaseAlpha = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("0"));
    txtPhaseAlpha->Wrap( -1 );
    fgSizer130->Add( txtPhaseAlpha, 1, flags, 5 );
    
    
    sbSizer96->Add( fgSizer130, 1, flags, 5 );
    
    
    sbSizer148->Add( sbSizer96);
    
    wxStaticBoxSizer* sbSizer95;
    sbSizer95 = new wxStaticBoxSizer( new wxStaticBox( sbSizer148->GetStaticBox(), wxID_ANY, wxT("DC Corrector") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer129;
    fgSizer129 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer129->AddGrowableCol( 1 );
    fgSizer129->SetFlexibleDirection( wxBOTH );
    fgSizer129->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT8 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("I:"));
    ID_STATICTEXT8->Wrap( -1 );
    fgSizer129->Add( ID_STATICTEXT8, 0, flags, 0 );
    
    cmbDCCORRI_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
    cmbDCCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer129->Add( cmbDCCORRI_TXTSP);
    
    ID_STATICTEXT9 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Q:"));
    ID_STATICTEXT9->Wrap( -1 );
    fgSizer129->Add( ID_STATICTEXT9, 0, flags, 0 );
    
    cmbDCCORRQ_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
    cmbDCCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer129->Add( cmbDCCORRQ_TXTSP);
    
    
    sbSizer95->Add( fgSizer129, 1, flags, 0 );
    
    
    sbSizer148->Add( sbSizer95);
    
    wxFlexGridSizer* fgSizer248;
    fgSizer248 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer248->SetFlexibleDirection( wxBOTH );
    fgSizer248->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    btnCalibrateTx = new wxButton( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Calibrate TX"));
    fgSizer248->Add( btnCalibrateTx, 0, wxALL, 5 );
    
    
    sbSizer148->Add( fgSizer248, 1, flags, 5 );
    
    
    fgSizer309->Add( sbSizer148, 1, flags, 5 );
    
    wxStaticBoxSizer* sbSizer165;
    sbSizer165 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Full calibration") ), wxVERTICAL );
    
    btnCalibrateAll = new wxButton( sbSizer165->GetStaticBox(), wxID_ANY, wxT("Calibrate All"));
    sbSizer165->Add( btnCalibrateAll, 0, wxALL, 5 );
    
    wxFlexGridSizer* fgSizer328;
    fgSizer328 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer328->SetFlexibleDirection( wxBOTH );
    fgSizer328->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText431 = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxT("CGEN Ref. Clk (MHz):"));
    m_staticText431->Wrap( -1 );
    fgSizer328->Add( m_staticText431, 0, wxALL, 5 );
    
    lblCGENrefClk = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxT("???"));
    lblCGENrefClk->Wrap( -1 );
    fgSizer328->Add( lblCGENrefClk, 0, wxALL, 5 );
    
    m_staticText372 = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxT("Calibration bandwidth (MHz):"));
    m_staticText372->Wrap( -1 );
    fgSizer328->Add( m_staticText372, 0, wxALL, 5 );
    
    txtCalibrationBW = new wxTextCtrl( sbSizer165->GetStaticBox(), wxID_ANY, wxT("5"));
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
    
    
    sbSizer165->Add( fgSizer328, 1, flags, 5 );
    
    wxFlexGridSizer* fgSizer246;
    fgSizer246 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer246->SetFlexibleDirection( wxVERTICAL );
    fgSizer246->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString rgrCalibrationMethodChoices[] = { wxT("Chip internal"), wxT("On board external") };
    int rgrCalibrationMethodNChoices = sizeof( rgrCalibrationMethodChoices ) / sizeof( wxString );
    rgrCalibrationMethod = new wxRadioBox( sbSizer165->GetStaticBox(), wxID_ANY, wxT("Calibration loopback:"), wxDefaultPosition, wxDefaultSize, rgrCalibrationMethodNChoices, rgrCalibrationMethodChoices, 1, wxRA_SPECIFY_COLS );
    rgrCalibrationMethod->SetSelection( 0 );
    fgSizer246->Add( rgrCalibrationMethod, 0, wxALL, 5 );
    
    lblCalibrationNote = new wxStaticText( sbSizer165->GetStaticBox(), wxID_ANY, wxEmptyString);
    lblCalibrationNote->Wrap( -1 );
    fgSizer246->Add( lblCalibrationNote, 0, wxALL, 5 );
    
    
    sbSizer165->Add( fgSizer246, 0, 0, 5 );
    
    
    fgSizer309->Add( sbSizer165, 0, 0, 5 );
    
    
    this->SetSizer( fgSizer309 );
    this->Layout();
    fgSizer309->Fit( this );
    
    // Connect Events
    cmbGCORRI_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbGCORRQ_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbIQCORR_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCOFFI_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCOFFQ_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    chkEN_DCOFF_RXFE_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    chkDCMODE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    btnCalibrateRx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnbtnCalibrateRx ), NULL, this );
    cmbGCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbGCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbIQCORR_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    btnCalibrateTx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnbtnCalibrateTx ), NULL, this );
    btnCalibrateAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnbtnCalibrateAll ), NULL, this );
    rgrCalibrationMethod->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnCalibrationMethodChange ), NULL, this );

    wndId2Enum[cmbIQCORR_TXTSP] = LMS7param(IQCORR_TXTSP);
    wndId2Enum[cmbDCCORRI_TXTSP] = LMS7param(DCCORRI_TXTSP);
    wndId2Enum[cmbDCCORRQ_TXTSP] = LMS7param(DCCORRQ_TXTSP);
    wndId2Enum[cmbGCORRI_TXTSP] = LMS7param(GCORRI_TXTSP);
    wndId2Enum[cmbGCORRQ_TXTSP] = LMS7param(GCORRQ_TXTSP);
    wndId2Enum[cmbGCORRI_RXTSP] = LMS7param(GCORRI_RXTSP);
    wndId2Enum[cmbGCORRQ_RXTSP] = LMS7param(GCORRQ_RXTSP);
    wndId2Enum[cmbIQCORR_RXTSP] = LMS7param(IQCORR_RXTSP);
    wndId2Enum[chkEN_DCOFF_RXFE_RFE] = LMS7param(EN_DCOFF_RXFE_RFE);
    wndId2Enum[cmbDCOFFI_RFE] = LMS7param(DCOFFI_RFE);
    wndId2Enum[cmbDCOFFQ_RFE] = LMS7param(DCOFFQ_RFE);
    wndId2Enum[chkDCMODE] = LMS7param(DCMODE);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
    rgrCalibrationMethod->Hide();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateRx(wxCommandEvent& event)
{
    int flags = 0;
    if(rgrCalibrationMethod->GetSelection() == 0)
        flags = 0;
    else
    {
        flags = 1;
    }
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    int status;
    {
#ifdef NDEBUG
        wxBusyInfo wait("Please wait, calibrating receiver...");
#endif
        uint16_t ch;
        LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
        ch = (ch == 2) ? 1 : 0;
        ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
        status = LMS_Calibrate(lmsControl, LMS_CH_RX, ch, bandwidth_MHz * 1e6, flags);
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Rx calibration failed: %s"), LMS_GetLastErrorMessage()));
    else
    {
        wxMessageBox(_("Rx Calibration Finished"), _("Info"), wxOK, this);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetInt(lime::LOG_LEVEL_INFO);
        evt.SetString(_("Rx Calibrated"));
        wxPostEvent(this, evt);
    }
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateTx( wxCommandEvent& event )
{
    bool useExtLoopback = false;
    if(rgrCalibrationMethod->GetSelection() == 0)
        useExtLoopback = false;
    else
    {
        useExtLoopback = true;
    }
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    int status = 0;
    {
#ifdef NDEBUG
        wxBusyInfo wait("Please wait, calibrating transmitter...");
#endif
        uint16_t ch;
        LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
        status = LMS_Calibrate(lmsControl,LMS_CH_TX,ch-1,bandwidth_MHz * 1e6,useExtLoopback);
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Tx calibration failed: %s"), LMS_GetLastErrorMessage()));
    else
    {
        wxMessageBox(_("Tx Calibration Finished"), _("Info"), wxOK, this);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetInt(lime::LOG_LEVEL_INFO);
        evt.SetString(_("Tx Calibrated"));
        wxPostEvent(this, evt);
    }
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateAll( wxCommandEvent& event )
{
    bool useExtLoopback = false;
    if(rgrCalibrationMethod->GetSelection() == 0)
        useExtLoopback = false;
    else
        useExtLoopback = true;
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    int status = LMS_Calibrate(lmsControl,LMS_CH_TX,ch-1,bandwidth_MHz * 1e6,useExtLoopback);

    if (status != 0)
    {
        wxMessageBox(wxString::Format(_("Tx Calibration Failed: %s"), LMS_GetLastErrorMessage()), _("Info"), wxOK, this);
        UpdateGUI();
        return;
    }

    status |= LMS_Calibrate(lmsControl,LMS_CH_RX,ch-1,bandwidth_MHz * 1e6,useExtLoopback);
    if (status != 0)
        wxMessageBox(wxString::Format(_("Rx Calibration Failed: %s"), LMS_GetLastErrorMessage()), _("Info"), wxOK, this);
    else
        wxMessageBox(_("Calibration Finished"), _("Info"), wxOK, this);
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
        value = 1;
    chkDCMODE->Enable(value);
}

void lms7002_pnlCalibrations_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlCalibrations_view::ParameterChangeHandler(wxCommandEvent& event)
{
    assert(lmsControl != nullptr);
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }
    if(event.GetEventObject() == cmbDCOFFI_RFE || event.GetEventObject() == cmbDCOFFQ_RFE)
    {
        int16_t value = (event.GetInt() < 0) << 6;
        value |= abs(event.GetInt()) & 0x3F;
        LMS_WriteParam(lmsControl,parameter,value);
    }
    else
        LMS_WriteParam(lmsControl,parameter,event.GetInt());
}

void lms7002_pnlCalibrations_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    int16_t value;
    LMS_ReadParam(lmsControl,LMS7param(IQCORR_RXTSP),(uint16_t*)&value);
    int bitsToShift = (15 - LMS7param(IQCORR_RXTSP).msb - LMS7param(IQCORR_RXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_RXTSP->SetValue(value);

    LMS_ReadParam(lmsControl,LMS7param(IQCORR_TXTSP),(uint16_t*)&value);
    bitsToShift = (15 - LMS7param(IQCORR_TXTSP).msb - LMS7param(IQCORR_TXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_TXTSP->SetValue(value);

    LMS_ReadParam(lmsControl,LMS7param(DCOFFI_RFE),(uint16_t*)&value);
    int16_t dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFI_RFE->SetValue(dcvalue);
    LMS_ReadParam(lmsControl,LMS7param(DCOFFQ_RFE),(uint16_t*)&value);
    dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFQ_RFE->SetValue(dcvalue);

    int8_t dccorr;
    LMS_ReadParam(lmsControl,LMS7param(DCCORRI_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRI_TXTSP->SetValue(dccorr);
    LMS_ReadParam(lmsControl,LMS7param(DCCORRQ_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRQ_TXTSP->SetValue(dccorr);
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblCGENrefClk->SetLabel(wxString::Format(_("%f"), freq/1e6));
}

void lms7002_pnlCalibrations_view::OnCalibrationMethodChange( wxCommandEvent& event )
{
    if(rgrCalibrationMethod->GetSelection() == 0)
        lblCalibrationNote->SetLabel("");
    else
        lblCalibrationNote->SetLabel("Some boards might not have onboard loopback for selected Rx/Tx paths");
}
