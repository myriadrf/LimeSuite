#ifndef PNL_QSPARK_H
#define PNL_QSPARK_H

#include <map>
#include <wx/frame.h>
#include <wx/panel.h>
class wxStaticText;
class wxFlexGridSizer;
class wxButton;
class wxSpinCtrl;
class wxComboBox;
class wxCheckBox;
class wxTextCtrl;
class wxChoice;
#include "LMS7SuiteAppFrame.h";

namespace lime
{
class IConnection;
}

class pnlQSpark : public wxFrame
{
public:

	LMS7SuiteAppFrame * parent;
	pnlQSpark(LMS7SuiteAppFrame * parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = "");
    void Initialize(lime::IConnection *pControl);
    virtual ~pnlQSpark();
    virtual void UpdatePanel();

    wxButton* btnUpdateAll;
    wxComboBox* cmbVCXOcontrolVoltage;

    wxPanel* mPanelStreamPLL;
    wxTextCtrl* txtPllFreqRxMHz;
    wxStaticText* lblRealFreqTx;
    wxStaticText* lblRealFreqRx;
    wxTextCtrl* txtPllFreqTxMHz;
    wxTextCtrl* txtPhaseOffsetDeg;
    wxButton* btnConfigurePLL;

    wxCheckBox* chkEN_TXTSP;
    wxCheckBox* chkEN_RXTSP;
    wxCheckBox* chkRX_DCCORR_BYP;
    wxCheckBox* chkRX_PHCORR_BYP;
    wxCheckBox* chkRX_GCORR_BYP;
    wxCheckBox* chkTX_DCCORR_BYP;
    wxCheckBox* chkTX_PHCORR_BYP;
    wxCheckBox* chkTX_GCORR_BYP;
    wxSpinCtrl* spinTX_DCCORRI;
    wxSpinCtrl* spinTX_DCCORRQ;
    wxSpinCtrl* spinTX_GCORRQ;
    wxSpinCtrl* spinTX_GCORRI;
    wxSpinCtrl* spinTX_PHCORR;
    wxSpinCtrl* spinRX_GCORRQ;
    wxSpinCtrl* spinRX_GCORRI;
    wxSpinCtrl* spinRX_PHCORR;
    wxChoice* cmbInsel;

	wxChoice* cmbPa1Sw;
	wxChoice* cmbPa2Sw;
	wxChoice* cmbPa1Mo;
	wxChoice* cmbPa2Mo;
	wxChoice* cmbRf;

    wxTextCtrl* txtNcoFreq;
	wxTextCtrl* txtVccPa1;
	wxTextCtrl* txtVccPa2;

	wxStaticText* lblPa1;
	wxStaticText* lblPa2;

    static const long ID_BUTTON_UPDATEALL;
    static const long ID_VCXOCV;

    void OnbtnUpdateAll(wxCommandEvent& event);
    void OnNcoFrequencyChanged(wxCommandEvent& event);
	void OnVccPa1Changed(wxCommandEvent& event);
	void OnVccPa2Changed(wxCommandEvent& event);
protected:
    void OnConfigurePLL(wxCommandEvent &event);
	void VoltageSetup(double vccPa_V, unsigned char addr, int * pValue);

    struct Register
    {
        Register();
		Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue, unsigned short twocomplement);
        unsigned short address;
        unsigned char msb;
        unsigned char lsb;
        unsigned short defaultValue;
		unsigned short twocomplement;
    };
    std::map<wxObject*, Register> controlsPtr2Registers;
    void RegisterParameterChangeHandler(wxCommandEvent& event);

protected:
    lime::IConnection *m_serPort;
    DECLARE_EVENT_TABLE()
};

#endif
