/**
@file   LMS_Programing_wxgui.cpp
@author Lime Microsystems
@brief  panel for uploading data to FPGA
*/
#include "LMS_Programing_wxgui.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/gauge.h>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>

const long LMS_Programing_wxgui::ID_PROGRAMING_FINISHED_EVENT = wxNewId();
const long LMS_Programing_wxgui::ID_PROGRAMING_STATUS_EVENT = wxNewId();
const long LMS_Programing_wxgui::ID_BUTTON1 = wxNewId();
const long LMS_Programing_wxgui::ID_BUTTON2 = wxNewId();
const long LMS_Programing_wxgui::ID_GAUGE1 = wxNewId();
const long LMS_Programing_wxgui::ID_CHOICE2 = wxNewId();
const long LMS_Programing_wxgui::ID_CHOICE1 = wxNewId();

BEGIN_EVENT_TABLE(LMS_Programing_wxgui, wxFrame)
END_EVENT_TABLE()

LMS_Programing_wxgui::LMS_Programing_wxgui(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, int styles, wxString idname)
{
    mProgrammingInProgress.store(false);
    mAbortProgramming.store(false);
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;

    wxFrame::Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, _T("id"));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 5, 5);
    btnOpen = new wxButton(this, ID_BUTTON1, _T("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(btnOpen, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, wxID_ANY, _T("File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblFilename = new wxStaticText(this, wxID_ANY, _T("\?"), wxDefaultPosition, wxSize(400, -1), wxST_ELLIPSIZE_START, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(lblFilename, 1, wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer6, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    btnStartStop = new wxButton(this, ID_BUTTON2, _T("Program"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(btnStartStop, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer8->AddGrowableCol(0);
    lblProgressPercent = new wxStaticText(this, wxID_ANY, _T(""), wxDefaultPosition, wxSize(48, -1), 0, _T("ID_STATICTEXT5"));

    progressBar = new wxGauge(this, ID_GAUGE1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GAUGE1"));
    FlexGridSizer8->Add(progressBar, 1, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer8->AddGrowableRow(0);
    FlexGridSizer2->Add(FlexGridSizer8, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer2->Add(lblProgressPercent, 1, wxEXPAND | wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 5);
    StaticText2 = new wxStaticText(this, wxID_ANY, _T("Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer7->Add(StaticText2, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    StaticText3 = new wxStaticText(this, wxID_ANY, _T("Programming mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer7->Add(StaticText3, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    cmbDevice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    cmbDevice->Append(_T("HPM1000/HMP7"));
    cmbDevice->Append(_T("FX3"));
    cmbDevice->SetSelection(cmbDevice->Append(_T("Altera FPGA")));
    FlexGridSizer7->Add(cmbDevice, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    cmbProgMode = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxSize(176, -1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    cmbProgMode->SetSelection(cmbProgMode->Append(_T("Flash")));
    cmbProgMode->Append(_T("1"));
    cmbProgMode->Append(_T("2"));
    cmbProgMode->Append(_T("3"));
    cmbProgMode->Append(_T("4"));
    cmbProgMode->Append(_T("5"));
    cmbProgMode->Append(_T("6"));
    cmbProgMode->Append(_T("7"));
    cmbProgMode->Append(_T("8"));
    FlexGridSizer7->Add(cmbProgMode, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer3->Add(FlexGridSizer7, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnOpenClick);
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnStartProgrammingClick);
    Connect(ID_CHOICE2, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&LMS_Programing_wxgui::OncmbDeviceSelect);
    Connect(ID_PROGRAMING_FINISHED_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&LMS_Programing_wxgui::OnProgramingFinished);
    Connect(ID_PROGRAMING_STATUS_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&LMS_Programing_wxgui::OnProgramingStatusUpdate);

    wxCommandEvent evt;
    OncmbDeviceSelect(evt);
}

LMS_Programing_wxgui::~LMS_Programing_wxgui()
{
    //make sure the thread has stopped before destroying data
    if(mProgrammingInProgress.load() == true)
    {
        mAbortProgramming.store(true);
        mWorkerThread.join();
    }
}

void LMS_Programing_wxgui::OnbtnOpenClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Select file"), _(""), _(""), _("rbf(*.rbf)|*.rbf|bin(*.bin)|*.bin|rpd(*.rpd)|*.rpd|img(*.img)|*.img|All files(*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    lblFilename->SetLabel(dlg.GetPath());
}

void LMS_Programing_wxgui::OnbtnStartProgrammingClick(wxCommandEvent& event)
{
    //if needed load program data from file
    if( (cmbDevice->GetSelection() == 2 && cmbProgMode->GetSelection() == 2) == false)
    {
        if (lblFilename->GetLabel().length() <= 1)
        {
            wxMessageBox(_("Program file not selected"), _("Warning"));
            return;
        }

        //using wxWidgets to read file, to support nonascii characters in path
        wxFFileInputStream fin(lblFilename->GetLabel());

        if(!fin.IsOk())
        {
            wxMessageBox(_("Error loading program file"), _("Error"));
            return;
        }

        fin.SeekI(0, wxFromEnd);
        unsigned int m_data_size = fin.TellI();

        mProgramData.resize(m_data_size, 0);
        fin.SeekI(0, wxFromStart);
        fin.Read(mProgramData.data(), m_data_size);
    }

    Disconnect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnStartProgrammingClick);
    btnOpen->Disable();
    btnStartStop->SetLabel(_("Abort"));

    mAbortProgramming.store(false);
    //run programming in separate thread, to prevent GUI freeze
    mWorkerThread = std::thread(&LMS_Programing_wxgui::DoProgramming, this);
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnAbortProgramming);
}

/** @brief Change programming modes according to selected device
*/
void LMS_Programing_wxgui::OncmbDeviceSelect(wxCommandEvent& event)
{
    int deviceSelection = cmbDevice->GetSelection();
    cmbProgMode->Clear();
    if(deviceSelection == 2)
    {
        cmbProgMode->Append("Bitstream to FPGA");
        cmbProgMode->Append("Bitstream to Flash");
        cmbProgMode->Append("Bitstream from Flash");
        cmbProgMode->SetSelection(0);
    }
    else if(deviceSelection == 1)
    {
		cmbProgMode->Append(_("Firmware to RAM"));
        cmbProgMode->Append(_("Firmware to Flash"));
        cmbProgMode->SetSelection(0);
    }
    else if(deviceSelection == 0)
    {
        cmbProgMode->Append(_("Flash"));
        for(int i=1; i<=8; ++i)
            cmbProgMode->Append(wxString::Format("%i", i));
        cmbProgMode->SetSelection(0);
    }
}

void LMS_Programing_wxgui::OnProgramingFinished(wxCommandEvent& event)
{
    mWorkerThread.join();
    wxMessageBox(event.GetString(), _("INFO"), wxICON_INFORMATION | wxOK);
    btnOpen->Enable();
    Disconnect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnAbortProgramming);
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnStartProgrammingClick);
    btnStartStop->SetLabel(_("Program"));
}

void LMS_Programing_wxgui::OnAbortProgramming(wxCommandEvent& event)
{
    mAbortProgramming.store(true);
}

void LMS_Programing_wxgui::SetConnection(lms_device_t* port)
{
    lmsControl = port;
}

LMS_Programing_wxgui* LMS_Programing_wxgui::obj_ptr=nullptr;
bool LMS_Programing_wxgui::OnProgrammingCallback(int bsent, int btotal, const char* progressMsg)
{
    wxCommandEvent evt;
    evt.SetEventObject(obj_ptr);
    evt.SetInt(100.0 * bsent / btotal); //round to int
    evt.SetString(wxString::From8BitData(progressMsg));
    evt.SetEventType(wxEVT_COMMAND_THREAD);
    evt.SetId(ID_PROGRAMING_STATUS_EVENT);
    wxPostEvent(obj_ptr, evt);
    return obj_ptr->mAbortProgramming.load();
}


void LMS_Programing_wxgui::DoProgramming()
{
    mProgrammingInProgress.store(true);
    obj_ptr = this;
    int device = cmbDevice->GetSelection();
    int progMode = cmbProgMode->GetSelection();
    int status = -1;
    if (device == 1)
    {
        status = LMS_ProgramFirmware(lmsControl, mProgramData.data(), mProgramData.size(), (lms_target_t)progMode,OnProgrammingCallback);
    }
    else if (device == 2)
    {

       status = LMS_ProgramFPGA(lmsControl, mProgramData.data(), mProgramData.size(), (lms_target_t)progMode,OnProgrammingCallback);
    }
    else if (device == 0)
    {
       status = LMS_ProgramHPM7(lmsControl, mProgramData.data(), mProgramData.size(), progMode,OnProgrammingCallback);
    }
    wxCommandEvent evt;
    evt.SetEventObject(this);
    evt.SetId(ID_PROGRAMING_FINISHED_EVENT);
    evt.SetEventType(wxEVT_COMMAND_THREAD);

    if(status == 0)
        evt.SetString(_("Programming Completed!"));
    else
    {
        evt.SetString(_("Programming failed!"));
        wxPostEvent(this, evt);
        mProgrammingInProgress.store(false);
        return;
    }

    //inform user about device reset
    if(device == 1)
    {
        if (progMode == 1) //reset FX3 only after programming flash
            status = LMS_ProgramFirmware(lmsControl, nullptr, 0, LMS_TARGET_BOOT,OnProgrammingCallback);
        if(status == 0)
            evt.SetString("FX3 firmware uploaded, device is going to be reset, please reconnect in connection settings");
    }
    else if(device == 2 && progMode == 1) //reset FPGA and FX3 after FPGA programming
    {
        status = LMS_ProgramFPGA(lmsControl, nullptr, 0, LMS_TARGET_BOOT,OnProgrammingCallback);
        status = LMS_ProgramFirmware(lmsControl, nullptr, 0, LMS_TARGET_BOOT,OnProgrammingCallback);
        if(status == 0)
            evt.SetString("FPGA gateware uploaded, device is going to be reset, please reconnect in connection settings");
    }
    wxPostEvent(this, evt);
    mProgrammingInProgress.store(false);
}

/** Updates GUI elements with programming status
*/
void LMS_Programing_wxgui::OnProgramingStatusUpdate(wxCommandEvent& event)
{
    progressBar->SetValue(event.GetInt());
    lblProgressPercent->SetLabel(event.GetString());
}
