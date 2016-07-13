/**
@file 	FPGAcontrols_wxgui.cpp
@author Lime Microsystems
@brief 	panel for uploading WFM files to Stream board
*/

#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/gauge.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "wx/checkbox.h"

#include "FPGAcontrols_wxgui.h"
#include <vector>
#include <fstream>
#include <wx/ffile.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <iostream>
#include "LMS_StreamBoard.h"

using namespace std;
using namespace lime;

const long FPGAcontrols_wxgui::ID_BUTTON6 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON7 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON8 = wxNewId();
const long FPGAcontrols_wxgui::ID_BITMAPBUTTON1 = wxNewId();
const long FPGAcontrols_wxgui::ID_STATICTEXT2 = wxNewId();
const long FPGAcontrols_wxgui::ID_STATICTEXT5 = wxNewId();
const long FPGAcontrols_wxgui::ID_GAUGE1 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON3 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON4 = wxNewId();
//*)
const long FPGAcontrols_wxgui::ID_STREAMING_TIMER = wxNewId();

BEGIN_EVENT_TABLE(FPGAcontrols_wxgui, wxFrame)
    EVT_TIMER(ID_STREAMING_TIMER, FPGAcontrols_wxgui::OnUpdateStats)
END_EVENT_TABLE()

const wxString gWFMdirectory = "lms7suite_wfm";

FPGAcontrols_wxgui::FPGAcontrols_wxgui(wxWindow* parent,wxWindowID id,const wxString &title, const wxPoint& pos,const wxSize& size, long styles)
{
    lmsControl = nullptr;
    fileForCyclicTransmitting = _("");
    mStreamingTimer = new wxTimer(this, ID_STREAMING_TIMER);

	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, title);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 5);
	FlexGridSizer1->AddGrowableCol(0);

    wxStaticBoxSizer* digitalInterfaceGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Digital Interface"));
    chkDigitalLoopbackEnable = new wxCheckBox(this, wxNewId(), _("Digital Loopback enable"));
    digitalInterfaceGroup->Add(chkDigitalLoopbackEnable, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkDigitalLoopbackEnable->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnChkDigitalLoopbackEnableClick);
    FlexGridSizer1->Add(digitalInterfaceGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxLEFT, 5);

	StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("WFM loader"));
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 5, 0);
	FlexGridSizer6->AddGrowableCol(0);
	FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 5);
	btnLoadOnetone = new wxToggleButton(this, ID_BUTTON6, _T("Onetone"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    btnLoadOnetone->SetToolTip(_T("Loads file named onetone.wfm from ") + gWFMdirectory +_("directory"));
	FlexGridSizer8->Add(btnLoadOnetone, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    btnLoadWCDMA = new wxToggleButton(this, ID_BUTTON7, _T("W-CDMA"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    btnLoadWCDMA->SetToolTip(_T("Loads file named wcdma.wfm from ") + gWFMdirectory + _("directory"));
	FlexGridSizer8->Add(btnLoadWCDMA, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	FlexGridSizer6->Add(FlexGridSizer8, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 5);
	FlexGridSizer10->AddGrowableCol(2);
    btnLoadCustom = new wxToggleButton(this, ID_BUTTON8, _T("Custom"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	btnLoadCustom->SetToolTip(_T("Loads user selected custom file"));
	FlexGridSizer10->Add(btnLoadCustom, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	btnOpenWFM = new wxBitmapButton(this, ID_BITMAPBUTTON1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_OPEN")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	FlexGridSizer10->Add(btnOpenWFM, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	txtFilename = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_START, _T("ID_STATICTEXT2"));
	FlexGridSizer10->Add(txtFilename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(FlexGridSizer10, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	lblProgressPercent = new wxStaticText(this, ID_STATICTEXT5, _T("0 %"), wxDefaultPosition, wxSize(32,-1), 0, _T("ID_STATICTEXT5"));
	FlexGridSizer3->Add(lblProgressPercent, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	progressBar = new wxGauge(this, ID_GAUGE1, 100, wxDefaultPosition, wxSize(-1,10), 0, wxDefaultValidator, _T("ID_GAUGE1"));
	FlexGridSizer3->Add(progressBar, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(FlexGridSizer3, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 5);
	btnPlayWFM = new wxButton(this, ID_BUTTON3, _T("Play >"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(btnPlayWFM, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	btnStopWFM = new wxButton(this, ID_BUTTON4, _T("Stop ||"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer2->Add(btnStopWFM, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	FlexGridSizer6->Add(FlexGridSizer2, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	StaticBoxSizer3->Add(FlexGridSizer6, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	FlexGridSizer1->Add(StaticBoxSizer3, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxStaticBoxSizer* streamingBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Samples streaming"));
    wxFlexGridSizer* streamingSizer = new wxFlexGridSizer(0, 4, 0, 0);
    long idLoadButton = wxNewId();
    btnLoadSamples = new wxButton(this, idLoadButton, _("Select File"));
    streamingSizer->Add(btnLoadSamples, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    long idStartButton = wxNewId();
    btnStartStreaming = new wxButton(this, idStartButton, _("Start streaming"));
    streamingSizer->Add(btnStartStreaming, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    long idStopButton = wxNewId();
    btnStopStreaming = new wxButton(this, idStopButton, _("Stop streaming"));
    streamingSizer->Add(btnStopStreaming, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    txtDataRate = new wxStaticText(this, wxNewId(), _("Data rate: ???? kB/s"));
    streamingSizer->Add(txtDataRate, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    streamingBox->Add(streamingSizer, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer6->Add(streamingBox, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);

	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON6, wxEVT_TOGGLEBUTTON, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnLoadOnetoneClick);
    Connect(ID_BUTTON7, wxEVT_TOGGLEBUTTON, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnLoadWCDMAClick);
    Connect(ID_BUTTON8, wxEVT_TOGGLEBUTTON, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnLoadCustomClick);
	Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnOpenFileClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnPlayWFMClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnStopWFMClick);

    Connect(idLoadButton, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnLoadFileClick);
    Connect(idStartButton, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnStartStreamingClick);
    Connect(idStopButton, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnStopStreamingClick);

    //create directory for wfm files
    wxDir dir(wxGetCwd() + _("/") + gWFMdirectory);
    if (!dir.IsOpened())
        dir.Make(gWFMdirectory);
}

void FPGAcontrols_wxgui::Initialize(lms_device_t* dataPort)
{
    lmsControl = dataPort;

}

FPGAcontrols_wxgui::~FPGAcontrols_wxgui()
{
    wxCommandEvent evt;
    OnbtnStopStreamingClick(evt);

}

/** @brief Reads WFM file and outputs iq sample pairs
    @param filename source file
    @param iSamples I samples destination vector
    @param qSamples Q samples destination vector
    @return number of samples returned, -1 file not found
*/
int ReadWFM(const wxString filename, std::vector<int16_t> &iSamples, std::vector<int16_t> &qSamples)
{
	if (filename.length() <= 1)
		return 0;
    wxFFile fpin(filename, "rb");
    unsigned char inputBuffer[4];
    unsigned char c1, c2, c3, c4; // To read Agilent Signal Studio file
    double iin, qin; // IQ inputs
    int iint, qint;	// IQ integer versions
    int cnt = 0;

    if( fpin.IsOpened() == false)
    {
#ifndef NDEBUG
        cout << _("WFM reading: Input file can not be opened (") << filename << _(")\n");
#endif
        return -1;
    }

    while( fpin.Read(inputBuffer, 4) == 4 )
    {
        c1 = inputBuffer[0];
        c2 = inputBuffer[1];
        c3 = inputBuffer[2];
        c4 = inputBuffer[3];

        cnt++;

        c1 &= 0xFF;
        c2 &= 0xFF;
        c3 &= 0xFF;
        c4 &= 0xFF;

        if( c1&0x80 ) iin = (double)(-1*(1<<15) + ((c1&0x7F)<<8) + c2);
        else iin = (double)(((c1&0x7F)<<8) + c2);

        if( c3&0x80 ) qin = (double)(-1*(1<<15) + ((c3&0x7F)<<8) + c4);
        else qin = (double)(((c3&0x7F)<<8) + c4);

        iint = (int)(iin);
        qint = (int)(qin);

        //convert from 16 bit to 12 bit values
        iint = iint >> 4;
        qint = qint >> 4;

        iSamples.push_back(iint);
        qSamples.push_back(qint);
    };
    return iSamples.size();
}

void FPGAcontrols_wxgui::OnbtnOpenFileClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Open file"), _(""), _(""), _("wfm (*.wfm)|*.wfm"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    txtFilename->SetLabelText( dlg.GetPath());
    Layout();
}

void FPGAcontrols_wxgui::OnbtnPlayWFMClick(wxCommandEvent& event)
{
    /*assert(mStreamer != nullptr);
    uint16_t regData = mStreamer->Reg_read(0x0005);
    mStreamer->Reg_write(0x0005, regData | 0x3);*/

}

void FPGAcontrols_wxgui::OnbtnStopWFMClick(wxCommandEvent& event)
{

    /*assert(mStreamer != nullptr);
    uint16_t regData = mStreamer->Reg_read(0x0005);
    mStreamer->Reg_write(0x0005, (regData & ~0x2) | 0x1);*/

}

int FPGAcontrols_wxgui::UploadFile(const wxString &filename)
{
    /*assert(mStreamer != nullptr);
    if (!m_serPort || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Device not connected"), _("Error"));
        return -2;
    }

    if (filename.length() == 0)
    {
        wxMessageBox(_("File not selected"), _("Error"));
        return -1;
    }
    vector<int16_t> isamples;
    vector<int16_t> qsamples;
    isamples.clear();
    qsamples.clear();
    if( ReadWFM(filename, isamples, qsamples) < 0)
    {
        wxMessageBox(_("File not found ") + filename, _("Error"));
        return -1;
    }
    progressBar->SetRange(isamples.size());
    progressBar->SetValue(0);
    bool success = true;

    btnPlayWFM->Enable(false);
    btnStopWFM->Enable(false);

    uint16_t regData = mStreamer->Reg_read(0x000A);
    mStreamer->Reg_write(0x000A, regData & ~0x7);

    int chCount = 0;
    uint16_t channelFlags = mStreamer->Reg_read(0x0007);
    for(int i=0; i<16; ++i)
        chCount += (channelFlags >> i) & 1;

    PacketLTE pkt;
    int samplesUsed = 0;

    while(samplesUsed<isamples.size())
    {
        pkt.counter = 0;
        pkt.reserved[0] = 0;
        int bufPos = 0;
        for(int i=0; i<1360/chCount && samplesUsed < isamples.size(); ++i)
        {
            for(int ch = 0; ch < chCount; ++ch)
            {
                pkt.data[bufPos] = isamples[samplesUsed] & 0xFF;
                pkt.data[bufPos+1] = (isamples[samplesUsed] >> 8) & 0x0F;
                pkt.data[bufPos+1] |= (qsamples[samplesUsed] << 4) & 0xF0;
                pkt.data[bufPos+2] = (qsamples[samplesUsed] >> 4) & 0xFF;
                bufPos += 3;
            }
            ++samplesUsed;
        }
        int payloadSize = bufPos / 4;
        if(bufPos % 4 != 0)
            printf("Packet samples count not multiple of 4\n");
        pkt.reserved[1] = (payloadSize >> 8) & 0xFF; //WFM loading
        pkt.reserved[2] = payloadSize & 0xFF; //WFM loading
        pkt.reserved[0] = 0x1 << 5; //WFM loading

        long bToSend = sizeof(pkt);
        int context = m_serPort->BeginDataSending((char*)&pkt, bToSend );
        if(m_serPort->WaitForSending(context, 250) == false)
        {
            success = false;
            m_serPort->FinishDataSending((char*)&pkt, bToSend , context);
            break;
        }
        m_serPort->FinishDataSending((char*)&pkt, bToSend , context);
        progressBar->SetValue(samplesUsed);
        lblProgressPercent->SetLabel(wxString::Format(_("%3.0f%%"), 100.0*samplesUsed/isamples.size()));
        wxYield();
    }
    progressBar->SetValue(progressBar->GetRange());
    lblProgressPercent->SetLabelText(_("100%"));

    regData = mStreamer->Reg_read(0x000A);
    mStreamer->Reg_write(0x000A, regData | 0x6);

    btnPlayWFM->Enable(true);
    btnStopWFM->Enable(true);

	if (!success)
	{
		wxMessageBox(_("Failed to upload WFM file"), _("Error"));
		return -3;
	}
	else*/
	return 0;
}

void FPGAcontrols_wxgui::OnbtnLoadOnetoneClick(wxCommandEvent& event)
{
    if (UploadFile(gWFMdirectory + _("/onetone.wfm")) < 0)
        btnLoadOnetone->SetValue(0);
    else
    {
        btnLoadOnetone->SetValue(1);
        btnLoadWCDMA->SetValue(0);
        btnLoadCustom->SetValue(0);
    }
}

void FPGAcontrols_wxgui::OnbtnLoadWCDMAClick(wxCommandEvent& event)
{
    if (UploadFile(gWFMdirectory + _("/wcdma.wfm")) < 0)
        btnLoadWCDMA->SetValue(0);
    else
    {
        btnLoadOnetone->SetValue(0);
        btnLoadWCDMA->SetValue(1);
        btnLoadCustom->SetValue(0);
    }
}

void FPGAcontrols_wxgui::OnbtnLoadCustomClick(wxCommandEvent& event)
{
    if( UploadFile(txtFilename->GetLabel()) < 0)
    {
        btnLoadCustom->SetValue(0);
    }
    else
    {
        btnLoadOnetone->SetValue(0);
        btnLoadWCDMA->SetValue(0);
        btnLoadCustom->SetValue(1);
    }
}

void FPGAcontrols_wxgui::OnUpdateStats(wxTimerEvent& event)
{
  /*  if (mStreamer == nullptr)
        return;
    LMS_StreamBoard::ProgressStats stats = mStreamer->GetStats();
    txtDataRate->SetLabelText(wxString::Format(_("Data rate: %.1f kB/s"), stats.TxRate_Bps/1000));

   */
}

void FPGAcontrols_wxgui::OnbtnLoadFileClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Open file"), _(""), _(""), _("wfm (*.wfm)|*.wfm"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    fileForCyclicTransmitting = dlg.GetPath();
}

void FPGAcontrols_wxgui::OnbtnStartStreamingClick(wxCommandEvent& event)
{
    mStreamingTimer->Start(500);
    vector<int16_t> isamples;
    vector<int16_t> qsamples;
    int framesCount = ReadWFM(fileForCyclicTransmitting, isamples, qsamples);
    if( framesCount < 0)
    {
        wxMessageBox(_("File not found ") + fileForCyclicTransmitting, _("Error"));
        return;
    }
    if (framesCount == 0)
    {
        wxMessageBox(_("No samples were loaded"));
        return;
    }
   // mStreamer->StartCyclicTransmitting(&isamples[0], &qsamples[0], framesCount);
}

void FPGAcontrols_wxgui::OnbtnStopStreamingClick(wxCommandEvent& event)
{
    mStreamingTimer->Stop();
    //mStreamer->StopCyclicTransmitting();
}

void FPGAcontrols_wxgui::OnChkDigitalLoopbackEnableClick(wxCommandEvent& event)
{
   /* if(!m_serPort)
    {
        wxMessageBox(_("FPGA controls: Connection not initialized"), _("ERROR"));
        return;
    }

    const uint16_t address = 0x0008;
    uint32_t dataRd = 0;
    int status;
    status = m_serPort->ReadRegister(address, dataRd);
    unsigned short regValue = 0;

    if (status == 0)
        regValue = dataRd & 0xFFFF;

    regValue = (regValue & ~(1<<10)) | chkDigitalLoopbackEnable->IsChecked() << 10;

    status = m_serPort->WriteRegister(address, regValue);

    if (status != 0)
        wxMessageBox(_("Failed to write SPI"), _("Error"), wxICON_ERROR);*/
}
