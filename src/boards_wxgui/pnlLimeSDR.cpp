#include "pnlLimeSDR.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/statbox.h>
#include "lms7suiteEvents.h"
#include "Logger.h"
#include "LMSBoards.h"

#include <ciso646>

#include <ErrorReporting.h>

using namespace lime;
using namespace std;

BEGIN_EVENT_TABLE(pnlLimeSDR, wxPanel)
END_EVENT_TABLE()

pnlLimeSDR::pnlLimeSDR(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;
    gpioCnt = 8;
    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    mainSizer = new wxFlexGridSizer(0, 2, 5, 5);
    controlsSizer = new wxFlexGridSizer(0, 2, 5, 5);

    SetSizer(mainSizer);
    chkRFLB_A_EN = new wxCheckBox(this, wxNewId(), _("RF loopback ch.A"));
    chkRFLB_A_EN->SetToolTip(_("[RFLB_A_EN] External RF loopback TxBAND2->RxLNAH channel A"));
    Connect(chkRFLB_A_EN->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkRFLB_A_EN, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkRFLB_B_EN = new wxCheckBox(this, wxNewId(), _("RF loopback ch.B"));
    chkRFLB_B_EN->SetToolTip(_("[RFLB_B_EN] External RF loopback TxBAND2->RxLNAH channel B"));
    Connect(chkRFLB_B_EN->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkRFLB_B_EN, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    chkTX1_2_LB_SH = new wxCheckBox(this, wxNewId(), _("Ch.A shunt"));
    chkTX1_2_LB_SH->SetToolTip(_("[TX1_2_LB_SH]"));
    Connect(chkTX1_2_LB_SH->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX1_2_LB_SH, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkTX2_2_LB_SH = new wxCheckBox(this, wxNewId(), _("Ch.B shunt"));
    chkTX2_2_LB_SH->SetToolTip(_("[TX2_2_LB_SH]"));
    Connect(chkTX2_2_LB_SH->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX2_2_LB_SH, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkTX1_2_LB_AT = new wxCheckBox(this, wxNewId(), _("Ch.A attenuator"));
    chkTX1_2_LB_AT->SetToolTip(_("[TX1_2_LB_AT]"));
    Connect(chkTX1_2_LB_AT->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX1_2_LB_AT, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkTX2_2_LB_AT = new wxCheckBox(this, wxNewId(), _("Ch.B attenuator"));
    chkTX2_2_LB_AT->SetToolTip(_("[TX2_2_LB_AT]"));
    Connect(chkTX2_2_LB_AT->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), NULL, this);
    controlsSizer->Add(chkTX2_2_LB_AT, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    auto groupSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("External loopback controls") ), wxVERTICAL );
    groupSizer->Add(controlsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    mainSizer->Add(groupSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    Bind(READ_ALL_VALUES, &pnlLimeSDR::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlLimeSDR::OnWriteAll, this, this->GetId());
}

void pnlLimeSDR::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    if(lmsControl)
    {
        auto info = LMS_GetDeviceInfo(lmsControl);
        if(info != nullptr)
        {
            if (string(info->deviceName) == string(GetDeviceName(LMS_DEV_LIMESDR_PCIE)))
                gpioCnt = 16;
            auto controls = controlsSizer->GetChildren();
            for(auto i : controls)
                i->GetWindow()->Enable();
        }
    }

    auto gpioSizer = new wxFlexGridSizer(0, gpioCnt+1, 0, 0);
    gpioSizer->Add(new wxStaticText(this, wxID_ANY, _("GPIO")), 1, wxALL, 5);

    for (int i = gpioCnt; i--;)
        gpioSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format("%d", i)), 1, wxALL, 5);

    wxStaticText* text = new wxStaticText(this, wxID_ANY, _("DIR"));
    text->SetToolTip(_("Check to set GPIO to output"));
    gpioSizer->Add(text, 1, wxALL, 5);

    for (int i = gpioCnt; i--;)
    {
        gpioDir[i] = new wxCheckBox(this, wxNewId(), _(""));
        gpioDir[i]->SetToolTip(_("Check to set GPIO to output"));
        Connect(gpioDir[i]->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnUsrGPIODirChange), NULL, this);
        gpioSizer->Add(gpioDir[i]);
    }
    text = new wxStaticText(this, wxID_ANY, _("OUT"));
    text->SetToolTip(_("GPIO output value (checked - High)"));
    gpioSizer->Add(text, 1, wxALL, 5);
    for (int i = gpioCnt; i--;)
    {
        gpioOut[i] = new wxCheckBox(this, wxNewId(), _(""));
        gpioOut[i]->SetToolTip(_("GPIO output value (checked - High)"));
        gpioOut[i]->Disable();
        Connect(gpioOut[i]->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlLimeSDR::OnUsrGPIOChange), NULL, this);
        gpioSizer->Add(gpioOut[i]);
    }
    text = new wxStaticText(this, wxID_ANY, _("IN"));
    text->SetToolTip(_("GPIO input value"));
    gpioSizer->Add(text, 1, wxALL, 5);
    for (int i = gpioCnt; i--;)
    {
        gpioIn[i] = new wxStaticText(this, wxNewId(), _("0"));
        gpioIn[i]->SetToolTip(_("GPIO input value"));
        gpioSizer->Add(gpioIn[i],1, wxALL, 5);
    }

    auto groupSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("GPIO Control") ), wxVERTICAL );
    groupSizer->Add(gpioSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    mainSizer->Add(groupSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();
}

pnlLimeSDR::~pnlLimeSDR()
{
    chkRFLB_A_EN->Disconnect(wxEVT_CHECKBOX, chkRFLB_A_EN->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);
    chkRFLB_B_EN->Disconnect(wxEVT_CHECKBOX, chkRFLB_B_EN->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);
    chkTX1_2_LB_SH->Disconnect(wxEVT_CHECKBOX, chkTX1_2_LB_SH->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
    chkTX1_2_LB_AT->Disconnect(wxEVT_CHECKBOX, chkTX1_2_LB_AT->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
    chkTX2_2_LB_SH->Disconnect(wxEVT_CHECKBOX, chkTX2_2_LB_SH->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
    chkTX2_2_LB_AT->Disconnect(wxEVT_CHECKBOX, chkTX2_2_LB_AT->GetId(), wxCommandEventHandler(pnlLimeSDR::OnGPIOChange), 0, this);;
    for (int i = 0; i < 8; i++)
    {
        gpioOut[i]->Disconnect(wxEVT_CHECKBOX, gpioOut[i]->GetId(), wxCommandEventHandler(pnlLimeSDR::OnUsrGPIOChange), 0, this);
        gpioDir[i]->Disconnect(wxEVT_CHECKBOX, gpioDir[i]->GetId(), wxCommandEventHandler(pnlLimeSDR::OnUsrGPIODirChange), 0, this);
    }
}

void pnlLimeSDR::OnGPIOChange(wxCommandEvent& event)
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    value |= chkRFLB_A_EN->GetValue() << 0;
    value |= chkTX1_2_LB_AT->GetValue() << 1;
    value |= chkTX1_2_LB_SH->GetValue() << 2;

    value |= chkRFLB_B_EN->GetValue() << 4;
    value |= chkTX2_2_LB_AT->GetValue() << 5;
    value |= chkTX2_2_LB_SH->GetValue() << 6;

    if(lmsControl && LMS_WriteFPGAReg(lmsControl, addr, value))
        lime::error("Board loopback: %s", LMS_GetLastErrorMessage());
}

void pnlLimeSDR::OnUsrGPIODirChange(wxCommandEvent& event)
{
    uint8_t value[2] = {0};

    for (int i = 0; i < gpioCnt; i++)
    {
        value[i/8] >>=1;
        bool check = gpioDir[i]->GetValue();
        if (check)
            value[i/8] |= 0x80;
        gpioOut[i]->Enable(check);
    }
    if(lmsControl && LMS_GPIODirWrite(lmsControl, value, gpioCnt/8))
        lime::error("GPIO: %s", LMS_GetLastErrorMessage());
}

void pnlLimeSDR::OnUsrGPIOChange(wxCommandEvent& event)
{
    uint8_t value[2] = {0};
    for (int i = 0; i < gpioCnt; i++)
    {
        value[i/8] >>=1;
        if (gpioOut[i]->GetValue())
            value[i/8] |= 0x80;
    }

    if(lmsControl && LMS_GPIOWrite(lmsControl, value, gpioCnt/8))
        lime::error("GPIO: %s", LMS_GetLastErrorMessage());
}

void pnlLimeSDR::UpdatePanel()
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    uint8_t gpio[2] = {0};
    uint8_t dir[2] = {0};
    if(lmsControl && LMS_ReadFPGAReg(lmsControl, addr, &value)==0)
    {
        chkRFLB_A_EN->SetValue((value >> 0) & 0x1);
        chkTX1_2_LB_AT->SetValue((value >> 1) & 0x1);
        chkTX1_2_LB_SH->SetValue((value >> 2) & 0x1);

        chkRFLB_B_EN->SetValue((value >> 4) & 0x1);
        chkTX2_2_LB_AT->SetValue((value >> 5) & 0x1);
        chkTX2_2_LB_SH->SetValue((value >> 6) & 0x1);
    }
    else
        lime::error("Board loopback: %s", LMS_GetLastErrorMessage());

    if(lmsControl && LMS_GPIODirRead(lmsControl, dir, gpioCnt/8)==0)
    {
        for (int i = 0; i < gpioCnt; i++)
        {
            gpioDir[i]->SetValue(dir[i/8] & 1);
            gpioOut[i]->Enable(dir[i/8] & 1);
            dir[i/8] >>= 1;
        }
    }
    else
    {
        lime::error("GPIO: %s", LMS_GetLastErrorMessage());
        return;
    }

    if(lmsControl && LMS_GPIORead(lmsControl, gpio, gpioCnt/8) == 0)
    {
        for (int i = 0; i < gpioCnt; i++)
        {
            gpioIn[i]->SetLabel(gpio[i/8] & 1 ? _("1") : _("0"));
            gpio[i/8] >>= 1;
        }
    }
    else
        lime::error("GPIO: %s", LMS_GetLastErrorMessage());
}

void pnlLimeSDR::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnlLimeSDR::OnWriteAll(wxCommandEvent &event)
{
    OnGPIOChange(event);
    OnUsrGPIODirChange(event);
    OnUsrGPIOChange(event);
}
