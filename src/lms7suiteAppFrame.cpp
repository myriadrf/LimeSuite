#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "lms7002_mainPanel.h"

#include "lms7suiteAppFrame.h"
#include "dlgAbout.h"
#include "dlgConnectionSettings.h"
#include "lms7suiteEvents.h"
#include "fftviewer_frFFTviewer.h"
#include "ADF4002_wxgui.h"
#include "Si5351C_wxgui.h"
#include "LMS_Programing_wxgui.h"
#include "pnlMiniLog.h"
#include "RFSpark_wxgui.h"
#include "HPM7_wxgui.h"
#include "FPGAcontrols_wxgui.h"
#include "myriad7_wxgui.h"
#include "lms7002m_novena_wxgui.h"
#include "SPI_wxgui.h"
#include <wx/string.h>
#include "dlgDeviceInfo.h"
#include <functional>
#include "lms7002_pnlTRF_view.h"
#include "lms7002_pnlRFE_view.h"
#include "lms7002_pnlLimeLightPAD_view.h"
#include "pnlBoardControls.h"
#include <LMSBoards.h>
#include <sstream>
#include <pnlQSpark.h>
#include "lms7_device.h"

using namespace std;
using namespace lime;

///////////////////////////////////////////////////////////////////////////

LMS7SuiteAppFrame* LMS7SuiteAppFrame::obj_ptr=nullptr;

const wxString LMS7SuiteAppFrame::cWindowTitle = _("LMS7Suite");

int LMS7SuiteAppFrame::m_lmsSelection = 0;

void LMS7SuiteAppFrame::HandleLMSevent(wxCommandEvent& event)
{
    double freq;
    if (event.GetEventType() == CGEN_FREQUENCY_CHANGED)
    {
        LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
        freq = lms->GetFrequencyCGEN();

        if (fftviewer)
            fftviewer->SetNyquistFrequency();
    }
    else if (event.GetEventType() == SAMPLE_POS_CHANGED)
        this->mContent->mTabLimeLight->UpdateGUI();
/*
    //in case of Novena board, need to update GPIO
    if (lms7controlPort && lms7controlPort->GetDeviceInfo().deviceName != GetDeviceName(LMS_DEV_NOVENA) &&
        (event.GetEventType() == LMS7_TXBAND_CHANGED || event.GetEventType() == LMS7_RXPATH_CHANGED))
    {
        //update external band-selection to match
        lmsControl->UpdateExternalBandSelect();
        if (novenaGui)
            novenaGui->UpdatePanel();
    }

    if (event.GetEventType() == LMS7_TXBAND_CHANGED)
    {
        const wxObject* eventSource = event.GetEventObject();
        const int bandIndex = event.GetInt();
        //update HPM7 if changes were made outside of it
        if (lms7controlPort && lms7controlPort->GetDeviceInfo().expansionName == GetExpansionBoardName(EXP_BOARD_HPM7) && eventSource != hpm7)
            hpm7->SelectBand(bandIndex);
        if (lms7controlPort && eventSource == hpm7)
        {
            lmsControl->Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), bandIndex == 0);
            lmsControl->Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), bandIndex == 1);
            mContent->mTabTRF->UpdateGUI();
        }
    }

    if (event.GetEventType() == LMS7_RXPATH_CHANGED)
    {
        const wxObject* eventSource = event.GetEventObject();
        const int pathIndex = event.GetInt();
        //update HPM7 if changes were made outside of it
        if (lms7controlPort && lms7controlPort->GetDeviceInfo().expansionName == GetExpansionBoardName(EXP_BOARD_HPM7) && eventSource != hpm7)
            hpm7->SelectRxPath(pathIndex);
        if (lms7controlPort && eventSource == hpm7)
        {
            lmsControl->Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), pathIndex);
            mContent->mTabRFE->UpdateGUI();
        }
    }
 */
}

void LMS7SuiteAppFrame::OnGlobalLogEvent(const lime::LogLevel level, const char *message)
{
    if (obj_ptr == nullptr || obj_ptr->mMiniLog == nullptr)
        return;
    wxCommandEvent evt;
    evt.SetString(wxString::FromAscii(message));
    evt.SetEventType(LOG_MESSAGE);
    evt.SetInt(int(level));
    wxPostEvent(obj_ptr, evt);
}

void LMS7SuiteAppFrame::OnLogEvent(const char* text, unsigned int type)
{
    if (obj_ptr == nullptr || obj_ptr->mMiniLog == nullptr)
        return;
    wxCommandEvent evt;
    evt.SetEventType(LOG_MESSAGE);
    wxString msg;

    switch(type)
    {
    case 0:
        msg = wxString::Format("INFO: %s", text);
        break;
    case 1:
        msg = wxString::Format("Warning: %s", text);
        break;
    case 2:
        msg = wxString::Format("ERROR: %s", text);
        break;
    }
    evt.SetString(msg);
    wxPostEvent(obj_ptr, evt);
}

LMS7SuiteAppFrame::LMS7SuiteAppFrame( wxWindow* parent ) :
    AppFrame_view( parent ), lmsControl(nullptr)
{
#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif
    programmer = nullptr;
    fftviewer = nullptr;
    adfGUI = nullptr;
    si5351gui = nullptr;
    hpm7 = nullptr;
    fpgaControls = nullptr;
    myriad7 = nullptr;
    deviceInfo = nullptr;
    spi = nullptr;
    boardControlsGui = nullptr;
    lmsControl = LMS7_Device::CreateDevice(nullptr);

    lime::registerLogHandler(&LMS7SuiteAppFrame::OnGlobalLogEvent);

    Connect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    Connect(SAMPLE_POS_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    Connect(LMS7_TXBAND_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    Connect(LMS7_RXPATH_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    mMiniLog = new pnlMiniLog(this, wxNewId());
    Connect(LOG_MESSAGE, wxCommandEventHandler(LMS7SuiteAppFrame::OnLogMessage), 0, this);

    contentSizer->Add(mMiniLog, 1, wxEXPAND, 5);
    contentSizer->AddGrowableRow(1);
    Layout();
    Fit();
    SetMinSize(GetSize());


    obj_ptr = this;
    wxCommandEvent event;
    OnControlBoardConnect(event);

    SetMinSize(GetBestSize());
    UpdateConnections(lmsControl);

    mnuCacheValues->Check(false);
    const int statusWidths[] = {-1, -3, -3};
    statusBar->SetStatusWidths(3, statusWidths);
    Bind(LMS_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::OnLmsChanged), this);
}

LMS7SuiteAppFrame::~LMS7SuiteAppFrame()
{
    wxCloseEvent evt;
    OnFFTviewerClose(evt);
    Disconnect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    Disconnect(SAMPLE_POS_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    LMS_Close(lmsControl);
}

void LMS7SuiteAppFrame::OnClose( wxCloseEvent& event )
{
    Destroy();
}

void LMS7SuiteAppFrame::OnQuit( wxCommandEvent& event )
{
    Destroy();
}

void LMS7SuiteAppFrame::OnShowConnectionSettings( wxCommandEvent& event )
{
	dlgConnectionSettings dlg(this);

    if (fftviewer)
        fftviewer->StopStreaming();

    dlg.SetConnectionManagers(&lmsControl);
    Bind(CONTROL_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), this);
    Bind(DATA_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), this);
    Bind(CONTROL_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), this);
    Bind(DATA_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), this);
	dlg.ShowModal();
}

void LMS7SuiteAppFrame::OnAbout( wxCommandEvent& event )
{
	dlgAbout dlg(this);
    dlg.ShowModal();
}

void LMS7SuiteAppFrame::UpdateConnections(lms_device_t* lms7controlPort)
{
    mContent->Initialize(lmsControl);
    if(si5351gui)
        si5351gui->Initialize(lmsControl);
    if(fftviewer)
        fftviewer->Initialize(lmsControl);
    if(adfGUI)
        adfGUI->Initialize(lmsControl);
    if(hpm7)
        hpm7->Initialize(lmsControl);
    if(fpgaControls)
        fpgaControls->Initialize(lmsControl);
    if(myriad7)
        myriad7->Initialize(lmsControl);
    if(deviceInfo)
        deviceInfo->Initialize(lmsControl);
    if(spi)
        spi->Initialize(lmsControl);
    if(boardControlsGui)
        boardControlsGui->Initialize(lmsControl);
    if(programmer)
        programmer->SetConnection(lmsControl);
}


void LMS7SuiteAppFrame::OnControlBoardConnect(wxCommandEvent& event)
{
    UpdateConnections(lmsControl);
    const int controlCollumn = 1;
    if (LMS_IsOpen(lmsControl,0))
    {
        //bind callback for spi data logging
        obj_ptr = this;
        const lms_dev_info_t* info;
        if ((info = LMS_GetDeviceInfo(lmsControl)) == nullptr)
                return;
        wxString controlDev = _("Control port: ");

        controlDev.Append(info->deviceName);
        LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
        double refClk = lms->GetReferenceClk_SX(lime::LMS7002M::Rx);
        controlDev.Append(wxString::Format(_(" FW:%s HW:%s Protocol:%s GW:%s GW_rev:%s Ref Clk: %1.2f MHz"), info->firmwareVersion, info->hardwareVersion, info->protocolVersion, info->gatewareVersion, info->gatewareRevision, refClk/1e6));
        statusBar->SetStatusText(controlDev, controlCollumn);

        auto conn =  ((LMS7_Device*)lmsControl)->GetConnection();
        conn->SetDataLogCallback(&LMS7SuiteAppFrame::OnLogDataTransfer);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Connected ") + controlDev);
        LMS_WriteParam(lmsControl, LMS7param(MAC), 1);
        wxPostEvent(this, evt);
        if (si5351gui)
            si5351gui->ModifyClocksGUI(info->deviceName);
        if (boardControlsGui)
            boardControlsGui->SetupControls(info->deviceName);
    }
    else
    {
        statusBar->SetStatusText(_("Control port: Not Connected"), controlCollumn);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Disconnected control port"));
        wxPostEvent(this, evt);
    }
}

void LMS7SuiteAppFrame::OnDataBoardConnect(wxCommandEvent& event)
{
   /* UpdateConnections(lms7controlPort, streamBoardPort);
    const int dataCollumn = 2;
    if (streamBoardPort && streamBoardPort->IsOpen())
    {
        //bind callback for spi data logging
        streamBoardPort->SetDataLogCallback(bind(&LMS7SuiteAppFrame::OnLogDataTransfer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        DeviceInfo info = streamBoardPort->GetDeviceInfo();
        wxString controlDev = _("Data port: ");
        controlDev.Append(info.deviceName);
        controlDev.Append(wxString::Format(_(" FW:%s HW:%s Protocol:%s GW:%s GW_rev:%s"), info.firmwareVersion, info.hardwareVersion, info.protocolVersion, info.gatewareVersion, info.gatewareRevision));
        statusBar->SetStatusText(controlDev, dataCollumn);

        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Connected ") + controlDev);
        wxPostEvent(this, evt);
    }
    else*/
    {
//        statusBar->SetStatusText(_("Data port: Not Connected"), dataCollumn);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Disconnected data port"));
        wxPostEvent(this, evt);
    }
}

void LMS7SuiteAppFrame::OnFFTviewerClose(wxCloseEvent& event)
{
    if(fftviewer)
    {
        fftviewer->StopStreaming();
        fftviewer->Destroy();
        fftviewer = nullptr;
    }
}

void LMS7SuiteAppFrame::OnShowFFTviewer(wxCommandEvent& event)
{
    if (fftviewer) //it's already opened
        fftviewer->Show();
    else
    {
        fftviewer = new fftviewer_frFFTviewer(this);
        fftviewer->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnFFTviewerClose), NULL, this);
        fftviewer->Show();
    }
    fftviewer->Initialize(lmsControl);
}

void LMS7SuiteAppFrame::OnLmsChanged(wxCommandEvent& event)
{
    m_lmsSelection = event.GetInt();
    if (fftviewer)
        fftviewer->Initialize(lmsControl);

    if (fpgaControls)
        fpgaControls->Initialize(lmsControl);
}

void LMS7SuiteAppFrame::OnADF4002Close(wxCloseEvent& event)
{
    adfGUI->Destroy();
    adfGUI = nullptr;
}

void LMS7SuiteAppFrame::OnShowADF4002(wxCommandEvent& event)
{
    if (adfGUI) //it's already opened
        adfGUI->Show();
    else
    {
        adfGUI = new ADF4002_wxgui(this, wxNewId(), _("ADF4002"));
        adfGUI->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnADF4002Close), NULL, this);
        adfGUI->Initialize(lmsControl);
        adfGUI->Show();
    }
}

void LMS7SuiteAppFrame::OnSi5351Close(wxCloseEvent& event)
{
    si5351gui->Destroy();
    si5351gui = nullptr;
}

void LMS7SuiteAppFrame::OnShowSi5351C(wxCommandEvent& event)
{
    if (si5351gui) //it's already opened
        si5351gui->Show();
    else
    {
        si5351gui = new Si5351C_wxgui(this, wxNewId(), _("Si5351C"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
        si5351gui->Initialize(lmsControl);
        const lms_dev_info_t *info = LMS_GetDeviceInfo(lmsControl);
        si5351gui->ModifyClocksGUI(info ? info->deviceName : "UNKNOWN");
        si5351gui->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnSi5351Close), NULL, this);
        si5351gui->Show();
    }
}

void LMS7SuiteAppFrame::OnProgramingClose(wxCloseEvent& event)
{
    programmer->Destroy();
    programmer = nullptr;
}

void LMS7SuiteAppFrame::OnShowPrograming(wxCommandEvent& event)
{
    if (programmer) //it's already opened
        programmer->Show();
    else
    {
        programmer = new LMS_Programing_wxgui(this, wxNewId(), _("Programming"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
        programmer->SetConnection(lmsControl);
        programmer->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnProgramingClose), NULL, this);
        programmer->Show();
    }
}

void LMS7SuiteAppFrame::OnLogMessage(wxCommandEvent &event)
{
    if (mMiniLog)
        mMiniLog->HandleMessage(event);
}


void LMS7SuiteAppFrame::OnHPM7Close(wxCloseEvent& event)
{
    hpm7->Destroy();
    hpm7 = nullptr;
}
void LMS7SuiteAppFrame::OnShowHPM7(wxCommandEvent& event)
{
    if (hpm7) //it's already opened
        hpm7->Show();
    else
    {
        hpm7 = new HPM7_wxgui(this, wxNewId(), _("HPM7"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
        hpm7->Initialize(lmsControl);
        hpm7->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnHPM7Close), NULL, this);
        hpm7->Show();
    }
}

void LMS7SuiteAppFrame::OnFPGAcontrolsClose(wxCloseEvent& event)
{
    fpgaControls->Destroy();
    fpgaControls = nullptr;
}
void LMS7SuiteAppFrame::OnShowFPGAcontrols(wxCommandEvent& event)
{
    if (fpgaControls) //it's already opened
        fpgaControls->Show();
    else
    {
        fpgaControls = new FPGAcontrols_wxgui(this, wxNewId(), _("FPGA Controls"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
        fpgaControls->Initialize(lmsControl);
        fpgaControls->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnFPGAcontrolsClose), NULL, this);
        fpgaControls->Show();
    }
}

void LMS7SuiteAppFrame::OnMyriad7Close(wxCloseEvent& event)
{
    myriad7->Destroy();
    myriad7 = nullptr;
}
void LMS7SuiteAppFrame::OnShowMyriad7(wxCommandEvent& event)
{
    if (myriad7) //it's already opened
        myriad7->Show();
    else
    {
        myriad7 = new Myriad7_wxgui(this, wxNewId(), _("Myriad7"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
        myriad7->Initialize(lmsControl);
        myriad7->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnMyriad7Close), NULL, this);
        myriad7->Show();
    }
}

void LMS7SuiteAppFrame::OnDeviceInfoClose(wxCloseEvent& event)
{
    deviceInfo->Destroy();
    deviceInfo = nullptr;
}

void LMS7SuiteAppFrame::OnShowDeviceInfo(wxCommandEvent& event)
{
    if (deviceInfo) //it's already opened
        deviceInfo->Show();
    else
    {
        deviceInfo = new dlgDeviceInfo(this, wxNewId(), _("Device Info"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
        deviceInfo->Initialize(lmsControl);
        deviceInfo->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnDeviceInfoClose), NULL, this);
        deviceInfo->Show();
    }
}

void LMS7SuiteAppFrame::OnSPIClose(wxCloseEvent& event)
{
    spi->Destroy();
    spi = nullptr;
}

void LMS7SuiteAppFrame::OnShowSPI(wxCommandEvent& event)
{
    if (spi) //it's already opened
        spi->Show();
    else
    {
        spi = new SPI_wxgui(this, wxNewId(), _("Device Info"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
        spi->Initialize(lmsControl,0);
        spi->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnSPIClose), NULL, this);
        spi->Show();
    }
}

#include <iomanip>
void LMS7SuiteAppFrame::OnLogDataTransfer(bool Tx, const unsigned char* data, const unsigned int length)
{
    if (obj_ptr->mMiniLog == nullptr || obj_ptr->mMiniLog->chkLogData->IsChecked() == false)
        return;
    std::stringstream ss;
    ss << (Tx ? "Wr(" : "Rd(");
    ss << length << "): ";
    ss << std::hex << std::setfill('0');
    int repeatedZeros = 0;
    for (int i = length - 1; i >= 0; --i)
        if (data[i] == 0)
            ++repeatedZeros;
        else
            break;
    if (repeatedZeros == 2)
        repeatedZeros = 0;
    repeatedZeros = repeatedZeros - (repeatedZeros & 0x1);
    for (size_t i = 0; i<length - repeatedZeros; ++i)
        //casting to short to print as numbers
        ss << " " << std::setw(2) << (unsigned short)data[i];
    if (repeatedZeros > 2)
        ss << " (00 x " << std::dec << repeatedZeros << " times)";
    cout << ss.str() << endl;
    wxCommandEvent *evt = new wxCommandEvent();
    evt->SetString(ss.str());
    evt->SetEventObject(obj_ptr);
    evt->SetEventType(LOG_MESSAGE);
    wxQueueEvent(obj_ptr, evt);
}

void LMS7SuiteAppFrame::OnShowBoardControls(wxCommandEvent& event)
{
    if (boardControlsGui) //it's already opened
        boardControlsGui->Show();
    else
    {
        boardControlsGui = new pnlBoardControls(this, wxNewId(), _("Board related controls"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        boardControlsGui->Initialize(lmsControl);
        boardControlsGui->UpdatePanel();
        boardControlsGui->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnBoardControlsClose), NULL, this);
        boardControlsGui->Show();
    }
}

void LMS7SuiteAppFrame::OnBoardControlsClose(wxCloseEvent& event)
{
    boardControlsGui->Destroy();
    boardControlsGui = nullptr;
}

void LMS7SuiteAppFrame::OnChangeCacheSettings(wxCommandEvent& event)
{
    int checked = event.GetInt();
    LMS_EnableCalibCache(lmsControl,checked);
}

