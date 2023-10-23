#include "pnlXTRX.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"
#include "limesuite/SDRDevice.h"

#include <ciso646>

using namespace std;
using namespace lime;

BEGIN_EVENT_TABLE(pnlXTRX, wxPanel)
END_EVENT_TABLE()

pnlXTRX::pnlXTRX(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int style, wxString name)
{
    chipSelect = -1;
    device = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer *mainSizer = new wxFlexGridSizer(0, 2, 1, 1);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RFSW_TX ")), 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cmbTxPath = new wxChoice(this, wxNewId());
    cmbTxPath->Append("Band 2");
    cmbTxPath->Append("Band 1");
    cmbTxPath->SetSelection(0);
    Connect(cmbTxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlXTRX::OnInputChange), NULL, this);
    mainSizer->Add(cmbTxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RFSW_RX ")), 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cmbRxPath = new wxChoice(this, wxNewId());
    cmbRxPath->Append("W Band");
    cmbRxPath->Append("L Band");
    cmbRxPath->Append("H Band");
    cmbRxPath->Append("-unconnected-");
    cmbRxPath->SetSelection(0);
    Connect(cmbRxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlXTRX::OnInputChange), NULL, this);
    mainSizer->Add(cmbRxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    TDDCntrl = new wxCheckBox(this, wxNewId(), _("TDD controls switches"));
    mainSizer->Add(TDDCntrl, 1, wxEXPAND | wxALIGN_RIGHT | wxALIGN_TOP, 5);
    Connect(TDDCntrl->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlXTRX::OnInputChange), NULL, this);

    wxStaticBoxSizer *mainBoxSizer;
    mainBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxString::Format("RF controls %s", name)), wxHORIZONTAL);
    mainBoxSizer->Add(mainSizer, 0, 0, 5);

    mainBoxSizer->Fit(this);
    mainBoxSizer->SetSizeHints(this);
    SetSizer(mainBoxSizer);
    Layout();
    Bind(READ_ALL_VALUES, &pnlXTRX::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlXTRX::OnWriteAll, this, this->GetId());
}

void pnlXTRX::Initialize(lime::SDRDevice *dev, const string& spiSlaveName)
{
    chipSelect = -1;
    device = dev;
    if (!device)
        return;

    const SDRDevice::Descriptor &desc = device->GetDescriptor();
    for (const auto &nameIds : desc.spiSlaveIds)
    {
        if (nameIds.first == spiSlaveName)
        {
            chipSelect = nameIds.second;
            break;
        }
    }
}

pnlXTRX::~pnlXTRX()
{

}

void pnlXTRX::OnInputChange(wxCommandEvent &event)
{
    uint16_t addr = 0x000a;
    uint16_t value = 0;

    if (pnlXTRX::LMS_ReadFPGAReg (device,addr,&value))
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
    value &= ~(1 << 11 | 1 << 4 | 1 << 3 | 1 << 2);
    bool autoSwitching = TDDCntrl->IsChecked();
    value |= autoSwitching << 11;
    value |= cmbTxPath->GetSelection() << 4;
    value |= cmbRxPath->GetSelection() << 2;
    cmbRxPath->Enable(!autoSwitching);
    cmbTxPath->Enable(!autoSwitching);

    if (LMS_WriteFPGAReg(device, addr, value))
        wxMessageBox(_("Failed to write FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
}

int pnlXTRX::LMS_WriteFPGAReg(lime::SDRDevice *device, uint32_t address, uint16_t val)
{
    if (!device)
        return -1;

    const uint32_t mosi = (1 << 31) | address << 16 | val;
    try {
        device->SPI(chipSelect, &mosi, nullptr, 1);
        return 0;
    }
    catch (...) {
        return -1;
    }
}

int pnlXTRX::LMS_ReadFPGAReg(lime::SDRDevice *device, uint32_t address, uint16_t *val)
{
    if (!device)
        return -1;
    const uint32_t mosi = address;
    uint32_t miso = 0;

    try {
        device->SPI(chipSelect, &mosi, &miso, 1);
        *val = miso & 0xFFFF;
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}

void pnlXTRX::UpdatePanel()
{
    uint16_t addr = 0x000A;
    uint16_t value = 0;

    if (LMS_ReadFPGAReg(device, addr, &value))
    {
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    bool autoSwitching = value & (1 << 11);
    cmbRxPath->Enable(!autoSwitching);
    cmbTxPath->Enable(!autoSwitching);
    TDDCntrl->SetValue(autoSwitching);
    cmbTxPath->SetSelection ((value >> 4) & 1);
    cmbRxPath->SetSelection ((value >> 2) & 3);
}

void pnlXTRX::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnlXTRX::OnWriteAll(wxCommandEvent &event)
{
    OnInputChange(event);
}
