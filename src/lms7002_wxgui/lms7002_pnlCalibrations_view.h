#ifndef __lms7002_pnlCalibrations_view__
#define __lms7002_pnlCalibrations_view__

#include <map>
#include "lime/LimeSuite.h"

#include "ILMS7002MTab.h"

class lms7002_pnlCalibrations_view : public ILMS7002MTab
{
protected:
  virtual void ParameterChangeHandler(wxCommandEvent &event) override;
  void ParameterChangeHandler(wxSpinEvent &event);
  void OnbtnCalibrateRx(wxCommandEvent &event);
  void OnbtnCalibrateTx(wxCommandEvent &event);
  void OnbtnCalibrateAll(wxCommandEvent &event);
  void OnCalibrationMethodChange(wxCommandEvent &event);

public:
    lms7002_pnlCalibrations_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    virtual void Initialize(lime::SDRDevice *pControl) override;
    virtual void UpdateGUI() override;

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
		wxStaticText* txtRxPhaseAlpha;
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
};

#endif // __lms7002_pnlCalibrations_view__
