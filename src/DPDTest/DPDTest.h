
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
#include "../kissFFT/kiss_fft.h"
#include <wx/timer.h>
#include <wx/wx.h>
#include <wx/radiobut.h>
#include "lms7_device.h"
#include <thread>

enum
{
    TIMER_ID = 10
};

#include "../DPDTest/qadpd.h"

class LMScomms;

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
    static const long ID_STATICTEXT_ADPD8;
    static const long ID_STATICTEXT_ADPD9;
    static const long ID_STATICTEXT_ADPD10;
    static const long ID_STATICTEXT_ADPD19;
    static const long ID_SPINCTRL_ADPD_N;
    static const long ID_SPINCTRL_ADPD_M;
    static const long ID_TEXTCTRL_ADPD1;
    static const long ID_TEXTCTRL_ADPD2;
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
    static const long ID_BUTTON_SAVE;
    static const long ID_BUTTON_READ;
    static const long ID_BUTTON_CAL;
    static const long ID_BUTTON_CAL2;
    static const long ID_BUTTON_SEND;
    static const long ID_BUTTON_RTS;
    static const long ID_STATICTEXT_2;
    static const long ID_TEXTCTRL_FFTSAMPLES;
    static const long ID_BUTTON_CALFFT;
    static const long ID_STATICTEXT_ADPD11;
    static const long ID_STATICTEXT_ADPD13;
    static const long ID_STATICTEXT_12;
    static const long ID_TEXTCTRL_12;
    static const long ID_COMBOBOX_WINDOW;
    static const long ID_BUTTON_TIMER;
    static const long ID_CHECKBOX_TRAIN;
    static const long ID_TRAINMODE;
    static const long ID_STATICTEXT_FFT1;
    static const long ID_TEXTCTRL_FFT1;
    static const long ID_STATICTEXT_FFT2;
    static const long ID_TEXTCTRL_FFT2;
    static const long ID_STATICTEXT_FFT3;
    static const long ID_TEXTCTRL_FFT3;
    static const long ID_READING_FINISHED_EVENT;
    //static const long ID_CHECKBOX_EXTDACS;

    wxNotebook *m_MainTabs;
    wxPanel *m_ADPD_tabSpectrum;
    wxSplitterWindow *m_ADPD_plotSplitter;
    wxSplitterWindow *m_ADPD_plotSplitter_up;
    wxSplitterWindow *m_ADPD_plotSplitter_down;
    wxPanel *Panel_ADPD0;
    wxPanel *Panel_ADPD1;
    wxPanel *Panel_ADPD2;
    wxPanel *Panel_ADPD3;
    OpenGLGraph *m_plot_ADPD0;
    OpenGLGraph *m_plot_ADPD1;
    OpenGLGraph *m_plot_ADPD2;
    OpenGLGraph *m_plot_ADPD3;
    wxStaticText *StaticText_ADPD0;
    wxStaticText *StaticText_ADPD1;
    wxStaticText *StaticText_ADPD2;
    wxStaticText *StaticText_ADPD3;
    //wxStaticText *StaticText_ADPD8;
    //wxStaticText *StaticText_ADPD9;
    wxStaticText *StaticText_ADPD6;
    //wxStaticText *StaticText_ADPD19;
    wxStaticText *StaticText_2;
    wxStaticText *StaticText_PLOT1;
    wxStaticText *StaticText_PLOT2;
    wxStaticText *StaticText_PLOT3;
    wxStaticText *StaticText_PLOT4;
    //wxRadioBox *m_ADPD_plotType;
    wxSpinCtrl *spin_ADPD_M;
    wxSpinCtrl *spin_ADPD_N;
    wxTextCtrl *m_ADPD_txtLambda;
    wxSpinCtrlDouble *m_ADPD_txtGain;
    wxSpinCtrl *spin_ADPD_Delay;
    wxFlexGridSizer *m_ADPD_controlsSizer_X;
    wxFlexGridSizer *m_ADPD_controlsSizer_Y;
    wxFlexGridSizer *m_ADPD_spectrumSizer;
    wxFlexGridSizer *m_ADPD_plotSizer;
    wxFlexGridSizer *m_ADPD_controlsSizer;
    wxFlexGridSizer *m_plot_ADPD0_Sizer;
    wxFlexGridSizer *m_plot_ADPD1_Sizer;
    wxFlexGridSizer *m_plot_ADPD2_Sizer;
    wxFlexGridSizer *m_plot_ADPD3_Sizer;
    wxStaticBoxSizer *StaticBoxSizerADPD1;
    wxFlexGridSizer *m_ADPD_controlsSizer2;
    wxStaticBoxSizer *StaticBoxSizerADPD2;
    wxFlexGridSizer *m_ADPD_controlsSizer3;
    wxFlexGridSizer *m_ADPD_controlsSizer5;
    wxFlexGridSizer *FlexGridSizer1;

    wxStaticBoxSizer *StaticBox_PLOT1;
    wxFlexGridSizer *Sizer_PLOT1;
    wxCheckBox *CheckBox_PLOT1;
    wxButton *Button_PLOT1;
    wxCheckBox *CheckBox_PLOT2;
    wxButton *Button_PLOT2;
    wxCheckBox *CheckBox_PLOT3;
    wxButton *Button_PLOT3;
    wxCheckBox *CheckBox_PLOT4;
    wxButton *Button_PLOT4;
    wxButton *Button_TRAIN;
    wxButton *Button_START;
    wxButton *Button_END;
    wxButton *Button_CAL;
    wxButton *Button_CAL2;
    wxButton *Button_SEND;
    wxButton *Button_SAVE;
    wxButton *Button_READ;
    wxStaticBoxSizer *StaticBoxSizerADPD3;
    wxStaticBoxSizer *StaticBoxSizerADPD5;
    wxStaticText *StaticText_11;
    wxStaticText *StaticText_12;
    wxTextCtrl *m_ADPD_txtTrain;
    wxStaticText *StaticText_13;
    wxComboBox *cmbWindowFunction;
    wxRadioBox *TrainMode;
    wxStaticBoxSizer *StaticBoxSizerADPD6;
    wxFlexGridSizer *m_ADPD_controlsSizer6;
    wxStaticBoxSizer *StaticBoxSizerADPD7;
    wxFlexGridSizer *m_ADPD_controlsSizer7;
    wxCheckBox *CheckBox_Train;
    wxStaticText *StaticText_FFT1;
    wxSpinCtrl *m_ADPD_txtFFT1;
    wxStaticText *StaticText_FFT2;
    wxSpinCtrl *m_ADPD_txtFFT2;
    wxStaticText *StaticText_FFT3;
    wxTextCtrl *m_ADPD_txtFFT3;
    kiss_fft_cpx *xp_samples;
    kiss_fft_cpx *yp_samples;
    kiss_fft_cpx *x_samples;
    kiss_fft_cpx *y_samples;
    kiss_fft_cpx *x1_samples;
    kiss_fft_cpx *y1_samples;
    kiss_fft_cpx *xp1_samples;
    kiss_fft_cpx *yp1_samples;
    kiss_fft_cpx *u_samples;
    kiss_fft_cpx *error_samples;
    kiss_fft_cpx *xp_fft;
    kiss_fft_cpx *yp_fft;
    kiss_fft_cpx *x_fft;
    kiss_fft_cpx *y_fft;
    wxButton *Button_TIMER;
    wxRadioButton *rbChannelA;
    wxRadioButton *rbChannelB;
    int samplesReceived;
    bool m_bTrain;
    int offset;

protected:
    int m_iThread;
    std::thread mWorkerThread;
    void threadReadData();
    void OnReadingFinished(wxCommandEvent &event);
    //void OnChangeExtDACs(wxCommandEvent &event);

    int updatedCoeff;
    int QADPD_N;
    int QADPD_M;
    int QADPD_ND;
    int QADPD_AM;
    int QADPD_SKIP;
    double QADPD_GAIN;
    double QADPD_LAMBDA;
    bool QADPD_YPFPGA;
    int QADPD_FFTSAMPLES;
    int QADPD_FFT1;
    int QADPD_FFT2;
    int QADPD_UPDATE;
    double QADPD_SPAN;

    qadpd *Qadpd;
    wxTimer *m_timer;
    wxButton *btnCapture;

    void OnGainChanged(wxCommandEvent &event);

    void OnbtnCaptureClicked(wxCommandEvent &event);
    void OnbtnTrainClick(wxCommandEvent &event);
    void OnbtnStartClick(wxCommandEvent &event);
    void OnbtnEndClick(wxCommandEvent &event);
    void OnbtnCalibrate(wxCommandEvent &event);
    void OnbtnCalibrate2(wxCommandEvent &event);
    void OnbtnSendClick(wxCommandEvent &event);
    void OnbtnTimerClick(wxCommandEvent &event);
    void onTrainMode(wxCommandEvent &evt);
    void OnbtnTrain(wxCommandEvent &evt);
    void OncmbWindowFunctionSelected(wxCommandEvent &event);
    void OnTimer(wxTimerEvent &event);
    void onEnableDisable();
    void CreateArrays();
    int SelectSource(int CHA);
    void save_coef();
    void read_coef();
    void update_coef();

    int LMS_ReadFPGARegs(lms_device_t *lmsControl, uint32_t *address, uint16_t *val, int num);

    double coeffa_chA[9][4];
    double coeffb_chA[9][4];
    double coeffa_chB[9][4];
    double coeffb_chB[9][4];

    void OnbtnSaveClick(wxCommandEvent &event);
    void OnbtnReadClick(wxCommandEvent &event);

    void OnChangePlot();
    void CalibrateND(int Size, double *a, double *b, int *pND);
    double CalculateSignalPower(int Size, double *a, double *b, double Am);
    double CalculateGainCorr(double p1, double p2, double g1, double g2, double p);

    int train();
    int prepare_train();
    int readdata_QPCIe();
    int readdata_QPCIe2();

    lms_device_t *lmsControl;
    float mLmsNyquist_MHz;
    bool timer_enabled;

    wxString PlotFFT(OpenGLGraph *plot, int m_iValue, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
                     int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot, int m_iYaxisTopPlot, int m_iYaxisBottomPlot);
    wxString PlotTimeDomain(OpenGLGraph *plot, int m_iValue, int m_iXaxisLeftPlot, int m_iXaxisRightPlot);
    wxString PlotIQ(OpenGLGraph *plot, int m_iValue);

    void changePlotType(OpenGLGraph *graph, int plotType, int m_iXaxisLeftPlot, int m_iXaxisRightPlot);
    void OnbtnPlot1ControlsClick(wxCommandEvent &event);
    void OnbtnPlot2ControlsClick(wxCommandEvent &event);
    void OnbtnPlot3ControlsClick(wxCommandEvent &event);
    void OnbtnPlot4ControlsClick(wxCommandEvent &event);
    void OnChangePlotVisibility(wxCommandEvent &event);

    void OnSwitchToChannelA(wxCommandEvent &event);
    void OnSwitchToChannelB(wxCommandEvent &event);

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

    void Initialize(lms_device_t *dataPort);
    void SetNyquist(float Nyquist_MHz);
    DPDTest(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = wxT("DPDviewer"), const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxSize(-1, -1), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~DPDTest();

    int SPI_write(lms_device_t *dataPort, uint16_t address, uint16_t data);
    int SPI_read(lms_device_t *lmsControl, uint16_t address, uint16_t *data);

    void OnPlot(int m_iValue, OpenGLGraph *plot, wxStaticText *text, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
                int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot, int m_iYaxisTopPlot, int m_iYaxisBottomPlot, int rb, int m_iXaxisLeftPlot, int m_iXaxisRightPlot);
    void SaveConfig();
    void OpenConfig();
    void OnYaxisChange(OpenGLGraph *plot, int m_iYaxisTopPlot, int m_iYaxisBottomPlot);
    void OnCenterSpanChange(OpenGLGraph *plot, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
                            int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot);
    void send_coef();
    void run_QADPD();
    float *windowFcoefs;
    void GenerateWindowCoefficients(int func, int fftsize);

    void SaveWfm();
    double mAmplitudeCorrectionCoef;
    int m_iWindowFunc;
    int ind;
    double range;
    int chA;

    FILE *fp;
    std::string fname;

    DECLARE_EVENT_TABLE();
};

#endif //__FFTVIEWER_WXGUI_H__
