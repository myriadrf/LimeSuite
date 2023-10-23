#include "pnlGPIO.h"
#include "Logger.h"
#include "LMSBoards.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>

#include "limesuite/SDRDevice.h"

using namespace lime;
using namespace std;

pnlGPIO::pnlGPIO(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    device = nullptr;
    gpioCount = 8;
    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
}

void pnlGPIO::Initialize(lime::SDRDevice* pControl)
{
    device = pControl;
    if (device)
    {
        auto info = device->GetDescriptor();

        const std::unordered_map<std::string, int> specialGpioCounts {
            {string(GetDeviceName(LMS_DEV_LIMESDRMINI)), 10},
            {string(GetDeviceName(LMS_DEV_LIMESDRMINI_V2)), 10},
            {string(GetDeviceName(LMS_DEV_LIMESDR_PCIE)), 16},
        };

        if (specialGpioCounts.find(info.name) != specialGpioCounts.end())
        {
            gpioCount = specialGpioCounts.at(info.name);
        }
    }

    auto gpioSizer = new wxFlexGridSizer(0, gpioCount+1, 0, 0);
    gpioSizer->Add(new wxStaticText(this, wxID_ANY, _("GPIO")), 1, wxEXPAND | wxALL, 5);

    for (int i = gpioCount; i--;)
        gpioSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format("%d", i)), 1, wxEXPAND | wxALL, 5);

    wxStaticText* text = new wxStaticText(this, wxID_ANY, _("DIR"));
    text->SetToolTip(_("Check to set GPIO to output"));
    gpioSizer->Add(text, 1, wxEXPAND | wxALL, 5);

    for (int i = gpioCount; i--;)
    {
        gpioDir[i] = new wxCheckBox(this, wxNewId(), _(""));
        gpioDir[i]->SetToolTip(_("Check to set GPIO to output"));
        Connect(gpioDir[i]->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlGPIO::OnUsrGPIODirChange), NULL, this);
        gpioSizer->Add(gpioDir[i]);
    }
    text = new wxStaticText(this, wxID_ANY, _("OUT"));
    text->SetToolTip(_("GPIO output value (checked - High)"));
    gpioSizer->Add(text, 1, wxEXPAND | wxALL, 5);
    for (int i = gpioCount; i--;)
    {
        gpioOut[i] = new wxCheckBox(this, wxNewId(), _(""));
        gpioOut[i]->SetToolTip(_("GPIO output value (checked - High)"));
        gpioOut[i]->Disable();
        Connect(gpioOut[i]->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlGPIO::OnUsrGPIOChange), NULL, this);
        gpioSizer->Add(gpioOut[i]);
    }
    text = new wxStaticText(this, wxID_ANY, _("IN"));
    text->SetToolTip(_("GPIO input value"));
    gpioSizer->Add(text, 1, wxEXPAND | wxALL, 5);
    for (int i = gpioCount; i--;)
    {
        gpioIn[i] = new wxStaticText(this, wxNewId(), _("0"));
        gpioIn[i]->SetToolTip(_("GPIO input value"));
        gpioSizer->Add(gpioIn[i],1, wxEXPAND | wxALL, 5);
    }

    auto groupSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("GPIO Control") ), wxVERTICAL );
    groupSizer->Add(gpioSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    SetSizer(groupSizer);
    groupSizer->Fit(this);
    groupSizer->SetSizeHints(this);
    Layout();
}

pnlGPIO::~pnlGPIO()
{
    for (int i = 0; i < 8; i++)
    {
        gpioOut[i]->Disconnect(wxEVT_CHECKBOX, gpioOut[i]->GetId(), wxCommandEventHandler(pnlGPIO::OnUsrGPIOChange), 0, this);
        gpioDir[i]->Disconnect(wxEVT_CHECKBOX, gpioDir[i]->GetId(), wxCommandEventHandler(pnlGPIO::OnUsrGPIODirChange), 0, this);
    }
}

void pnlGPIO::OnUsrGPIODirChange(wxCommandEvent& event)
{
    uint8_t value[2] = {0};

    for (int i = 0; i < gpioCount; i++)
    {
        bool check = gpioDir[i]->GetValue();
        if (check)
            value[i/8] |= 1 << (i%8);
        gpioOut[i]->Enable(check);
    }

    if (device && device->GPIODirWrite(value, gpioCount > 8 ? 2 : 1))
        lime::error("GPIO direction change failed");
}

void pnlGPIO::OnUsrGPIOChange(wxCommandEvent& event)
{
    uint8_t value[2] = {0};
    for (int i = 0; i < gpioCount; i++)
    {
        if (gpioOut[i]->GetValue())
            value[i/8] |= 1 << (i%8);
    }

    if (device && device->GPIOWrite(value, gpioCount > 8 ? 2 : 1))
        lime::error("GPIO write failed");
}

void pnlGPIO::UpdatePanel()
{
    uint8_t gpio[2] = {0};
    uint8_t dir[2] = {0};

    if (device && device->GPIODirRead(dir, gpioCount > 8 ? 2 : 1)==0)
    {
        for (int i = 0; i < gpioCount; i++)
        {
            gpioDir[i]->SetValue(dir[i/8] & 1);
            gpioOut[i]->Enable(dir[i/8] & 1);
            dir[i/8] >>= 1;
        }
    }
    else
    {
        lime::error("GPIO direction read failed");
    }

    if (device && device->GPIORead(gpio, gpioCount > 8 ? 2 : 1) == 0)
    {
        for (int i = 0; i < gpioCount; i++)
        {
            gpioIn[i]->SetLabel(gpio[i/8] & 1 ? _("1") : _("0"));
            gpio[i/8] >>= 1;
        }
    }
    else
    {
        lime::error("GPIO read failed");
    }
}
