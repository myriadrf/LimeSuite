#ifndef __DPDTest_H__
#define __DPDTest_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <stdint.h>
class OpenGLGraph;

#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/gauge.h>
#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/combobox.h>
#include "kiss_fft.h"
#include <wx/timer.h>
#include <wx/wx.h>
enum
{
	TIMER_ID = 10
};

namespace lime{
	class IConnection;
}

// class LMScomms; -- was 11.09.2016
// class wxPanel;
// class wxNotebook;

#include "qadpd.h"


class DPDTest : public wxFrame
{
private:
	static const long ID_NOTEBOOK1;
	static const long ID_ADPD_TAB_SPECTRUM;
	static const long ID_ADPD_SPLITTERWINDOW1;
	static const long ID_ADPD_SPLITTERWINDOW2;
	static const long ID_ADPD_SPLITTERWINDOW3;
	static const long ID_GLCANVAS_ADPD0;
	static const long ID_GLCANVAS_ADPD1;
	static const long ID_GLCANVAS_ADPD2;
	static const long ID_GLCANVAS_ADPD3;
	static const long ID_PANEL_ADPD0;
	static const long ID_PANEL_ADPD1;
	static const long ID_PANEL_ADPD2;
	static const long ID_PANEL_ADPD3;
	static const long ID_STATICTEXT_ADPD0;
	static const long ID_STATICTEXT_ADPD1;
	static const long ID_STATICTEXT_ADPD2;
	static const long ID_STATICTEXT_ADPD3;
	static const long ID_STATICTEXT_ADPD4;
	static const long ID_STATICTEXT_ADPD5;
	static const long ID_STATICTEXT_ADPD6;
	static const long ID_STATICTEXT_ADPD7;
	//static const long ID_STATICTEXT_ADPD8;
	//static const long ID_STATICTEXT_ADPD9;
	static const long ID_STATICTEXT_ADPD10;	
	static const long ID_SPINCTRL_ADPD_N;
	static const long ID_SPINCTRL_ADPD_M;
	static const long ID_TEXTCTRL_ADPD1;
	static const long ID_TEXTCTRL_ADPD2;
	//static const long ID_TEXTCTRL_ADPD3;
	//static const long ID_TEXTCTRL_ADPD4;
	static const long ID_SPINCTRL_ADPD_DELAY;
	static const long ID_CHECKBOX_PLOT1;
	static const long ID_BUTTON_PLOT1;
	static const long ID_CHECKBOX_PLOT2;
	static const long ID_BUTTON_PLOT2;
	static const long ID_CHECKBOX_PLOT3;
	static const long ID_BUTTON_PLOT3;
	static const long ID_CHECKBOX_PLOT4;
	static const long ID_BUTTON_PLOT4;
	static const long ID_BUTTON_CAPTURE;
	static const long ID_BUTTON_TRAIN;
	static const long ID_BUTTON_START;
	static const long ID_BUTTON_END;
	static const long ID_BUTTON_SEND;
	static const long ID_BUTTON_RTS;
	//static const long ID_CHECKBOX_YPFPGA;
	//static const long ID_STATICTEXT_1;
	static const long ID_STATICTEXT_2;
	static const long ID_TEXTCTRL_FFTSAMPLES;
	static const long ID_BUTTON_CALFFT;
	static const long ID_STATICTEXT_ADPD11;
	static const long ID_STATICTEXT_12;
	static const long ID_TEXTCTRL_12;
	static const long ID_COMBOBOX_WINDOW;

	static const long ID_TEXTCTRL_NUM;
	static const long ID_BUTTON_TIMER;
	
	
	static const long ID_CHECKBOX_TRAIN;
	static const long ID_TRAINMODE;

	static const long ID_STATICTEXT_FFT1;
	static const long ID_TEXTCTRL_FFT1;
	static const long ID_STATICTEXT_FFT2;
	static const long ID_TEXTCTRL_FFT2;

	static const long ID_STATICTEXT_FFT3;
	static const long ID_TEXTCTRL_FFT3;

	
	wxNotebook * m_MainTabs;
	wxPanel * m_ADPD_tabSpectrum;
	wxSplitterWindow * m_ADPD_plotSplitter;
	wxSplitterWindow * m_ADPD_plotSplitter_up;
	wxSplitterWindow * m_ADPD_plotSplitter_down;
	wxPanel * Panel_ADPD0;
	wxPanel * Panel_ADPD1;
	wxPanel * Panel_ADPD2;
	wxPanel * Panel_ADPD3;
	OpenGLGraph * m_plot_ADPD0;
	OpenGLGraph * m_plot_ADPD1;
	OpenGLGraph * m_plot_ADPD2;
	OpenGLGraph * m_plot_ADPD3;
	wxStaticText * StaticText_ADPD0;
	wxStaticText * StaticText_ADPD1;
	wxStaticText * StaticText_ADPD2;
	wxStaticText * StaticText_ADPD3;
	//wxStaticText * StaticText_ADPD4;
	//wxStaticText * StaticText_ADPD5;
	wxStaticText * StaticText_ADPD6;
	//wxStaticText * StaticText_1;
	wxStaticText * StaticText_2;

	wxStaticText * StaticText_PLOT1;
	wxStaticText * StaticText_PLOT2;
	wxStaticText * StaticText_PLOT3;
	wxStaticText * StaticText_PLOT4;
	wxRadioBox * m_ADPD_plotType;
	wxSpinCtrl * spin_ADPD_M;
	wxSpinCtrl * spin_ADPD_N;
	wxTextCtrl * m_ADPD_txtLambda;
	wxTextCtrl * m_ADPD_txtGain;
	//wxSpinCtrl * m_ADPD_txtAm;
	//wxTextCtrl * m_ADPD_txtSkip;
	wxSpinCtrl * spin_ADPD_Delay;

	wxFlexGridSizer * m_ADPD_controlsSizer_X;

	wxFlexGridSizer * m_ADPD_spectrumSizer;
	wxFlexGridSizer * m_ADPD_plotSizer;
	wxFlexGridSizer * m_ADPD_controlsSizer;
	wxFlexGridSizer * m_plot_ADPD0_Sizer;
	wxFlexGridSizer * m_plot_ADPD1_Sizer;
	wxFlexGridSizer * m_plot_ADPD2_Sizer;
	wxFlexGridSizer *  m_plot_ADPD3_Sizer;
	wxStaticBoxSizer * StaticBoxSizerADPD1;
	wxFlexGridSizer* m_ADPD_controlsSizer2;
	wxStaticBoxSizer * StaticBoxSizerADPD2;
	wxFlexGridSizer* m_ADPD_controlsSizer3;
	//wxFlexGridSizer* m_ADPD_controlsSizer4;
	wxFlexGridSizer* m_ADPD_controlsSizer5;
	wxFlexGridSizer* FlexGridSizer1;

	wxStaticBoxSizer* StaticBox_PLOT1;
	wxFlexGridSizer* Sizer_PLOT1;
	wxCheckBox * CheckBox_PLOT1;
	wxButton * Button_PLOT1;

	wxStaticBoxSizer* StaticBox_PLOT2;
	wxFlexGridSizer* Sizer_PLOT2;
	wxCheckBox * CheckBox_PLOT2;
	wxButton * Button_PLOT2;

	wxStaticBoxSizer* StaticBox_PLOT3;
	wxFlexGridSizer* Sizer_PLOT3;
	wxCheckBox * CheckBox_PLOT3;
	wxButton * Button_PLOT3;

	wxStaticBoxSizer* StaticBox_PLOT4;
	wxFlexGridSizer* Sizer_PLOT4;
	wxCheckBox * CheckBox_PLOT4;
	wxButton * Button_PLOT4;

	wxButton * Button_TRAIN;
	wxButton * Button_RTS;
	wxButton * Button_START;
	wxButton * Button_END;
	//wxCheckBox * CheckBox_YPFPGA;

	//wxTextCtrl * m_ADPD_FFTSamples;

	wxTextCtrl * m_ADPD_Num;
	//wxButton * btnCalculateFFT;
	wxButton *  Button_SEND;
	wxStaticBoxSizer * StaticBoxSizerADPD3;
	//wxStaticBoxSizer * StaticBoxSizerADPD4;
	wxStaticBoxSizer * StaticBoxSizerADPD5;

	wxStaticText * StaticText_11;
	wxStaticText * StaticText_12;
	wxTextCtrl * m_ADPD_txtTrain;

	wxComboBox * cmbWindowFunction;

	wxRadioBox * TrainMode;
	wxStaticBoxSizer * StaticBoxSizerADPD6;
	wxFlexGridSizer * m_ADPD_controlsSizer6;
	wxStaticBoxSizer * StaticBoxSizerADPD7;
	wxFlexGridSizer * m_ADPD_controlsSizer7;
	wxCheckBox * CheckBox_Train;

	wxStaticText * StaticText_FFT1;
	wxSpinCtrl * m_ADPD_txtFFT1;
	wxStaticText * StaticText_FFT2;
	wxSpinCtrl * m_ADPD_txtFFT2;

	wxStaticText * StaticText_FFT3;
	wxTextCtrl * m_ADPD_txtFFT3;

	kiss_fft_cpx* xp_samples;
	kiss_fft_cpx* yp_samples;
	kiss_fft_cpx* x_samples;
	kiss_fft_cpx* y_samples;
	kiss_fft_cpx* xp1_samples;
	kiss_fft_cpx* yp1_samples;
	kiss_fft_cpx* x1_samples;
	kiss_fft_cpx* y1_samples;

	kiss_fft_cpx* u_samples;
	kiss_fft_cpx* error_samples;
	kiss_fft_cpx* xp_fft;
	kiss_fft_cpx* yp_fft;
	kiss_fft_cpx* x_fft;
	kiss_fft_cpx* y_fft;

	wxButton * Button_TIMER;

	int samplesReceived;
	bool m_bTrain;


	//void changePlotType(OpenGLGraph * graph);
	//void On_plotType(wxTimerEvent & event);
	//wxFont * pFont;
	// Borko end

protected:
    //OpenGLGraph* mFFT_xp;
    //OpenGLGraph* mFFT_yp;
    //OpenGLGraph* mFFT_x;
    //OpenGLGraph* mTime_xp;
    //OpenGLGraph* mTime_yp;
    //OpenGLGraph* mTime_x;

	int QADPD_N;
	int QADPD_M;
	int QADPD_ND;
	int QADPD_AM;
	int QADPD_SKIP;
	double  QADPD_GAIN;
	double  QADPD_LAMBDA;
	bool QADPD_YPFPGA;
	int QADPD_FFTSAMPLES;
	int QADPD_FFT1;
	int QADPD_FFT2;
	int QADPD_UPDATE;
	double QADPD_SPAN;

	qadpd * Qadpd;
	wxTimer * m_timer;


	//qadpd * Qadpd;
    wxButton* btnCapture;
    
    void OnbtnCaptureClicked(wxCommandEvent& event);
	void OnbtnTrainClick(wxCommandEvent& event);
	void OnbtnRTSClick(wxCommandEvent& event);
	void OnbtnStartClick(wxCommandEvent& event);
	void OnbtnEndClick(wxCommandEvent& event);
	void OnbtnSendClick(wxCommandEvent& event);
    void OnbtnTimerClick(wxCommandEvent& event);
	void onTrainMode(wxCommandEvent &evt);
	void OnbtnTrain(wxCommandEvent &evt);
	void OncmbWindowFunctionSelected(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void onEnableDisable();
	void CreateArrays();

	void OnChangePlot();

	int train();
	int prepare_train();
	//void readdata();
	void readdata_qspark();

    //LMScomms *mDataPort;
	lime::IConnection* mDataPort;

    float mNyquist_MHz;

	bool timer_enabled;

	wxString PlotFFT(OpenGLGraph* plot, int m_iValue, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
		int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot, int m_iYaxisTopPlot, int  m_iYaxisBottomPlot);
	//const kiss_fft_cpx* fftOutput, const int samplesCount, const float nyquist_MHz);
	wxString PlotTimeDomain(OpenGLGraph* plot, int m_iValue, int m_iXaxisLeftPlot, int m_iXaxisRightPlot);
	wxString PlotIQ(OpenGLGraph* plot, int m_iValue);

	//, const kiss_fft_cpx* samples, const int samplesCount);
	void changePlotType(OpenGLGraph * graph, int plotType,int  m_iXaxisLeftPlot, int m_iXaxisRightPlot);
	void OnbtnPlot1ControlsClick(wxCommandEvent& event);
	void OnbtnPlot2ControlsClick(wxCommandEvent& event);
	void OnbtnPlot3ControlsClick(wxCommandEvent& event);
	void OnbtnPlot4ControlsClick(wxCommandEvent& event);
	void OnChangePlotVisibility(wxCommandEvent& event);

public:

	
	int m_iValuePlot2;
	int m_iValuePlot3;
	int m_iValuePlot4;

	int m_iValuePlot1;
	int m_iYaxisTopPlot1;
	int m_iYaxisBottomPlot1;
	int m_iCenterFreqRatioPlot1;
	int m_iFreqSpanRatioPlot1;
	double m_dCenterFreqRatioPlot1;
	double m_dFreqSpanRatioPlot1;
	int m_iXaxisLeftPlot1;
	int m_iXaxisRightPlot1;

	int m_iYaxisTopPlot2;
	int m_iYaxisBottomPlot2;
	int m_iCenterFreqRatioPlot2;
	int m_iFreqSpanRatioPlot2;
	double m_dCenterFreqRatioPlot2;
	double m_dFreqSpanRatioPlot2;
	int m_iXaxisLeftPlot2;
	int m_iXaxisRightPlot2;

	int m_iYaxisTopPlot3;
	int m_iYaxisBottomPlot3;
	int m_iCenterFreqRatioPlot3;
	int m_iFreqSpanRatioPlot3;
	double m_dCenterFreqRatioPlot3;
	double m_dFreqSpanRatioPlot3;
	int m_iXaxisLeftPlot3;
	int m_iXaxisRightPlot3;

	int m_iYaxisTopPlot4;
	int m_iYaxisBottomPlot4;
	int m_iCenterFreqRatioPlot4;
	int m_iFreqSpanRatioPlot4;
	double m_dCenterFreqRatioPlot4;
	double m_dFreqSpanRatioPlot4;
	int m_iXaxisLeftPlot4;
	int m_iXaxisRightPlot4;

    //void Initialize(LMScomms* dataPort);
	void Initialize(lime::IConnection* dataPort);
    void SetNyquist(float Nyquist_MHz);
    DPDTest(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("FFT viewer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~DPDTest();
    //int SPI_write(LMScomms* dataPort, uint16_t address, uint16_t data);
    //uint16_t SPI_read(LMScomms* dataPort, uint16_t address);

	int SPI_write(lime::IConnection* dataPort, uint16_t address, uint16_t data);
	//uint16_t SPI_read(lime::IConnection* dataPort, uint16_t address);

	void OnPlot(int m_iValue, OpenGLGraph * plot, wxStaticText *text, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
		int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot, int m_iYaxisTopPlot, int  m_iYaxisBottomPlot, int rb, int m_iXaxisLeftPlot, int m_iXaxisRightPlot);
	void SaveConfig();
	void OpenConfig();
	void OnYaxisChange(OpenGLGraph * plot, int m_iYaxisTopPlot, int  m_iYaxisBottomPlot);
	void OnCenterSpanChange(OpenGLGraph * plot, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
		int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot);
	void send_coef();
	void run_QADPD();
	float * windowFcoefs;
	void GenerateWindowCoefficients(int func, int fftsize);
	double mAmplitudeCorrectionCoef;
	int m_iWindowFunc;
	int ind;

	double range;

	DECLARE_EVENT_TABLE();
};

#endif //__FFTVIEWER_WXGUI_H__
