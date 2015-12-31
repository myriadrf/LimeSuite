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
#include "IConnection.h"
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
#include "lms7002m_novena_wxgui.h"
#include "SPI_wxgui.h"
#include <wx/string.h>
#include "dlgDeviceInfo.h"
#include <functional>
#include "lms7002_pnlTRF_view.h"
#include "lms7002_pnlRFE_view.h"
#include "pnlBoardControls.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////

const wxString LMS7SuiteAppFrame::cWindowTitle = _("LMS7Suite");

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
        if (streamBoardPort->IsOpen() && streamBoardPort->GetDeviceInfo().deviceName != GetDeviceName(LMS_DEV_NOVENA))
        {
// TODO : get FPGA device index
            const int fpgaIndex = 0;
            LMS_StreamBoard::Status status = LMS_StreamBoard::ConfigurePLL(streamBoardPort, fpgaIndex, lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Tx), lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx), 90);
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
        {
            int decimation = lmsControl->Get_SPI_Reg_bits(HBD_OVR_RXTSP);
            float samplingFreq_MHz = lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx);
            if (decimation != 7)
                samplingFreq_MHz /= pow(2.0, decimation);
            fftviewer->SetNyquistFrequency(samplingFreq_MHz / 2);
        }
    }

    //in case of Novena board, need to update GPIO
    if (lms7controlPort->GetDeviceInfo().deviceName != GetDeviceName(LMS_DEV_NOVENA) &&
        (event.GetEventType() == LMS7_TXBAND_CHANGED || event.GetEventType() == LMS7_RXPATH_CHANGED))
    {
        const uint16_t NOVENA_GPIO_ADDR = 0x0706;
        uint16_t regValue = lmsControl->SPI_read(NOVENA_GPIO_ADDR) & 0xFFF8;
        //lms_gpio2 - tx output selection:
        //		0 - TX1_A and TX1_B (Band 1),
        //		1 - TX2_A and TX2_B (Band 2)
        regValue |= lmsControl->Get_SPI_Reg_bits(SEL_BAND2_TRF, false) << 2; //gpio2
        //RX active paths
        //lms_gpio0 | lms_gpio1      	RX_A		RX_B
        //  0 			0       =>  	no active path
        //  1   		0 		=>	LNAW_A  	LNAW_B
        //  0			1		=>	LNAH_A  	LNAH_B
        //  1			1		=>	LNAL_A 	 	LNAL_B
        switch(lmsControl->Get_SPI_Reg_bits(SEL_PATH_RFE, false))
        {
            //set gpio1:gpio0
            case 0: regValue |= 0x0; break;
            case 1: regValue |= 0x2; break;
            case 2: regValue |= 0x3; break;
            case 3: regValue |= 0x1; break;
        }
        lmsControl->SPI_write(NOVENA_GPIO_ADDR, regValue);
        if (novenaGui)
            novenaGui->UpdatePanel();
    }

    if (event.GetEventType() == LMS7_TXBAND_CHANGED)
    {
        const wxObject* eventSource = event.GetEventObject();
        const int bandIndex = event.GetInt();
        //update HPM7 if changes were made outside of it
        if (lms7controlPort->GetDeviceInfo().expansionName == GetExpansionBoardName(EXP_BOARD_HPM7) && eventSource != hpm7)
            hpm7->SelectBand(bandIndex);
        if (eventSource == hpm7)
        {
            lmsControl->Modify_SPI_Reg_bits(SEL_BAND1_TRF, bandIndex == 0);
            lmsControl->Modify_SPI_Reg_bits(SEL_BAND2_TRF, bandIndex == 1);
            mContent->mTabTRF->UpdateGUI();
        }
    }
    if (event.GetEventType() == LMS7_RXPATH_CHANGED)
    {
        const wxObject* eventSource = event.GetEventObject();
        const int pathIndex = event.GetInt();
        //update HPM7 if changes were made outside of it
        if (lms7controlPort->GetDeviceInfo().expansionName == GetExpansionBoardName(EXP_BOARD_HPM7) && eventSource != hpm7)
            hpm7->SelectRxPath(pathIndex);
        if (eventSource == hpm7)
        {
            lmsControl->Modify_SPI_Reg_bits(SEL_PATH_RFE, pathIndex);
            mContent->mTabRFE->UpdateGUI();
        }
    }
}

LMS7SuiteAppFrame::LMS7SuiteAppFrame( wxWindow* parent ) :
    AppFrame_view( parent ), lms7controlPort(nullptr), streamBoardPort(nullptr)
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
    deviceInfo = nullptr;
    spi = nullptr;
    novenaGui = nullptr;
    boardControlsGui = nullptr;

    lmsControl = new LMS7002M();
    mContent->Initialize(lmsControl);
    Connect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    Connect(LMS7_TXBAND_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    Connect(LMS7_RXPATH_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    mMiniLog = new pnlMiniLog(this, wxNewId());
    Connect(LOG_MESSAGE, wxCommandEventHandler(LMS7SuiteAppFrame::OnLogMessage), 0, this);

    contentSizer->Add(mMiniLog, 1, wxEXPAND, 5);

    adfModule = new ADF4002();
    si5351module = new Si5351C();

	Layout();
	Fit();

    SetMinSize(GetSize());
    UpdateConnections(lms7controlPort, streamBoardPort);
}

LMS7SuiteAppFrame::~LMS7SuiteAppFrame()
{
    Disconnect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);

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

    dlg.SetConnectionManagers(&lms7controlPort, &streamBoardPort);
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


void LMS7SuiteAppFrame::OnControlBoardConnect(wxCommandEvent& event)
{
    const int controlCollumn = 1;
    if (lms7controlPort && lms7controlPort->IsOpen())
    {
        //bind callback for spi data logging
        lms7controlPort->SetDataLogCallback(bind(&LMS7SuiteAppFrame::OnLogDataTransfer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        UpdateConnections(lms7controlPort, streamBoardPort);

        DeviceInfo info = lms7controlPort->GetDeviceInfo();
        wxString controlDev = _("Control port: ");
        controlDev.Append(info.deviceName);
        controlDev.Append(wxString::Format(_(" FW:%s HW:%s Protocol:%s"), info.firmwareVersion, info.hardwareVersion, info.protocolVersion));
        statusBar->SetStatusText(controlDev, controlCollumn);

        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Connected ") + controlDev);
        wxPostEvent(this, evt);
//  TODO : setup controls according to connected board
//        if (si5351gui)
//            si5351gui->ModifyClocksGUI(info.device);
//
//        if (boardControlsGui)
//            boardControlsGui->SetupControls(info.device);

        //must configure synthesizer before using SoDeRa
        if (info.deviceName == GetDeviceName(LMS_DEV_SODERA))
        {
            si5351module->SetPLL(0, 25000000, 0);
            si5351module->SetPLL(1, 25000000, 0);
            si5351module->SetClock(0, 27000000, true, false);
            si5351module->SetClock(1, 27000000, true, false);
            for (int i = 2; i < 8; ++i)
                si5351module->SetClock(i, 27000000, false, false);
            Si5351C::Status status = si5351module->ConfigureClocks();
            if (status != Si5351C::SUCCESS)
            {
                wxMessageBox(_("Failed to configure Si5351C"), _("Warning"));
                return;
            }
            status = si5351module->UploadConfiguration();
            if (status != Si5351C::SUCCESS)
                wxMessageBox(_("Failed to upload Si5351C configuration"), _("Warning"));
        }
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
    const int dataCollumn = 2;
    if (streamBoardPort && streamBoardPort->IsOpen())
    {
        //bind callback for spi data logging
        streamBoardPort->SetDataLogCallback(bind(&LMS7SuiteAppFrame::OnLogDataTransfer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        UpdateConnections(lms7controlPort, streamBoardPort);

        DeviceInfo info = streamBoardPort->GetDeviceInfo();
        wxString controlDev = _("Data port: ");
        controlDev.Append(info.deviceName);
        controlDev.Append(wxString::Format(_(" FW:%s HW:%s Protocol:%s"), info.firmwareVersion, info.hardwareVersion, info.protocolVersion));
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
        fftviewer->Show();
        int decimation = lmsControl->Get_SPI_Reg_bits(HBD_OVR_RXTSP);
        float samplingFreq_MHz = lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx);
        if (decimation != 7)
            samplingFreq_MHz /= pow(2.0, decimation);
        fftviewer->SetNyquistFrequency(samplingFreq_MHz / 2);
    }
    fftviewer->Initialize(streamBoardPort);
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
// TODO : modify clock names according to connected board
//        si5351gui->ModifyClocksGUI(lms7controlPort->GetInfo().device);
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
    if (mMiniLog)
        mMiniLog->HandleMessage(event);
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
        rfspark = new RFSpark_wxgui(this, wxNewId(), _("RF-ESpark"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
// TODO : initialize with IConnection, currently used only by RFSpark board
//        rfspark->Initialize(lms7controlPort);
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
// TODO : initialize with IConnection, currently used only for HPM7 expansion board
//        hpm7->Initialize(lms7controlPort);
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
        deviceInfo->Initialize(lms7controlPort, streamBoardPort);
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
        spi->Initialize(lms7controlPort, streamBoardPort);
        spi->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnSPIClose), NULL, this);
        spi->Show();
    }
}

#include <iomanip>
void LMS7SuiteAppFrame::OnLogDataTransfer(bool Tx, const unsigned char* data, const unsigned int length)
{
    if (mMiniLog == nullptr || mMiniLog->chkLogData->IsChecked() == false)
        return;
    stringstream ss;
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
    for (int i = 0; i<length - repeatedZeros; ++i)
        //casting to short to print as numbers
        ss << " " << std::setw(2) << (unsigned short)data[i];
    if (repeatedZeros > 2)
        ss << " (00 x " << std::dec << repeatedZeros << " times)";
    cout << ss.str() << endl;
    wxCommandEvent *evt = new wxCommandEvent();
    evt->SetString(ss.str());
    evt->SetEventObject(this);
    evt->SetEventType(LOG_MESSAGE);
    wxQueueEvent(this, evt);
}

void LMS7SuiteAppFrame::OnShowNovena(wxCommandEvent& event)
{
    if (novenaGui) //it's already opened
        novenaGui->Show();
    else
    {
        novenaGui = new LMS7002M_Novena_wxgui(this, wxNewId(), _("Novena"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
        novenaGui->Initialize(lms7controlPort);
        novenaGui->UpdatePanel();
        novenaGui->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnNovenaClose), NULL, this);
        novenaGui->Show();
    }
}

void LMS7SuiteAppFrame::OnNovenaClose(wxCloseEvent& event)
{
    novenaGui->Destroy();
    novenaGui = nullptr;
}

void LMS7SuiteAppFrame::OnShowBoardControls(wxCommandEvent& event)
{
    if (boardControlsGui) //it's already opened
        boardControlsGui->Show();
    else
    {
        boardControlsGui = new pnlBoardControls(this, wxNewId(), _("Board related controls"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
// TODO : initialize with IConnection, currently used only by SoDeRa board
        //boardControlsGui->Initialize(lms7controlPort);
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

void LMS7SuiteAppFrame::UpdateConnections(IConnection* lms7controlPort, IConnection* streamBoardPort)
{
    if(lmsControl)
        lmsControl->SetConnection(lms7controlPort);
    if(si5351module)
        si5351module->Initialize(lms7controlPort);
    if(fftviewer)
        fftviewer->Initialize(streamBoardPort);
    if(adfGUI)
        adfGUI->Initialize(adfModule, lms7controlPort);
//    if(rfspark)
//        rfspark->Initialize(lms7controlPort);
//    if(hpm7)
//        hpm7->Initialize(lms7controlPort);
//    if(fpgaControls)
//        fpgaControls->Initialize(streamBoardPort);
    if(myriad7)
        myriad7->Initialize(lms7controlPort);
    if(deviceInfo)
        deviceInfo->Initialize(lms7controlPort, streamBoardPort);
    if(spi)
        spi->Initialize(lms7controlPort, streamBoardPort);
    if(novenaGui)
        novenaGui->Initialize(lms7controlPort);
//    if(boardControlsGui)
//        boardControlsGui->Initialize(lms7controlPort);
}
