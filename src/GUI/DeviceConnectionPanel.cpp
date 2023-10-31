#include "DeviceConnectionPanel.h"

#include <wx/choice.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <vector>

#include "limesuite/DeviceHandle.h"
#include "limesuite/DeviceRegistry.h"

#include "GUI/events.h"

namespace lime {

void DeviceConnectionPanel::EnumerateDevicesToChoice()
{
    wxChoice* choice = cmbDevHandle;
    choice->Clear();
    try
    {
        std::vector<lime::DeviceHandle> handles;
        handles = lime::DeviceRegistry::enumerate();

        if (handles.size() == 0)
        {
            choice->Append("No devices found");
            choice->Select(0);
            choice->Disable();
            btnDisconnect->Disable();
        }

        for (size_t i = 0; i < handles.size(); i++)
        {
            choice->Append(handles[i].Serialize().c_str());
            choice->Enable();
            btnDisconnect->Enable();
        }
    } catch (std::runtime_error& e)
    {
        choice->Disable();
    }
}

DeviceConnectionPanel::DeviceConnectionPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style, "DeviceConnectionPanel")
{
    wxBoxSizer* szBox = new wxBoxSizer(wxHORIZONTAL);

    szBox->Add(new wxStaticText(this, wxID_ANY, _("Device:")), 0, wxALIGN_CENTER_VERTICAL, 0);
    cmbDevHandle = new wxChoice(this, wxNewId());
    cmbDevHandle->Append(_("No devices found"));
    cmbDevHandle->SetSelection(0);
    cmbDevHandle->Disable();
    szBox->Add(cmbDevHandle, 0, wxEXPAND, 0);

    btnDisconnect = new wxButton(this, wxNewId(), wxT("Disconnect"));
    btnDisconnect->Disable();
    szBox->Add(btnDisconnect);

    EnumerateDevicesToChoice();

    this->SetSizerAndFit(szBox);

    // when device choice changes generate CONNECT_DEVICE
    cmbDevHandle->Bind(wxEVT_CHOICE, wxCommandEventHandler(DeviceConnectionPanel::SendHandleChangeEvent), this);
    btnDisconnect->Bind(wxEVT_BUTTON, wxCommandEventHandler(DeviceConnectionPanel::SendDisconnectEvent), this);
}

DeviceConnectionPanel::~DeviceConnectionPanel()
{
}

void DeviceConnectionPanel::SendDisconnectEvent(wxCommandEvent& inEvent)
{
    wxCommandEvent event(limeEVT_SDR_HANDLE_SELECTED, GetId());
    event.SetString(wxEmptyString);
    ProcessWindowEvent(event);
    cmbDevHandle->SetSelection(wxNOT_FOUND);
}

void DeviceConnectionPanel::SendHandleChangeEvent(wxCommandEvent& inEvent)
{
    wxCommandEvent event(limeEVT_SDR_HANDLE_SELECTED, GetId());
    event.SetString(inEvent.GetString());
    ProcessWindowEvent(event);
}

} // namespace lime
