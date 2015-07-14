#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "lms7002_mainPanel.h"

#include "lms7suiteAppFrame.h"
#include "LMS7002M.h"
#include "lmsComms.h"
#include "dlgAbout.h"
#include "dlgConnectionSettings.h"
#include "lms7suiteEvents.h"
#include "fftviewer_frFFTviewer.h"
#include "LMS_StreamBoard.h"
#include "ADF4002.h"
#include "ADF4002_wxgui.h"
#include "Si5351C.h"
#include "Si5351C_wxgui.h"
#include "LMS_Programing_wxgui.h"
#include "pnlMiniLog.h"
#include "RFSpark_wxgui.h"
#include "HPM7_wxgui.h"
#include "FPGAcontrols_wxgui.h"
#include "myriad7_wxgui.h"
#include <wx/string.h>

///////////////////////////////////////////////////////////////////////////

void LMS7SuiteAppFrame::HandleLMSevent(wxCommandEvent& event)
{
    if (event.GetEventType() == CGEN_FREQUENCY_CHANGED)
    {
        liblms7_status status = lmsControl->SetInterfaceFrequency(lmsControl->GetFrequencyCGEN_MHz(), lmsControl->Get_SPI_Reg_bits(HBI_OVR_TXTSP), lmsControl->Get_SPI_Reg_bits(HBD_OVR_RXTSP));
        if (status == LIBLMS7_SUCCESS)
        {
            wxCommandEvent evt;
            evt.SetEventType(LOG_MESSAGE);
            wxString msg;
            msg += _("Parameters modified: ");
            msg += wxString::Format(_("HBI_OVR: %i "), lmsControl->Get_SPI_Reg_bits(HBI_OVR_TXTSP, false));
            msg += wxString::Format(_("TXTSPCLKA_DIV: %i "), lmsControl->Get_SPI_Reg_bits(TXTSPCLKA_DIV, false));
            msg += wxString::Format(_("TXDIVEN: %i "), lmsControl->Get_SPI_Reg_bits(TXDIVEN, false));
            msg += wxString::Format(_("MCLK1SRC: %i "), lmsControl->Get_SPI_Reg_bits(MCLK1SRC, false));
            msg += wxString::Format(_("HBD_OVR: %i "), lmsControl->Get_SPI_Reg_bits(HBD_OVR_RXTSP, false));
            msg += wxString::Format(_("RXTSPCLKA_DIV: %i "), lmsControl->Get_SPI_Reg_bits(RXTSPCLKA_DIV, false));
            msg += wxString::Format(_("RXDIVEN: %i "), lmsControl->Get_SPI_Reg_bits(RXDIVEN, false));
            msg += wxString::Format(_("MCLK2SRC: %i "), lmsControl->Get_SPI_Reg_bits(MCLK2SRC, false));
            evt.SetString(msg);
            wxPostEvent(this, evt);
        }
        if (streamBoardPort->IsOpen())
        {
            LMS_StreamBoard::Status status = LMS_StreamBoard::ConfigurePLL(streamBoardPort, lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Tx), lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx), 90);
            if (status != LMS_StreamBoard::SUCCESS)
                wxMessageBox(_("Failed to configure Stream board PLL"), _("Warning"));
            else
            {
                wxCommandEvent evt;
                evt.SetEventType(LOG_MESSAGE);
                evt.SetString(wxString::Format(_("Stream board PLL configured Tx: %.3f MHz Rx: %.3f MHz Angle: %.0f deg"), lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Tx), lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx), 90.0));
                wxPostEvent(this, evt);
            }
        }
        if (fftviewer)
            fftviewer->SetNyquistFrequency(lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx));
    }
}

LMS7SuiteAppFrame::LMS7SuiteAppFrame( wxWindow* parent ) : AppFrame_view( parent )
{
#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif
    programmer = nullptr;
    fftviewer = nullptr;
    adfGUI = nullptr;
    si5351gui = nullptr;
    rfspark = nullptr;
    hpm7 = nullptr;
    fpgaControls = nullptr;
    myriad7 = nullptr;

    lms7controlPort = new LMScomms();
    streamBoardPort = new LMScomms();
    lmsControl = new LMS7002M(lms7controlPort, streamBoardPort);
	mContent->Initialize(lmsControl);
    Connect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    log = new pnlMiniLog(this, wxNewId());
    Connect(LOG_MESSAGE, wxCommandEventHandler(LMS7SuiteAppFrame::OnLogMessage), 0, this);
    contentSizer->Add(log, 1, wxEXPAND, 5);

    adfModule = new ADF4002();

    si5351module = new Si5351C();
    si5351module->Initialize(lms7controlPort);

	Layout();
	Fit();

    SetMinSize(GetSize());

	wxCommandEvent evt;
	OnDataBoardConnect(evt);
	OnControlBoardConnect(evt);
}

LMS7SuiteAppFrame::~LMS7SuiteAppFrame()
{
    Disconnect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
	delete lms7controlPort;
	delete streamBoardPort;

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

    dlg.SetConnectionManagers(lms7controlPort, streamBoardPort);
    dlg.Connect(CONTROL_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), NULL, this);
    dlg.Connect(DATA_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), NULL, this);
    dlg.Connect(CONTROL_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), NULL, this);
    dlg.Connect(DATA_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), NULL, this);
	dlg.ShowModal();
    dlg.Disconnect(CONTROL_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), NULL, this);
    dlg.Disconnect(DATA_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), NULL, this);
    dlg.Disconnect(CONTROL_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), NULL, this);
    dlg.Disconnect(DATA_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), NULL, this);
}

void LMS7SuiteAppFrame::OnAbout( wxCommandEvent& event )
{
	dlgAbout dlg(this);
    dlg.ShowModal();
}


void LMS7SuiteAppFrame::OnControlBoardConnect(wxCommandEvent& event)
{
    const int controlCollumn = 1;
    if (lms7controlPort->IsOpen())
    {
        LMSinfo info = lms7controlPort->GetInfo();
        wxString controlDev = _("Control port: ");
        controlDev.Append(wxString::From8BitData(GetDeviceName(info.device)));
        controlDev.Append(wxString::Format(_(" FW:%i HW:%i Protocol:%i"), (int)info.firmware, (int)info.hardware, (int)info.protocol));
        statusBar->SetStatusText(controlDev, controlCollumn);

        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);        
        evt.SetString(_("Connected ") + controlDev);
        wxPostEvent(this, evt);

        if (si5351gui)
            si5351gui->ModifyClocksGUI(info.device);
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
    assert(streamBoardPort != nullptr);
    const int dataCollumn = 2;
    if (streamBoardPort->IsOpen())
    {
        LMSinfo info = streamBoardPort->GetInfo();
        wxString controlDev = _("Data port: ");
        controlDev.Append(wxString::From8BitData(GetDeviceName(info.device)));
        controlDev.Append(wxString::Format(_(" FW:%i HW:%i Protocol:%i"), (int)info.firmware, (int)info.hardware, (int)info.protocol));
        statusBar->SetStatusText(controlDev, dataCollumn);

        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Connected ") + controlDev);
        wxPostEvent(this, evt);
    }
    else
    {
        statusBar->SetStatusText(_("Data port: Not Connected"), dataCollumn);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Disconnected data port"));
        wxPostEvent(this, evt);
    }
}

void LMS7SuiteAppFrame::OnFFTviewerClose(wxCloseEvent& event)
{
    fftviewer->StopStreaming();
    fftviewer->Destroy();
    fftviewer = nullptr;
}

void LMS7SuiteAppFrame::OnShowFFTviewer(wxCommandEvent& event)
{
    if (fftviewer) //it's already opened
        fftviewer->Show();
    else
    {
        fftviewer = new fftviewer_frFFTviewer(this);
        fftviewer->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnFFTviewerClose), NULL, this);
        fftviewer->Initialize(streamBoardPort);
        fftviewer->Show();
    }
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
        adfGUI->Initialize(adfModule, lms7controlPort);
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
        si5351gui->Initialize(si5351module);
        si5351gui->ModifyClocksGUI(lms7controlPort->GetInfo().device);
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
        programmer = new LMS_Programing_wxgui(lms7controlPort, this, wxNewId(), _("Programing"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
        programmer->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnProgramingClose), NULL, this);
        programmer->Show();
    }
}

void LMS7SuiteAppFrame::OnLogMessage(wxCommandEvent &event)
{
    log->HandleMessage(event);
}

void LMS7SuiteAppFrame::OnRFSparkClose(wxCloseEvent& event)
{
    rfspark->Destroy();
    rfspark = nullptr;
}
void LMS7SuiteAppFrame::OnShowRFSpark(wxCommandEvent& event)
{
    if (rfspark) //it's already opened
        rfspark->Show();
    else
    {
        rfspark = new RFSpark_wxgui(this, wxNewId(), _("RF-Spark"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
        rfspark->Initialize(lms7controlPort);
        rfspark->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnRFSparkClose), NULL, this);
        rfspark->Show();
    }
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
        hpm7->Initialize(lms7controlPort);
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
        fpgaControls->Initialize(streamBoardPort);
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
        myriad7->Initialize(lms7controlPort);
        myriad7->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnMyriad7Close), NULL, this);
        myriad7->Show();
    }
}