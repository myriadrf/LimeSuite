#ifndef PNL_QSPARK_H
#define PNL_QSPARK_H

#include <map>
#include <wx/frame.h>
#include <wx/panel.h>
#include "lime/LimeSuite.h"
class wxStaticText;
class wxFlexGridSizer;
class wxButton;
class wxSpinCtrl;
class wxComboBox;
class wxCheckBox;
class wxTextCtrl;
class wxChoice;
class wxRadioButton;



class pnlQSpark : public wxPanel
{
public:
    pnlQSpark(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = "");
    void Initialize(lms_device_t *pControl);
    virtual ~pnlQSpark();

    wxComboBox* cmbVCXOcontrolVoltage;
    
    wxCheckBox* chkLB_1A;
    wxCheckBox* chkLB_1B;
    wxCheckBox* chkLB_2A;
    wxCheckBox* chkLB_2B;
    wxCheckBox* chkSH_1A;
    wxCheckBox* chkSH_1B;
    wxCheckBox* chkSH_2A;
    wxCheckBox* chkSH_2B;
    wxCheckBox* chkAT_1A;
    wxCheckBox* chkAT_1B;
    wxCheckBox* chkAT_2A;
    wxCheckBox* chkAT_2B;

    wxPanel* mPanelStreamPLL;
    wxPanel* mPanelPACtrl;
    wxTextCtrl* txtPllFreqRxMHz;
    wxStaticText* lblRealFreqTx;
    wxStaticText* lblRealFreqRx;
    wxTextCtrl* txtPllFreqTxMHz;
    wxTextCtrl* txtPhaseOffsetDeg;
    wxButton* btnConfigurePLL;

    /*
    wxCheckBox* chkEN_TXTSP;
    wxCheckBox* chkEN_RXTSP;
    wxCheckBox* chkRX_DCCORR_BYP;
    wxCheckBox* chkRX_PHCORR_BYP;
    wxCheckBox* chkRX_GCORR_BYP;
    wxCheckBox* chkTX_DCCORR_BYP;
    wxCheckBox* chkTX_PHCORR_BYP;
    wxCheckBox* chkTX_GCORR_BYP;
    wxCheckBox* chkTX_INVSINC_BYP;

    wxCheckBox* chkTX_INTERPOLATION_BYP0;
    wxCheckBox* chkTX_INTERPOLATION_BYP1;
    wxCheckBox* chkTX_INTERPOLATION_BYP2;   
    
    wxCheckBox *  chk_delay_HBP;
    wxCheckBox *  chk_delay_HB1;
    wxCheckBox *  chk_delay_HB2;
   
    wxStaticText * txtRX_PHCORR;
    wxStaticText * txtTX_PHCORR;
    wxStaticText * txtTX_GCORR;
    wxStaticText * txtRX_GCORR;
    */
    
    wxButton* btnUpdateAll;
    wxButton* btnLoadSettings;
    wxButton* btnSaveSettings;

   /*
    wxSpinCtrl* spinTX_DCCORRI;
    wxSpinCtrl* spinTX_DCCORRQ;
    wxSpinCtrl* spinTX_GCORRQ;
    wxSpinCtrl* spinTX_GCORRI;
    wxSpinCtrl* spinTX_PHCORR;
    wxSpinCtrl* spinRX_GCORRQ;
    wxSpinCtrl* spinRX_GCORRI;
    wxSpinCtrl* spinRX_PHCORR;
   */ 
    
    wxChoice* cmbInsel;
    wxChoice* cmbPAsrc;

    wxRadioButton* rbChannelA;
    wxRadioButton* rbChannelB;

    wxTextCtrl* txtNcoFreq;

    wxCheckBox * chkBYPASS_CFR;
    wxCheckBox * chkBYPASSGAIN_CFR;
    wxCheckBox * chkSLEEP_CFR;     
    wxCheckBox * chkODD_CFR;
    wxSpinCtrl * spinCFR_ORDER;
    wxSpinCtrlDouble * thresholdSpin;
    wxSpinCtrlDouble * thresholdGain;
    wxChoice* cmbINTER_CFR;


    wxCheckBox * chkBYPASS_CFR_chB;
    wxCheckBox * chkBYPASSGAIN_CFR_chB;
    wxCheckBox * chkSLEEP_CFR_chB;
    wxCheckBox * chkODD_CFR_chB;
    wxSpinCtrl * spinCFR_ORDER_chB;
    wxSpinCtrlDouble * thresholdSpin_chB;
    wxSpinCtrlDouble * thresholdGain_chB;
    wxChoice* cmbINTER_CFR_chB;

    void OnThresholdChanged(wxCommandEvent& event);
    void OnGainChanged(wxCommandEvent& event);
    void OnOrderChanged(wxCommandEvent& event);

    static const long ID_BUTTON_UPDATEALL;
    static const long ID_VCXOCV;

    void OnbtnUpdateAll(wxCommandEvent& event);
    void OnNcoFrequencyChanged(wxCommandEvent& event);

    // added	
    void LoadQSparkSettings(wxCommandEvent& event);
    void SaveQSparkSettings(wxCommandEvent& event);
    //void UpdateDegrees();
    int chA;

protected:
    void OnConfigurePLL(wxCommandEvent &event);
    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);
    void OnSwitchToChannelA(wxCommandEvent& event);
    void OnSwitchToChannelB(wxCommandEvent& event);
    int SPI_write(lms_device_t * lmsControl, uint16_t address, uint16_t data);
    void UpdateHannCoeff2(lms_device_t * lmsControl, uint16_t Filt_N, int chA, int interpolation);
    void UpdateHannCoeff(uint16_t Filt_N);
    void onbtnFIRCoefA( wxCommandEvent& event );
    void onbtnFIRCoefB( wxCommandEvent& event );
    void onbtnFIRCoef(int ch);

    struct Register
    {
        Register();
        Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue, unsigned short twocomplement);
        unsigned short address;
        unsigned char msb;
        unsigned char lsb;
        unsigned short defaultValue;
        unsigned short twocomplement; // added
    };
    std::map<wxObject*, Register> controlsPtr2Registers;
    void RegisterParameterChangeHandler(wxCommandEvent& event);
    void SetRegValue(Register reg, uint16_t newValue);

protected:
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};

#endif
