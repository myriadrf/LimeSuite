/**
@file 	FPGAcontrols_wxgui.cpp
@author Lime Microsystems
@brief 	panel for uploading WFM files to Stream board
*/

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/msgdlg.h>
#include "wx/checkbox.h"

#include "dataTypes.h"
#include "GPS_wxgui.h"
#include <wx/spinctrl.h>
#include "lms7_device.h"


//const long GPS_wxgui::ID_STREAMING_TIMER = wxNewId();

BEGIN_EVENT_TABLE(GPS_wxgui, wxFrame)
END_EVENT_TABLE()

GPS_wxgui::GPS_wxgui(wxWindow* parent,wxWindowID id,const wxString &title, const wxPoint& pos,const wxSize& size, long styles)
{
    //SetFont( wxFont( 10, 74, 90, 90, false, wxEmptyString ) );
    lmsControl = nullptr;
    //mStreamingTimer = new wxTimer(this, ID_STREAMING_TIMER);

    wxSize spinBoxSize(64, -1);
    long spinBoxStyle = wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER;

    Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, title);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    auto mainSizer = new wxFlexGridSizer(0, 2, 10, 30);
    mainSizer->AddGrowableCol(0);
    chkEnGPS = new wxCheckBox(this, wxNewId(), _("Enable GPS Module"));
    mainSizer->Add(chkEnGPS, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_TOP, 5);
    btnRefresh = new wxButton(this, wxNewId(), _("Refresh"));
    mainSizer->Add(btnRefresh, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_TOP, 5);
        
    auto gpsSizer = new wxFlexGridSizer(0, 2, 5, 5);
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("UTC:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblUTC = new wxStaticText(this, wxID_ANY, _("HH:MM:SS.SSS")); 
    gpsSizer->Add(lblUTC, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);  
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("Status:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblStatus = new wxStaticText(this, wxID_ANY, _("Warning")); 
    gpsSizer->Add(lblStatus, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);  
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("Latitude:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblLatitude = new wxStaticText(this, wxID_ANY, _("0.0000")); 
    gpsSizer->Add(lblLatitude, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);  
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("Longitude:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblLongitude = new wxStaticText(this, wxID_ANY, _("0.0000")); 
    gpsSizer->Add(lblLongitude, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5); 
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("Speed:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblSpeed = new wxStaticText(this, wxID_ANY, _("0.000 km/h")); 
    gpsSizer->Add(lblSpeed, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5); 
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("Course:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblCourse = new wxStaticText(this, wxID_ANY, _("0.00")); 
    gpsSizer->Add(lblCourse, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5); 
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("Date:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblDate = new wxStaticText(this, wxID_ANY, _("YYYY-MM-DD")); 
    gpsSizer->Add(lblDate, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5); 
    gpsSizer->Add(new wxStaticText(this, wxID_ANY, _("Fix:")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblFix = new wxStaticText(this, wxID_ANY, _("No Fix")); 
    gpsSizer->Add(lblFix, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5); 
    
    mainSizer->Add(gpsSizer, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    
    auto freqSizer = new wxFlexGridSizer(0, 1, 10, 25);
    chkEnFreq = new wxCheckBox(this, wxNewId(), _("Enable VCTCXO adjustment"));
    freqSizer->Add(chkEnFreq, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_TOP, 5);
    
    auto  dacSizer = new wxFlexGridSizer(3, 2, 5, 25);
    dacSizer->Add(new wxStaticText(this, wxID_ANY, _("State: ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblState = new wxStaticText(this, wxID_ANY, _("0")); 
    dacSizer->Add(lblState, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    dacSizer->Add(new wxStaticText(this, wxID_ANY, _("Accuracy: ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblAccuracy = new wxStaticText(this, wxID_ANY, _("0")); 
    dacSizer->Add(lblAccuracy, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    dacSizer->Add(new wxStaticText(this, wxID_ANY, _("DAC: ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblDAC = new wxStaticText(this, wxID_ANY, _("0")); 
    dacSizer->Add(lblDAC, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);  
    freqSizer->Add(dacSizer, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_TOP, 5);
    
    auto errorSizer = new wxFlexGridSizer(4, 3, 5, 5);
    errorSizer->Add(new wxStaticText(this, wxID_ANY, _("Period")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    errorSizer->Add(new wxStaticText(this, wxID_ANY, _("Error")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    errorSizer->Add(new wxStaticText(this, wxID_ANY, _("Tolerance")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    errorSizer->Add(new wxStaticText(this, wxID_ANY, _("1s")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblError1s = new wxStaticText(this, wxID_ANY, _("0")); 
    errorSizer->Add(lblError1s, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);  
    spinTol1s = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 0x3FFFFFFF, 16);
    errorSizer->Add(spinTol1s, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    errorSizer->Add(new wxStaticText(this, wxID_ANY, _("10s")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblError10s = new wxStaticText(this, wxID_ANY, _("0")); 
    errorSizer->Add(lblError10s, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);  
    spinTol10s = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 0x3FFFFFFF, 16);
    errorSizer->Add(spinTol10s, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    errorSizer->Add(new wxStaticText(this, wxID_ANY, _("100s")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblError100s = new wxStaticText(this, wxID_ANY, _("0")); 
    errorSizer->Add(lblError100s, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);  
    spinTol100s = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 0x3FFFFFFF, 16);
    errorSizer->Add(spinTol100s, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    freqSizer->Add(errorSizer, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_TOP, 5);
    
    auto freqBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("VCTCXO adjustment"));
    freqBoxSizer->Add(freqSizer, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(freqBoxSizer, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    
    chkEnFreq->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GPS_wxgui::OnChkEnFreq ), NULL, this );
    chkEnGPS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GPS_wxgui::OnChkEnGPS ), NULL, this );
    btnRefresh->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GPS_wxgui::OnBtnRefresh ), NULL, this );
    spinTol1s->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GPS_wxgui::OnSpin1s ), NULL, this );
    spinTol10s->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GPS_wxgui::OnSpin10s ), NULL, this );
    spinTol100s->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GPS_wxgui::OnSpin100s ), NULL, this );

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

}

void GPS_wxgui::Initialize(lms_device_t* dataPort)
{
    lmsControl = dataPort;
    wxCommandEvent event;
    OnBtnRefresh(event);
}

GPS_wxgui::~GPS_wxgui()
{
    chkEnFreq->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GPS_wxgui::OnChkEnFreq ), NULL, this );
    chkEnGPS->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GPS_wxgui::OnChkEnGPS ), NULL, this );
    btnRefresh->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GPS_wxgui::OnBtnRefresh ), NULL, this );
    spinTol1s->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GPS_wxgui::OnSpin1s ), NULL, this );
    spinTol10s->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GPS_wxgui::OnSpin10s ), NULL, this );
    spinTol100s->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GPS_wxgui::OnSpin100s ), NULL, this );
}

void GPS_wxgui::OnChkEnGPS(wxCommandEvent& event)
{
    uint16_t val = chkEnGPS->GetValue();
    LMS_WriteFPGAReg(lmsControl,0x100,val);
    if (!val)
    {
        LMS_WriteFPGAReg(lmsControl,0xE0,0);
        chkEnFreq->SetValue(false);
    }
}
void GPS_wxgui::OnChkEnFreq(wxCommandEvent& event)
{
    uint16_t val = chkEnFreq->GetValue();
    LMS_WriteFPGAReg(lmsControl,0xE0,val);
    if (val)
    {
        LMS_WriteFPGAReg(lmsControl,0x100,1);
        chkEnGPS->SetValue(true);
    }
}
void GPS_wxgui::OnBtnRefresh(wxCommandEvent& event)
{
    lime::LMS7_Device* device = (lime::LMS7_Device*)lmsControl;
    auto conn = device->GetConnection();
    if (!conn)
        return;
    static const uint32_t addr[] = {
        0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE,
        0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x114
    };   
    static const int size = sizeof(addr)/sizeof(*addr);
    uint32_t vals[size];
    conn->ReadRegisters(addr,vals,size);
    
    auto bcdToInt = [](uint16_t bcd)->int {return (bcd>>12)*1000 + ((bcd>>8)&0xF)*100 + ((bcd>>4)&0xF)*10 + (bcd&0xF);};
    
    chkEnFreq->SetValue(vals[0]&1);
    lblState->SetLabel(vals[1]&0xF ? "Fine tune" : "Coarse tune");
    lblAccuracy->SetLabel(wxString::Format(_("%d"), (vals[1]>>4)&0xF));
    lblDAC->SetLabel(wxString::Format(_("%d"), vals[2]));
    spinTol1s->SetValue(vals[3]+(vals[4]<<16));
    spinTol10s->SetValue(vals[5]+(vals[6]<<16));
    spinTol100s->SetValue(vals[7]+(vals[8]<<16));
    lblError1s->SetLabel(wxString::Format(_("%d"), vals[9]+(vals[10]<<16)));
    lblError10s->SetLabel(wxString::Format(_("%d"), vals[11]+(vals[12]<<16)));
    lblError100s->SetLabel(wxString::Format(_("%d"), vals[13]+(vals[14]<<16)));
    chkEnGPS->SetValue(vals[15]&1);
    lblUTC->SetLabel(wxString::Format(_("%02X:%02X:%02X.%03X"), vals[18]&0xFF, vals[17]>>8, vals[17]&0xFF,vals[16]&0xFFF));
    lblStatus->SetLabel(vals[19]&0x1 ? "Data valid" : "Warning");
    lblLatitude->SetLabel(wxString::Format(_("%X.%06d deg %c"), vals[21]>>8, (bcdToInt(vals[21]&0xFF)*10000 + bcdToInt(vals[20]))*10/6, vals[22]&1 ? 'S' : 'N'));
    lblLongitude->SetLabel(wxString::Format(_("%X.%06d deg %c"), vals[25]<<8 | vals[24]>>8,(bcdToInt(vals[24]&0xFF)*10000 + bcdToInt(vals[23]))*10/6, vals[26]&1 ? 'W' : 'E'));
    lblSpeed->SetLabel(wxString::Format(_("%f km/h"), 0.01852*(bcdToInt(vals[28]&0xF)*10000 + bcdToInt(vals[27]))));
    lblCourse->SetLabel(wxString::Format(_("%X.%02X deg"), vals[30]<<8 | vals[29]>>8, vals[29]&0xFF));
    lblDate->SetLabel(wxString::Format(_("20%02X-%02X-%02X"), vals[31]&0xFF,vals[31]>>8, vals[32]&0xFF));
    lblFix->SetLabel(vals[33]==0x20 ? "2D" : vals[33]==0x30 ? "3D" : "No fix");
    Layout();
    Fit();
}

void GPS_wxgui::OnSpin1s(wxSpinEvent& event)
{
    int value = spinTol1s->GetValue();
    LMS_WriteFPGAReg(lmsControl,0xE3,value&0xFFFF);
    LMS_WriteFPGAReg(lmsControl,0xE4,value>>16);
}
void GPS_wxgui::OnSpin10s(wxSpinEvent& event)
{
    int value = spinTol10s->GetValue();
    LMS_WriteFPGAReg(lmsControl,0xE5,value&0xFFFF);
    LMS_WriteFPGAReg(lmsControl,0xE6,value>>16);
}
void GPS_wxgui::OnSpin100s(wxSpinEvent& event)
{
    int value = spinTol100s->GetValue();
    LMS_WriteFPGAReg(lmsControl,0xE7,value&0xFFFF);
    LMS_WriteFPGAReg(lmsControl,0xE8,value>>16);
}


