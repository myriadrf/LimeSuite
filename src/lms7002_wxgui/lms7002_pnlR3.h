#ifndef __lms7002_R3__
#define __lms7002_R3__

#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include "numericSlider.h"

#include <map>
#include <vector>
#include "lime/LimeSuite.h"
namespace lime{

}

class lms7002_pnlR3_view : public wxPanel
{
    protected:
		wxComboBox* cmbFCLK1_DLY;
		wxComboBox* cmbFCLK2_DLY;
		wxComboBox* cmbMCLK1_DLY;
		wxComboBox* cmbMCLK2_DLY;
		wxCheckBox* chkMCLK1_INV;
		wxCheckBox* chkMCLK2_INV;
		wxCheckBox* chkLML1_TRXIQPULSE;
		wxCheckBox* chkLML2_TRXIQPULSE;
		wxCheckBox* chkLML1_SISODDR;
		wxCheckBox* chkLML2_SISODDR;
		wxComboBox* cmbCMIX_GAIN_TXTSP;
		wxComboBox* cmbCMIX_GAIN_RXTSP;
		wxComboBox* cmbHBD_DLY;
		wxComboBox* cmbISINK_SPIBUFF;
		wxCheckBox* chkTRX_GAIN_SRC;
		NumericSlider* spinCG_IAMP_TBB;
		NumericSlider* spinLOSS_LIN_TXPAD_TRF;
		NumericSlider* spinLOSS_MAIN_TXPAD_TRF;
		wxCheckBox* chkR5_LPF_BYP_TBB;
		NumericSlider* spinC_CTL_PGA_RBB;
		wxComboBox* cmbG_PGA_RBB;
		wxComboBox* cmbG_LNA_RFE;
		wxComboBox* cmbG_TIA_RFE;
		wxStaticText* lblADCI_o;
		wxStaticText* lblADCQ_o;
		wxButton* btnReadADC;
		wxStaticText* dccal_statuses[8];
		wxStaticText* dccal_cmpstatuses[8];
		wxCheckBox* dccal_cmpcfg[8];
		wxCheckBox* dccal_start[8];
		wxStaticText* rssi_vals[2];
		wxStaticText* pdet_vals[2];
		wxStaticText* tref_val;
		wxStaticText* tvptat_val;
		wxStaticText* rssidc_cmpstatus;
		std::vector<NumericSlider*> cmbDCControlsRx;
		std::vector<NumericSlider*> cmbDCControlsTx;

        wxStaticText* rssiCMPSTATUS[6];
        wxCheckBox* rssiCMPCFG[6];

		// Virtual event handlers, overide them in your derived class
		void ParameterChangeHandler( wxSpinEvent& event );
        void ParameterChangeHandler( wxCommandEvent& event );
        void OnRSSICMPCFGChanged(wxCommandEvent& event);
        void OnDCCMPCFGChanged(wxCommandEvent& event);
        void OnReadRSSICMP(wxCommandEvent& event);
        void OnReadDCCMP(wxCommandEvent& event);
        void OnReadADC( wxCommandEvent& event );

        void OnWriteRxDC( wxCommandEvent& event );
        void OnWriteTxDC( wxCommandEvent& event );
        void OnDCCMPCFGRead();
        void OnReadDC( wxCommandEvent& event );
        void ParameterChangeHandlerCMPRead( wxCommandEvent& event );
    public:
        lms7002_pnlR3_view( wxWindow* parent );
        lms7002_pnlR3_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
        ~lms7002_pnlR3_view();
        void Initialize(lms_device_t* pControl);
        void UpdateGUI();
protected:
    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;
};

#endif // __lms7002_pnlAFE_view__
