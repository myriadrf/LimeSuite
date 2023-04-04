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
#include "ADF4002/ADF4002_wxgui.h"
#include "Si5351C/Si5351C_wxgui.h"
//#include "LMS_Programing_wxgui.h"
#include "pnlMiniLog.h"
#include "FPGAcontrols_wxgui.h"
#include "SPI_wxgui.h"
#include <wx/string.h>
#include "dlgDeviceInfo.h"
#include <functional>
#include "lms7002_pnlTRF_view.h"
#include "lms7002_pnlRFE_view.h"
#include "lms7002_pnlLimeLightPAD_view.h"
#include "pnlBoardControls.h"
#include "LMSBoards.h"
#include <sstream>
#include "pnlAPI.h"
//#include "limeRFE_wxgui.h"
#include "SPI_wxgui.h"

#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"
//#include "LimeSDR.h"
using namespace std;
using namespace lime;

///////////////////////////////////////////////////////////////////////////

LMS7SuiteAppFrame* LMS7SuiteAppFrame::obj_ptr=nullptr;

int LMS7SuiteAppFrame::m_lmsSelection = 0;

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

static void EnumerateDevicesToChoice(wxChoice *choise)
{
    choise->Clear();
    try {
        std::vector<lime::DeviceHandle> handles;
        handles = lime::DeviceRegistry::enumerate();

        for (size_t i = 0; i < handles.size(); i++)
            choise->Append(handles[i].serialize().c_str());
    }
    catch (std::runtime_error &e) {
    }
}

LMS7SuiteAppFrame::LMS7SuiteAppFrame(wxWindow *parent)
    : wxFrame(parent, wxNewId(), _("LimeSuite")), lmsControl(nullptr)
{
    obj_ptr = this;

    mbar = new wxMenuBar(0);
    fileMenu = new wxMenu();
    wxMenuItem *menuFileQuit =
        new wxMenuItem(fileMenu, idMenuQuit, wxString(wxT("&Quit")) + wxT('\t') + wxT("Alt+F4"),
                       wxT("Quit the application"), wxITEM_NORMAL);
    fileMenu->Append(menuFileQuit);

    mnuModules = new wxMenu();
    mbar->Append(mnuModules, wxT("Modules"));

    helpMenu = new wxMenu();
    wxMenuItem *menuHelpAbout =
        new wxMenuItem(helpMenu, idMenuAbout, wxString(wxT("&About")) + wxT('\t') + wxT("F1"),
                       wxT("Show info about this application"), wxITEM_NORMAL);
    helpMenu->Append(menuHelpAbout);

    mbar->Append(helpMenu, wxT("&Help"));
    this->SetMenuBar(mbar);

    statusBar = this->CreateStatusBar(3, wxSTB_DEFAULT_STYLE, wxNewId());
    const int mainCollumns = 1;
    mainSizer = new wxFlexGridSizer(mainCollumns, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(1);
    mainSizer->SetFlexibleDirection(wxBOTH);
    mainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    { // Device connection selection
        wxPanel *container = new wxPanel(this, wxID_ANY);
        wxBoxSizer *szBox = new wxBoxSizer(wxHORIZONTAL);

        mDeviceChoice = new wxChoice(container, wxNewId());
        mDeviceChoice->Append(_("No devices connected"));
        mDeviceChoice->SetSelection(0);
        szBox->Add(new wxStaticText(container, wxID_ANY, _("Device:")), 0, wxALIGN_CENTER_VERTICAL,
                   0);
        szBox->Add(mDeviceChoice, 0, wxEXPAND, 0);
        // TODO: add disconnect button
        container->SetSizerAndFit(szBox);
        mainSizer->Add(container, 0, wxEXPAND, 0);
    }

    m_scrolledWindow1 = new wxScrolledWindow(this, wxNewId(), wxDefaultPosition, wxDefaultSize,
                                             wxHSCROLL | wxVSCROLL);
    m_scrolledWindow1->SetScrollRate(5, 5);
    contentSizer = new wxFlexGridSizer(0, 1, 0, 0);
    contentSizer->AddGrowableCol(0);
    contentSizer->AddGrowableRow(0);
    contentSizer->SetFlexibleDirection(wxBOTH);
    contentSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    mContent = new lms7002_mainPanel(m_scrolledWindow1, wxNewId(), wxDefaultPosition, wxDefaultSize,
                                     wxTAB_TRAVERSAL);
    contentSizer->Add(mContent, 0, 0, 5);

    m_scrolledWindow1->SetSizerAndFit(contentSizer);
    //m_scrolledWindow1->Layout();
    mainSizer->Add(m_scrolledWindow1, 4, wxEXPAND, 5);

    mMiniLog = new pnlMiniLog(this, wxNewId());
    mainSizer->Add(mMiniLog, 1, wxEXPAND, 5);

    //SetSizer( mainSizer );
    SetSizerAndFit(mainSizer);
    // Layout();
    // Fit();

#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif

    fftviewer = new fftviewer_frFFTviewer(this, wxNewId());
    AddModule(fftviewer, "fftviewer");

    SPI_wxgui *spigui = new SPI_wxgui(this, wxNewId());
    AddModule(spigui, "SPI");

    boardControlsGui = new pnlBoardControls(this, wxNewId());
    AddModule(boardControlsGui, "Board controls");

    //Connect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    int x,y1,y2;
    m_scrolledWindow1->GetVirtualSize(&x,&y1);
    mMiniLog->GetSize(nullptr,&y2);

    wxCommandEvent event;
    //OnControlBoardConnect(event);

    //UpdateConnections(lmsControl);

    // mnuCacheValues->Check(false);
    const int statusWidths[] = {-1, -3, -3};
    statusBar->SetStatusWidths(3, statusWidths);
    //Bind(LMS_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::OnLmsChanged), this);

    Bind(wxEVT_CHOICE, wxCommandEventHandler(LMS7SuiteAppFrame::OnDeviceConnect), this,
         mDeviceChoice->GetId());
    // #ifndef LIMERFE
    // 	mnuModules->Delete(ID_MENUITEM_LIMERFE);
    // #endif
    Connect(LOG_MESSAGE, wxCommandEventHandler(LMS7SuiteAppFrame::OnLogMessage), 0, this);
    lime::registerLogHandler(&LMS7SuiteAppFrame::OnGlobalLogEvent);

    EnumerateDevicesToChoice(mDeviceChoice);
}

LMS7SuiteAppFrame::~LMS7SuiteAppFrame()
{
    wxCloseEvent evt;
    for (auto iter : mModules)
        iter.second->Destroy();
    //Disconnect(CGEN_FREQUENCY_CHANGED, wxCommandEventHandler(LMS7SuiteAppFrame::HandleLMSevent), NULL, this);
    if (lmsControl)
        DeviceRegistry::freeDevice(lmsControl);
}

void LMS7SuiteAppFrame::OnClose( wxCloseEvent& event )
{
    Destroy();
}

void LMS7SuiteAppFrame::OnQuit( wxCommandEvent& event )
{
    Destroy();
}

void LMS7SuiteAppFrame::OnAbout( wxCommandEvent& event )
{
	dlgAbout dlg(this);
    dlg.ShowModal();
}

void LMS7SuiteAppFrame::UpdateConnections(SDRDevice *device)
{
    mContent->Initialize(lmsControl);
    for (auto iter : mModules) {
        iter.second->Initialize(device);
    }
}

void LMS7SuiteAppFrame::OnDeviceConnect(wxCommandEvent &event)
{
    DeviceHandle handle(event.GetString().ToStdString());

    try {
        if (fftviewer)
            fftviewer->StopStreaming();
        if (lmsControl)
            lime::DeviceRegistry::freeDevice(lmsControl);

        lmsControl = lime::DeviceRegistry::makeDevice(handle);

        if (lmsControl)
        {
            //bind callback for spi data logging
            const SDRDevice::Descriptor &info = lmsControl->GetDescriptor();
            lmsControl->SetDataLogCallback(&LMS7SuiteAppFrame::OnLogDataTransfer);
            wxString controlDev = _("Device: ");
            controlDev.Append(info.name);
            double refClk = lmsControl->GetClockFreq(LMS_CLOCK_REF,
                                                     0); // use reference clock of the 0th channel
            controlDev.Append(wxString::Format(
                _(" FW:%s HW:%s Protocol:%s GW:%s Ref Clk: %1.2f MHz"), info.firmwareVersion,
                info.hardwareVersion, info.protocolVersion, info.gatewareVersion, refClk / 1e6));
            const int controlCollumn = 1;
            statusBar->SetStatusText(controlDev, controlCollumn);

            //LMS_EnableCache(lmsControl, mnuCacheValues->IsChecked());

            wxCommandEvent evt;
            evt.SetEventType(LOG_MESSAGE);
            evt.SetInt(lime::LOG_LEVEL_INFO);
            evt.SetString(_("Connected ") + controlDev);
            wxPostEvent(this, evt);
            // if (si5351gui)
            //     si5351gui->ModifyClocksGUI(info->deviceName);
            // if (boardControlsGui)
            //     boardControlsGui->SetupControls(info->deviceName);
            UpdateConnections(lmsControl);
        }
    }
    catch (std::runtime_error &e) {
        printf("Failed to connect %s\n", e.what());
    }
    /*else // TODO:
    {
        statusBar->SetStatusText(_("Control port: Not Connected"), controlCollumn);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetInt(lime::LOG_LEVEL_INFO);
        evt.SetString(_("Disconnected control port"));
        wxPostEvent(this, evt);
    }*/
}

// void LMS7SuiteAppFrame::OnLmsChanged(wxCommandEvent& event)
// {
//     m_lmsSelection = event.GetInt();
// }

void LMS7SuiteAppFrame::OnLogMessage(wxCommandEvent &event)
{
    if (mMiniLog)
        mMiniLog->HandleMessage(event);
}

#include <iomanip>
void LMS7SuiteAppFrame::OnLogDataTransfer(bool Tx, const uint8_t* data, const uint32_t length)
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
    evt->SetInt(lime::LOG_LEVEL_INFO);
    wxQueueEvent(obj_ptr, evt);
}

void LMS7SuiteAppFrame::AddModule(IModuleFrame *module, const char *title)
{
    wxWindowID moduleId = module->GetId();
    printf("Add module %i\n", moduleId);
    wxMenuItem *item;
    item = new wxMenuItem(mnuModules, moduleId, wxString(title), wxEmptyString, wxITEM_NORMAL);
    mnuModules->Append(item);

    mModules[moduleId] = module;
    module->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnModuleClose),
                       NULL, this);
    this->Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED,
                  wxCommandEventHandler(LMS7SuiteAppFrame::OnShowModule));
}

void LMS7SuiteAppFrame::RemoveModule(IModuleFrame *module) {}

void LMS7SuiteAppFrame::OnModuleClose(wxCloseEvent &event)
{
    printf("Close module %i\n", event.GetId());
    IModuleFrame *module = mModules.at(event.GetId());
    if (module) {
        module->Show(false);
    }
}

void LMS7SuiteAppFrame::OnShowModule(wxCommandEvent &event)
{
    printf("show module %i\n", event.GetId());
    IModuleFrame *module = mModules.at(event.GetId());
    if (module) //it's already opened
    {
        module->Initialize(lmsControl);
        module->Show(true);
        module->Iconize(false); // restore the window if minimized
        module->SetFocus();     // focus on my window
        module->Raise();        // bring window to front
    }
}
