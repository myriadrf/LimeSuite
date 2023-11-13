#pragma once

#include "limesuite/SDRDevice.h"
#include <map>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>

class wxStaticText;
class wxFlexGridSizer;
class wxCheckBox;

// B.J.
class wxRadioButton;
class wxButton;

class pnlX3 : public wxPanel
{
  public:
    pnlX3(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        int style = 0,
        wxString name = wxEmptyString);
    void Initialize(lime::SDRDevice* pControl);
    virtual ~pnlX3();
    virtual void UpdatePanel();

    void OnReadAll(wxCommandEvent& event);
    void OnWriteAll(wxCommandEvent& event);

  protected:
    int LMS_ReadFPGAReg(lime::SDRDevice* device, uint32_t address, uint16_t* val);
    int LMS_WriteFPGAReg(lime::SDRDevice* device, uint32_t address, uint16_t val);
    int LMS_WriteCustomBoardParam(lime::SDRDevice* device, int32_t param_id, double val, const std::string& units);
    int LMS_ReadCustomBoardParam(lime::SDRDevice* device, int32_t param_id, double* val, std::string& units);
    void OnInputChange(wxCommandEvent& event);
    void OnDacChange(wxCommandEvent& event);

    // LMS1 Ch0
    wxChoice* cmbLms1Rx1Path;
    wxChoice* cmbLms1Tx1Path;
    wxCheckBox* Lms1tx1En;
    wxSpinCtrl* spinDac1;

    // LMS1 Ch1
    wxChoice* cmbLms1Rx2Path;
    wxChoice* cmbLms1Tx2Path;
    wxCheckBox* Lms1tx2En;
    wxSpinCtrl* spinDac2;

    // LMS2 Ch0
    wxCheckBox* Lms2tx1En;
    wxCheckBox* Lms2Lna1En;
    wxChoice* cmbLms2Trx1TPath;
    wxChoice* cmbLms2Trx1Path;
    wxChoice* cmbLms2Rx1CPath;
    wxChoice* cmbLms2Rx1InPath;

    // LMS2 Ch1
    wxCheckBox* Lms2tx2En;
    wxCheckBox* Lms2Lna2En;
    wxChoice* cmbLms2Trx2TPath;
    wxChoice* cmbLms2Trx2Path;
    wxChoice* cmbLms2Rx2CPath;
    wxChoice* cmbLms2Rx2InPath;

    //LMS3
    wxChoice* cmbLms3Rx1Path;
    wxChoice* cmbLms3Rx2Path;

    //B.J.

    struct Register {
        Register();
        Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue);
        unsigned short address;
        unsigned char msb;
        unsigned char lsb;
        unsigned short defaultValue;
    };

    void SetRegValue(lime::SDRDevice* pLmsControl, Register reg, uint16_t value);
    wxRadioButton* rbLMS1LowFreq;
    wxRadioButton* rbLMS1HighFreq;
    int m_bLMS1LowFreq;
    wxRadioButton* rbLMS1DownLink;
    wxRadioButton* rbLMS1UpLink;
    int m_bLMS1DownLink;
    wxCheckBox* cbLMS1ChA;
    wxCheckBox* cbLMS1ChB;
    wxButton* btnLMS1Settings;
    void OnLMS1LowFreq(wxCommandEvent& event);
    void OnLMS1HighFreq(wxCommandEvent& event);
    void OnLMS1DownLink(wxCommandEvent& event);
    void OnLMS1UpLink(wxCommandEvent& event);
    void OnLMS1Configure(wxCommandEvent& event);

    wxRadioButton* rbLMS2DownLink;
    wxRadioButton* rbLMS2UpLink;
    int m_bLMS2DownLink;
    wxCheckBox* cbLMS2ChA;
    wxCheckBox* cbLMS2ChB;
    wxButton* btnLMS2Settings;
    void OnLMS2DownLink(wxCommandEvent& event);
    void OnLMS2UpLink(wxCommandEvent& event);
    void OnLMS2Configure(wxCommandEvent& event);
    // end B.J.

    int chipSelect;
    lime::SDRDevice* device;

    DECLARE_EVENT_TABLE()
};
