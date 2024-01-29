#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif //WX_PRECOMP
#include <wx/msgdlg.h>

#include "lms7002_mainPanel.h"

#include "lms7suiteAppFrame.h"
#include "dlgAbout.h"
#include "lms7suiteEvents.h"
#include "fftviewer_frFFTviewer.h"
#include "ADF4002/ADF4002_wxgui.h"
#include "Si5351C/Si5351C_wxgui.h"
#include "CDCM6208/CDCM6208_panelgui.h"
#include "LMS_Programing/LMS_Programing_wxgui.h"
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
#include "SPI_wxgui.h"
#include "GUI/events.h"
#include "GUI/ISOCPanel.h"
#include "SDRConfiguration_view.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"
#include "limesuite/DeviceNode.h"
//#include "LimeSDR.h"

#include "Logger.h"

using namespace std;
using namespace lime;
using namespace std::literals::string_literals;

static constexpr int controlColumn = 1;

LMS7SuiteAppFrame* LMS7SuiteAppFrame::obj_ptr = nullptr;

int LMS7SuiteAppFrame::m_lmsSelection = 0;

void LMS7SuiteAppFrame::OnGlobalLogEvent(const lime::LogLevel level, const char* message)
{
    if (obj_ptr == nullptr || obj_ptr->mMiniLog == nullptr)
        return;
    wxCommandEvent evt;
    evt.SetString(wxString::FromAscii(message));
    evt.SetEventType(LOG_MESSAGE);
    evt.SetInt(int(level));
    wxPostEvent(obj_ptr, evt);
}

struct DeviceTreeItemData : public wxTreeItemData {
    DeviceTreeItemData(const std::shared_ptr<DeviceNode> soc)
        : wxTreeItemData()
        , gui(nullptr)
        , soc(soc)
    {
    }

    ~DeviceTreeItemData()
    {
        if (gui)
            gui->Destroy();
    }
    ISOCPanel* gui;
    const std::shared_ptr<DeviceNode> soc;
};

LMS7SuiteAppFrame::LMS7SuiteAppFrame(wxWindow* parent)
    : wxFrame(parent, wxNewId(), _("LimeSuite"))
    , lmsControl(nullptr)
{
    obj_ptr = this;

    mbar = new wxMenuBar(0);
    fileMenu = new wxMenu();
    wxMenuItem* menuFileQuit = new wxMenuItem(
        fileMenu, idMenuQuit, wxString(wxT("&Quit")) + wxT('\t') + wxT("Alt+F4"), wxT("Quit the application"), wxITEM_NORMAL);
    fileMenu->Append(menuFileQuit);
    Connect(menuFileQuit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnQuit));
    mbar->Append(fileMenu, wxT("&File"));

    mnuModules = new wxMenu();
    mbar->Append(mnuModules, wxT("Modules"));

    helpMenu = new wxMenu();
    wxMenuItem* menuHelpAbout = new wxMenuItem(helpMenu,
        idMenuAbout,
        wxString(wxT("&About")) + wxT('\t') + wxT("F1"),
        wxT("Show info about this application"),
        wxITEM_NORMAL);
    helpMenu->Append(menuHelpAbout);
    Connect(menuHelpAbout->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnAbout));

    mbar->Append(helpMenu, wxT("&Help"));
    SetMenuBar(mbar);

    statusBar = CreateStatusBar(3, wxSTB_DEFAULT_STYLE, wxNewId());
    const int mainColumns = 1;
    mainSizer = new wxFlexGridSizer(mainColumns, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(1);
    mainSizer->SetFlexibleDirection(wxBOTH);
    mainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    pnlDeviceConnection = new DeviceConnectionPanel(this, wxNewId());
    mainSizer->Add(pnlDeviceConnection, 0, wxEXPAND, 0);

    m_scrolledWindow1 = new wxScrolledWindow(this, wxNewId(), wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    m_scrolledWindow1->SetScrollRate(5, 5);
    contentSizer = new wxFlexGridSizer(0, 2, 0, 0);
    contentSizer->AddGrowableCol(1);
    contentSizer->AddGrowableRow(0);
    contentSizer->SetFlexibleDirection(wxBOTH);
    contentSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    mContent = nullptr; //new lms7002_mainPanel(m_scrolledWindow1, wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    //contentSizer->Add(mContent, 0, 0, 5);
    deviceTree = new wxTreeCtrl(
        m_scrolledWindow1, wxNewId(), wxPoint(0, 0), wxSize(200, 385), wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_ROW_LINES);
    contentSizer->Add(deviceTree, 0, wxEXPAND, 0);

    m_scrolledWindow1->SetSizerAndFit(contentSizer);
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

    SPI_wxgui* spigui = new SPI_wxgui(this, wxNewId());
    AddModule(spigui, "SPI");

    boardControlsGui = new pnlBoardControls(this, wxNewId());
    AddModule(boardControlsGui, "Board controls");

    programmer = new LMS_Programing_wxgui(this, wxNewId());
    AddModule(programmer, "Programming");

    int x, y1, y2;
    m_scrolledWindow1->GetVirtualSize(&x, &y1);
    mMiniLog->GetSize(nullptr, &y2);

    wxCommandEvent event;
    //OnControlBoardConnect(event);

    //UpdateConnections(lmsControl);

    // mnuCacheValues->Check(false);
    const int statusWidths[] = { -1, -3, -3 };
    statusBar->SetStatusWidths(3, statusWidths);
    Bind(limeEVT_SDR_HANDLE_SELECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDeviceHandleChange), this);
    Connect(LOG_MESSAGE, wxCommandEventHandler(LMS7SuiteAppFrame::OnLogMessage), 0, this);
    lime::registerLogHandler(&LMS7SuiteAppFrame::OnGlobalLogEvent);

    deviceTree->Bind(
        wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler(LMS7SuiteAppFrame::DeviceTreeSelectionChanged), this, deviceTree->GetId());
}

LMS7SuiteAppFrame::~LMS7SuiteAppFrame()
{
    for (auto iter : mModules)
        iter.second->Destroy();

    OnDeviceDisconnect();

    Disconnect(idMenuQuit, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnQuit));
    Disconnect(idMenuAbout, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnAbout));
    Disconnect(LOG_MESSAGE, wxCommandEventHandler(LMS7SuiteAppFrame::OnLogMessage), 0, this);
}

void LMS7SuiteAppFrame::OnClose(wxCloseEvent& event)
{
    Destroy();
}

void LMS7SuiteAppFrame::OnQuit(wxCommandEvent& event)
{
    Destroy();
}

void LMS7SuiteAppFrame::OnAbout(wxCommandEvent& event)
{
    dlgAbout dlg(this);
    dlg.ShowModal();
}

void LMS7SuiteAppFrame::UpdateConnections(SDRDevice* device)
{
    for (auto iter : mModules)
    {
        iter.second->Initialize(device);
    }
}

void LMS7SuiteAppFrame::OnDeviceDisconnect()
{
    if (fftviewer)
        fftviewer->StopStreaming();
    if (lmsControl)
    {
        const SDRDevice::Descriptor& info = lmsControl->GetDescriptor();
        statusBar->SetStatusText(_("Control port: Not Connected"), controlColumn);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetInt(static_cast<int>(lime::LogLevel::INFO));
        evt.SetString("Disconnected: " + info.name);
        wxPostEvent(this, evt);
        UpdateConnections(nullptr);
        lime::DeviceRegistry::freeDevice(lmsControl);
        lmsControl = nullptr;
    }

    deviceTree->DeleteAllItems();
    mContent = nullptr;
}

void CreateBranch(wxTreeCtrl* treeRoot, wxTreeItemId parentId, const std::shared_ptr<DeviceNode> node)
{
    wxTreeItemId branchId = treeRoot->AppendItem(parentId, node->name, 0, 0, new DeviceTreeItemData(node));
    if (node->children.size() == 0)
        return;

    for (const auto& soc : node->children)
        CreateBranch(treeRoot, branchId, soc);
}

void FillDeviceTree(wxTreeCtrl* root, lime::SDRDevice* device, wxWindow* parentWindow)
{
    if (!root || !device)
        return;

    SDRConfiguration_view* sdrUI = new SDRConfiguration_view(parentWindow, wxNewId());
    sdrUI->Setup(device);
    sdrUI->Hide();

    std::shared_ptr<DeviceNode> node = device->GetDescriptor().socTree;
    DeviceTreeItemData* treeRootData = new DeviceTreeItemData(node);
    treeRootData->gui = sdrUI;
    wxTreeItemId rootId = root->AddRoot(node->name, 0, 0, treeRootData);

    for (const auto& soc : node->children)
        CreateBranch(root, rootId, soc);
    root->ExpandAll();
    root->SelectItem(rootId, true);
}

void LMS7SuiteAppFrame::OnDeviceHandleChange(wxCommandEvent& event)
{
    OnDeviceDisconnect();
    // event.GetString() is the target device handle text
    try
    {
        if (event.GetString().length() == 0)
            return;

        DeviceHandle handle(event.GetString().ToStdString());
        lmsControl = lime::DeviceRegistry::makeDevice(handle);

        if (!lmsControl)
        {
            wxMessageBox("Failed to connect to: " + event.GetString(), wxT("Connection error"), wxICON_ERROR);
            return;
        }

        //bind callback for spi data logging
        const SDRDevice::Descriptor& info = lmsControl->GetDescriptor();
        lmsControl->SetDataLogCallback(&LMS7SuiteAppFrame::OnLogDataTransfer);
        wxString controlDev = _("Device: ");
        controlDev.Append(handle.ToString());
        double refClk = lmsControl->GetClockFreq(LMS_CLOCK_REF,
            0); // use reference clock of the 0th channel
        controlDev.Append(wxString::Format(_(" FW:%s HW:%s Protocol:%s GW:%s.%s Ref Clk: %1.2f MHz"),
            info.firmwareVersion,
            info.hardwareVersion,
            info.protocolVersion,
            info.gatewareVersion,
            info.gatewareRevision,
            refClk / 1e6));
        statusBar->SetStatusText(controlDev, controlColumn);

        FillDeviceTree(deviceTree, lmsControl, m_scrolledWindow1);
        wxTreeEvent eee(wxEVT_TREE_SEL_CHANGED, deviceTree, deviceTree->GetFocusedItem());
        DeviceTreeSelectionChanged(eee);

        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetInt(static_cast<int>(lime::LogLevel::INFO));
        evt.SetString(_("Connected ") + controlDev);
        wxPostEvent(this, evt);
        UpdateConnections(lmsControl);

        Fit();
    } catch (std::runtime_error& e)
    {
        lime::error("Failed to connect "s + e.what());
    }
}

void LMS7SuiteAppFrame::OnLogMessage(wxCommandEvent& event)
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
    for (size_t i = 0; i < length - repeatedZeros; ++i)
        //casting to short to print as numbers
        ss << " " << std::setw(2) << (unsigned short)data[i];
    if (repeatedZeros > 2)
        ss << " (00 x " << std::dec << repeatedZeros << " times)";
    lime::debug(ss.str());
    wxCommandEvent* evt = new wxCommandEvent();
    evt->SetString(ss.str());
    evt->SetEventObject(obj_ptr);
    evt->SetEventType(LOG_MESSAGE);
    evt->SetInt(static_cast<int>(lime::LogLevel::INFO));
    wxQueueEvent(obj_ptr, evt);
}

void LMS7SuiteAppFrame::AddModule(IModuleFrame* module, const std::string& title)
{
    wxWindowID moduleId = module->GetId();
    lime::debug("Add module %i", moduleId);
    wxMenuItem* item;
    item = new wxMenuItem(mnuModules, moduleId, title, wxEmptyString, wxITEM_NORMAL);
    mnuModules->Append(item);

    mModules[moduleId] = module;
    module->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnModuleClose), NULL, this);
    Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnShowModule));
}

void LMS7SuiteAppFrame::RemoveModule(IModuleFrame* module)
{
}

void LMS7SuiteAppFrame::OnModuleClose(wxCloseEvent& event)
{
    lime::debug("Close module %i", event.GetId());
    IModuleFrame* module = mModules.at(event.GetId());
    if (module)
    {
        module->Show(false);
    }
}

void LMS7SuiteAppFrame::OnShowModule(wxCommandEvent& event)
{
    lime::debug("show module %i", event.GetId());
    IModuleFrame* module = mModules.at(event.GetId());
    if (module) //it's already opened
    {
        module->Initialize(lmsControl);
        module->Show(true);
        module->Iconize(false); // restore the window if minimized
        module->SetFocus(); // focus on my window
        module->Raise(); // bring window to front
    }
}

ISOCPanel* CreateGUI(wxWindow* parent, eDeviceNodeClass deviceNodeClass, void* socPtr)
{
    switch (deviceNodeClass)
    {
    case eDeviceNodeClass::ADF4002: {
        ADF4002_wxgui* adfPanel = new ADF4002_wxgui(parent, wxNewId());
        adfPanel->Initialize(reinterpret_cast<lime::ADF4002*>(socPtr));
        return adfPanel;
    }
    case eDeviceNodeClass::CDCM6208: {
        CDCM6208_panelgui* cdcmPanel = new CDCM6208_panelgui(parent, wxNewId());
        cdcmPanel->Initialize(reinterpret_cast<CDCM_Dev*>(socPtr));
        return cdcmPanel;
    }
    case eDeviceNodeClass::LMS7002M: {
        lms7002_mainPanel* lmsPanel = new lms7002_mainPanel(parent, wxNewId());
        lmsPanel->Initialize(reinterpret_cast<LMS7002M*>(socPtr));
        return lmsPanel;
    }
    case eDeviceNodeClass::SDRDevice: {
        SDRConfiguration_view* sdrPanel = new SDRConfiguration_view(parent, wxNewId());
        sdrPanel->Setup(reinterpret_cast<SDRDevice*>(socPtr));
        sdrPanel->Hide();
        return sdrPanel;
    }
    default:
        lime::warning("Unrecognized device class(%u)", static_cast<uint8_t>(deviceNodeClass));
        return nullptr;
    }
}

void LMS7SuiteAppFrame::DeviceTreeSelectionChanged(wxTreeEvent& event)
{
    DeviceTreeItemData* item = reinterpret_cast<DeviceTreeItemData*>(deviceTree->GetItemData(event.GetItem()));
    if (item->gui == nullptr)
        item->gui = CreateGUI(m_scrolledWindow1, item->soc->deviceNodeClass, item->soc->ptr);

    if (mContent && mContent != item->gui)
    {
        contentSizer->Remove(1);
        mContent->Hide();
        mContent = nullptr;
    }

    mContent = item->gui;
    if (!mContent)
        return;
    mContent->Show();
    contentSizer->Add(mContent, 0, 0, 5);
    contentSizer->Layout();
    m_scrolledWindow1->SetSizerAndFit(contentSizer);
    Layout();
    Fit();
}
