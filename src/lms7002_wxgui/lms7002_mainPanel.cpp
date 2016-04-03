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
#include "lms7002_pnlCalibrations_view.h"
#include "LMS7002M.h"
#include <wx/time.h>
#include <wx/msgdlg.h>
#include <iostream>
#include <wx/filedlg.h>
#include "lms7suiteEvents.h"
#include "lms7002_pnlMCU_BD_view.h"
#include "lms7002_pnlBuffers_view.h"
using namespace std;
using namespace lime;

lms7002_mainPanel::lms7002_mainPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    :
    mainPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    mTabMCU = new lms7002_pnlMCU_BD_view(tabsNotebook);
    tabsNotebook->AddPage(mTabMCU, _("MCU"));
}

lms7002_mainPanel::~lms7002_mainPanel()
{
}

void lms7002_mainPanel::UpdateVisiblePanel()
{
    wxLongLong t1, t2;
    t1 = wxGetUTCTimeMillis();
    long visibleTabId = tabsNotebook->GetCurrentPage()->GetId();
    switch (visibleTabId)
    {
    case ID_TAB_AFE:
        mTabAFE->UpdateGUI();
        break;
    case ID_TAB_BIAS:
        mTabBIAS->UpdateGUI();
        break;
    case ID_TAB_BIST:
        mTabBIST->UpdateGUI();
        break;
    case ID_TAB_CDS:
        mTabCDS->UpdateGUI();
        break;
    case ID_TAB_CGEN:
        mTabCGEN->UpdateGUI();
        break;
    case ID_TAB_LDO:
        mTabLDO->UpdateGUI();
        break;
    case ID_TAB_LIMELIGHT:
        mTabLimeLight->UpdateGUI();
        break;
    case ID_TAB_TXTSP:
        mTabTxTSP->UpdateGUI();
        break;
    case ID_TAB_RXTSP:
        mTabRxTSP->UpdateGUI();
        break;
    case ID_TAB_RBB:
        mTabRBB->UpdateGUI();
        break;
    case ID_TAB_RFE:
        mTabRFE->UpdateGUI();
        break;
    case ID_TAB_SXR:
        mTabSXR->UpdateGUI();
        break;
    case ID_TAB_SXT:
        mTabSXT->UpdateGUI();
        break;
    case ID_TAB_TBB:
        mTabTBB->UpdateGUI();
        break;
    case ID_TAB_TRF:
        mTabTRF->UpdateGUI();
        break;
    case ID_TAB_XBUF:
        mTabXBUF->UpdateGUI();
        break;
    case ID_TAB_CALIBRATIONS:
        mTabCalibrations->UpdateGUI();
        break;
    case ID_TAB_BUFFERS:
        mTabBuffers->UpdateGUI();
        break;
    }
    t2 = wxGetUTCTimeMillis();
#ifndef NDEBUG
    cout << "Visible GUI update time: " << (t2 - t1).ToString() << endl;
#endif
}

void lms7002_mainPanel::Initialize(LMS7002M* pControl)
{
    assert(pControl != nullptr);
    lmsControl = pControl;
    mTabRFE->Initialize(lmsControl);
    mTabRBB->Initialize(lmsControl);
    mTabTRF->Initialize(lmsControl);
    mTabTBB->Initialize(lmsControl);
    mTabAFE->Initialize(lmsControl);
    mTabBIAS->Initialize(lmsControl);
    mTabLDO->Initialize(lmsControl);
    mTabXBUF->Initialize(lmsControl);
    mTabCGEN->Initialize(lmsControl);
    mTabSXR->Initialize(lmsControl);
    mTabSXT->Initialize(lmsControl);
    mTabLimeLight->Initialize(lmsControl);
    mTabTxTSP->Initialize(lmsControl);
    mTabRxTSP->Initialize(lmsControl);
    mTabCDS->Initialize(lmsControl);
    mTabBIST->Initialize(lmsControl);
    mTabCalibrations->Initialize(lmsControl);
    mTabMCU->Initialize(lmsControl->GetMCUControls());
// TODO setup buffers gui
    //mTabBuffers->Initialize(lmsControl->GetControlPort());
    UpdateGUI();
}

void lms7002_mainPanel::OnResetChip(wxCommandEvent &event)
{
    liblms7_status status = lmsControl->ResetChip();
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Chip reset: %s"), wxString::From8BitData(liblms7_status2string(status))), _("Warning"));
    wxNotebookEvent evt;
    Onnotebook_modulesPageChanged(evt); //after reset chip active channel might change, this refresh channel for active tab
}

void lms7002_mainPanel::UpdateGUI()
{
    wxLongLong t1, t2;
    t1 = wxGetUTCTimeMillis();
    lmsControl->IsSynced();
    t2 = wxGetUTCTimeMillis();
    LMS7002M::Channel channel = lmsControl->GetActiveChannel();
    if (channel == LMS7002M::ChA)
    {
        rbChannelA->SetValue(true);
        rbChannelB->SetValue(false);
    }
    else if (channel == LMS7002M::ChB)
    {
        rbChannelA->SetValue(false);
        rbChannelB->SetValue(true);
    }
    else
    {
        lmsControl->SetActiveChannel(LMS7002M::ChA);
        rbChannelA->SetValue(true);
        rbChannelB->SetValue(false);
    }

    UpdateVisiblePanel();
#ifndef NDEBUG
    cout << "GUI update time: " << (t2 - t1).ToString() << endl;
#endif
}

void lms7002_mainPanel::OnNewProject( wxCommandEvent& event )
{
    lmsControl->ResetChip();
    lmsControl->DownloadAll();
    lmsControl->SetActiveChannel(rbChannelA->GetValue() == 1 ? LMS7002M::ChA : LMS7002M::ChB);
    UpdateGUI();
}

void lms7002_mainPanel::OnOpenProject( wxCommandEvent& event )
{
    wxFileDialog dlg(this, _("Open config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    liblms7_status status = lmsControl->LoadConfig(dlg.GetPath().To8BitData());
    if (status != LIBLMS7_SUCCESS)
    {
        if (status != LIBLMS7_NOT_CONNECTED)
            wxMessageBox(wxString::Format(_("Failed to load file: %s"), liblms7_status2string(status)), _("Warning"));
    }
    wxCommandEvent tevt;
    lmsControl->SetActiveChannel(rbChannelA->GetValue() == 1 ? LMS7002M::ChA : LMS7002M::ChB);
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
    liblms7_status status = lmsControl->SaveConfig(dlg.GetPath().To8BitData());
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(_("Failed to save file"), _("Warning"));
}

void lms7002_mainPanel::OnRegistersTest( wxCommandEvent& event )
{
    liblms7_status status = lmsControl->RegistersTest();
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(_("Registers test failed!"), _("WARNING"));
    else
        wxMessageBox(_("Registers test passed!"), _("INFO"));
}

void lms7002_mainPanel::OnSwitchToChannelA(wxCommandEvent& event)
{
    lmsControl->SetActiveChannel(LMS7002M::ChA);
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnSwitchToChannelB(wxCommandEvent& event)
{
    lmsControl->SetActiveChannel(LMS7002M::ChB);
    UpdateVisiblePanel();
}

void lms7002_mainPanel::Onnotebook_modulesPageChanged( wxNotebookEvent& event )
{
    wxNotebookPage* page = tabsNotebook->GetCurrentPage();
    if (page == mTabAFE || page == mTabBIAS || page == mTabLDO || page == mTabXBUF || page == mTabCGEN || page == mTabCDS || page == mTabBIST)
    {
        rbChannelA->Disable();
        rbChannelB->Disable();
    }
    else if (page == mTabSXR) //change active channel to A
    {
        lmsControl->SetActiveChannel(LMS7002M::ChA);
        rbChannelA->Disable();
        rbChannelB->Disable();
    }
    else if (page == mTabSXT) //change active channel to B
    {
        lmsControl->SetActiveChannel(LMS7002M::ChB);
        rbChannelA->Disable();
        rbChannelB->Disable();
    }
    else
    {
        lmsControl->SetActiveChannel(rbChannelA->GetValue() == 1 ? LMS7002M::ChA : LMS7002M::ChB);
        rbChannelA->Enable();
        rbChannelB->Enable();
    }
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnDownloadAll(wxCommandEvent& event)
{
    liblms7_status status = lmsControl->DownloadAll();
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Download all registers: %s"), wxString::From8BitData(liblms7_status2string(status))), _("Warning"));
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnUploadAll(wxCommandEvent& event)
{
    liblms7_status status = lmsControl->UploadAll();
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Upload all registers: %s"), wxString::From8BitData(liblms7_status2string(status))), _("Warning"));
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    UpdateVisiblePanel();
}
