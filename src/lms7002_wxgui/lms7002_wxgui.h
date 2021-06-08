///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep 24 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __LMS7002_WXGUI_H__
#define __LMS7002_WXGUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
class NumericSlider;
class lms7002_pnlAFE_view;
class lms7002_pnlBIAS_view;
class lms7002_pnlBIST_view;
class lms7002_pnlCDS_view;
class lms7002_pnlCLKGEN_view;
class lms7002_pnlCalibrations_view;
class lms7002_pnlGains_view;
class lms7002_pnlLDO_view;
class lms7002_pnlLimeLightPAD_view;
class lms7002_pnlRBB_view;
class lms7002_pnlRFE_view;
class lms7002_pnlRxTSP_view;
class lms7002_pnlSX_view;
class lms7002_pnlTBB_view;
class lms7002_pnlTRF_view;
class lms7002_pnlTxTSP_view;
class lms7002_pnlXBUF_view;

#include <wx/string.h>
#include <wx/button.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/spinctrl.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/gauge.h>
#include <wx/grid.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class mainPanel
///////////////////////////////////////////////////////////////////////////////
class mainPanel : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_G_LNA_RFE = 2048,
			ID_BTN_CH_A,
			ID_BTN_CH_B,
			ID_BTN_CHIP_TO_GUI,
			ID_BTN_RESET_CHIP,
			ID_TABS_NOTEBOOK,
			ID_TAB_CALIBRATIONS,
			ID_TAB_RFE,
			ID_TAB_RBB,
			ID_TAB_TRF,
			ID_TAB_TBB,
			ID_TAB_AFE,
			ID_TAB_BIAS,
			ID_TAB_LDO,
			ID_TAB_XBUF,
			ID_TAB_CGEN,
			ID_TAB_SXR,
			ID_TAB_SXT,
			ID_TAB_LIMELIGHT,
			ID_TAB_TXTSP,
			ID_TAB_RXTSP,
			ID_TAB_CDS,
			ID_TAB_BIST,
			ID_TAB_GAINS
		};
		
		wxButton* ID_BUTTON2;
		wxButton* ID_BUTTON3;
		wxComboBox* cmbLmsDevice;
		wxRadioButton* rbChannelA;
		wxRadioButton* rbChannelB;
		wxCheckBox* chkEnableMIMO;
		wxButton* btnDownloadAll;
		wxButton* btnUploadAll;
		wxButton* btnResetChip;
		wxButton* btnLoadDefault;
		wxStaticText* txtTemperature;
		wxButton* btnReadTemperature;
		wxNotebook* tabsNotebook;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnOpenProject( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveProject( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLmsDeviceSelect( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSwitchToChannelA( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSwitchToChannelB( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEnableMIMOchecked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDownloadAll( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnUploadAll( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnResetChip( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoadDefault( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnReadTemperature( wxCommandEvent& event ) { event.Skip(); }
		virtual void Onnotebook_modulesPageChanged( wxNotebookEvent& event ) { event.Skip(); }
		
	
	public:
		lms7002_pnlCalibrations_view* mTabCalibrations;
		lms7002_pnlRFE_view* mTabRFE;
		lms7002_pnlRBB_view* mTabRBB;
		lms7002_pnlTRF_view* mTabTRF;
		lms7002_pnlTBB_view* mTabTBB;
		lms7002_pnlAFE_view* mTabAFE;
		lms7002_pnlBIAS_view* mTabBIAS;
		lms7002_pnlLDO_view* mTabLDO;
		lms7002_pnlXBUF_view* mTabXBUF;
		lms7002_pnlCLKGEN_view* mTabCGEN;
		lms7002_pnlSX_view* mTabSXR;
		lms7002_pnlSX_view* mTabSXT;
		lms7002_pnlLimeLightPAD_view* mTabLimeLight;
		lms7002_pnlTxTSP_view* mTabTxTSP;
		lms7002_pnlRxTSP_view* mTabRxTSP;
		lms7002_pnlCDS_view* mTabCDS;
		lms7002_pnlBIST_view* mTabBIST;
		lms7002_pnlGains_view* mTabTrxGain;
		
		mainPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~mainPanel();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlRFE_view
///////////////////////////////////////////////////////////////////////////////
class pnlRFE_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_LNA_RFE = 2048,
			ID_PD_RLOOPB_1_RFE,
			ID_PD_RLOOPB_2_RFE,
			ID_PD_MXLOBUF_RFE,
			ID_PD_QGEN_RFE,
			ID_PD_RSSI_RFE,
			ID_PD_TIA_RFE,
			ID_EN_G_RFE,
			ID_EN_DIR_RFE,
			ID_EN_INSHSW_LB1_RFE,
			ID_EN_INSHSW_LB2_RFE,
			ID_EN_INSHSW_L_RFE,
			ID_EN_INSHSW_W_RFE,
			ID_SEL_PATH_RFE,
			ID_CAP_RXMXO_RFE,
			ID_CGSIN_LNA_RFE,
			ID_RCOMP_TIA_RFE,
			ID_RFB_TIA_RFE,
			ID_EN_NEXTRX_RFE,
			ID_ICT_LNACMO_RFE,
			ID_ICT_LNA_RFE,
			ID_DCOFFI_RFE,
			ID_DCOFFQ_RFE,
			ID_ICT_LODC_RFE,
			ID_EN_DCOFF_RXFE_RFE,
			ID_CCOMP_TIA_RFE,
			ID_CFB_TIA_RFE,
			ID_G_LNA_RFE,
			ID_G_RXLOOPB_RFE,
			ID_G_TIA_RFE,
			ID_ICT_LOOPB_RFE,
			ID_ICT_TIAMAIN_RFE,
			ID_ICT_TIAOUT_RFE,
			ID_CDC_I_RFE,
			ID_CDC_Q_RFE
		};
		
		wxCheckBox* chkPD_LNA_RFE;
		wxCheckBox* chkPD_RLOOPB_1_RFE;
		wxCheckBox* chkPD_RLOOPB_2_RFE;
		wxCheckBox* chkPD_MXLOBUF_RFE;
		wxCheckBox* chkPD_QGEN_RFE;
		wxCheckBox* chkPD_RSSI_RFE;
		wxCheckBox* chkPD_TIA_RFE;
		wxCheckBox* chkEN_G_RFE;
		wxCheckBox* chkEN_DIR_RFE;
		wxCheckBox* chkEN_INSHSW_LB1_RFE;
		wxCheckBox* chkEN_INSHSW_LB2_RFE;
		wxCheckBox* chkEN_INSHSW_L_RFE;
		wxCheckBox* chkEN_INSHSW_W_RFE;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbSEL_PATH_RFE;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbCAP_RXMXO_RFE;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbCGSIN_LNA_RFE;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbRCOMP_TIA_RFE;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbRFB_TIA_RFE;
		wxCheckBox* chkEN_NEXTRX_RFE;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbICT_LNACMO_RFE;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbICT_LNA_RFE;
		wxStaticText* ID_STATICTEXT6;
		NumericSlider* cmbDCOFFI_RFE;
		wxStaticText* ID_STATICTEXT7;
		NumericSlider* cmbDCOFFQ_RFE;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbICT_LODC_RFE;
		wxCheckBox* chkEN_DCOFF_RXFE_RFE;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbCCOMP_TIA_RFE;
		wxStaticText* ID_STATICTEXT15;
		NumericSlider* cmbCFB_TIA_RFE;
		wxStaticText* ID_STATICTEXT16;
		wxComboBox* cmbG_LNA_RFE;
		wxStaticText* ID_STATICTEXT17;
		wxComboBox* cmbG_RXLOOPB_RFE;
		wxStaticText* ID_STATICTEXT18;
		wxComboBox* cmbG_TIA_RFE;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbICT_LOOPB_RFE;
		wxStaticText* ID_STATICTEXT10;
		NumericSlider* cmbICT_TIAMAIN_RFE;
		wxStaticText* ID_STATICTEXT11;
		NumericSlider* cmbICT_TIAOUT_RFE;
		wxStaticText* ID_STATICTEXT19;
		wxComboBox* cmbCDC_I_RFE;
		wxStaticText* ID_STATICTEXT20;
		wxComboBox* cmbCDC_Q_RFE;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlRFE_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlRFE_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlRBB_view
///////////////////////////////////////////////////////////////////////////////
class pnlRBB_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_LPFH_RBB = 2048,
			ID_PD_LPFL_RBB,
			ID_PD_PGA_RBB,
			ID_EN_G_RBB,
			ID_EN_DIR_RBB,
			ID_BBLoopback,
			ID_INPUT_CTL_PGA_RBB,
			ID_G_PGA_RBB,
			ID_C_CTL_PGA_RBB,
			ID_OSW_PGA_RBB,
			ID_R_CTL_LPF_RBB,
			ID_C_CTL_LPFH_RBB,
			ID_C_CTL_LPFL_RBB,
			ID_TXT_LOWBW,
			ID_BTN_TUNE_FILTER,
			ID_RCC_CTL_LPFH_RBB,
			ID_RCC_CTL_LPFL_RBB,
			ID_ICT_LPF_IN_RBB,
			ID_ICT_LPF_OUT_RBB,
			ID_ICT_PGA_OUT_RBB,
			ID_ICT_PGA_IN_RBB,
			ID_RCC_CTL_PGA_RBB
		};
		
		wxCheckBox* chkPD_LPFH_RBB;
		wxCheckBox* chkPD_LPFL_RBB;
		wxCheckBox* chkPD_PGA_RBB;
		wxCheckBox* chkEN_G_RBB;
		wxCheckBox* chkEN_DIR_RBB;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbBBLoopback;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbINPUT_CTL_PGA_RBB;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbG_PGA_RBB;
		wxStaticText* ID_STATICTEXT3;
		NumericSlider* cmbC_CTL_PGA_RBB;
		wxRadioBox* rgrOSW_PGA_RBB;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbR_CTL_LPF_RBB;
		wxStaticText* ID_STATICTEXT5;
		NumericSlider* cmbC_CTL_LPFH_RBB;
		wxStaticText* ID_STATICTEXT6;
		NumericSlider* cmbC_CTL_LPFL_RBB;
		wxStaticText* m_staticText309;
		wxTextCtrl* txtLowBW_MHz;
		wxButton* btnTuneFilter;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbRCC_CTL_LPFH_RBB;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbRCC_CTL_LPFL_RBB;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbICT_LPF_IN_RBB;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbICT_LPF_OUT_RBB;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbICT_PGA_OUT_RBB;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbICT_PGA_IN_RBB;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbRCC_CTL_PGA_RBB;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OncmbBBLoopbackSelected( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnbtnTuneFilter( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlRBB_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlRBB_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlTRF_view
///////////////////////////////////////////////////////////////////////////////
class pnlTRF_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_PDET_TRF = 2048,
			ID_PD_TLOBUF_TRF,
			ID_PD_TXPAD_TRF,
			ID_EN_G_TRF,
			ID_EN_DIR_TRF,
			ID_LOADR_PDET_TRF,
			ID_ICT_LIN_TXPAD_TRF,
			ID_ICT_MAIN_TXPAD_TRF,
			ID_CDC_I_TRF,
			ID_CDC_Q_TRF,
			ID_GCAS_GNDREF_TXPAD_TRF,
			ID_EN_NEXTTX_TRF,
			ID_EN_LOOPB_TXPAD_TRF,
			ID_TXFEoutput,
			ID_EN_LOWBWLOMX_TMX_TRF,
			ID_EN_AMPHF_PDET_TRF,
			ID_F_TXPAD_TRF,
			ID_L_LOOPB_TXPAD_TRF,
			ID_LOSS_LIN_TXPAD_TRF,
			ID_LOSS_MAIN_TXPAD_TRF,
			ID_VGCAS_TXPAD_TRF,
			ID_LOBIASN_TXM_TRF,
			ID_LOBIASP_TXX_TRF
		};
		
		wxCheckBox* chkPD_PDET_TRF;
		wxCheckBox* chkPD_TLOBUF_TRF;
		wxCheckBox* chkPD_TXPAD_TRF;
		wxCheckBox* chkEN_G_TRF;
		wxCheckBox* chkEN_DIR_TRF;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbLOADR_PDET_TRF;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbICT_LIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbICT_MAIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbCDC_I_TRF;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbCDC_Q_TRF;
		wxRadioBox* rgrGCAS_GNDREF_TXPAD_TRF;
		wxCheckBox* chkEN_NEXTTX_TRF;
		wxCheckBox* chkEN_LOOPB_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT16;
		wxComboBox* cmbTXFEoutput;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbEN_LOWBWLOMX_TMX_TRF;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbEN_AMPHF_PDET_TRF;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbF_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbL_LOOPB_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT6;
		wxComboBox* cmbLOSS_LIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbLOSS_MAIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbVGCAS_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbLOBIASN_TXM_TRF;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbLOBIASP_TXX_TRF;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBandChange( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlTRF_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlTRF_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlTBB_view
///////////////////////////////////////////////////////////////////////////////
class pnlTBB_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_LPFH_TBB = 2048,
			ID_PD_LPFIAMP_TBB,
			ID_PD_LPFLAD_TBB,
			ID_PD_LPFS5_TBB,
			ID_EN_G_TBB,
			ID_EN_DIR_TBB,
			ID_BYPLADDER_TBB,
			ID_LOOPB_TBB,
			ID_TSTIN_TBB,
			ID_CG_IAMP_TBB,
			ID_ICT_IAMP_FRP_TBB,
			ID_ICT_IAMP_GG_FRP_TBB,
			ID_ICT_LPFS5_F_TBB,
			ID_ICT_LPFS5_PT_TBB,
			ID_ICT_LPF_H_PT_TBB,
			ID_ICT_LPFH_F_TBB,
			ID_ICT_LPFLAD_F_TBB,
			ID_ICT_LPFLAD_PT_TBB,
			ID_RCAL_LPFH_TBB,
			ID_RCAL_LPFLAD_TBB,
			ID_RCAL_LPFS5_TBB,
			ID_CCAL_LPFLAD_TBB,
			ID_BTN_TUNE_FILTER
		};
		
		wxCheckBox* chkPD_LPFH_TBB;
		wxCheckBox* chkPD_LPFIAMP_TBB;
		wxCheckBox* chkPD_LPFLAD_TBB;
		wxCheckBox* chkPD_LPFS5_TBB;
		wxCheckBox* chkEN_G_TBB;
		wxCheckBox* chkEN_DIR_TBB;
		wxCheckBox* chkBYPLADDER_TBB;
		wxCheckBox* chkR5_LPF_BYP_TBB;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbLOOPB_TBB;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbTSTIN_TBB;
		wxStaticText* ID_STATICTEXT2;
		NumericSlider* cmbCG_IAMP_TBB;
		wxStaticText* ID_STATICTEXT3;
		NumericSlider* cmbICT_IAMP_FRP_TBB;
		wxStaticText* ID_STATICTEXT4;
		NumericSlider* cmbICT_IAMP_GG_FRP_TBB;
		wxStaticText* ID_STATICTEXT6;
		wxComboBox* cmbICT_LPFS5_F_TBB;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbICT_LPFS5_PT_TBB;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbICT_LPF_H_PT_TBB;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbICT_LPFH_F_TBB;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbICT_LPFLAD_F_TBB;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbICT_LPFLAD_PT_TBB;
		wxStaticText* ID_STATICTEXT12;
		NumericSlider* cmbRCAL_LPFH_TBB;
		wxStaticText* ID_STATICTEXT13;
		NumericSlider* cmbRCAL_LPFLAD_TBB;
		wxStaticText* ID_STATICTEXT15;
		NumericSlider* cmbRCAL_LPFS5_TBB;
		wxStaticText* ID_STATICTEXT14;
		NumericSlider* cmbCCAL_LPFLAD_TBB;
		wxStaticText* lblFilterInputName;
		wxTextCtrl* txtFilterFrequency;
		wxButton* btnTuneFilter;
		wxButton* btnTuneTxGain;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnbtnTuneFilter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnTuneTxGain( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlTBB_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlTBB_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlAFE_view
///////////////////////////////////////////////////////////////////////////////
class pnlAFE_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_AFE = 2048,
			ID_PD_RX_AFE1,
			ID_PD_RX_AFE2,
			ID_PD_TX_AFE1,
			ID_PD_TX_AFE2,
			ID_EN_G_AFE,
			ID_ISEL_DAC_AFE,
			ID_MUX_AFE_1,
			ID_MUX_AFE_2,
			ID_MODE_INTERLEAVE_AFE
		};
		
		wxCheckBox* chkPD_AFE;
		wxCheckBox* chkPD_RX_AFE1;
		wxCheckBox* chkPD_RX_AFE2;
		wxCheckBox* chkPD_TX_AFE1;
		wxCheckBox* chkPD_TX_AFE2;
		wxCheckBox* chkEN_G_AFE;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbISEL_DAC_AFE;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbMUX_AFE_1;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbMUX_AFE_2;
		wxRadioBox* rgrMODE_INTERLEAVE_AFE;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlAFE_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlAFE_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlBIAS_view
///////////////////////////////////////////////////////////////////////////////
class pnlBIAS_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_FRP_BIAS = 2048,
			ID_PD_F_BIAS,
			ID_PD_PTRP_BIAS,
			ID_PD_PT_BIAS,
			ID_PD_BIAS_MASTER,
			ID_MUX_BIAS_OUT,
			ID_RP_CALIB_BIAS
		};
		
		wxCheckBox* chkPD_FRP_BIAS;
		wxCheckBox* chkPD_F_BIAS;
		wxCheckBox* chkPD_PTRP_BIAS;
		wxCheckBox* chkPD_PT_BIAS;
		wxCheckBox* chkPD_BIAS_MASTER;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbMUX_BIAS_OUT;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbRP_CALIB_BIAS;
		wxButton* btnCalibrateRP_BIAS;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCalibrateRP_BIAS( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlBIAS_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlBIAS_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlLDO_view
///////////////////////////////////////////////////////////////////////////////
class pnlLDO_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_EN_G_LDOP = 2048,
			ID_EN_G_LDO,
			ID_EN_LDO_DIVGN,
			ID_EN_LDO_DIGGN,
			ID_EN_LDO_CPGN,
			ID_EN_LDO_VCOGN,
			ID_EN_LDO_MXRFE,
			ID_EN_LDO_LNA14,
			ID_EN_LDO_LNA12,
			ID_EN_LDO_TIA14,
			ID_EN_LDO_DIVSXT,
			ID_EN_LDO_DIGSXT,
			ID_EN_LDO_CPSXT,
			ID_EN_LDO_VCOSXT,
			ID_EN_LDO_DIVSXR,
			ID_EN_LDO_DIGSXR,
			ID_EN_LDO_CPSXR,
			ID_EN_LDO_VCOSXR,
			ID_PD_LDO_DIGIp1,
			ID_PD_LDO_DIGIp2,
			ID_EN_LDO_TXBUF,
			ID_EN_LDO_RXBUF,
			ID_EN_LDO_TPAD,
			ID_EN_LDO_TLOB,
			ID_PD_LDO_SPIBUF,
			ID_RDIV_TXBUF,
			ID_EN_LDO_AFE,
			ID_EN_LDO_TBB,
			ID_EN_LDO_RBB,
			ID_EN_LDO_TIA12,
			ID_EN_LDO_DIG,
			ID_SPDUP_LDO_TBB,
			ID_SPDUP_LDO_TIA12,
			ID_SPDUP_LDO_TIA14,
			ID_SPDUP_LDO_TLOB,
			ID_SPDUP_LDO_TPAD,
			ID_SPDUP_LDO_TXBUF,
			ID_SPDUP_LDO_VCOGN,
			ID_SPDUP_LDO_DIVSXR,
			ID_SPDUP_LDO_DIVSXT,
			ID_SPDUP_LDO_AFE,
			ID_SPDUP_LDO_CPGN,
			ID_SPDUP_LDO_VCOSXR,
			ID_SPDUP_LDO_VCOSXT,
			ID_SPDUP_LDO_DIG,
			ID_SPDUP_LDO_DIGGN,
			ID_SPDUP_LDO_DIGSXR,
			ID_SPDUP_LDO_DIGSXT,
			ID_SPDUP_LDO_DIVGN,
			ID_SPDUP_LDO_CPSXR,
			ID_SPDUP_LDO_CPSXT,
			ID_SPDUP_LDO_LNA12,
			ID_SPDUP_LDO_LNA14,
			ID_SPDUP_LDO_MXRFE,
			ID_SPDUP_LDO_RBB,
			ID_SPDUP_LDO_RXBUF,
			ID_SPDUP_LDO_SPIBUF,
			ID_SPDUP_LDO_DIGIp2,
			ID_SPDUP_LDO_DIGIp1,
			ID_EN_LOADIMP_LDO_TBB,
			ID_EN_LOADIMP_LDO_TIA12,
			ID_EN_LOADIMP_LDO_TIA14,
			ID_EN_LOADIMP_LDO_TLOB,
			ID_EN_LOADIMP_LDO_TPAD,
			ID_EN_LOADIMP_LDO_TXBUF,
			ID_EN_LOADIMP_LDO_VCOGN,
			ID_EN_LOADIMP_LDO_VCOSXR,
			ID_EN_LOADIMP_LDO_VCOSXT,
			ID_EN_LOADIMP_LDO_AFE,
			ID_EN_LOADIMP_LDO_CPGN,
			ID_EN_LOADIMP_LDO_DIVSXR,
			ID_EN_LOADIMP_LDO_DIVSXT,
			ID_EN_LOADIMP_LDO_DIG,
			ID_EN_LOADIMP_LDO_DIGGN,
			ID_EN_LOADIMP_LDO_DIGSXR,
			ID_EN_LOADIMP_LDO_DIGSXT,
			ID_EN_LOADIMP_LDO_DIVGN,
			ID_EN_LOADIMP_LDO_CPSXR,
			ID_EN_LOADIMP_LDO_CPSXT,
			ID_EN_LOADIMP_LDO_LNA12,
			ID_EN_LOADIMP_LDO_LNA14,
			ID_EN_LOADIMP_LDO_MXRFE,
			ID_EN_LOADIMP_LDO_RBB,
			ID_EN_LOADIMP_LDO_RXBUF,
			ID_EN_LOADIMP_LDO_SPIBUF,
			ID_EN_LOADIMP_LDO_DIGIp2,
			ID_EN_LOADIMP_LDO_DIGIp1,
			ID_RDIV_VCOSXR,
			ID_RDIV_VCOSXT,
			ID_RDIV_VCOGN,
			ID_RDIV_TLOB,
			ID_RDIV_TPAD,
			ID_RDIV_TIA12,
			ID_RDIV_TIA14,
			ID_RDIV_RXBUF,
			ID_RDIV_TBB,
			ID_RDIV_MXRFE,
			ID_RDIV_RBB,
			ID_RDIV_LNA12,
			ID_RDIV_LNA14,
			ID_RDIV_DIVSXR,
			ID_RDIV_DIVSXT,
			ID_RDIV_DIGSXT,
			ID_RDIV_DIVGN,
			ID_RDIV_DIGGN,
			ID_RDIV_DIGSXR,
			ID_RDIV_CPSXT,
			ID_RDIV_DIG,
			ID_RDIV_CPGN,
			ID_RDIV_CPSXR,
			ID_RDIV_SPIBUF,
			ID_RDIV_AFE,
			ID_RDIV_DIGIp2,
			ID_RDIV_DIGIp1
		};
		
		wxNotebook* ID_NOTEBOOK1;
		wxPanel* ID_PANEL3;
		wxCheckBox* chkEN_G_LDOP;
		wxCheckBox* chkEN_G_LDO;
		wxCheckBox* chkEN_LDO_DIVGN;
		wxCheckBox* chkEN_LDO_DIGGN;
		wxCheckBox* chkEN_LDO_CPGN;
		wxCheckBox* chkEN_LDO_VCOGN;
		wxCheckBox* chkEN_LDO_MXRFE;
		wxCheckBox* chkEN_LDO_LNA14;
		wxCheckBox* chkEN_LDO_LNA12;
		wxCheckBox* chkEN_LDO_TIA14;
		wxCheckBox* chkEN_LDO_DIVSXT;
		wxCheckBox* chkEN_LDO_DIGSXT;
		wxCheckBox* chkEN_LDO_CPSXT;
		wxCheckBox* chkEN_LDO_VCOSXT;
		wxCheckBox* chkEN_LDO_DIVSXR;
		wxCheckBox* chkEN_LDO_DIGSXR;
		wxCheckBox* chkEN_LDO_CPSXR;
		wxCheckBox* chkEN_LDO_VCOSXR;
		wxCheckBox* chkPD_LDO_DIGIp1;
		wxCheckBox* chkPD_LDO_DIGIp2;
		wxCheckBox* chkEN_LDO_TXBUF;
		wxCheckBox* chkEN_LDO_RXBUF;
		wxCheckBox* chkEN_LDO_TPAD;
		wxCheckBox* chkEN_LDO_TLOB;
		wxCheckBox* chkPD_LDO_SPIBUF;
		wxStaticText* m_staticText349;
		wxComboBox* cmbISINK_SPIBUFF;
		wxCheckBox* chkEN_LDO_AFE;
		wxCheckBox* chkEN_LDO_TBB;
		wxCheckBox* chkEN_LDO_RBB;
		wxCheckBox* chkEN_LDO_TIA12;
		wxCheckBox* chkEN_LDO_DIG;
		wxCheckBox* chkSPDUP_LDO_TBB;
		wxCheckBox* chkSPDUP_LDO_TIA12;
		wxCheckBox* chkSPDUP_LDO_TIA14;
		wxCheckBox* chkSPDUP_LDO_TLOB;
		wxCheckBox* chkSPDUP_LDO_TPAD;
		wxCheckBox* chkSPDUP_LDO_TXBUF;
		wxCheckBox* chkSPDUP_LDO_VCOGN;
		wxCheckBox* chkSPDUP_LDO_DIVSXR;
		wxCheckBox* chkSPDUP_LDO_DIVSXT;
		wxCheckBox* chkSPDUP_LDO_AFE;
		wxCheckBox* chkSPDUP_LDO_CPGN;
		wxCheckBox* chkSPDUP_LDO_VCOSXR;
		wxCheckBox* chkSPDUP_LDO_VCOSXT;
		wxCheckBox* chkSPDUP_LDO_DIG;
		wxCheckBox* chkSPDUP_LDO_DIGGN;
		wxCheckBox* chkSPDUP_LDO_DIGSXR;
		wxCheckBox* chkSPDUP_LDO_DIGSXT;
		wxCheckBox* chkSPDUP_LDO_DIVGN;
		wxCheckBox* chkSPDUP_LDO_CPSXR;
		wxCheckBox* chkSPDUP_LDO_CPSXT;
		wxCheckBox* chkSPDUP_LDO_LNA12;
		wxCheckBox* chkSPDUP_LDO_LNA14;
		wxCheckBox* chkSPDUP_LDO_MXRFE;
		wxCheckBox* chkSPDUP_LDO_RBB;
		wxCheckBox* chkSPDUP_LDO_RXBUF;
		wxCheckBox* chkSPDUP_LDO_SPIBUF;
		wxCheckBox* chkSPDUP_LDO_DIGIp2;
		wxCheckBox* chkSPDUP_LDO_DIGIp1;
		wxCheckBox* chkEN_LOADIMP_LDO_TBB;
		wxCheckBox* chkEN_LOADIMP_LDO_TIA12;
		wxCheckBox* chkEN_LOADIMP_LDO_TIA14;
		wxCheckBox* chkEN_LOADIMP_LDO_TLOB;
		wxCheckBox* chkEN_LOADIMP_LDO_TPAD;
		wxCheckBox* chkEN_LOADIMP_LDO_TXBUF;
		wxCheckBox* chkEN_LOADIMP_LDO_VCOGN;
		wxCheckBox* chkEN_LOADIMP_LDO_VCOSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_VCOSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_AFE;
		wxCheckBox* chkEN_LOADIMP_LDO_CPGN;
		wxCheckBox* chkEN_LOADIMP_LDO_DIVSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_DIVSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_DIG;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGGN;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_DIVGN;
		wxCheckBox* chkEN_LOADIMP_LDO_CPSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_CPSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_LNA12;
		wxCheckBox* chkEN_LOADIMP_LDO_LNA14;
		wxCheckBox* chkEN_LOADIMP_LDO_MXRFE;
		wxCheckBox* chkEN_LOADIMP_LDO_RBB;
		wxCheckBox* chkEN_LOADIMP_LDO_RXBUF;
		wxCheckBox* chkEN_LOADIMP_LDO_SPIBUF;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGIp2;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGIp1;
		wxPanel* ID_PANEL2;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbRDIV_VCOSXR;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbRDIV_VCOSXT;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbRDIV_TXBUF;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbRDIV_VCOGN;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbRDIV_TLOB;
		wxStaticText* ID_STATICTEXT6;
		wxComboBox* cmbRDIV_TPAD;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbRDIV_TIA12;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbRDIV_TIA14;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbRDIV_RXBUF;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbRDIV_TBB;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbRDIV_MXRFE;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbRDIV_RBB;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbRDIV_LNA12;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbRDIV_LNA14;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbRDIV_DIVSXR;
		wxStaticText* ID_STATICTEXT16;
		wxComboBox* cmbRDIV_DIVSXT;
		wxStaticText* ID_STATICTEXT17;
		wxComboBox* cmbRDIV_DIGSXT;
		wxStaticText* ID_STATICTEXT18;
		wxComboBox* cmbRDIV_DIVGN;
		wxStaticText* ID_STATICTEXT19;
		wxComboBox* cmbRDIV_DIGGN;
		wxStaticText* ID_STATICTEXT20;
		wxComboBox* cmbRDIV_DIGSXR;
		wxStaticText* ID_STATICTEXT21;
		wxComboBox* cmbRDIV_CPSXT;
		wxStaticText* ID_STATICTEXT22;
		wxComboBox* cmbRDIV_DIG;
		wxStaticText* ID_STATICTEXT23;
		wxComboBox* cmbRDIV_CPGN;
		wxStaticText* ID_STATICTEXT24;
		wxComboBox* cmbRDIV_CPSXR;
		wxStaticText* ID_STATICTEXT25;
		wxComboBox* cmbRDIV_SPIBUF;
		wxStaticText* ID_STATICTEXT26;
		wxComboBox* cmbRDIV_AFE;
		wxStaticText* ID_STATICTEXT27;
		wxComboBox* cmbRDIV_DIGIp2;
		wxStaticText* ID_STATICTEXT28;
		wxComboBox* cmbRDIV_DIGIp1;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlLDO_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1180,550 ), long style = wxTAB_TRAVERSAL ); 
		~pnlLDO_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlXBUF_view
///////////////////////////////////////////////////////////////////////////////
class pnlXBUF_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_XBUF_RX = 2048,
			ID_PD_XBUF_TX,
			ID_EN_G_XBUF,
			ID_SLFB_XBUF_RX,
			ID_SLFB_XBUF_TX,
			ID_BYP_XBUF_RX,
			ID_BYP_XBUF_TX,
			ID_EN_OUT2_XBUF_TX,
			ID_EN_TBUFIN_XBUF_RX
		};
		
		wxCheckBox* chkPD_XBUF_RX;
		wxCheckBox* chkPD_XBUF_TX;
		wxCheckBox* chkEN_G_XBUF;
		wxCheckBox* chkSLFB_XBUF_RX;
		wxCheckBox* chkSLFB_XBUF_TX;
		wxCheckBox* chkBYP_XBUF_RX;
		wxCheckBox* chkBYP_XBUF_TX;
		wxCheckBox* chkEN_OUT2_XBUF_TX;
		wxCheckBox* chkEN_TBUFIN_XBUF_RX;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlXBUF_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlXBUF_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlCLKGEN_view
///////////////////////////////////////////////////////////////////////////////
class pnlCLKGEN_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_CP_CGEN = 2048,
			ID_PD_FDIV_FB_CGEN,
			ID_PD_FDIV_O_CGEN,
			ID_PD_SDM_CGEN,
			ID_PD_VCO_CGEN,
			ID_PD_VCO_COMP_CGEN,
			ID_EN_G_CGEN,
			ID_SPDUP_VCO_CGEN,
			ID_RESET_N_CGEN,
			ID_EN_COARSE_CKLGEN,
			ID_EN_INTONLY_SDM_CGEN,
			ID_EN_SDM_CLK_CGEN,
			ID_REV_SDMCLK_CGEN,
			ID_SX_DITHER_EN_CGEN,
			ID_REV_CLKDAC_CGEN,
			ID_REV_CLKADC_CGEN,
			ID_REVPH_PFD_CGEN,
			ID_COARSE_START_CGEN,
			ID_SEL_SDMCLK_CGEN,
			ID_TST_CGEN,
			ID_CP2_CGEN,
			ID_CP3_CGEN,
			ID_CZ_CGEN,
			ID_EN_ADCCLKH_CLKGN,
			ID_CLKH_OV_CLKL_CGEN,
			ID_GFIR1_N_TXTSP,
			ID_AUTO_PHASE,
			ID_INT_SDM_CGEN,
			ID_FRAC_SDM_CGEN,
			ID_VCO_CMPHO_CGEN,
			ID_VCO_CMPLO_CGEN,
			ID_COARSE_STEPDONE_CGEN,
			ID_COARSEPLL_COMPO_CGEN,
			ID_CSW_VCO_CGEN,
			ID_ICT_VCO_CGEN,
			ID_IOFFSET_CP_CGEN,
			ID_IPULSE_CP_CGEN
		};
		
		wxCheckBox* chkPD_CP_CGEN;
		wxCheckBox* chkPD_FDIV_FB_CGEN;
		wxCheckBox* chkPD_FDIV_O_CGEN;
		wxCheckBox* chkPD_SDM_CGEN;
		wxCheckBox* chkPD_VCO_CGEN;
		wxCheckBox* chkPD_VCO_COMP_CGEN;
		wxCheckBox* chkEN_G_CGEN;
		wxCheckBox* chkSPDUP_VCO_CGEN;
		wxCheckBox* chkRESET_N_CGEN;
		wxCheckBox* chkEN_COARSE_CKLGEN;
		wxCheckBox* chkEN_INTONLY_SDM_CGEN;
		wxCheckBox* chkEN_SDM_CLK_CGEN;
		wxCheckBox* chkREV_SDMCLK_CGEN;
		wxCheckBox* chkSX_DITHER_EN_CGEN;
		wxCheckBox* chkREV_CLKDAC_CGEN;
		wxCheckBox* chkREV_CLKADC_CGEN;
		wxCheckBox* chkREVPH_PFD_CGEN;
		wxCheckBox* chkCOARSE_START_CGEN;
		wxRadioBox* rgrSEL_SDMCLK_CGEN;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbTST_CGEN;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbCP2_CGEN;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbCP3_CGEN;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbCZ_CGEN;
		wxRadioBox* cmbEN_ADCCLKH_CLKGN;
		wxStaticText* m_staticText369;
		wxStaticText* lblRxTSPfreq;
		wxStaticText* m_staticText367;
		wxStaticText* lblTxTSPfreq;
		wxStaticText* ID_STATICTEXT10;
		wxStaticText* ID_STATICTEXT18;
		wxStaticText* ID_STATICTEXT3;
		wxTextCtrl* txtFrequency;
		wxComboBox* cmbCLKH_OV_CLKL_CGEN;
		wxTextCtrl* txtFrequencyCLKL;
		wxStaticText* ID_STATICTEXT101;
		wxStaticText* ID_STATICTEXT102;
		wxCheckBox* chkAutoPhase;
		wxButton* btnCalculate;
		wxButton* btnTune;
		wxStaticText* ID_STATICTEXT7;
		wxStaticText* lblINT_SDM_CGEN;
		wxStaticText* ID_STATICTEXT23;
		wxStaticText* lblRefClk_MHz;
		wxStaticText* ID_STATICTEXT21;
		wxStaticText* lblFRAC_SDM_CGEN;
		wxStaticText* ID_STATICTEXT2;
		wxStaticText* lblDivider;
		wxStaticText* ID_STATICTEXT25;
		wxStaticText* lblRealOutFrequency;
		wxStaticText* ID_STATICTEXT161;
		wxStaticText* lblVCO_CMPHO_CGEN;
		wxStaticText* ID_STATICTEXT171;
		wxStaticText* lblVCO_CMPLO_CGEN;
		wxButton* btnUpdateValues1;
		wxStaticText* ID_STATICTEXT5;
		wxStaticText* lblCOARSE_STEPDONE_CGEN;
		wxStaticText* ID_STATICTEXT6;
		wxStaticText* lblCOARSEPLL_COMPO_CGEN;
		wxButton* btnUpdateCoarse;
		wxStaticText* ID_STATICTEXT20;
		NumericSlider* cmbCSW_VCO_CGEN;
		wxStaticText* ID_STATICTEXT9;
		NumericSlider* cmbICT_VCO_CGEN;
		wxButton* btnShowVCO;
		wxStaticText* ID_STATICTEXT16;
		NumericSlider* cmbIOFFSET_CP_CGEN;
		wxStaticText* ID_STATICTEXT17;
		NumericSlider* cmbIPULSE_CP_CGEN;
		wxFlexGridSizer* sizerR3;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnCalculateClick( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnAutoPhase( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnCalculateClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnTuneClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnReadComparators( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnUpdateCoarse( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnShowVCOclicked( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxSpinCtrl* rxPhase;
		wxSpinCtrl* txPhase;
		
		pnlCLKGEN_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlCLKGEN_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlSX_view
///////////////////////////////////////////////////////////////////////////////
class pnlSX_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_PD_FBDIV = 2048,
			ID_PD_LOCH_T2RBUF,
			ID_PD_CP,
			ID_PD_FDIV,
			ID_PD_SDM,
			ID_PD_VCO_COMP,
			ID_PD_VCO,
			ID_EN_G,
			ID_EN_DIR_SXRSXT,
			ID_RESET_N,
			ID_SPDUP_VCO,
			ID_BYPLDO_VCO,
			ID_EN_COARSEPLL,
			ID_CURLIM_VCO,
			ID_REVPH_PFD,
			ID_EN_INTONLY_SDM,
			ID_EN_SDM_CLK,
			ID_REV_SDMCLK,
			ID_SX_DITHER_EN,
			ID_EN_DIV2_DIVPROGenabled,
			ID_TST_SX,
			ID_PW_DIV2_LOCH,
			ID_PW_DIV4_LOCH,
			ID_DIV_LOCH,
			ID_CP2_PLL,
			ID_CP3_PLL,
			ID_CZ,
			ID_SEL_VCO,
			ID_EN_DIV2_DIVPROG,
			ID_VCO_CMPHO,
			ID_VCO_CMPLO,
			ID_CSW_VCO,
			ID_ICT_VCO,
			ID_BTN_SHOW_VCO,
			ID_VDIV_VCO,
			ID_IPULSE_CP,
			ID_SEL_SDMCLK,
			ID_RSEL_LDO_VCO,
			ID_IOFFSET_CP
		};
		
		wxCheckBox* chkPD_FBDIV;
		wxCheckBox* chkPD_LOCH_T2RBUF;
		wxCheckBox* chkPD_CP;
		wxCheckBox* chkPD_FDIV;
		wxCheckBox* chkPD_SDM;
		wxCheckBox* chkPD_VCO_COMP;
		wxCheckBox* chkPD_VCO;
		wxCheckBox* chkEN_G;
		wxCheckBox* chkEN_DIR_SXRSXT;
		wxCheckBox* chkRESET_N;
		wxCheckBox* chkSPDUP_VCO;
		wxCheckBox* chkBYPLDO_VCO;
		wxCheckBox* chkEN_COARSEPLL;
		wxCheckBox* chkCURLIM_VCO;
		wxCheckBox* chkREVPH_PFD;
		wxCheckBox* chkEN_INTONLY_SDM;
		wxCheckBox* chkEN_SDM_CLK;
		wxCheckBox* chkREV_SDMCLK;
		wxCheckBox* chkSX_DITHER_EN;
		wxCheckBox* chkEN_DIV2_DIVPROGenabled;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbTST_SX;
		wxStaticText* ID_STATICTEXT4;
		wxSpinCtrl* ctrPW_DIV2_LOCH;
		wxStaticText* ID_STATICTEXT5;
		wxSpinCtrl* ctrPW_DIV4_LOCH;
		wxStaticText* ID_STATICTEXT6;
		wxSpinCtrl* ctrDIV_LOCH;
		wxStaticText* ID_STATICTEXT18;
		wxComboBox* cmbCP2_PLL;
		wxStaticText* ID_STATICTEXT19;
		wxComboBox* cmbCP3_PLL;
		wxStaticText* ID_STATICTEXT20;
		wxComboBox* cmbCZ;
		wxRadioBox* rgrSEL_VCO;
		wxTextCtrl* txtFrequency;
		wxButton* btnCalculate;
		wxButton* btnTune;
		wxButton* ID_BUTTON5;
		wxPanel* pnlRefClkSpur;
		wxStaticBoxSizer* RefClkSpurSizer;
		wxCheckBox* chkEnableRefSpurCancelation;
		wxStaticText* m_staticText359;
		wxTextCtrl* txtRefSpurBW;
		wxStaticText* ID_STATICTEXT2;
		wxStaticText* lblINT_SDM;
		wxStaticText* ID_STATICTEXT21;
		wxStaticText* lblFRAC_SDM;
		wxStaticText* ID_STATICTEXT25;
		wxStaticText* lblRealOutFrequency;
		wxStaticText* ID_STATICTEXT23;
		wxStaticText* lblDivider;
		wxStaticText* ID_STATICTEXT30;
		wxStaticText* lblEN_DIV2_DIVPROG;
		wxButton* btnChangeRefClk;
		wxStaticText* lblRefClk_MHz;
		wxStaticText* ID_STATICTEXT16;
		wxStaticText* lblVCO_CMPHO;
		wxStaticText* ID_STATICTEXT17;
		wxStaticText* lblVCO_CMPLO;
		wxButton* btnUpdateValues;
		wxStaticText* ID_STATICTEXT1;
		NumericSlider* ctrCSW_VCO;
		wxStaticText* ID_STATICTEXT10;
		NumericSlider* cmbICT_VCO;
		wxButton* btnShowVCOparams;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbVDIV_VCO;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbIPULSE_CP;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbSEL_SDMCLK;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbRSEL_LDO_VCO;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbIOFFSET_CP;
		wxFlexGridSizer* sizerR3;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDIV2PrescalerChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnbtnCalculateClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnTuneClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEnableRefSpurCancelation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnChangeRefClkClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnReadComparators( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowVCOclicked( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlSX_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlSX_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlLimeLightPAD_view
///////////////////////////////////////////////////////////////////////////////
class pnlLimeLightPAD_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_SDA_PE = 2048,
			ID_SCL_PE,
			ID_RX_CLK_PE,
			ID_SDIO_PE,
			ID_SDO_PE,
			ID_TX_CLK_PE,
			ID_SEN_PE,
			ID_DIQ1_PE,
			ID_TXNRX1_PE,
			ID_SCLK_PE,
			ID_DIQ2_PE,
			ID_TXNRX2_PE,
			ID_IQ_SEL_EN_1_PE,
			ID_FCLK1_PE,
			ID_MCLK1_PE,
			ID_IQ_SEL_EN_2_PE,
			ID_FCLK2_PE,
			ID_MCLK2_PE,
			ID_SRST_RXFIFO,
			ID_SRST_TXFIFO,
			ID_LRST_TX_A,
			ID_LRST_TX_B,
			ID_LRST_RX_A,
			ID_LRST_RX_B,
			ID_MRST_TX_A,
			ID_MRST_TX_B,
			ID_MRST_RX_A,
			ID_MRST_RX_B,
			ID_RXEN_A,
			ID_TXEN_A,
			ID_RXEN_B,
			ID_TXEN_B,
			ID_MIMO_SISO,
			ID_MOD_EN,
			ID_LML_FIDM1,
			ID_LML_FIDM2,
			ID_LML_MODE1,
			ID_LML_MODE2,
			ID_LML_TXNRXIQ1,
			ID_LML_TXNRXIQ2,
			ID_SDA_DS,
			ID_SCL_DS,
			ID_SDIO_DS,
			ID_DIQ2_DS,
			ID_DIQ1_DS,
			ID_RX_MUX,
			ID_TX_MUX,
			ID_SPIMODE,
			ID_VER,
			ID_REV,
			ID_MASK,
			ID_LML1_S3S,
			ID_LML1_S2S,
			ID_LML1_S1S,
			ID_LML1_S0S,
			ID_LML2_S3S,
			ID_LML2_S2S,
			ID_LML2_S1S,
			ID_LML2_S0S,
			ID_LML1_BQP,
			ID_LML1_BIP,
			ID_LML1_AQP,
			ID_LML1_AIP,
			ID_LML2_BQP,
			ID_LML2_BIP,
			ID_LML2_AQP,
			ID_LML2_AIP,
			ID_TXRDCLK_MUX,
			ID_TXWRCLK_MUX,
			ID_RXRDCLK_MUX,
			ID_RXWRCLK_MUX,
			ID_MCLK2SRC,
			ID_MCLK1SRC,
			ID_TXDIVEN,
			ID_TXTSPCLKA_DIV,
			ID_RXDIVEN,
			ID_RXTSPCLKA_DIV,
			ID_MCLK1DLY,
			ID_MCLK2DLY,
			ID_FCLK1_INV,
			ID_FCLK2_INV,
			ID_FCLK1DLY,
			ID_FCLK2DLY,
			ID_LML1_TX_PST,
			ID_LML1_TX_PRE,
			ID_LML1_RX_PST,
			ID_LML1_RX_PRE,
			ID_LML2_TX_PST,
			ID_LML2_TX_PRE,
			ID_LML2_RX_PST,
			ID_LML2_RX_PRE,
			ID_DIQDIRCTR1,
			ID_DIQDIR1,
			ID_ENABLEDIRCTR1,
			ID_ENABLEDIR1,
			ID_DIQDIRCTR2,
			ID_DIQDIR2,
			ID_ENABLEDIRCTR2,
			ID_ENABLEDIR2
		};
		
		wxNotebook* ID_NOTEBOOK1;
		wxPanel* ID_PANEL1;
		wxCheckBox* chkSDA_PE;
		wxCheckBox* chkSCL_PE;
		wxCheckBox* chkRX_CLK_PE;
		wxCheckBox* chkSDIO_PE;
		wxCheckBox* chkSDO_PE;
		wxCheckBox* chkTX_CLK_PE;
		wxCheckBox* chkSEN_PE;
		wxCheckBox* chkDIQ1_PE;
		wxCheckBox* chkTXNRX1_PE;
		wxCheckBox* chkSCLK_PE;
		wxCheckBox* chkDIQ2_PE;
		wxCheckBox* chkTXNRX2_PE;
		wxCheckBox* chkIQ_SEL_EN_1_PE;
		wxCheckBox* chkFCLK1_PE;
		wxCheckBox* chkMCLK1_PE;
		wxCheckBox* chkIQ_SEL_EN_2_PE;
		wxCheckBox* chkFCLK2_PE;
		wxCheckBox* chkMCLK2_PE;
		wxCheckBox* chkSRST_RXFIFO;
		wxCheckBox* chkSRST_TXFIFO;
		wxCheckBox* chkLRST_TX_A;
		wxCheckBox* chkLRST_TX_B;
		wxCheckBox* chkLRST_RX_A;
		wxCheckBox* chkLRST_RX_B;
		wxCheckBox* chkMRST_TX_A;
		wxCheckBox* chkMRST_TX_B;
		wxCheckBox* chkMRST_RX_A;
		wxCheckBox* chkMRST_RX_B;
		wxCheckBox* chkRXEN_A;
		wxCheckBox* chkTXEN_A;
		wxCheckBox* chkRXEN_B;
		wxCheckBox* chkTXEN_B;
		wxCheckBox* chkMIMO_SISO;
		wxCheckBox* chkMOD_EN;
		wxRadioBox* rgrLML_FIDM1;
		wxRadioBox* rgrLML_FIDM2;
		wxRadioBox* rgrLML_MODE1;
		wxRadioBox* rgrLML_MODE2;
		wxRadioBox* rgrLML_TXNRXIQ1;
		wxRadioBox* rgrLML_TXNRXIQ2;
		wxCheckBox* chkLML1_TRXIQPULSE;
		wxCheckBox* chkLML2_TRXIQPULSE;
		wxCheckBox* chkLML1_SISODDR;
		wxCheckBox* chkLML2_SISODDR;
		wxRadioBox* rgrSDA_DS;
		wxRadioBox* rgrSCL_DS;
		wxRadioBox* rgrSDIO_DS;
		wxRadioBox* rgrDIQ2_DS;
		wxRadioBox* rgrDIQ1_DS;
		wxRadioBox* rgrRX_MUX;
		wxRadioBox* rgrTX_MUX;
		wxRadioBox* rgrSPIMODE;
		wxStaticText* ID_STATICTEXT36;
		wxStaticText* lblVER;
		wxStaticText* ID_STATICTEXT38;
		wxStaticText* lblREV;
		wxStaticText* ID_STATICTEXT40;
		wxStaticText* lblMASK;
		wxButton* btnReadVerRevMask;
		wxPanel* ID_PANEL2;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbLML1_S3S;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbLML1_S2S;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbLML1_S1S;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbLML1_S0S;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbLML2_S3S;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbLML2_S2S;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbLML2_S1S;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbLML2_S0S;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbLML1_BQP;
		wxStaticText* ID_STATICTEXT6;
		wxComboBox* cmbLML1_BIP;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbLML1_AQP;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbLML1_AIP;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbLML2_BQP;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbLML2_BIP;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbLML2_AQP;
		wxStaticText* ID_STATICTEXT16;
		wxComboBox* cmbLML2_AIP;
		wxStaticText* ID_STATICTEXT25;
		wxComboBox* cmbTXRDCLK_MUX;
		wxStaticText* ID_STATICTEXT26;
		wxComboBox* cmbTXWRCLK_MUX;
		wxStaticText* ID_STATICTEXT27;
		wxComboBox* cmbRXRDCLK_MUX;
		wxStaticText* ID_STATICTEXT28;
		wxComboBox* cmbRXWRCLK_MUX;
		wxStaticText* ID_STATICTEXT29;
		wxComboBox* cmbMCLK2SRC;
		wxStaticText* ID_STATICTEXT30;
		wxComboBox* cmbMCLK1SRC;
		wxCheckBox* chkTXDIVEN;
		wxSpinCtrl* cmbTXTSPCLKA_DIV;
		wxCheckBox* chkRXDIVEN;
		wxSpinCtrl* cmbRXTSPCLKA_DIV;
		wxStaticText* ID_STATICTEXT37;
		wxSpinCtrl* cmbMCLK1DLY;
		wxStaticText* ID_STATICTEXT39;
		wxSpinCtrl* cmbMCLK2DLY;
		wxCheckBox* chkFCLK1_INV;
		wxCheckBox* chkFCLK2_INV;
		wxCheckBox* chkMCLK1_INV;
		wxCheckBox* chkMCLK2_INV;
		wxStaticText* ID_STATICTEXT301;
		wxComboBox* cmbFCLK1_DLY;
		wxStaticText* ID_STATICTEXT302;
		wxComboBox* cmbFCLK2_DLY;
		wxStaticText* ID_STATICTEXT303;
		wxComboBox* cmbMCLK1_DLY;
		wxStaticText* ID_STATICTEXT304;
		wxComboBox* cmbMCLK2_DLY;
		wxStaticText* ID_STATICTEXT17;
		wxSpinCtrl* cmbLML1_TX_PST;
		wxStaticText* ID_STATICTEXT18;
		wxSpinCtrl* cmbLML1_TX_PRE;
		wxStaticText* ID_STATICTEXT19;
		wxSpinCtrl* cmbLML1_RX_PST;
		wxStaticText* ID_STATICTEXT20;
		wxSpinCtrl* cmbLML1_RX_PRE;
		wxStaticText* ID_STATICTEXT21;
		wxSpinCtrl* cmbLML2_TX_PST;
		wxStaticText* ID_STATICTEXT22;
		wxSpinCtrl* cmbLML2_TX_PRE;
		wxStaticText* ID_STATICTEXT23;
		wxSpinCtrl* cmbLML2_RX_PST;
		wxStaticText* ID_STATICTEXT24;
		wxSpinCtrl* cmbLML2_RX_PRE;
		wxRadioBox* rgrDIQDIRCTR1;
		wxRadioBox* rgrDIQDIR1;
		wxRadioBox* rgrENABLEDIRCTR1;
		wxRadioBox* rgrENABLEDIR1;
		wxRadioBox* rgrDIQDIRCTR2;
		wxRadioBox* rgrDIQDIR2;
		wxRadioBox* rgrENABLEDIRCTR2;
		wxRadioBox* rgrENABLEDIR2;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnReadVerRevMask( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlLimeLightPAD_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlLimeLightPAD_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlTxTSP_view
///////////////////////////////////////////////////////////////////////////////
class pnlTxTSP_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_EN_TXTSP = 2048,
			ID_DC_BYP_TXTSP,
			ID_GC_BYP_TXTSP,
			ID_PH_BYP_TXTSP,
			ID_CMIX_BYP_TXTSP,
			ID_ISINC_BYP_TXTSP,
			ID_GFIR1_BYP_TXTSP,
			ID_GFIR2_BYP_TXTSP,
			ID_GFIR3_BYP_TXTSP,
			ID_BSTART_TXTSP,
			ID_BSTATE_TXTSP,
			ID_BSIGI_TXTSP,
			ID_BSIGQ_TXTSP,
			ID_MODE_TX,
			ID_DTHBIT_TX,
			ID_TSGSWAPIQ_TXTSP,
			ID_TSGFCW_TXTSP,
			ID_TSGMODE_TXTSP,
			ID_INSEL_TXTSP,
			ID_TSGFC_TXTSP,
			ID_DC_REG_TXTSP,
			ID_CMIX_SC_TXTSP,
			ID_CMIX_GAIN_TXTSP,
			ID_HBI_OVR_TXTSP,
			ID_GFIR1_L_TXTSP,
			ID_GFIR1_N_TXTSP,
			ID_GFIR2_L_TXTSP,
			ID_GFIR2_N_TXTSP,
			ID_GFIR3_L_TXTSP,
			ID_GFIR3_N_TXTSP,
			ID_IQCORR_TXTSP,
			ID_GCORRI_TXTSP,
			ID_GCORRQ_TXTSP,
			ID_DCCORRI_TXTSP,
			ID_DCCORRQ_TXTSP
		};
		
		wxCheckBox* chkEN_TXTSP;
		wxCheckBox* chkDC_BYP_TXTSP;
		wxCheckBox* chkGC_BYP_TXTSP;
		wxCheckBox* chkPH_BYP_TXTSP;
		wxCheckBox* chkCMIX_BYP_TXTSP;
		wxCheckBox* chkISINC_BYP_TXTSP;
		wxCheckBox* chkGFIR1_BYP_TXTSP;
		wxCheckBox* chkGFIR2_BYP_TXTSP;
		wxCheckBox* chkGFIR3_BYP_TXTSP;
		wxCheckBox* chkBSTART_TXTSP;
		wxStaticText* ID_STATICTEXT2;
		wxStaticText* lblBSTATE_TXTSP;
		wxStaticText* ID_STATICTEXT1;
		wxStaticText* lblBSIGI_TXTSP;
		wxStaticText* ID_STATICTEXT3;
		wxStaticText* lblBSIGQ_TXTSP;
		wxButton* btnReadBIST;
		wxFlexGridSizer* sizerNCOgrid;
		wxStaticText* tableTitleCol1;
		wxStaticText* tableTitleCol2;
		wxRadioButton* rgrSEL0;
		wxTextCtrl* txtFCWPHO0;
		wxStaticText* txtAnglePHO0;
		wxRadioButton* rgrSEL01;
		wxTextCtrl* txtFCWPHO01;
		wxStaticText* txtAnglePHO01;
		wxRadioButton* rgrSEL02;
		wxTextCtrl* txtFCWPHO02;
		wxStaticText* txtAnglePHO02;
		wxRadioButton* rgrSEL03;
		wxTextCtrl* txtFCWPHO03;
		wxStaticText* txtAnglePHO03;
		wxRadioButton* rgrSEL04;
		wxTextCtrl* txtFCWPHO04;
		wxStaticText* txtAnglePHO04;
		wxRadioButton* rgrSEL05;
		wxTextCtrl* txtFCWPHO05;
		wxStaticText* txtAnglePHO05;
		wxRadioButton* rgrSEL06;
		wxTextCtrl* txtFCWPHO06;
		wxStaticText* txtAnglePHO06;
		wxRadioButton* rgrSEL07;
		wxTextCtrl* txtFCWPHO07;
		wxStaticText* txtAnglePHO07;
		wxRadioButton* rgrSEL08;
		wxTextCtrl* txtFCWPHO08;
		wxStaticText* txtAnglePHO08;
		wxRadioButton* rgrSEL09;
		wxTextCtrl* txtFCWPHO09;
		wxStaticText* txtAnglePHO09;
		wxRadioButton* rgrSEL10;
		wxTextCtrl* txtFCWPHO10;
		wxStaticText* txtAnglePHO10;
		wxRadioButton* rgrSEL11;
		wxTextCtrl* txtFCWPHO11;
		wxStaticText* txtAnglePHO11;
		wxRadioButton* rgrSEL12;
		wxTextCtrl* txtFCWPHO12;
		wxStaticText* txtAnglePHO12;
		wxRadioButton* rgrSEL13;
		wxTextCtrl* txtFCWPHO13;
		wxStaticText* txtAnglePHO13;
		wxRadioButton* rgrSEL14;
		wxTextCtrl* txtFCWPHO14;
		wxStaticText* txtAnglePHO14;
		wxRadioButton* rgrSEL15;
		wxTextCtrl* txtFCWPHO15;
		wxStaticText* txtAnglePHO15;
		wxStaticText* ID_STATICTEXT21;
		wxStaticText* lblRefClk;
		wxButton* btnUploadNCO;
		wxRadioBox* rgrMODE_TX;
		wxStaticText* lblFCWPHOmodeName;
		wxTextCtrl* txtFCWPHOmodeAdditional;
		wxStaticText* ID_STATICTEXT25;
		wxComboBox* cmbDTHBIT_TX;
		wxCheckBox* chkTSGSWAPIQ_TXTSP;
		wxRadioBox* rgrTSGFCW_TXTSP;
		wxRadioBox* rgrTSGMODE_TXTSP;
		wxRadioBox* rgrINSEL_TXTSP;
		wxRadioBox* rgrTSGFC_TXTSP;
		wxStaticText* ID_STATICTEXT23;
		wxTextCtrl* txtDC_REG_TXTSP;
		wxButton* btnLoadDCI;
		wxButton* btnLoadDCQ;
		wxComboBox* cmbCMIX_SC_TXTSP;
		wxStaticText* ID_STATICTEXT24;
		wxComboBox* cmbCMIX_GAIN_TXTSP;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbHBI_OVR_TXTSP;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbGFIR1_L_TXTSP;
		wxStaticText* ID_STATICTEXT11;
		wxSpinCtrl* cmbGFIR1_N_TXTSP;
		wxButton* btnGFIR1Coef;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbGFIR2_L_TXTSP;
		wxStaticText* ID_STATICTEXT13;
		wxSpinCtrl* cmbGFIR2_N_TXTSP;
		wxButton* btnGFIR2Coef;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbGFIR3_L_TXTSP;
		wxStaticText* ID_STATICTEXT15;
		wxSpinCtrl* cmbGFIR3_N_TXTSP;
		wxButton* btnGFIR3Coef;
		wxStaticText* txtBW;
		wxTextCtrl* txtLPFBW;
		wxButton* btnSetLPF;
		wxStaticText* txtRATE;
		wxStaticText* txtRATEVAL;
		NumericSlider* cmbIQCORR_TXTSP;
		wxStaticText* ID_STATICTEXT16;
		wxStaticText* txtPhaseAlpha;
		wxStaticText* ID_STATICTEXT5;
		NumericSlider* cmbGCORRI_TXTSP;
		wxStaticText* ID_STATICTEXT4;
		NumericSlider* cmbGCORRQ_TXTSP;
		wxButton* ID_BUTTON10;
		wxStaticText* ID_STATICTEXT8;
		NumericSlider* cmbDCCORRI_TXTSP;
		wxStaticText* ID_STATICTEXT9;
		NumericSlider* cmbDCCORRQ_TXTSP;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnReadBISTSignature( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNCOSelectionChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void PHOinputChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnUploadNCOClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void txtFCWPHOmodeAdditional_OnMouseWheel( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnbtnLoadDCIClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnLoadDCQClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void onbtnGFIR1Coef( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnGFIR2Coef( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnGFIR3Coef( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnSetLPFClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlTxTSP_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlTxTSP_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlRxTSP_view
///////////////////////////////////////////////////////////////////////////////
class pnlRxTSP_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_EN_RXTSP = 2048,
			ID_DC_BYP_RXTSP,
			ID_GC_BYP_RXTSP,
			ID_PH_BYP_RXTSP,
			ID_CMIX_BYP_RXTSP,
			ID_AGC_BYP_RXTSP,
			ID_GFIR1_BYP_RXTSP,
			ID_GFIR2_BYP_RXTSP,
			ID_GFIR3_BYP_RXTSP,
			ID_BSTART_RXTSP,
			ID_MODE_RX,
			ID_DTHBIT_RX,
			ID_TSGSWAPIQ_RXTSP,
			ID_TSGFCW_RXTSP,
			ID_TSGMODE_RXTSP,
			ID_INSEL_RXTSP,
			ID_TSGFC_RXTSP,
			ID_DCCORR_AVG,
			ID_DC_REG_RXTSP,
			ID_CMIX_SC_RXTSP,
			ID_CMIX_GAIN_RXTSP,
			ID_HBD_OVR_RXTSP,
			ID_GFIR1_L_RXTSP,
			ID_GFIR1_N_RXTSP,
			ID_GFIR2_L_RXTSP,
			ID_GFIR2_N_RXTSP,
			ID_GFIR3_L_RXTSP,
			ID_GFIR3_N_RXTSP,
			ID_IQCORR_RXTSP,
			ID_GCORRI_RXTSP,
			ID_GCORRQ_RXTSP,
			ID_AGC_MODE_RXTSP,
			ID_AGC_AVG_RXTSP,
			ID_AGC_K_RXTSP,
			ID_AGC_ADESIRED_RXTSP
		};
		
		wxCheckBox* chkEN_RXTSP;
		wxCheckBox* chkDC_BYP_RXTSP;
		wxCheckBox* chkDC_LOOP_RXTSP;
		wxCheckBox* chkGC_BYP_RXTSP;
		wxCheckBox* chkPH_BYP_RXTSP;
		wxCheckBox* chkCMIX_BYP_RXTSP;
		wxCheckBox* chkAGC_BYP_RXTSP;
		wxCheckBox* chkGFIR1_BYP_RXTSP;
		wxCheckBox* chkGFIR2_BYP_RXTSP;
		wxCheckBox* chkGFIR3_BYP_RXTSP;
		wxStaticText* ID_STATICTEXT29;
		wxStaticText* lblBISTI;
		wxStaticText* ID_STATICTEXT30;
		wxStaticText* lblBSTATE_I;
		wxStaticText* ID_STATICTEXT31;
		wxStaticText* lblBISTQ;
		wxStaticText* ID_STATICTEXT32;
		wxStaticText* lblBSTATE_Q;
		wxCheckBox* chkBSTART_RXTSP;
		wxButton* btnReadBIST;
		wxStaticText* ID_STATICTEXT71;
		wxStaticText* lblADCI;
		wxStaticText* ID_STATICTEXT28;
		wxStaticText* lblADCQ;
		wxStaticText* ID_STATICTEXT51;
		wxStaticText* lblRSSI;
		wxCheckBox* chkCAPSEL_ADC_RXTSP;
		wxButton* ID_BUTTON1;
		wxFlexGridSizer* sizerNCOgrid;
		wxStaticText* tableTitleCol1;
		wxStaticText* tableTitleCol2;
		wxRadioButton* rgrSEL0;
		wxTextCtrl* txtFCWPHO0;
		wxStaticText* txtAnglePHO0;
		wxRadioButton* rgrSEL01;
		wxTextCtrl* txtFCWPHO01;
		wxStaticText* txtAnglePHO01;
		wxRadioButton* rgrSEL02;
		wxTextCtrl* txtFCWPHO02;
		wxStaticText* txtAnglePHO02;
		wxRadioButton* rgrSEL03;
		wxTextCtrl* txtFCWPHO03;
		wxStaticText* txtAnglePHO03;
		wxRadioButton* rgrSEL04;
		wxTextCtrl* txtFCWPHO04;
		wxStaticText* txtAnglePHO04;
		wxRadioButton* rgrSEL05;
		wxTextCtrl* txtFCWPHO05;
		wxStaticText* txtAnglePHO05;
		wxRadioButton* rgrSEL06;
		wxTextCtrl* txtFCWPHO06;
		wxStaticText* txtAnglePHO06;
		wxRadioButton* rgrSEL07;
		wxTextCtrl* txtFCWPHO07;
		wxStaticText* txtAnglePHO07;
		wxRadioButton* rgrSEL08;
		wxTextCtrl* txtFCWPHO08;
		wxStaticText* txtAnglePHO08;
		wxRadioButton* rgrSEL09;
		wxTextCtrl* txtFCWPHO09;
		wxStaticText* txtAnglePHO09;
		wxRadioButton* rgrSEL10;
		wxTextCtrl* txtFCWPHO10;
		wxStaticText* txtAnglePHO10;
		wxRadioButton* rgrSEL11;
		wxTextCtrl* txtFCWPHO11;
		wxStaticText* txtAnglePHO11;
		wxRadioButton* rgrSEL12;
		wxTextCtrl* txtFCWPHO12;
		wxStaticText* txtAnglePHO12;
		wxRadioButton* rgrSEL13;
		wxTextCtrl* txtFCWPHO13;
		wxStaticText* txtAnglePHO13;
		wxRadioButton* rgrSEL14;
		wxTextCtrl* txtFCWPHO14;
		wxStaticText* txtAnglePHO14;
		wxRadioButton* rgrSEL15;
		wxTextCtrl* txtFCWPHO15;
		wxStaticText* txtAnglePHO15;
		wxStaticText* ID_STATICTEXT21;
		wxStaticText* lblRefClk;
		wxButton* btnUploadNCO;
		wxRadioBox* rgrMODE_RX;
		wxStaticText* lblFCWPHOmodeName;
		wxTextCtrl* txtFCWPHOmodeAdditional;
		wxStaticText* ID_STATICTEXT25;
		wxComboBox* cmbDTHBIT_RX;
		wxCheckBox* chkTSGSWAPIQ_RXTSP;
		wxRadioBox* rgrTSGFCW_RXTSP;
		wxRadioBox* rgrTSGMODE_RXTSP;
		wxRadioBox* rgrINSEL_RXTSP;
		wxRadioBox* rgrTSGFC_RXTSP;
		wxComboBox* cmbDCCORR_AVG;
		wxStaticText* ID_STATICTEXT23;
		wxTextCtrl* txtDC_REG_RXTSP;
		wxButton* btnLoadDCI;
		wxButton* btnLoadDCQ;
		wxComboBox* cmbCMIX_SC_RXTSP;
		wxStaticText* ID_STATICTEXT24;
		wxComboBox* cmbCMIX_GAIN_RXTSP;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbHBD_OVR_RXTSP;
		wxStaticText* ID_STATICTEXT72;
		wxComboBox* cmbHBD_DLY;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbGFIR1_L_RXTSP;
		wxStaticText* ID_STATICTEXT11;
		wxSpinCtrl* cmbGFIR1_N_RXTSP;
		wxButton* btnGFIR1Coef;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbGFIR2_L_RXTSP;
		wxStaticText* ID_STATICTEXT13;
		wxSpinCtrl* cmbGFIR2_N_RXTSP;
		wxButton* btnGFIR2Coef;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbGFIR3_L_RXTSP;
		wxStaticText* ID_STATICTEXT15;
		wxSpinCtrl* cmbGFIR3_N_RXTSP;
		wxButton* btnGFIR3Coef;
		wxStaticText* txtBW;
		wxTextCtrl* txtLPFBW;
		wxButton* btnSetLPF;
		wxStaticText* txtRate;
		wxStaticText* txtRateVal;
		NumericSlider* cmbIQCORR_RXTSP;
		wxStaticText* ID_STATICTEXT16;
		wxStaticText* txtPhaseAlpha;
		wxStaticText* ID_STATICTEXT5;
		NumericSlider* cmbGCORRI_RXTSP;
		wxStaticText* ID_STATICTEXT4;
		NumericSlider* cmbGCORRQ_RXTSP;
		wxButton* ID_BUTTON10;
		wxStaticText* ID_STATICTEXT91;
		wxComboBox* cmbAGC_MODE_RXTSP;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbAGC_AVG_RXTSP;
		wxStaticText* ID_STATICTEXT1;
		NumericSlider* spinAGC_K_RXTSP;
		wxStaticText* ID_STATICTEXT2;
		NumericSlider* cmbAGC_ADESIRED_RXTSP;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnReadBISTSignature( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnReadRSSI( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNCOSelectionChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void PHOinputChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnUploadNCOClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void txtFCWPHOmodeAdditional_OnMouseWheel( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnbtnLoadDCIClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnLoadDCQClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void onbtnGFIR1Coef( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnGFIR2Coef( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnGFIR3Coef( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnSetLPFClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlRxTSP_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1145,536 ), long style = wxTAB_TRAVERSAL ); 
		~pnlRxTSP_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlCDS_view
///////////////////////////////////////////////////////////////////////////////
class pnlCDS_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_CDSN_TXBTSP = 2048,
			ID_CDSN_TXATSP,
			ID_CDSN_RXBTSP,
			ID_CDSN_RXATSP,
			ID_CDSN_TXBLML,
			ID_CDSN_TXALML,
			ID_CDSN_RXBLML,
			ID_CDSN_RXALML ,
			ID_CDSN_MCLK2,
			ID_CDSN_MCLK1,
			ID_CDS_MCLK2,
			ID_CDS_MCLK1,
			ID_CDS_TXBTSP,
			ID_CDS_TXATSP,
			ID_CDS_RXBTSP,
			ID_CDS_RXATSP,
			ID_CDS_TXBLML,
			ID_CDS_TXALML,
			ID_CDS_RXBLML,
			ID_CDS_RXALML
		};
		
		wxCheckBox* chkCDSN_TXBTSP;
		wxCheckBox* chkCDSN_TXATSP;
		wxCheckBox* chkCDSN_RXBTSP;
		wxCheckBox* chkCDSN_RXATSP;
		wxCheckBox* chkCDSN_TXBLML;
		wxCheckBox* chkCDSN_TXALML;
		wxCheckBox* chkCDSN_RXBLML;
		wxCheckBox* chkCDSN_RXALML ;
		wxCheckBox* chkCDSN_MCLK2;
		wxCheckBox* chkCDSN_MCLK1;
		wxRadioBox* rgrCDS_MCLK2;
		wxRadioBox* rgrCDS_MCLK1;
		wxRadioBox* rgrCDS_TXBTSP;
		wxRadioBox* rgrCDS_TXATSP;
		wxRadioBox* rgrCDS_RXBTSP;
		wxRadioBox* rgrCDS_RXATSP;
		wxRadioBox* rgrCDS_TXBLML;
		wxRadioBox* rgrCDS_TXALML;
		wxRadioBox* rgrCDS_RXBLML;
		wxRadioBox* rgrCDS_RXALML;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlCDS_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlCDS_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlBIST_view
///////////////////////////////////////////////////////////////////////////////
class pnlBIST_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_BENC = 2048,
			ID_SDM_TSTO_CGEN,
			ID_BENR,
			ID_SDM_TSTO_SXR,
			ID_BENT,
			ID_SDM_TSTO_SXT,
			ID_BSTART,
			ID_BSTATE,
			ID_BSIGT,
			ID_BSIGR,
			ID_BSIGC,
			ID_BTN_READ_SIGNATURE
		};
		
		wxCheckBox* chkBENC;
		wxCheckBox* chkSDM_TSTO_CGEN;
		wxCheckBox* chkBENR;
		wxCheckBox* chkSDM_TSTO_SXR;
		wxCheckBox* chkBENT;
		wxCheckBox* chkSDM_TSTO_SXT;
		wxCheckBox* chkBSTART;
		wxStaticText* ID_STATICTEXT2;
		wxStaticText* lblBSTATE;
		wxStaticText* ID_STATICTEXT1;
		wxStaticText* lblBSIGT;
		wxStaticText* ID_STATICTEXT3;
		wxStaticText* lblBSIGR;
		wxStaticText* ID_STATICTEXT4;
		wxStaticText* lblBSIGC;
		wxButton* btnReadSignature;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void onbtnReadSignature( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlBIST_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~pnlBIST_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlMCU_BD_view
///////////////////////////////////////////////////////////////////////////////
class pnlMCU_BD_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_CHECKBOX_RESETMCU = 2048,
			ID_TESTNO,
			ID_DEBUGMODE,
			ID_RUNINSTR,
			ID_INSTRNO,
			ID_RESETPC,
			ID_SELDIV,
			ID_RADIOBUTTON4,
			ID_RADIOBUTTON5
		};
		
		wxStaticText* ID_STATICTEXT1;
		wxButton* Button_LOADHEX;
		wxCheckBox* chkReset;
		wxRadioBox* rgrMode;
		wxButton* btnStartProgramming;
		wxStaticText* ID_S_PROGFINISHED;
		wxStaticText* ID_STATICTEXT3;
		wxButton* btnRunTest;
		wxStaticText* ID_STATICTEXT4;
		wxTextCtrl* m_sTestNo;
		wxButton* btnRunProductionTest;
		wxCheckBox* DebugMode;
		wxButton* RunInstr;
		wxStaticText* ID_STATICTEXT6;
		wxTextCtrl* InstrNo;
		wxButton* ResetPC;
		wxStaticText* ID_STATICTEXT7;
		wxStaticText* PCValue;
		wxButton* ViewSFRs;
		wxButton* ViewIRAM;
		wxButton* EraseIRAM;
		wxStaticText* ID_STATICTEXT10;
		wxChoice* SelDiv;
		wxRadioButton* m_cCtrlBaseband;
		wxRadioButton* m_cCtrlMCU_BD;
		wxStaticText* ID_STATICTEXT5;
		wxChoice* cmbRegAddr;
		wxRadioButton* rbtnRegWrite;
		wxStaticText* ID_STATICTEXT11;
		wxTextCtrl* txtRegValueWr;
		wxRadioButton* rbtnRegRead;
		wxStaticText* ReadResult;
		wxButton* btnRdWr;
		wxStaticText* ID_STATICTEXT2;
		wxGauge* progressBar;
		wxStaticText* lblProgCodeFile;
		wxStaticText* lblTestResultsFile;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnButton_LOADHexClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnchkResetClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnStartProgrammingClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnRunTestClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnRunProductionTestClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDebugModeClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRunInstruction( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnResetPCClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnViewSFRsClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnViewIRAMClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEraseIRAMClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelDivSelect( wxCommandEvent& event ) { event.Skip(); }
		virtual void Onm_cCtrlBasebandSelect( wxCommandEvent& event ) { event.Skip(); }
		virtual void Onm_cCtrlMCU_BDSelect( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRegWriteRead( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlMCU_BD_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~pnlMCU_BD_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlCalibrations_view
///////////////////////////////////////////////////////////////////////////////
class pnlCalibrations_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_GCORRI_RXTSP = 2048,
			ID_GCORRQ_RXTSP,
			ID_IQCORR_RXTSP,
			ID_DCOFFI_RFE,
			ID_DCOFFQ_RFE,
			ID_EN_DCOFF_RXFE_RFE,
			ID_DCMODE,
			ID_GCORRI_TXTSP,
			ID_GCORRQ_TXTSP,
			ID_IQCORR_TXTSP,
			ID_DCCORRI_TXTSP,
			ID_DCCORRQ_TXTSP
		};
		
		wxStaticText* ID_STATICTEXT51;
		NumericSlider* cmbGCORRI_RXTSP;
		wxStaticText* ID_STATICTEXT41;
		NumericSlider* cmbGCORRQ_RXTSP;
		wxButton* ID_BUTTON101;
		NumericSlider* cmbIQCORR_RXTSP;
		wxStaticText* ID_STATICTEXT161;
		wxStaticText* txtPhaseAlpha1;
		wxStaticText* ID_STATICTEXT6;
		NumericSlider* cmbDCOFFI_RFE;
		wxStaticText* ID_STATICTEXT7;
		NumericSlider* cmbDCOFFQ_RFE;
		wxCheckBox* chkEN_DCOFF_RXFE_RFE;
		wxCheckBox* chkDCMODE;
		wxButton* btnCalibrateRx;
		wxStaticText* ID_STATICTEXT5;
		NumericSlider* cmbGCORRI_TXTSP;
		wxStaticText* ID_STATICTEXT4;
		NumericSlider* cmbGCORRQ_TXTSP;
		wxButton* ID_BUTTON10;
		NumericSlider* cmbIQCORR_TXTSP;
		wxStaticText* ID_STATICTEXT16;
		wxStaticText* txtPhaseAlpha;
		wxStaticText* ID_STATICTEXT8;
		NumericSlider* cmbDCCORRI_TXTSP;
		wxStaticText* ID_STATICTEXT9;
		NumericSlider* cmbDCCORRQ_TXTSP;
		wxButton* btnCalibrateTx;
		wxButton* btnCalibrateAll;
		wxStaticText* m_staticText431;
		wxStaticText* lblCGENrefClk;
		wxStaticText* m_staticText372;
		wxTextCtrl* txtCalibrationBW;
		wxRadioBox* rgrCalibrationMethod;
		wxStaticText* lblCalibrationNote;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnCalibrateRx( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnCalibrateTx( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnCalibrateAll( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCalibrationMethodChange( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlCalibrations_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~pnlCalibrations_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlGains_view
///////////////////////////////////////////////////////////////////////////////
class pnlGains_view : public wxPanel 
{
	private:
	
	protected:
		enum
		{
			ID_EN_NEXTRX_RFE = 2048,
			ID_G_LNA_RFE,
			ID_G_TIA_RFE,
			ID_G_PGA_RBB,
			ID_C_CTL_PGA_RBB,
			ID_LOSS_LIN_TXPAD_TRF,
			ID_LOSS_MAIN_TXPAD_TRF,
			ID_CG_IAMP_TBB
		};
		
		wxCheckBox* chkTRX_GAIN_SRC;
		wxComboBox* cmbG_LNA_RFE;
		wxComboBox* cmbG_TIA_RFE;
		wxComboBox* cmbG_PGA_RBB;
		NumericSlider* cmbC_CTL_PGA_RBB;
		wxComboBox* cmbLOSS_LIN_TXPAD_TRF;
		wxComboBox* cmbLOSS_MAIN_TXPAD_TRF;
		NumericSlider* cmbCG_IAMP_TBB;
		wxCheckBox* chkAGC;
		wxStaticText* m_staticText360;
		wxTextCtrl* txtCrestFactor;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ParameterChangeHandler( wxCommandEvent& event ) { event.Skip(); }
		virtual void ParameterChangeHandler( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnAGCStateChange( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		pnlGains_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~pnlGains_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgGFIR_Coefficients
///////////////////////////////////////////////////////////////////////////////
class dlgGFIR_Coefficients : public wxDialog 
{
	private:
	
	protected:
		wxButton* btnLoadFromFile;
		wxButton* btnSaveToFile;
		wxButton* btnClearTable;
		wxStaticText* ID_STATICTEXT3;
		wxSpinCtrl* spinCoefCount;
		wxGrid* gridCoef;
		wxButton* btnOk;
		wxButton* btnCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnLoadFromFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveToFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClearTable( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnspinCoefCountChange( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnBtnOkClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBtnCancelClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dlgGFIR_Coefficients( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Coefficients"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~dlgGFIR_Coefficients();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgVCOfrequencies
///////////////////////////////////////////////////////////////////////////////
class dlgVCOfrequencies : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText341;
		wxTextCtrl* txtVCOH_low;
		wxStaticText* m_staticText342;
		wxTextCtrl* txtVCOH_high;
		wxStaticText* m_staticText3411;
		wxTextCtrl* txtVCOM_low;
		wxStaticText* m_staticText3421;
		wxTextCtrl* txtVCOM_high;
		wxStaticText* m_staticText3412;
		wxTextCtrl* txtVCOL_low;
		wxStaticText* m_staticText3422;
		wxTextCtrl* txtVCOL_high;
		wxStaticText* m_staticText3413;
		wxTextCtrl* txtVCOCGEN_low;
		wxStaticText* m_staticText3423;
		wxTextCtrl* txtVCOCGEN_high;
		wxButton* btnOk;
		wxButton* btnCancel;
		wxButton* btnLoadFile;
		wxButton* btnSaveFile;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnBtnOkClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBtnCancelClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoadFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveFile( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dlgVCOfrequencies( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("VCO frequencies"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~dlgVCOfrequencies();
	
};

#endif //__LMS7002_WXGUI_H__
