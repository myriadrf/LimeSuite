#include <assert.h>
#include "lms7002_mainPanel.h"
#include "lms7002_pnlAFE_view.h"
#include "lms7002_pnlBIAS_view.h"
#include "lms7002_pnlBIST_view.h"
#include "lms7002_pnlCDS_view.h"
#include "lms7002_pnlCLKGEN_view.h"
#include "lms7002_pnlLDO_view.h"
#include "lms7002_pnlLimeLightPAD_view.h"
#include "lms7002_pnlTxTSP_view.h"
#include "lms7002_pnlRxTSP_view.h"
#include "lms7002_pnlRBB_view.h"
#include "lms7002_pnlRFE_view.h"
#include "lms7002_pnlSX_view.h"
#include "lms7002_pnlTBB_view.h"
#include "lms7002_pnlTRF_view.h"
#include "lms7002_pnlXBUF_view.h"
#include "lms7002_pnlGains_view.h"
#include "lms7002_pnlCalibrations_view.h"
#include "SDRConfiguration_view.h"
#include <wx/time.h>
#include <wx/msgdlg.h>
#include <iostream>
#include <wx/filedlg.h>
#include "lms7suiteEvents.h"
#include "lms7002_pnlMCU_BD_view.h"
#include "lms7002_pnlR3.h"

#include "SDRDevice.h"
#include "LMS7002M.h"
using namespace std;
using namespace lime;

lms7002_mainPanel::lms7002_mainPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                                     const wxSize &size, long style)
    : wxPanel(parent, id, pos, size, style), sdrDevice(nullptr)
{
    wxFlexGridSizer *mainSizer;
    mainSizer = new wxFlexGridSizer(3, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(1);
    mainSizer->SetFlexibleDirection(wxBOTH);
    mainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxFlexGridSizer *szToolbar;
    szToolbar = new wxFlexGridSizer(7, 0, 0);
    szToolbar->AddGrowableCol(3);
    szToolbar->SetFlexibleDirection(wxHORIZONTAL);
    szToolbar->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    btnOpen = new wxButton(this, wxID_ANY, wxT("Open"));
    szToolbar->Add(btnOpen, 1, 0, 5);

    btnSave = new wxButton(this, wxID_ANY, wxT("Save"));
    szToolbar->Add(btnSave, 1, 0, 5);

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
    fgSizer249->SetFlexibleDirection( wxHORIZONTAL );
    fgSizer249->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    cmbLmsDevice = new wxChoice( this, wxNewId());
    cmbLmsDevice->Append( wxT("LMS 1") );
    cmbLmsDevice->Append( wxT("LMS 2") );
    cmbLmsDevice->SetSelection( 0 );
    cmbLmsDevice->SetToolTip( wxT("Controls the gain of the LNA") );

    fgSizer249->Add( cmbLmsDevice, 0, wxALL, 0 );

    rbChannelA = new wxRadioButton( this, ID_BTN_CH_A, wxT("A CHANNEL"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( rbChannelA, 0, wxALIGN_CENTER_VERTICAL, 5 );

    rbChannelB = new wxRadioButton( this, ID_BTN_CH_B, wxT("B CHANNEL"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( rbChannelB, 0, wxALIGN_CENTER_VERTICAL, 5 );

    chkEnableMIMO = new wxCheckBox( this, wxID_ANY, wxT("Enable MIMO"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEnableMIMO->SetToolTip( wxT("Enables required registers for MIMO mode") );

    fgSizer249->Add( chkEnableMIMO, 0, wxALIGN_CENTER_VERTICAL, 5 );

    fgSizer248->Add( fgSizer249, 0, wxEXPAND, 5 );

    fgSizer300->Add( fgSizer248, 0, wxEXPAND, 5 );

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

    szToolbar->Add(fgSizer300, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    txtTemperature = new wxStaticText( this, wxID_ANY, wxT("Temperature: ?????"), wxDefaultPosition, wxDefaultSize, 0 );
    txtTemperature->Wrap( -1 );
    szToolbar->Add(txtTemperature, 0, wxTOP | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);

    btnReadTemperature = new wxButton( this, wxID_ANY, wxT("Read Temp"), wxDefaultPosition, wxDefaultSize, 0 );
    szToolbar->Add(btnReadTemperature, 0, wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(szToolbar, 0, wxALIGN_LEFT | wxALIGN_TOP, 0);

    tabsNotebook = new wxNotebook(this, ID_TABS_NOTEBOOK);

    coarseConfigTab = new SDRConfiguration_view(tabsNotebook, wxNewId());
    tabsNotebook->AddPage(coarseConfigTab, wxT("Coarse Setup"), true);

    ILMS7002MTab *tab;
    tab = new lms7002_pnlCalibrations_view(tabsNotebook, ID_TAB_CALIBRATIONS);
    tabsNotebook->AddPage(tab, _("Calibration"), false);
    mTabs[ID_TAB_CALIBRATIONS] = tab;
#define CreatePage(klass, title)                                                                   \
    {                                                                                              \
        tab = new lms7002_pnl##klass##_view(tabsNotebook, ID_TAB_##klass);                         \
        tabsNotebook->AddPage(tab, wxT(title), false);                                             \
        mTabs[ID_TAB_##klass] = tab;                                                               \
    }
    CreatePage(RFE, "RFE");
    CreatePage(RBB, "RBB");
    CreatePage(TRF, "TRF");
    CreatePage(TBB, "TBB");
    CreatePage(AFE, "AFE");
    CreatePage(BIAS, "BIAS");
    CreatePage(LDO, "LDO");
    CreatePage(XBUF, "XBUF");
    CreatePage(CLKGEN, "CLKGEN");

    tab = new lms7002_pnlSX_view(tabsNotebook, ID_TAB_SXR);
    tab->SetChannel(0);
    tabsNotebook->AddPage(tab, wxT("SXR"), false);
    mTabs[ID_TAB_SXR] = tab;

    lms7002_pnlSX_view *sxtTab = new lms7002_pnlSX_view(tabsNotebook, ID_TAB_SXT);
    sxtTab->mIsSXT = true;
    sxtTab->SetChannel(1);
    tabsNotebook->AddPage(sxtTab, wxT("SXT"), false);
    mTabs[ID_TAB_SXT] = sxtTab;

    tab = new lms7002_pnlLimeLightPAD_view(tabsNotebook, ID_TAB_LIMELIGHT);
    tabsNotebook->AddPage(tab, wxT("LimeLight && PAD"), false);
    mTabs[ID_TAB_LIMELIGHT] = tab;

    CreatePage(TXTSP, "TXTSP");
    CreatePage(RXTSP, "RXTSP");
    CreatePage(CDS, "CDS");
    CreatePage(BIST, "BIST");

    tab = new lms7002_pnlGains_view(tabsNotebook, ID_TAB_GAINS);
    tabsNotebook->AddPage(tab, wxT("TRX Gain"), false);
    mTabs[ID_TAB_GAINS] = tab;

    lms7002_pnlMCU_BD_view *mTabMCU = new lms7002_pnlMCU_BD_view(tabsNotebook, ID_TAB_MCU);
    tabsNotebook->AddPage(mTabMCU, _("MCU"));
    mTabs[ID_TAB_MCU] = tab;

    lms7002_pnlR3_view *mTabR3 = new lms7002_pnlR3_view(tabsNotebook, ID_TAB_R3);
    tabsNotebook->AddPage(mTabR3, _("R3 Controls"));
    mTabs[ID_TAB_R3] = tab;
#undef CreatePage

    mainSizer->Add(tabsNotebook, 0, wxEXPAND, 5);
    this->SetSizerAndFit(mainSizer);

    // Connect Events
    btnOpen->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(lms7002_mainPanel::OnOpenProject), NULL, this);
    btnSave->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(lms7002_mainPanel::OnSaveProject), NULL, this);
    cmbLmsDevice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( lms7002_mainPanel::OnChannelOrSOCChange), NULL, this );
    rbChannelA->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                        wxCommandEventHandler(lms7002_mainPanel::OnChannelOrSOCChange), NULL, this);
    rbChannelB->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                        wxCommandEventHandler(lms7002_mainPanel::OnChannelOrSOCChange), NULL, this);
    chkEnableMIMO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnEnableMIMOchecked ), NULL, this );
    btnDownloadAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnDownloadAll ), NULL, this );
    btnUploadAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnUploadAll ), NULL, this );
    btnResetChip->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnResetChip ), NULL, this );
    btnLoadDefault->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnLoadDefault ), NULL, this );
    btnReadTemperature->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnReadTemperature ), NULL, this );
    tabsNotebook->Bind( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, &lms7002_mainPanel::Onnotebook_modulesPageChanged, this);
    Connect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(lms7002_mainPanel::OnCGENFrequencyChange), NULL, this);
}

lms7002_mainPanel::~lms7002_mainPanel()
{
    tabsNotebook->Unbind( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, &lms7002_mainPanel::Onnotebook_modulesPageChanged, this);
}

void lms7002_mainPanel::UpdateVisiblePanel()
{
    if (sdrDevice == nullptr)
        return;

    wxWindow* currentPage = tabsNotebook->GetCurrentPage();
    const wxWindowID pageId = currentPage->GetId();
    uint16_t spisw_ctrl = 0;

    LMS7002M* chip = GetSelectedChip();

    if (pageId == ID_TAB_SXR) //change active channel to A
        chip->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    else if (pageId == ID_TAB_SXT) //change active channel to B
        chip->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    else
        chip->Modify_SPI_Reg_bits(LMS7param(MAC), rbChannelA->GetValue() == 1 ? 1 : 2);
    spisw_ctrl = chip->SPI_read(0x0006);

    if(spisw_ctrl & 1) // transceiver controlled by MCU
    {
        if (pageId != ID_TAB_MCU && pageId != ID_TAB_GAINS)
            currentPage->Disable();
        return;
    }
    else
        currentPage->Enable();

    wxLongLong t1, t2;
    t1 = wxGetUTCTimeMillis();
    auto currentTab = mTabs.find(pageId);
    if (currentTab != mTabs.end())
        currentTab->second->UpdateGUI();
    t2 = wxGetUTCTimeMillis();
#ifndef NDEBUG
    cout << "Visible GUI update time: " << (t2 - t1).ToString() << endl;
#endif
}

void lms7002_mainPanel::Initialize(SDRDevice *pControl)
{
    sdrDevice = pControl;
    if (sdrDevice == nullptr)
    {
        coarseConfigTab->Initialize(sdrDevice);
        for (auto &tab : mTabs)
            tab.second->Initialize(nullptr);
        return;
    }
    cmbLmsDevice->SetSelection(0);

    coarseConfigTab->Initialize(sdrDevice);
    const int socCount = sdrDevice->GetDescriptor().rfSOC.size();
    if (socCount > 1)
    {

        cmbLmsDevice->Clear();
        for(int i=0; i<socCount; ++i)
            cmbLmsDevice->Append( wxString::Format("LMS %i", i+1));
        cmbLmsDevice->SetSelection( 0 );
        cmbLmsDevice->Show();
    }
    else
        cmbLmsDevice->Hide();
    rbChannelA->SetValue(true);
    rbChannelB->SetValue(false);
    LMS7002M* chip = GetSelectedChip();
    for (auto &tab : mTabs)
        tab.second->Initialize(chip);
    UpdateGUI();
    Layout();
}

void lms7002_mainPanel::OnResetChip(wxCommandEvent &event)
{
    try {
        sdrDevice->Reset();
    }
    catch (std::runtime_error &e)
    {
        wxMessageBox(wxString::Format("Reset failed: %s", e.what()), _("Warning"));
        return;
    }
    wxNotebookEvent evt;
    chkEnableMIMO->SetValue(false);
    Onnotebook_modulesPageChanged(evt); //after reset chip active channel might change, this refresh channel for active tab
}

void lms7002_mainPanel::OnLoadDefault(wxCommandEvent& event)
{
    try {
        sdrDevice->Reset();
    }
    catch (std::runtime_error &e) {
        wxMessageBox(wxString::Format("Load Default failed: %s", e.what()), _("Warning"));
        return;
    }
    // TODO: LMS_EnableChannel(sdrDevice, LMS_CH_TX, 0, true); //enable TX, LMS_Init() no longer does it
    wxNotebookEvent evt;
    chkEnableMIMO->SetValue(false);
    //((LMS7_Device*)sdrDevice)->SetActiveChip(cmbLmsDevice->GetSelection());
    Onnotebook_modulesPageChanged(evt); //after reset chip active channel might change, this refresh channel for active tab
    wxCommandEvent evt2;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt2);
}

void lms7002_mainPanel::UpdateGUI()
{
    wxLongLong t1, t2;
    t1 = wxGetUTCTimeMillis();
    t2 = wxGetUTCTimeMillis();
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnOpenProject( wxCommandEvent& event )
{
    wxFileDialog dlg(this, _("Open config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    LMS7002M* chip = GetSelectedChip();
    if(chip == nullptr)
    {
        wxMessageBox(_("Failed to load file: invalid chip index"), _("Warning"));
        return;
    }
    try {
         if (chip->LoadConfig(dlg.GetPath().To8BitData()) != 0)
            throw;
    }
    catch (std::runtime_error &e)
    {
        wxMessageBox(wxString::Format("Failed to load file: %s", e.what()), _("Warning"));
        return;
    }
    wxCommandEvent tevt;
    // LMS_WriteParam(sdrDevice, LMS7param(MAC), rbChannelA->GetValue() == 1 ? 1 : 2);
    UpdateGUI();
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
}

void lms7002_mainPanel::OnSaveProject( wxCommandEvent& event )
{
    wxFileDialog dlg(this, _("Save config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    LMS7002M* chip = GetSelectedChip();
    if(chip == nullptr)
    {
        wxMessageBox(_("Failed to save file: invalid chip index"), _("Warning"));
        return;
    }

    if (chip->SaveConfig(dlg.GetPath().To8BitData()) != 0)
        wxMessageBox(_("Failed to save file"), _("Warning"));
}

void lms7002_mainPanel::OnChannelOrSOCChange(wxCommandEvent &event)
{
    int channel = -1;
    if (rbChannelA->GetValue())
        channel = 0;
    else if (rbChannelB->GetValue())
        channel = 1;

    LMS7002M* chip = GetSelectedChip();
    for (auto iter : mTabs) {
        if (iter.first == ID_TAB_SXR || iter.first == ID_TAB_SXT)
            continue; // do not change assigned channel for SXR/SXT tabs
        iter.second->Initialize(chip);
        iter.second->SetChannel(channel);
    }
    UpdateVisiblePanel();
}

void lms7002_mainPanel::Onnotebook_modulesPageChanged( wxNotebookEvent& event )
{
    const wxWindowID pageId = tabsNotebook->GetCurrentPage()->GetId();
    switch (pageId) {
    case ID_TAB_AFE:
    case ID_TAB_BIAS:
    case ID_TAB_LDO:
    case ID_TAB_XBUF:
    case ID_TAB_CLKGEN:
    case ID_TAB_CDS:
    case ID_TAB_BIST:
        rbChannelA->Disable();
        rbChannelB->Disable();
        break;
    case ID_TAB_SXR: //change active channel to A
    case ID_TAB_SXT: //change active channel to B
        rbChannelA->Disable();
        rbChannelB->Disable();
        break;
    default:
        rbChannelA->Enable();
        rbChannelB->Enable();
    }

#ifdef __APPLE__
    //force show the page selected by the event (needed on apple)
    if (event.GetSelection() != -1)
    {
        dynamic_cast<wxNotebook*>(event.GetEventObject())->GetPage(event.GetSelection())->Show(true);
    }
#endif

    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnDownloadAll(wxCommandEvent& event)
{
    try {
        sdrDevice->Synchronize(false);
    }
    catch (std::runtime_error &e)
    {
        wxMessageBox(wxString::Format("Download all registers failed: %s", e.what()), _("Warning"));
        return;
    }
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnUploadAll(wxCommandEvent& event)
{
    try {
        sdrDevice->Synchronize(false);
    }
    catch (std::runtime_error &e)
    {
        wxMessageBox(wxString::Format("Download all registers failed: %s", e.what()), _("Warning"));
        return;
    }

    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnReadTemperature(wxCommandEvent& event)
{
    double t = 0.0;
    int status = -1; // TODO: LMS_GetChipTemperature(sdrDevice, 0, &t);
    if (status != 0)
        wxMessageBox(_("Failed to read chip temperature"), _("Warning"));
    txtTemperature->SetLabel(wxString::Format("Temperature: %.0f C", t));
}

void lms7002_mainPanel::OnEnableMIMOchecked(wxCommandEvent& event)
{
    // TODO:
    // uint16_t chBck;
    // LMS_ReadParam(sdrDevice, LMS7param(MAC), &chBck);
    // bool enable = chkEnableMIMO->IsChecked();
    // for (int ch = enable ? 0 : 1; ch < LMS_GetNumChannels(sdrDevice, false); ch++) {
    //     LMS_EnableChannel(sdrDevice, LMS_CH_RX, ch, enable);
    //     LMS_EnableChannel(sdrDevice, LMS_CH_TX, ch, enable);
    // }
    // LMS_WriteParam(sdrDevice, LMS7param(MAC), chBck);
    // UpdateVisiblePanel();
}

LMS7002M* lms7002_mainPanel::GetSelectedChip() const
{
    LMS7002M* chip = static_cast<LMS7002M*>(sdrDevice->GetInternalChip(cmbLmsDevice->GetSelection()));
    return chip;
}

void lms7002_mainPanel::OnCGENFrequencyChange(wxCommandEvent &event)
{
    //if (event.GetEventType() == CGEN_FREQUENCY_CHANGED)
    {
        LMS7002M* lms = GetSelectedChip();
        int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        float phaseOffset = -999;
        sdrDevice->SetFPGAInterfaceFreq(decim, interp, phaseOffset, phaseOffset); // TODO: switch for automatic phase offset
    }
}
