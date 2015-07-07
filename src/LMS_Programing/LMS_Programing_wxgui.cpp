/**
@file 	LMS_Programing_wxgui.cpp
@author	Lime Microsystems
@brief	panel for uploading data to FPGA
*/
#include "LMS_Programing_wxgui.h"
#include "lmsComms.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/gauge.h>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include "LMS_Programing.h"

const long LMS_Programing_wxgui::ID_BUTTON1 = wxNewId();
const long LMS_Programing_wxgui::ID_STATICTEXT1 = wxNewId();
const long LMS_Programing_wxgui::ID_STATICTEXT2 = wxNewId();
const long LMS_Programing_wxgui::ID_BUTTON2 = wxNewId();
const long LMS_Programing_wxgui::ID_STATICTEXT5 = wxNewId();
const long LMS_Programing_wxgui::ID_GAUGE1 = wxNewId();
const long LMS_Programing_wxgui::ID_STATICTEXT3 = wxNewId();
const long LMS_Programing_wxgui::ID_STATICTEXT6 = wxNewId();
const long LMS_Programing_wxgui::ID_CHOICE2 = wxNewId();
const long LMS_Programing_wxgui::ID_CHOICE1 = wxNewId();

BEGIN_EVENT_TABLE(LMS_Programing_wxgui, wxFrame)	
END_EVENT_TABLE()

LMS_Programing_wxgui::LMS_Programing_wxgui(LMScomms* serPort, wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, int styles, wxString idname)    
{
    progressPooler = new wxTimer(this, wxNewId());
    Connect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(LMS_Programing_wxgui::OnProgressPoll), NULL, this);    
    m_programmer = new LMS_Programing(serPort);
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;

    wxFrame::Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, _T("id"));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    btnOpen = new wxButton(this, ID_BUTTON1, _T("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(btnOpen, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _T("File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblFilename = new wxStaticText(this, ID_STATICTEXT2, _T("\?"), wxDefaultPosition, wxSize(400, -1), 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(lblFilename, 1, wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer6, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    btnStartStop = new wxButton(this, ID_BUTTON2, _T("Program"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(btnStartStop, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);        
    FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    lblProgressPercent = new wxStaticText(this, ID_STATICTEXT5, _T("0 %"), wxDefaultPosition, wxSize(48, -1), 0, _T("ID_STATICTEXT5"));
    FlexGridSizer8->Add(lblProgressPercent, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    progressBar = new wxGauge(this, ID_GAUGE1, 100, wxDefaultPosition, wxSize(-1, 21), 0, wxDefaultValidator, _T("ID_GAUGE1"));
    FlexGridSizer8->Add(progressBar, 1, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer8, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT3, _T("Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer7->Add(StaticText2, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT6, _T("Programming mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer7->Add(StaticText3, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    cmbDevice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    cmbDevice->Append(_T("HPM1000/HMP7"));
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
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnProgMyriadClick);    
    Connect(ID_CHOICE2, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&LMS_Programing_wxgui::OncmbDeviceSelect);
    
    wxCommandEvent evt;
    OncmbDeviceSelect(evt);
}

LMS_Programing_wxgui::~LMS_Programing_wxgui()
{   
    Disconnect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(LMS_Programing_wxgui::OnProgressPoll), NULL, this);
    progressPooler->Stop();
    m_programmer->AbortPrograming();
}


void LMS_Programing_wxgui::OnbtnOpenClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Select file"), "", "", "rbf (*.rbf)| *.rbf|bin (*.bin)| *.bin|All files(*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    lblFilename->SetLabel(dlg.GetPath());
}

void LMS_Programing_wxgui::OnbtnProgMyriadClick(wxCommandEvent& event)
{
    assert(m_programmer != nullptr);
    progressBar->SetValue(0);
    lblProgressPercent->SetLabel(wxString::Format("%3.1f%%", 0.0));    
    if( (cmbDevice->GetSelection() == 1 && cmbProgMode->GetSelection() == 2) == false)
    {
		if (lblFilename->GetLabel().length() <= 1)
		{
			wxMessageBox("Program file not selected");
			return;
		}
        wxFFileInputStream fin(lblFilename->GetLabel()); //using wxWidgets to read file, to support nonascii characters in path

        if(!fin.IsOk())
        {
            wxMessageBox("Error loading program file");
            return;
        }
        fin.SeekI(0, wxFromEnd);
        unsigned int m_data_size = fin.TellI();

        unsigned char *m_data = new unsigned char[m_data_size];
        fin.SeekI(0, wxFromStart);
        fin.Read((char*)m_data, m_data_size);
        m_programmer->LoadArray(m_data, m_data_size);
        delete m_data;
    }            
    assert(m_programmer != nullptr);
    Disconnect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnProgMyriadClick);
    btnStartStop->SetLabel(_("Abort"));
    int status = m_programmer->StartUploadProgram(cmbDevice->GetSelection(), cmbProgMode->GetSelection());    
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnAbortProgramming);    
    if (status == LMS_Programing::SUCCESS)
    {
        progressPooler->Start(250);
        btnOpen->Disable();
    }
    else    
        btnOpen->Enable();
}

void LMS_Programing_wxgui::OncmbDeviceSelect(wxCommandEvent& event)
{
    if(cmbDevice->GetSelection() == 1)
    {
        cmbProgMode->Clear();
        cmbProgMode->Append("Bitstream to FPGA");
        cmbProgMode->Append("Bitstream to Flash");
        cmbProgMode->Append("Bitstream from Flash");
        cmbProgMode->SetSelection(0);
    }
    else
    {
        cmbProgMode->Clear();
        cmbProgMode->Append(_("Flash"));
        for(int i=1; i<=8; ++i)
            cmbProgMode->Append(wxString::Format("%i", i));
        cmbProgMode->SetSelection(0);
    }
}

/** @brief timer polls programming status, updates progress bar
*/
void LMS_Programing_wxgui::OnProgressPoll(wxTimerEvent& evt)
{
    assert(m_programmer != nullptr);
    LMS_Programing::Info info = m_programmer->GetProgressInfo();
    progressBar->SetRange(100);
    float percent = 100.0*info.bytesSent / info.bytesCount;
    progressBar->SetValue(percent);
    lblProgressPercent->SetLabel(wxString::Format("%3.1f", percent));
    if (info.bytesSent == info.bytesCount)
        OnProgramingFinished();
}

void LMS_Programing_wxgui::OnProgramingFinished()
{
    progressPooler->Stop();
    progressBar->SetValue(100);
    lblProgressPercent->SetLabel(wxString::Format("%3.1f", 100.0));
    LMS_Programing::Info info = m_programmer->GetProgressInfo();
    if (info.aborted)
        wxMessageBox(_("Programming aborted"), _("ERROR"));
    else
        wxMessageBox(wxString::Format(_("Programming Finished: %i bytes sent"), info.bytesSent), _("INFO"));
    btnOpen->Enable();
    Disconnect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnAbortProgramming);
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnProgMyriadClick);
    btnStartStop->SetLabel(_("Program"));
}

void LMS_Programing_wxgui::OnAbortProgramming(wxCommandEvent& event)
{
    assert(m_programmer != nullptr);
    m_programmer->AbortPrograming();
    OnProgramingFinished();
}