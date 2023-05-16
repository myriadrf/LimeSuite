#pragma once

#include "limesuite/SDRDevice.h"
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
class wxStaticText;
class wxFlexGridSizer;
class wxCheckBox;

class wxRadioButton;
class wxButton;

class pnlXTRX : public wxPanel
{
public:
    pnlXTRX(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lime::SDRDevice *device);
    virtual ~pnlXTRX();
    virtual void UpdatePanel();

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    void OnInputChange(wxCommandEvent &event);
    int LMS_WriteFPGAReg(lime::SDRDevice *device, uint32_t address, uint16_t val);
    int LMS_ReadFPGAReg(lime::SDRDevice *device, uint32_t address, uint16_t *val);

    wxCheckBox  *TDDCntrl;
    wxChoice    *cmbTxPath;
    wxChoice    *cmbRxPath;

    int chipSelect;
    lime::SDRDevice *device;
    DECLARE_EVENT_TABLE()
};

