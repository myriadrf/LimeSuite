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

///////////////////////////////////////////////////////////////////////////

void LMS7SuiteAppFrame::HandleLMSevent(wxCommandEvent& event)
{
    if (event.GetEventType() == CGEN_FREQUENCY_CHANGED)
    {
        lmsControl->SetInterfaceFrequency(lmsControl->GetFrequencyCGEN_MHz(), lmsControl->Get_SPI_Reg_bits(HBI_OVR_TXTSP), lmsControl->Get_SPI_Reg_bits(HBD_OVR_RXTSP));
        LMS_StreamBoard::Status status = LMS_StreamBoard::ConfigurePLL(streamBoardPort, lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Tx), lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx), 90);
        if (status != LMS_StreamBoard::STREAM_BRD_SUCCESS)
            wxMessageBox(_("Failed to configure Stream board PLL"), _("Warning"));
        if (fftviewer)
            fftviewer->SetNyquistFrequency(lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx));
    }
}

LMS7SuiteAppFrame::LMS7SuiteAppFrame( wxWindow* parent ) : AppFrame_view( parent )
{
    programmer = nullptr;
    fftviewer = nullptr;
    adfGUI = nullptr;
    si5351gui = nullptr;

    lms7controlPort = new LMScomms();
    streamBoardPort = new LMScomms();
    lmsControl = new LMS7002M(lms7controlPort, streamBoardPort);
	mContent->Initialize(lmsControl);
    Connect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);

    adfModule = new ADF4002();

    si5351module = new Si5351C();
    si5351module->Initialize(lms7controlPort);

	Layout();
	Fit();

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

        if (si5351gui)
            si5351gui->ModifyClocksGUI(info.device);
    }
    else
        statusBar->SetStatusText(_("Control port: Not Connected"), controlCollumn);
    //SetTitle(stemp);
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
    }
    else
        statusBar->SetStatusText(_("Data port: Not Connected"), dataCollumn);
    //SetTitle(stemp);
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
