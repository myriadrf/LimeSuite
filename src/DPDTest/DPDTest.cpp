#include "DPDTest.h"
#include <wx/timer.h>
#include <vector>
#include "../oglGraph/OpenGLGraph.h"
#include "kiss_fft.h"
#include "iniParser.h"
#include "../DPDTest/dlgADPDControls.h"

#include "hb1.h"
#include "hb2.h"
#include "lms7_device.h"

using namespace lime;
using namespace std;

const long DPDTest::ID_NOTEBOOK1 = wxNewId();
const long DPDTest::ID_ADPD_TAB_SPECTRUM = wxNewId();
const long DPDTest::ID_ADPD_SPLITTERWINDOW1 = wxNewId();
const long DPDTest::ID_ADPD_SPLITTERWINDOW2 = wxNewId();
const long DPDTest::ID_ADPD_SPLITTERWINDOW3 = wxNewId();
const long DPDTest::ID_GLCANVAS_ADPD0 = wxNewId();
const long DPDTest::ID_GLCANVAS_ADPD1 = wxNewId();
const long DPDTest::ID_GLCANVAS_ADPD2 = wxNewId();
const long DPDTest::ID_GLCANVAS_ADPD3 = wxNewId();
const long DPDTest::ID_PANEL_ADPD0 = wxNewId();
const long DPDTest::ID_PANEL_ADPD1 = wxNewId();
const long DPDTest::ID_PANEL_ADPD2 = wxNewId();
const long DPDTest::ID_PANEL_ADPD3 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD0 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD1 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD2 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD3 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD4 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD5 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD6 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD7 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD8 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD9 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD10 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD11 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD13 = wxNewId();
const long DPDTest::ID_STATICTEXT_ADPD19 = wxNewId();
const long DPDTest::ID_BUTTON_CAPTURE = wxNewId();
const long DPDTest::ID_SPINCTRL_ADPD_N = wxNewId();
const long DPDTest::ID_SPINCTRL_ADPD_M = wxNewId();

const long DPDTest::ID_TEXTCTRL_ADPD1 = wxNewId();
const long DPDTest::ID_TEXTCTRL_ADPD2 = wxNewId();
const long DPDTest::ID_SPINCTRL_ADPD_DELAY = wxNewId();
const long DPDTest::ID_CHECKBOX_PLOT1 = wxNewId();
const long DPDTest::ID_BUTTON_PLOT1 = wxNewId();
const long DPDTest::ID_CHECKBOX_PLOT2 = wxNewId();
const long DPDTest::ID_BUTTON_PLOT2 = wxNewId();
const long DPDTest::ID_CHECKBOX_PLOT3 = wxNewId();
const long DPDTest::ID_BUTTON_PLOT3 = wxNewId();
const long DPDTest::ID_CHECKBOX_PLOT4 = wxNewId();
const long DPDTest::ID_BUTTON_PLOT4 = wxNewId();
const long DPDTest::ID_BUTTON_TRAIN = wxNewId();
const long DPDTest::ID_BUTTON_START = wxNewId();
const long DPDTest::ID_BUTTON_END = wxNewId();
const long DPDTest::ID_BUTTON_CAL = wxNewId();
const long DPDTest::ID_BUTTON_CAL2 = wxNewId();
const long DPDTest::ID_BUTTON_SEND = wxNewId();
const long DPDTest::ID_TEXTCTRL_FFTSAMPLES = wxNewId();
const long DPDTest::ID_BUTTON_CALFFT = wxNewId();
const long DPDTest::ID_STATICTEXT_12 = wxNewId();
const long DPDTest::ID_TEXTCTRL_12 = wxNewId();
const long DPDTest::ID_COMBOBOX_WINDOW = wxNewId();
const long DPDTest::ID_STATICTEXT_2 = wxNewId();
const long DPDTest::ID_BUTTON_TIMER = wxNewId();
const long DPDTest::ID_CHECKBOX_TRAIN = wxNewId();
const long DPDTest::ID_TRAINMODE = wxNewId();
const long DPDTest::ID_STATICTEXT_FFT1 = wxNewId();
const long DPDTest::ID_TEXTCTRL_FFT1 = wxNewId();
const long DPDTest::ID_STATICTEXT_FFT2 = wxNewId();
const long DPDTest::ID_TEXTCTRL_FFT2 = wxNewId();
const long DPDTest::ID_STATICTEXT_FFT3 = wxNewId();
const long DPDTest::ID_TEXTCTRL_FFT3 = wxNewId();
const long DPDTest::ID_BUTTON_SAVE = wxNewId();
const long DPDTest::ID_BUTTON_READ = wxNewId();
const long DPDTest::ID_READING_FINISHED_EVENT = wxNewId();
//const long DPDTest::ID_CHECKBOX_EXTDACS = wxNewId();

wxBEGIN_EVENT_TABLE(DPDTest, wxFrame)
    EVT_TIMER(TIMER_ID, DPDTest::OnTimer)
        wxEND_EVENT_TABLE()

#define AVERAGE 1
#define NOAVERAGE 0

            const char *config_filename = "QADPDconfig.ini";

DPDTest::~DPDTest()
{

    Button_START->Enable(true);
    Button_TRAIN->Enable(false);
    Button_END->Enable(false);
    Button_SEND->Enable(false);
    spin_ADPD_N->Enable(true);
    spin_ADPD_M->Enable(true);
    spin_ADPD_Delay->Enable(true);
    m_ADPD_txtGain->Enable(true);
    m_ADPD_txtLambda->Enable(true);
    m_ADPD_txtTrain->Enable(true);
    TrainMode->Enable(false);
    btnCapture->Enable(false);
    Button_TRAIN->Enable(false);
    Button_SEND->Enable(false);
    Button_TIMER->Enable(false);
    Button_READ->Enable(false);
    Button_SAVE->Enable(false);
    CheckBox_Train->Enable(false);
    timer_enabled = false;
    Button_CAL->Enable(false);
    Button_CAL2->Enable(false);
    m_timer->Stop();
    ind = 0;
    offset = 0;
    if (m_iThread == 1)
        mWorkerThread.join();
    m_iThread = 0;
};

DPDTest::DPDTest(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : wxFrame(parent, id, title, pos, size, style), lmsControl(nullptr)
{

    fp=NULL;
    offset = 0;
    m_iThread = 0;
    chA = 1;
    range = 16.0;
    updatedCoeff = 0;
    ind = 0;
    m_timer = new wxTimer(this, TIMER_ID);
    m_bTrain = true;
    m_iWindowFunc = 0;
    windowFcoefs = NULL;
    xp_samples = NULL;
    yp_samples = NULL;
    x_samples = NULL;
    y_samples = NULL;
    xp1_samples = NULL;
    yp1_samples = NULL;
    x1_samples = NULL;
    y1_samples = NULL;
    u_samples = NULL;
    error_samples = NULL;
    xp_fft = NULL;
    yp_fft = NULL;
    x_fft = NULL;
    y_fft = NULL;
    samplesReceived = 0;
    m_iValuePlot1 = 0;
    m_iValuePlot2 = 0;
    m_iValuePlot3 = 0;
    m_iValuePlot4 = 0;
    m_iYaxisTopPlot1 = 0;
    m_iYaxisBottomPlot1 = -120;
    m_iCenterFreqRatioPlot1 = 0;
    m_iFreqSpanRatioPlot1 = 0;
    m_dCenterFreqRatioPlot1 = 0.0;
    m_dFreqSpanRatioPlot1 = 0.0;
    m_iXaxisLeftPlot1 = 0;
    m_iXaxisRightPlot1 = 1000;
    m_iYaxisTopPlot2 = 0;
    m_iYaxisBottomPlot2 = -120;
    m_iCenterFreqRatioPlot2 = 0;
    m_iFreqSpanRatioPlot2 = 0;
    m_dCenterFreqRatioPlot2 = 0.0;
    m_dFreqSpanRatioPlot2 = 0.0;
    m_iXaxisLeftPlot2 = 0;
    m_iXaxisRightPlot2 = 1000;
    m_iYaxisTopPlot3 = 0;
    m_iYaxisBottomPlot3 = -120;
    m_iCenterFreqRatioPlot3 = 0;
    m_iFreqSpanRatioPlot3 = 0;
    m_dCenterFreqRatioPlot3 = 0.0;
    m_dFreqSpanRatioPlot3 = 0.0;
    m_iXaxisLeftPlot3 = 0;
    m_iXaxisRightPlot3 = 1000;
    m_iYaxisTopPlot4 = 0;
    m_iYaxisBottomPlot4 = -120;
    m_iCenterFreqRatioPlot4 = 0;
    m_iFreqSpanRatioPlot4 = 0;
    m_dCenterFreqRatioPlot4 = 0.0;
    m_dFreqSpanRatioPlot4 = 0.0;
    m_iXaxisLeftPlot4 = 0;
    m_iXaxisRightPlot4 = 1000;
    mLmsNyquist_MHz = 1;
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
    this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif

    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);

    m_MainTabs = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    m_ADPD_tabSpectrum = new wxPanel(m_MainTabs, ID_ADPD_TAB_SPECTRUM, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_ADPD_TAB_SPECTRUM"));
    m_ADPD_spectrumSizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ADPD_spectrumSizer->AddGrowableCol(0);
    m_ADPD_spectrumSizer->AddGrowableRow(0);

    m_ADPD_tabSpectrum->SetSizer(m_ADPD_spectrumSizer);
    m_ADPD_spectrumSizer->Fit(m_ADPD_tabSpectrum);
    m_ADPD_spectrumSizer->SetSizeHints(m_ADPD_tabSpectrum);

    m_ADPD_plotSizer = new wxFlexGridSizer(0, 1, 5, 5);
    m_ADPD_plotSizer->AddGrowableCol(0);
    m_ADPD_plotSizer->AddGrowableRow(0);

    m_ADPD_controlsSizer = new wxFlexGridSizer(0, 1, 0, 0);
    m_ADPD_controlsSizer->AddGrowableCol(0);

    m_ADPD_spectrumSizer->Add(m_ADPD_plotSizer, 1, wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    m_ADPD_spectrumSizer->Add(m_ADPD_controlsSizer, 1, wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    m_ADPD_plotSplitter = new wxSplitterWindow(m_ADPD_tabSpectrum, ID_ADPD_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxFULL_REPAINT_ON_RESIZE, _T("ID_ADPD_SPLITTERWINDOW1"));
    m_ADPD_plotSplitter->SetMinSize(wxSize(100, 100));
    m_ADPD_plotSplitter->SetMinimumPaneSize(100);
    m_ADPD_plotSplitter->SetSashGravity(0.5);
    m_ADPD_plotSizer->Add(m_ADPD_plotSplitter, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    m_ADPD_plotSplitter_up = new wxSplitterWindow(m_ADPD_plotSplitter, ID_ADPD_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxFULL_REPAINT_ON_RESIZE, _T("ID_ADPD_SPLITTERWINDOW2"));
    m_ADPD_plotSplitter_up->SetMinSize(wxSize(100, 100));
    m_ADPD_plotSplitter_up->SetMinimumPaneSize(100);
    m_ADPD_plotSplitter_up->SetSashGravity(0.5);
    m_ADPD_plotSplitter_up->Fit();

    m_ADPD_plotSplitter_down = new wxSplitterWindow(m_ADPD_plotSplitter, ID_ADPD_SPLITTERWINDOW3, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxFULL_REPAINT_ON_RESIZE, _T("ID_ADPD_SPLITTERWINDOW3"));
    m_ADPD_plotSplitter_down->SetMinSize(wxSize(100, 100));
    m_ADPD_plotSplitter_down->SetMinimumPaneSize(100);
    m_ADPD_plotSplitter_down->SetSashGravity(0.5);
    m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
    m_ADPD_plotSplitter_down->Fit();

    int GLCanvasAttributes_ADPD0[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 0, 0, 0};
    Panel_ADPD0 = new wxPanel(m_ADPD_plotSplitter_up, ID_PANEL_ADPD0, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ADPD0"));
    Panel_ADPD0->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_plot_ADPD0 = new OpenGLGraph(Panel_ADPD0, ID_GLCANVAS_ADPD0, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GLCANVAS_ADPD0"), GLCanvasAttributes_ADPD0);
    m_plot_ADPD0->SetMinSize(wxSize(225, 150));
    m_plot_ADPD0->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

    m_plot_ADPD0_Sizer = new wxFlexGridSizer(0, 1, 0, 0);
    m_plot_ADPD0_Sizer->Add(m_plot_ADPD0, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    m_plot_ADPD0_Sizer->AddGrowableCol(0);
    m_plot_ADPD0_Sizer->AddGrowableRow(0);
    Panel_ADPD0->SetSizer(m_plot_ADPD0_Sizer);

    int GLCanvasAttributes_ADPD1[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 0, 0, 0};
    Panel_ADPD1 = new wxPanel(m_ADPD_plotSplitter_down, ID_PANEL_ADPD1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ADPD1"));
    Panel_ADPD1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_plot_ADPD1 = new OpenGLGraph(Panel_ADPD1, ID_GLCANVAS_ADPD1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GLCANVAS_ADPD1"), GLCanvasAttributes_ADPD1);
    m_plot_ADPD1->SetMinSize(wxSize(225, 150));
    m_plot_ADPD1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

    m_plot_ADPD1_Sizer = new wxFlexGridSizer(0, 1, 0, 0);
    m_plot_ADPD1_Sizer->Add(m_plot_ADPD1, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    m_plot_ADPD1_Sizer->AddGrowableCol(0);
    m_plot_ADPD1_Sizer->AddGrowableRow(0);
    Panel_ADPD1->SetSizer(m_plot_ADPD1_Sizer);

    int GLCanvasAttributes_ADPD2[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 0, 0, 0};
    Panel_ADPD2 = new wxPanel(m_ADPD_plotSplitter_up, ID_PANEL_ADPD2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ADPD2"));
    Panel_ADPD2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_plot_ADPD2 = new OpenGLGraph(Panel_ADPD2, ID_GLCANVAS_ADPD2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GLCANVAS_ADPD2"), GLCanvasAttributes_ADPD2);
    m_plot_ADPD2->SetMinSize(wxSize(225, 150));
    m_plot_ADPD2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

    m_plot_ADPD2_Sizer = new wxFlexGridSizer(0, 1, 0, 0);
    m_plot_ADPD2_Sizer->Add(m_plot_ADPD2, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    m_plot_ADPD2_Sizer->AddGrowableCol(0);
    m_plot_ADPD2_Sizer->AddGrowableRow(0);
    Panel_ADPD2->SetSizer(m_plot_ADPD2_Sizer);

    int GLCanvasAttributes_ADPD3[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 0, 0, 0};
    Panel_ADPD3 = new wxPanel(m_ADPD_plotSplitter_down, ID_PANEL_ADPD3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ADPD3"));
    Panel_ADPD3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_plot_ADPD3 = new OpenGLGraph(Panel_ADPD3, ID_GLCANVAS_ADPD3, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GLCANVAS_ADPD3"), GLCanvasAttributes_ADPD3);
    m_plot_ADPD3->SetMinSize(wxSize(225, 150));
    m_plot_ADPD3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

    m_plot_ADPD3_Sizer = new wxFlexGridSizer(0, 1, 0, 0);
    m_plot_ADPD3_Sizer->Add(m_plot_ADPD3, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    Panel_ADPD3->SetSizer(m_plot_ADPD3_Sizer);
    Panel_ADPD3->Fit();
    m_plot_ADPD3_Sizer->AddGrowableCol(0);
    m_plot_ADPD3_Sizer->AddGrowableRow(0);

    m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
    m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);

    wxStaticBoxSizer *chBox = new wxStaticBoxSizer(wxVERTICAL, m_ADPD_tabSpectrum, _T(""));
    wxFlexGridSizer *chSizer = new wxFlexGridSizer(6, 0, 0, 0);
    wxFlexGridSizer *chSelect = new wxFlexGridSizer(0, 3, 0, 0);

    rbChannelA = new wxRadioButton(m_ADPD_tabSpectrum, wxNewId(), wxT("A CHANNEL"), wxDefaultPosition, wxDefaultSize, 0);
    rbChannelA->SetValue(false);
    rbChannelA->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(DPDTest::OnSwitchToChannelA), NULL, this);
    chSelect->Add(rbChannelA, 0, wxEXPAND, 5);

    rbChannelB = new wxRadioButton(m_ADPD_tabSpectrum, wxNewId(), wxT("B CHANNEL"), wxDefaultPosition, wxDefaultSize, 0);
    rbChannelB->SetValue(false);
    chSelect->Add(rbChannelB, 0, wxEXPAND, 5);
    rbChannelB->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(DPDTest::OnSwitchToChannelB), NULL, this);

    /*
    CheckBox_ExtDACs = new wxCheckBox(m_ADPD_tabSpectrum, ID_CHECKBOX_EXTDACS, _T("Ext. ADC/DACs"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_EXTDACS"));
    CheckBox_ExtDACs->SetValue(false);
    CheckBox_ExtDACs->Disable();
    chSelect->Add(CheckBox_ExtDACs, 0, wxEXPAND, 5);
    Connect(ID_CHECKBOX_EXTDACS, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&DPDTest::OnChangeExtDACs);
    */

    chSizer->Add(chSelect, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    chBox->Add(chSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    m_ADPD_controlsSizer->Add(chBox, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    StaticBox_PLOT1 = new wxStaticBoxSizer(wxVERTICAL, m_ADPD_tabSpectrum, _T("Plot setup"));
    Sizer_PLOT1 = new wxFlexGridSizer(0, 4, 0, 0);
    Sizer_PLOT1->AddGrowableCol(1);
    StaticBox_PLOT1->Add(Sizer_PLOT1);

    CheckBox_PLOT1 = new wxCheckBox(m_ADPD_tabSpectrum, ID_CHECKBOX_PLOT1, _T("Plot 1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PLOT1"));
    CheckBox_PLOT1->SetValue(true);
    Connect(ID_CHECKBOX_PLOT1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&DPDTest::OnChangePlotVisibility);

    Button_PLOT1 = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_PLOT1, _T("Setup"), wxDefaultPosition, wxSize(60, -1), 0, wxDefaultValidator, _T("ID_BUTTON_PLOT1"));
    Connect(ID_BUTTON_PLOT1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnPlot1ControlsClick);

    StaticText_PLOT1 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD4, _T(""), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD4"));
    m_ADPD_controlsSizer->Add(StaticBox_PLOT1);

    CheckBox_PLOT2 = new wxCheckBox(m_ADPD_tabSpectrum, ID_CHECKBOX_PLOT2, _T("Plot 2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PLOT2"));
    CheckBox_PLOT2->SetValue(true);

    Connect(ID_CHECKBOX_PLOT2, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&DPDTest::OnChangePlotVisibility);
    Button_PLOT2 = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_PLOT2, _T("Setup"), wxDefaultPosition, wxSize(60, -1), 0, wxDefaultValidator, _T("ID_BUTTON_PLOT2"));
    Connect(ID_BUTTON_PLOT2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnPlot2ControlsClick);

    StaticText_PLOT2 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD5, _T(""), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD5"));

    CheckBox_PLOT3 = new wxCheckBox(m_ADPD_tabSpectrum, ID_CHECKBOX_PLOT3, _T("Plot 3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PLOT3"));
    CheckBox_PLOT3->SetValue(true);
    Connect(ID_CHECKBOX_PLOT3, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&DPDTest::OnChangePlotVisibility);
    Button_PLOT3 = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_PLOT3, _T("Setup"), wxDefaultPosition, wxSize(60, -1), 0, wxDefaultValidator, _T("ID_BUTTON_PLOT3"));

    Connect(ID_BUTTON_PLOT3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnPlot3ControlsClick);

    StaticText_PLOT3 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD6, _T(""), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD6"));
    CheckBox_PLOT4 = new wxCheckBox(m_ADPD_tabSpectrum, ID_CHECKBOX_PLOT4, _T("Plot 4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PLOT4"));
    CheckBox_PLOT4->SetValue(true);
    Connect(ID_CHECKBOX_PLOT4, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&DPDTest::OnChangePlotVisibility);

    Button_PLOT4 = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_PLOT4, _T("Setup"), wxDefaultPosition, wxSize(60, -1), 0, wxDefaultValidator, _T("ID_BUTTON_PLOT4"));
    Connect(ID_BUTTON_PLOT4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnPlot4ControlsClick);

    StaticText_PLOT4 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD7, _T(""), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD7"));
    Sizer_PLOT1->Add(CheckBox_PLOT1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(Button_PLOT1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(CheckBox_PLOT3, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(Button_PLOT3, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    Sizer_PLOT1->Add(StaticText_PLOT1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(new wxStaticText(m_ADPD_tabSpectrum, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 5);
    Sizer_PLOT1->Add(StaticText_PLOT3, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(new wxStaticText(m_ADPD_tabSpectrum, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 5);

    Sizer_PLOT1->Add(CheckBox_PLOT2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(Button_PLOT2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(CheckBox_PLOT4, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(Button_PLOT4, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    Sizer_PLOT1->Add(StaticText_PLOT2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(new wxStaticText(m_ADPD_tabSpectrum, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 5);
    Sizer_PLOT1->Add(StaticText_PLOT4, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Sizer_PLOT1->Add(new wxStaticText(m_ADPD_tabSpectrum, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 5);

    StaticBoxSizerADPD3 = new wxStaticBoxSizer(wxVERTICAL, m_ADPD_tabSpectrum, _T("FFT setup"));
    m_ADPD_controlsSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    m_ADPD_controlsSizer3->AddGrowableCol(0);
    StaticBoxSizerADPD3->Add(m_ADPD_controlsSizer3);

    StaticText_2 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_2, _T("FFT window:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_2"));
    m_ADPD_controlsSizer3->Add(StaticText_2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    cmbWindowFunction = new wxComboBox(m_ADPD_tabSpectrum, ID_COMBOBOX_WINDOW, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX_WINDOW"));
    cmbWindowFunction->SetSelection(cmbWindowFunction->Append(_T("No window")));
    cmbWindowFunction->Append(_T("B.-Harris"));
    cmbWindowFunction->Append(_T("Hamming"));
    cmbWindowFunction->Append(_T("Hanning"));
    Connect(ID_COMBOBOX_WINDOW, wxEVT_COMMAND_COMBOBOX_SELECTED, (wxObjectEventFunction)&DPDTest::OncmbWindowFunctionSelected);
    m_ADPD_controlsSizer3->Add(cmbWindowFunction, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_FFT1 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_FFT1, _T("Samples=2**x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FFT1"));
    m_ADPD_controlsSizer3->Add(StaticText_FFT1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    m_ADPD_txtFFT1 = new wxSpinCtrl(m_ADPD_tabSpectrum, ID_TEXTCTRL_FFT1, _T("14"), wxDefaultPosition, wxSize(60, -1), 0, 10, 14, 2, _T("ID_TEXTCTRL_FFT1"));
    m_ADPD_txtFFT1->SetValue(_T("14"));
    m_ADPD_controlsSizer3->Add(m_ADPD_txtFFT1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_FFT2 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_FFT2, _T("Ave=2**x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FFT2"));
    m_ADPD_controlsSizer3->Add(StaticText_FFT2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    m_ADPD_txtFFT2 = new wxSpinCtrl(m_ADPD_tabSpectrum, ID_TEXTCTRL_FFT1, _T("3"), wxDefaultPosition, wxSize(60, -1), 0, 0, 6, 2, _T("ID_TEXTCTRL_FFT2"));
    m_ADPD_txtFFT2->SetValue(_T("3"));
    m_ADPD_controlsSizer3->Add(m_ADPD_txtFFT2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_FFT3 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_FFT3, _T("Clock[MHz]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FFT3"));
    m_ADPD_controlsSizer3->Add(StaticText_FFT3, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    m_ADPD_txtFFT3 = new wxTextCtrl(m_ADPD_tabSpectrum, ID_TEXTCTRL_FFT3, _T("20.000"), wxDefaultPosition, wxSize(60, -1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_FFT3"));
    m_ADPD_txtFFT3->SetExtraStyle(m_ADPD_txtFFT3->GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    m_ADPD_controlsSizer3->Add(m_ADPD_txtFFT3, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    m_ADPD_controlsSizer_Y = new wxFlexGridSizer(0, 2, 0, 0);
    m_ADPD_controlsSizer_X = new wxFlexGridSizer(0, 2, 0, 0);

    StaticBoxSizerADPD1 = new wxStaticBoxSizer(wxVERTICAL, m_ADPD_tabSpectrum, _T("QADPD setup"));

    m_ADPD_controlsSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    m_ADPD_controlsSizer2->AddGrowableCol(0);
    StaticBoxSizerADPD1->Add(m_ADPD_controlsSizer2);

    StaticText_ADPD0 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD0, _T("N (mem.):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD0"));
    m_ADPD_controlsSizer2->Add(StaticText_ADPD0, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    spin_ADPD_N = new wxSpinCtrl(m_ADPD_tabSpectrum, ID_SPINCTRL_ADPD_N, _T("2"), wxDefaultPosition, wxSize(60, -1), 0, 0, 4, 2, _T("ID_SPINCTRL_ADPD_N"));
    spin_ADPD_N->SetValue(_T("2"));
    m_ADPD_controlsSizer2->Add(spin_ADPD_N, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_ADPD1 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD1, _T("M (nonl.):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD1"));
    m_ADPD_controlsSizer2->Add(StaticText_ADPD1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    spin_ADPD_M = new wxSpinCtrl(m_ADPD_tabSpectrum, ID_SPINCTRL_ADPD_M, _T("2"), wxDefaultPosition, wxSize(60, -1), 0, 0, 3, 2, _T("ID_SPINCTRL_ADPD_M")); //
    spin_ADPD_M->SetValue(_T("2"));
    m_ADPD_controlsSizer2->Add(spin_ADPD_M, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_ADPD3 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD3, _T("Lambda:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD3"));
    m_ADPD_controlsSizer2->Add(StaticText_ADPD3, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    m_ADPD_txtLambda = new wxTextCtrl(m_ADPD_tabSpectrum, ID_TEXTCTRL_ADPD2, _T("0.998"), wxDefaultPosition, wxSize(60, -1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_ADPD2"));
    m_ADPD_txtLambda->SetExtraStyle(m_ADPD_txtLambda->GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    m_ADPD_controlsSizer2->Add(m_ADPD_txtLambda, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_12 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_12, _T("Train cyc.:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_12"));
    m_ADPD_controlsSizer2->Add(StaticText_12, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    m_ADPD_txtTrain = new wxTextCtrl(m_ADPD_tabSpectrum, ID_TEXTCTRL_12, _T("1"), wxDefaultPosition, wxSize(60, -1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_12"));
    m_ADPD_txtTrain->SetExtraStyle(m_ADPD_txtTrain->GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    m_ADPD_controlsSizer2->Add(m_ADPD_txtTrain, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_11 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD11, _T("ND (delay):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD11"));
    m_ADPD_controlsSizer2->Add(StaticText_11, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    spin_ADPD_Delay = new wxSpinCtrl(m_ADPD_tabSpectrum, ID_SPINCTRL_ADPD_DELAY, _T("0"), wxDefaultPosition, wxSize(60, -1), 0, 0, 2000, 2, _T("ID_SPINCTRL_ADPD_DELAY"));
    spin_ADPD_Delay->SetValue(_T("0"));
    m_ADPD_controlsSizer2->Add(spin_ADPD_Delay, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    StaticText_ADPD6 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD10, _T(""), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD10"));
    m_ADPD_controlsSizer2->Add(StaticText_ADPD6, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    Button_CAL = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_CAL, _T("Calibrate"), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_CAL"));
    m_ADPD_controlsSizer2->Add(Button_CAL, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Connect(ID_BUTTON_CAL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnCalibrate);

    StaticText_13 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD13, _T("Gain:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD13"));
    m_ADPD_controlsSizer2->Add(StaticText_13, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);

    m_ADPD_txtGain = new wxSpinCtrlDouble(m_ADPD_tabSpectrum, wxNewId(), _("1.0"), wxDefaultPosition, wxSize(100, -1), 0, 0.3, 6.0, 1.0, 0.01);
    m_ADPD_controlsSizer2->Add(m_ADPD_txtGain, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    Connect(m_ADPD_txtGain->GetId(), wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(DPDTest::OnGainChanged), NULL, this);

    StaticText_ADPD2 = new wxStaticText(m_ADPD_tabSpectrum, ID_STATICTEXT_ADPD2, _T(""), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ADPD2"));
    m_ADPD_controlsSizer2->Add(StaticText_ADPD2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    Button_CAL2 = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_CAL2, _T("Calibrate"), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_CAL2"));
    m_ADPD_controlsSizer2->Add(Button_CAL2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    Connect(ID_BUTTON_CAL2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnCalibrate2);

    StaticBoxSizerADPD5 = new wxStaticBoxSizer(wxVERTICAL, m_ADPD_tabSpectrum, _T("Train QADPD"));
    m_ADPD_controlsSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    m_ADPD_controlsSizer5->AddGrowableCol(0);
    StaticBoxSizerADPD5->Add(m_ADPD_controlsSizer5);

    Button_START = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_START, _T("Start"), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_START"));
    m_ADPD_controlsSizer5->Add(Button_START, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_START, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnStartClick);

    Button_END = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_END, _T("End"), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_END"));
    m_ADPD_controlsSizer5->Add(Button_END, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_END, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnEndClick);

    wxString __wxValue[2] = {_("One step"), _("Continuous")};
    TrainMode = new wxRadioBox(m_ADPD_tabSpectrum, ID_TRAINMODE, _T(""), wxDefaultPosition, wxDefaultSize, 2, __wxValue, 0, wxVERTICAL, wxDefaultValidator, "");
    Connect(ID_TRAINMODE, wxEVT_COMMAND_RADIOBOX_SELECTED, (wxObjectEventFunction)&DPDTest::onTrainMode);

    m_ADPD_controlsSizer5->Add(TrainMode, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    TrainMode->Enable(true);
    TrainMode->SetSelection(0);

    StaticBoxSizerADPD6 = new wxStaticBoxSizer(wxVERTICAL, m_ADPD_tabSpectrum, _T("One step"));
    m_ADPD_controlsSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    m_ADPD_controlsSizer6->AddGrowableCol(0);
    m_ADPD_controlsSizer6->AddGrowableRow(0);
    StaticBoxSizerADPD6->Add(m_ADPD_controlsSizer6);

    btnCapture = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_CAPTURE, _T("Read"), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_CAPTURE"));
    m_ADPD_controlsSizer6->Add(btnCapture, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_CAPTURE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnCaptureClicked);

    Button_TRAIN = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_TRAIN, _T("Train"), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_TRAIN"));
    m_ADPD_controlsSizer6->Add(Button_TRAIN, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_TRAIN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnTrainClick);

    Button_SEND = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_SEND, _T("Send coeff."), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_SEND"));
    m_ADPD_controlsSizer6->Add(Button_SEND, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_SEND, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnSendClick);

    Button_TIMER = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_TIMER, _T("Reset coeff."), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_TIMER"));
    m_ADPD_controlsSizer6->Add(Button_TIMER, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_TIMER, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnTimerClick);

    Button_READ = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_READ, _T("Read coeff."), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_READ"));
    m_ADPD_controlsSizer6->Add(Button_READ, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_READ, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnReadClick);

    Button_SAVE = new wxButton(m_ADPD_tabSpectrum, ID_BUTTON_SAVE, _T("Save coeff."), wxDefaultPosition, wxSize(100, -1), 0, wxDefaultValidator, _T("ID_BUTTON_SAVE"));
    m_ADPD_controlsSizer6->Add(Button_SAVE, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);
    Connect(ID_BUTTON_SAVE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnSaveClick);

    CheckBox_Train = new wxCheckBox(m_ADPD_tabSpectrum, ID_CHECKBOX_TRAIN, _T("Cont.train"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_TRAIN"));
    CheckBox_Train->SetValue(true);
    Connect(ID_CHECKBOX_TRAIN, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&DPDTest::OnbtnTrain);

    m_ADPD_controlsSizer5->Add(CheckBox_Train, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    m_ADPD_controlsSizer_Y->Add(StaticBoxSizerADPD5);
    m_ADPD_controlsSizer_Y->Add(StaticBoxSizerADPD3);

    m_ADPD_controlsSizer_X->Add(StaticBoxSizerADPD6);
    m_ADPD_controlsSizer_X->Add(StaticBoxSizerADPD1);

    m_ADPD_controlsSizer->Add(m_ADPD_controlsSizer_Y);
    m_ADPD_controlsSizer->Add(m_ADPD_controlsSizer_X);

    FlexGridSizer1->Add(m_MainTabs, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    SetSize(800, 600);

    m_MainTabs->AddPage(m_ADPD_tabSpectrum, _T(""), false);

    m_plot_ADPD0->settings.useVBO = true;
    m_plot_ADPD0->settings.title = "Plot 1";
    m_plot_ADPD0->settings.markersEnabled = true;
    m_plot_ADPD0->settings.marginLeft = 40;
    m_plot_ADPD0->settings.staticGrid = true;
    m_plot_ADPD0->AddSerie(new cDataSerie());
    m_plot_ADPD0->AddSerie(new cDataSerie());
    m_plot_ADPD0->AddSerie(new cDataSerie());
    m_plot_ADPD0->AddSerie(new cDataSerie());

    m_plot_ADPD1->settings.title = "Plot 2";
    m_plot_ADPD1->settings.markersEnabled = true;
    m_plot_ADPD1->settings.marginLeft = 40;
    m_plot_ADPD1->settings.staticGrid = true;
    m_plot_ADPD1->AddSerie(new cDataSerie());
    m_plot_ADPD1->AddSerie(new cDataSerie());
    m_plot_ADPD1->AddSerie(new cDataSerie());
    m_plot_ADPD1->AddSerie(new cDataSerie());

    m_plot_ADPD2->settings.title = "Plot 3";
    m_plot_ADPD2->settings.markersEnabled = true;
    m_plot_ADPD2->settings.marginLeft = 40;
    m_plot_ADPD2->settings.staticGrid = true;
    m_plot_ADPD2->AddSerie(new cDataSerie());
    m_plot_ADPD2->AddSerie(new cDataSerie());
    m_plot_ADPD2->AddSerie(new cDataSerie());
    m_plot_ADPD2->AddSerie(new cDataSerie());

    m_plot_ADPD3->settings.title = "Plot 4";
    m_plot_ADPD3->settings.markersEnabled = true;
    m_plot_ADPD3->settings.marginLeft = 40;
    m_plot_ADPD3->settings.staticGrid = true;
    m_plot_ADPD3->AddSerie(new cDataSerie());
    m_plot_ADPD3->AddSerie(new cDataSerie());
    m_plot_ADPD3->AddSerie(new cDataSerie());
    m_plot_ADPD3->AddSerie(new cDataSerie());

    wxFlexGridSizer *sizerPlots;
    sizerPlots = new wxFlexGridSizer(4, 2, 0, 0);
    sizerPlots->AddGrowableCol(0);
    sizerPlots->AddGrowableCol(1);
    sizerPlots->AddGrowableRow(0);
    sizerPlots->AddGrowableRow(1);
    sizerPlots->AddGrowableRow(2);

    xp_samples = new kiss_fft_cpx[32768];
    yp_samples = new kiss_fft_cpx[32768];
    x_samples = new kiss_fft_cpx[32768];
    y_samples = new kiss_fft_cpx[32768];
    xp1_samples = new kiss_fft_cpx[32768];
    yp1_samples = new kiss_fft_cpx[32768];
    x1_samples = new kiss_fft_cpx[32768];
    y1_samples = new kiss_fft_cpx[32768];
    u_samples = new kiss_fft_cpx[32768];
    error_samples = new kiss_fft_cpx[32768];
    xp_fft = new kiss_fft_cpx[32768];
    yp_fft = new kiss_fft_cpx[32768];
    x_fft = new kiss_fft_cpx[32768];
    y_fft = new kiss_fft_cpx[32768];

    for (int i = 0; i < 32768; i++)
    {
        xp_samples[i].i = 0;
        xp_samples[i].r = 0;
        yp_samples[i].i = 0;
        yp_samples[i].r = 0;

        x_samples[i].i = 0;
        x_samples[i].r = 0;
        y_samples[i].i = 0;
        y_samples[i].r = 0;

        xp1_samples[i].i = 0;
        xp1_samples[i].r = 0;
        yp1_samples[i].i = 0;
        yp1_samples[i].r = 0;

        x1_samples[i].i = 0;
        x1_samples[i].r = 0;
        y1_samples[i].i = 0;
        y1_samples[i].r = 0;

        u_samples[i].i = 0;
        u_samples[i].r = 0;
        error_samples[i].i = 0;
        error_samples[i].r = 0;

        xp_fft[i].i = 0;
        xp_fft[i].r = 0;
        yp_fft[i].i = 0;
        yp_fft[i].r = 0;

        x_fft[i].i = 0;
        x_fft[i].r = 0;
        y_fft[i].i = 0;
        y_fft[i].r = 0;
    }
    QADPD_N = 2;
    QADPD_M = 2;
    QADPD_ND = 0;
    QADPD_AM = 14;
    QADPD_SKIP = 0;
    QADPD_GAIN = 1.0;
    QADPD_LAMBDA = 0.998;
    QADPD_YPFPGA = true;
    QADPD_FFTSAMPLES = 16384;
    QADPD_SPAN = 20.0;
    QADPD_UPDATE = 1;
    OpenConfig(); 

    Button_START->Enable(false);
    Button_TRAIN->Enable(false);
    Button_END->Enable(false);
    Button_SEND->Enable(false);

    spin_ADPD_N->Enable(true);
    spin_ADPD_M->Enable(true);
    spin_ADPD_Delay->Enable(true);
    m_ADPD_txtGain->Enable(true);
    m_ADPD_txtLambda->Enable(true);

    m_ADPD_txtTrain->Enable(true);
    TrainMode->Enable(false);

    btnCapture->Enable(false);
    Button_TIMER->Enable(false);
    Button_READ->Enable(false);
    Button_SAVE->Enable(false);
    timer_enabled = false;

    rbChannelA->Enable(true);
    rbChannelB->Enable(true);

    CheckBox_Train->Enable(false);
    Button_CAL->Enable(false);
    Button_CAL2->Enable(false);

    samplesReceived = 4 * 8192;
    chA = 1;
    SelectSource(chA);    
    OpenConfig();
    
    Qadpd = new qadpd(QADPD_N, QADPD_M, QADPD_ND);
    Qadpd->enlog = true;

    for (int i = 0; i <= 8; i++)
    {
        for (int j = 0; j <= 3; j++)
        {
            coeffa_chA[i][j] = 0.0;
            coeffb_chA[i][j] = 0.0;
            coeffa_chB[i][j] = 0.0;
            coeffb_chB[i][j] = 0.0;
        }
    }

    Connect(ID_READING_FINISHED_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&DPDTest::OnReadingFinished);
    fname = "log.txt";
}

void DPDTest::OncmbWindowFunctionSelected(wxCommandEvent &event)
{
    m_iWindowFunc = cmbWindowFunction->GetSelection();
    SaveConfig();
}

void DPDTest::OnCenterSpanChange(OpenGLGraph *plot, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
                                 int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot)
{
    double dFreq = m_dCenterFreqRatioPlot * (pow(10.0, 3 * (m_iCenterFreqRatioPlot + 1)));
    double dSpan = m_dFreqSpanRatioPlot * (pow(10.0, 3 * (m_iFreqSpanRatioPlot + 1)));
    plot->ZoomX(dFreq, dSpan);
}

void DPDTest::OnYaxisChange(OpenGLGraph *plot, int m_iYaxisTopPlot, int m_iYaxisBottomPlot)
{
    plot->ZoomY((m_iYaxisTopPlot + m_iYaxisBottomPlot) / 2, abs(m_iYaxisTopPlot - m_iYaxisBottomPlot));
}

void DPDTest::OpenConfig()
{
    //load gui configuration from file

    iniParser m_options;
    std::string temps;
    std::string temps_CHA;
    std::string temps_CHB;
    int tempi_CHA;
    int tempi_CHB;

    double tempd;
    m_options.Open(config_filename);
    m_options.SelectSection("QADPD");

    m_iValuePlot1 = m_options.Get("ValuePlot1", 0);
    m_iYaxisTopPlot1 = m_options.Get("YaxisTopPlot1", 0);
    m_iYaxisBottomPlot1 = m_options.Get("YaxisBottomPlot1", -100);
    m_iCenterFreqRatioPlot1 = m_options.Get("iCenterFreqRatioPlot1", 0);
    m_iFreqSpanRatioPlot1 = m_options.Get("iFreqSpanRatioPlot1", 0);
    temps = m_options.Get("dCenterFreqRatioPlot1", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dCenterFreqRatioPlot1 = tempd;
    temps = m_options.Get("dFreqSpanRatioPlot1", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dFreqSpanRatioPlot1 = tempd;
    m_iXaxisLeftPlot1 = m_options.Get("XaxisLeftPlot1", 0);
    m_iXaxisRightPlot1 = m_options.Get("XaxisRightPlot1", 1000);

    m_iValuePlot2 = m_options.Get("ValuePlot2", 0);
    m_iYaxisTopPlot2 = m_options.Get("YaxisTopPlot2", 0);
    m_iYaxisBottomPlot2 = m_options.Get("YaxisBottomPlot2", -100);
    m_iCenterFreqRatioPlot2 = m_options.Get("iCenterFreqRatioPlot2", 0);
    m_iFreqSpanRatioPlot2 = m_options.Get("iFreqSpanRatioPlot2", 0);
    temps = m_options.Get("dCenterFreqRatioPlot2", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dCenterFreqRatioPlot2 = tempd;
    temps = m_options.Get("dFreqSpanRatioPlot2", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dFreqSpanRatioPlot2 = tempd;
    m_iXaxisLeftPlot2 = m_options.Get("XaxisLeftPlot2", 0);
    m_iXaxisRightPlot2 = m_options.Get("XaxisRightPlot2", 1000);

    m_iValuePlot3 = m_options.Get("ValuePlot3", 0);
    m_iYaxisTopPlot3 = m_options.Get("YaxisTopPlot3", 0);
    m_iYaxisBottomPlot3 = m_options.Get("YaxisBottomPlot3", -100);
    m_iCenterFreqRatioPlot3 = m_options.Get("iCenterFreqRatioPlot3", 0);
    m_iFreqSpanRatioPlot3 = m_options.Get("iFreqSpanRatioPlot3", 0);
    temps = m_options.Get("dCenterFreqRatioPlot3", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dCenterFreqRatioPlot3 = tempd;
    temps = m_options.Get("dFreqSpanRatioPlot3", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dFreqSpanRatioPlot3 = tempd;
    m_iXaxisLeftPlot3 = m_options.Get("XaxisLeftPlot3", 0);
    m_iXaxisRightPlot3 = m_options.Get("XaxisRightPlot3", 1000);

    m_iValuePlot4 = m_options.Get("ValuePlot4", 0);
    m_iYaxisTopPlot4 = m_options.Get("YaxisTopPlot4", 0);
    m_iYaxisBottomPlot4 = m_options.Get("YaxisBottomPlot4", -100);
    m_iCenterFreqRatioPlot4 = m_options.Get("iCenterFreqRatioPlot4", 0);
    m_iFreqSpanRatioPlot4 = m_options.Get("iFreqSpanRatioPlot4", 0);
    temps = m_options.Get("dCenterFreqRatioPlot4", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dCenterFreqRatioPlot4 = tempd;
    temps = m_options.Get("dFreqSpanRatioPlot4", "0.0");
    ((wxString)temps).ToDouble(&tempd);
    m_dFreqSpanRatioPlot4 = tempd;
    m_iXaxisLeftPlot4 = m_options.Get("XaxisLeftPlot4", 0);
    m_iXaxisRightPlot4 = m_options.Get("XaxisRightPlot4", 1000);

    tempi_CHA = m_options.Get("QADPD_CHA_N", 2);
    tempi_CHB = m_options.Get("QADPD_CHB_N", 2);

    if (chA == 1)
        QADPD_N = tempi_CHA;
    else
        QADPD_N = tempi_CHB;
    spin_ADPD_N->SetValue(QADPD_N);

    tempi_CHA = m_options.Get("QADPD_CHA_M", 2);
    tempi_CHB = m_options.Get("QADPD_CHB_M", 2);

    if (chA == 1)
        QADPD_M = tempi_CHA;
    else
        QADPD_M = tempi_CHB;
    spin_ADPD_M->SetValue(QADPD_M);

    QADPD_AM = 14;

    tempi_CHA = m_options.Get("QADPD_CHA_ND", 0);
    tempi_CHB = m_options.Get("QADPD_CHB_ND", 0);
    if (chA == 1)
        QADPD_ND = tempi_CHA;
    else
        QADPD_ND = tempi_CHB;
    spin_ADPD_Delay->SetValue(QADPD_ND);

    QADPD_SKIP = m_options.Get("QADPD_SKIP", 0);
    QADPD_UPDATE = m_options.Get("QADPD_UPDATE", 1);

    wxString temps1;
    temps1.Printf(_T("%d"), QADPD_SKIP);
    temps1.Printf(_T("%d"), QADPD_UPDATE);
    m_ADPD_txtTrain->SetValue(temps1);

    if (m_options.Get("QADPD_YPFPGA", 1) == 1)
        QADPD_YPFPGA = true;
    else
        QADPD_YPFPGA = false;
    QADPD_YPFPGA = true;

    temps_CHA = m_options.Get("QADPD_CHA_GAIN", "1.000");
    temps_CHB = m_options.Get("QADPD_CHB_GAIN", "1.000");
    if (chA == 1)
        temps = temps_CHA;
    else
        temps = temps_CHB;
    m_ADPD_txtGain->SetValue(temps);
    ((wxString)temps).ToDouble(&tempd);
    QADPD_GAIN = tempd;

    temps_CHA = m_options.Get("QADPD_CHA_LAMBDA", "0.998");
    temps_CHB = m_options.Get("QADPD_CHB_LAMBDA", "0.998");
    if (chA == 1)
        temps = temps_CHA;
    else
        temps = temps_CHB;
    m_ADPD_txtLambda->SetValue(temps);
    ((wxString)temps).ToDouble(&tempd);
    QADPD_LAMBDA = tempd;

    int tempi = 0;
    tempi = m_options.Get("Plots", 15);
    if ((tempi & 0x01) == 0x01)
        CheckBox_PLOT1->SetValue(true);
    else
        CheckBox_PLOT1->SetValue(false);
    if ((tempi & 0x02) == 0x02)
        CheckBox_PLOT2->SetValue(true);
    else
        CheckBox_PLOT2->SetValue(false);
    if ((tempi & 0x04) == 0x04)
        CheckBox_PLOT3->SetValue(true);
    else
        CheckBox_PLOT3->SetValue(false);
    if ((tempi & 0x08) == 0x08)
        CheckBox_PLOT4->SetValue(true);
    else
        CheckBox_PLOT4->SetValue(false);

    QADPD_FFT1 = m_options.Get("QADPD_FFT1", 14);
    QADPD_FFTSAMPLES = pow(2, QADPD_FFT1);
    temps1.Printf(_T("%d"), QADPD_FFT1);
    m_ADPD_txtFFT1->SetValue(temps1);

    QADPD_FFT2 = m_options.Get("QADPD_FFT2", 3);
    temps1.Printf(_T("%d"), QADPD_FFT2);
    m_ADPD_txtFFT2->SetValue(temps1);

    temps = m_options.Get("QADPD_FFT3", "20.0");
    m_ADPD_txtFFT3->SetValue(temps);
    ((wxString)temps).ToDouble(&tempd);
    QADPD_SPAN = tempd;
    m_iWindowFunc = m_options.Get("QADPD_WINDOWFUNC", 0);
    cmbWindowFunction->SetSelection(m_iWindowFunc);
    OnChangePlot();
}

void DPDTest::SaveConfig()
{

    wxString temps;
    iniParser m_options;
    m_options.Open(config_filename);

    m_options.SelectSection("QADPD");
    m_options.Set("ValuePlot1", m_iValuePlot1);
    m_options.Set("YaxisTopPlot1", m_iYaxisTopPlot1);
    m_options.Set("YaxisBottomPlot1", m_iYaxisBottomPlot1);
    m_options.Set("iCenterFreqRatioPlot1", m_iCenterFreqRatioPlot1);
    m_options.Set("iFreqSpanRatioPlot1", m_iFreqSpanRatioPlot1);
    temps.Printf(_T("%9.4f"), m_dCenterFreqRatioPlot1);
    m_options.Set("dCenterFreqRatioPlot1", temps.ToAscii());
    temps.Printf(_T("%9.4f"), m_dFreqSpanRatioPlot1);
    m_options.Set("dFreqSpanRatioPlot1", temps.ToAscii());
    m_options.Set("XaxisLeftPlot1", m_iXaxisLeftPlot1);
    m_options.Set("XaxisRightPlot1", m_iXaxisRightPlot1);

    m_options.Set("ValuePlot2", m_iValuePlot2);
    m_options.Set("YaxisTopPlot2", m_iYaxisTopPlot2);
    m_options.Set("YaxisBottomPlot2", m_iYaxisBottomPlot2);
    m_options.Set("iCenterFreqRatioPlot2", m_iCenterFreqRatioPlot2);
    m_options.Set("iFreqSpanRatioPlot2", m_iFreqSpanRatioPlot2);
    temps.Printf(_T("%9.4f"), m_dCenterFreqRatioPlot2);
    m_options.Set("dCenterFreqRatioPlot2", temps.ToAscii());
    temps.Printf(_T("%9.4f"), m_dFreqSpanRatioPlot2);
    m_options.Set("dFreqSpanRatioPlot2", temps.ToAscii());
    m_options.Set("XaxisLeftPlot2", m_iXaxisLeftPlot2);
    m_options.Set("XaxisRightPlot2", m_iXaxisRightPlot2);

    m_options.Set("ValuePlot3", m_iValuePlot3);
    m_options.Set("YaxisTopPlot3", m_iYaxisTopPlot3);
    m_options.Set("YaxisBottomPlot3", m_iYaxisBottomPlot3);
    m_options.Set("iCenterFreqRatioPlot3", m_iCenterFreqRatioPlot3);
    m_options.Set("iFreqSpanRatioPlot3", m_iFreqSpanRatioPlot3);
    temps.Printf(_T("%9.4f"), m_dCenterFreqRatioPlot3);
    m_options.Set("dCenterFreqRatioPlot3", temps.ToAscii());
    temps.Printf(_T("%9.4f"), m_dFreqSpanRatioPlot3);
    m_options.Set("dFreqSpanRatioPlot3", temps.ToAscii());
    m_options.Set("XaxisLeftPlot3", m_iXaxisLeftPlot3);
    m_options.Set("XaxisRightPlot3", m_iXaxisRightPlot3);

    m_options.Set("ValuePlot4", m_iValuePlot4);
    m_options.Set("YaxisTopPlot4", m_iYaxisTopPlot4);
    m_options.Set("YaxisBottomPlot4", m_iYaxisBottomPlot4);
    m_options.Set("iCenterFreqRatioPlot4", m_iCenterFreqRatioPlot4);
    m_options.Set("iFreqSpanRatioPlot4", m_iFreqSpanRatioPlot4);
    temps.Printf(_T("%9.4f"), m_dCenterFreqRatioPlot4);
    m_options.Set("dCenterFreqRatioPlot4", temps.ToAscii());
    temps.Printf(_T("%9.4f"), m_dFreqSpanRatioPlot4);
    m_options.Set("dFreqSpanRatioPlot4", temps.ToAscii());
    m_options.Set("XaxisLeftPlot4", m_iXaxisLeftPlot4);
    m_options.Set("XaxisRightPlot4", m_iXaxisRightPlot4);

    long temp;
    double tempd;
    QADPD_N = spin_ADPD_N->GetValue();
    QADPD_M = spin_ADPD_M->GetValue();
    QADPD_ND = spin_ADPD_Delay->GetValue();
    QADPD_AM = 14;
    QADPD_SKIP = 100;
    m_ADPD_txtTrain->GetValue().ToLong(&temp);
    if ((temp >= 0) && (temp <= 32768))
        QADPD_UPDATE = (int)temp;
    else
    {
        QADPD_UPDATE = 1;
        m_ADPD_txtTrain->SetValue("1");
    }

    tempd = m_ADPD_txtGain->GetValue();
    QADPD_GAIN = tempd;

    m_ADPD_txtLambda->GetValue().ToDouble(&tempd);
    QADPD_LAMBDA = tempd;

    QADPD_YPFPGA = true;

    if (chA == 1)
        m_options.Set("QADPD_CHA_N", QADPD_N);
    else
        m_options.Set("QADPD_CHB_N", QADPD_N);
    if (chA == 1)
        m_options.Set("QADPD_CHA_M", QADPD_M);
    else
        m_options.Set("QADPD_CHB_M", QADPD_M);
    m_options.Set("QADPD_AM", QADPD_AM);
    m_options.Set("QADPD_SKIP", QADPD_SKIP);
    m_options.Set("QADPD_UPDATE", QADPD_UPDATE);

    if (chA == 1)
        m_options.Set("QADPD_CHA_ND", QADPD_ND);
    else
        m_options.Set("QADPD_CHB_ND", QADPD_ND);

    if (QADPD_YPFPGA == true)
        m_options.Set("QADPD_YPFPGA", 1);
    else
        m_options.Set("QADPD_YPFPGA", 0);

    temps.Printf(_T("%9.4f"), QADPD_GAIN);
    if (chA == 1)
        m_options.Set("QADPD_CHA_GAIN", temps.ToAscii());
    else
        m_options.Set("QADPD_CHB_GAIN", temps.ToAscii());

    temps.Printf(_T("%13.8f"), QADPD_LAMBDA);
    if (chA == 1)
        m_options.Set("QADPD_CHA_LAMBDA", temps.ToAscii());
    else
        m_options.Set("QADPD_CHB_LAMBDA", temps.ToAscii());

    int tempi = 0;
    if (CheckBox_PLOT1->IsChecked())
        tempi += 1;
    if (CheckBox_PLOT2->IsChecked())
        tempi += 2;
    if (CheckBox_PLOT3->IsChecked())
        tempi += 4;
    if (CheckBox_PLOT4->IsChecked())
        tempi += 8;
    m_options.Set("Plots", tempi);

    temp = m_ADPD_txtFFT1->GetValue();
    QADPD_FFT1 = (int)temp;
    m_options.Set("QADPD_FFT1", QADPD_FFT1);
    QADPD_FFTSAMPLES = pow(2, QADPD_FFT1);

    temp = m_ADPD_txtFFT2->GetValue();
    QADPD_FFT2 = (int)temp;
    m_options.Set("QADPD_FFT2", QADPD_FFT2);

    temps.Printf(_T("%9.4f"), QADPD_SPAN);
    m_options.Set("QADPD_FFT3", temps.ToAscii());

    m_iWindowFunc = cmbWindowFunction->GetSelection();
    m_options.Set("QADPD_WINDOWFUNC", m_iWindowFunc);
    m_options.Save(config_filename);
}

void DPDTest::OnbtnReadClick(wxCommandEvent &event)
{
    read_coef();
    update_coef();
}

void DPDTest::OnbtnSaveClick(wxCommandEvent &event)
{
    save_coef();
}

void DPDTest::save_coef()
{

    int i = 0;
    int j = 0;

    wxString temps;
    iniParser m_options;
    m_options.Open(config_filename);
    m_options.SelectSection("QADPD");

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++)
        {

            if (chA == 1)
            {
                temps.Printf(_T("%10.6f"), coeffa_chA[i][j]);
                m_options.Set(wxString::Format("coeffa_chA_%0d_%0d", i, j), temps.ToAscii());

                temps.Printf(_T("%10.6f"), coeffb_chA[i][j]);
                m_options.Set(wxString::Format("coeffb_chA_%0d_%0d", i, j), temps.ToAscii());
            }
            else
            {
                temps.Printf(_T("%10.6f"), coeffa_chB[i][j]);
                m_options.Set(wxString::Format("coeffa_chB_%0d_%0d", i, j), temps.ToAscii());

                temps.Printf(_T("%10.6f"), coeffb_chB[i][j]);
                m_options.Set(wxString::Format("coeffb_chB_%0d_%0d", i, j), temps.ToAscii());
            }
        }
    }

    m_options.Save(config_filename);
}

void DPDTest::read_coef()
{

    int i = 0;
    int j = 0;

    wxString temps;
    iniParser m_options;

    double tempd;
    m_options.Open(config_filename);
    m_options.SelectSection("QADPD");

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++)
        {

            if (chA == 1)
            {
                temps = m_options.Get(wxString::Format("coeffa_chA_%0d_%0d", i, j), "0.000");
                ((wxString)temps).ToDouble(&tempd);
                coeffa_chA[i][j] = tempd;
                temps = m_options.Get(wxString::Format("coeffb_chA_%0d_%0d", i, j), "0.000");
                ((wxString)temps).ToDouble(&tempd);
                coeffb_chA[i][j] = tempd;
            }
            else
            {
                temps = m_options.Get(wxString::Format("coeffa_chB_%0d_%0d", i, j), "0.000");
                ((wxString)temps).ToDouble(&tempd);
                coeffa_chB[i][j] = tempd;

                temps = m_options.Get(wxString::Format("coeffb_chB_%0d_%0d", i, j), "0.000");
                ((wxString)temps).ToDouble(&tempd);
                coeffb_chB[i][j] = tempd;
            }
        }
    }
}

void DPDTest::OnChangePlotVisibility(wxCommandEvent &event)
{
    OnChangePlot();
    SaveConfig();
}

void DPDTest::OnChangePlot()
{
    bool m_cPlot1 = CheckBox_PLOT1->IsChecked();
    bool m_cPlot2 = CheckBox_PLOT2->IsChecked();
    bool m_cPlot3 = CheckBox_PLOT3->IsChecked();
    bool m_cPlot4 = CheckBox_PLOT4->IsChecked();

    int temp = 0;
    if (m_cPlot1 == true)
    {
        temp = temp + 1;
        Panel_ADPD0->Show(true);
        Panel_ADPD0->Show(true);
        Panel_ADPD0->Enable(true);
        Panel_ADPD0->Enable(true);
    }
    else
    {
        Panel_ADPD0->Show(false);
        Panel_ADPD0->Show(false);
        Panel_ADPD0->Enable(false);
        Panel_ADPD0->Enable(false);
    }

    if (m_cPlot2 == true)
    {
        temp = temp + 2;
        Panel_ADPD1->Show(true);
        Panel_ADPD1->Show(true);
        Panel_ADPD1->Enable(true);
        Panel_ADPD1->Enable(true);
    }
    else
    {
        Panel_ADPD1->Show(false);
        Panel_ADPD1->Show(false);
        Panel_ADPD1->Enable(false);
        Panel_ADPD1->Enable(false);
    }

    if (m_cPlot3 == true)
    {
        temp = temp + 4;
        Panel_ADPD2->Show(true);
        Panel_ADPD2->Show(true);
        Panel_ADPD2->Enable(true);
        Panel_ADPD2->Enable(true);
    }
    else
    {
        Panel_ADPD2->Show(false);
        Panel_ADPD2->Show(false);
        Panel_ADPD2->Enable(false);
        Panel_ADPD2->Enable(false);
    }
    if (m_cPlot4 == true)
    {
        temp = temp + 8;
        Panel_ADPD3->Show(true);
        Panel_ADPD3->Show(true);
        Panel_ADPD3->Enable(true);
        Panel_ADPD3->Enable(true);
    }
    else
    {
        Panel_ADPD3->Show(false);
        Panel_ADPD3->Show(false);
        Panel_ADPD3->Enable(false);
        Panel_ADPD3->Enable(false);
    }

    m_ADPD_plotSplitter_up->Unsplit();
    m_ADPD_plotSplitter_down->Unsplit();
    m_ADPD_plotSplitter->Unsplit();

    switch (temp)
    {
    case 0: //Panel_ADPD0
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        m_ADPD_plotSplitter->Unsplit();
        CheckBox_PLOT1->SetValue(true);
        break;
    case 1: // Panel_ADPD0
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        m_ADPD_plotSplitter->Unsplit();
        break;
    case 2: //Panel_ADPD2
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_down, m_ADPD_plotSplitter_up);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        m_ADPD_plotSplitter->Unsplit();
        break;
    case 3: //Panel_ADPD0, Panel_ADPD2
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        //m_ADPD_plotSplitter->Unsplit();
        break;
    case 4: //Panel_ADPD1
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD2, Panel_ADPD0);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD3, Panel_ADPD1);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        m_ADPD_plotSplitter->Unsplit();
        break;
    case 5: //Panel_ADPD0, Panel_ADPD1
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD3, Panel_ADPD1);
        m_ADPD_plotSplitter->Unsplit();
        break;
    case 6: //Panel_ADPD2, Panel_ADPD1
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD2, Panel_ADPD0);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        break;
    case 7: //Panel_ADPD0, Panel_ADPD2, Panel_ADPD1
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_down->Unsplit();
        break;
    case 8:
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_down, m_ADPD_plotSplitter_up);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD2, Panel_ADPD0);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD3, Panel_ADPD1);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        m_ADPD_plotSplitter->Unsplit();
        break;
    case 9: //Panel_ADPD0, Panel_ADPD3
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_down, m_ADPD_plotSplitter_up);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD3, Panel_ADPD1);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        break;
    case 10: //Panel_ADPD2, Panel_ADPD3
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_down, m_ADPD_plotSplitter_up);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter->Unsplit();
        break;
    case 11: //Panel_ADPD0, Panel_ADPD2, Panel_ADPD3
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_up->Unsplit();
        break;
    case 12: //Panel_ADPD1, Panel_ADPD3
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD2, Panel_ADPD0);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD3, Panel_ADPD1);
        m_ADPD_plotSplitter_up->Unsplit();
        m_ADPD_plotSplitter_down->Unsplit();
        break;
    case 13: //Panel_ADPD0, Panel_ADPD1, Panel_ADPD3
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD3, Panel_ADPD1);
        m_ADPD_plotSplitter_down->Unsplit();
        break;
    case 14: //Panel_ADPD1, Panel_ADPD2, Panel_ADPD3
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD2, Panel_ADPD0);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        m_ADPD_plotSplitter_up->Unsplit();
        break;
    case 15:
        m_ADPD_plotSplitter->SplitHorizontally(m_ADPD_plotSplitter_up, m_ADPD_plotSplitter_down);
        m_ADPD_plotSplitter_up->SplitVertically(Panel_ADPD0, Panel_ADPD2);
        m_ADPD_plotSplitter_down->SplitVertically(Panel_ADPD1, Panel_ADPD3);
        break;
    }
}

void DPDTest::onTrainMode(wxCommandEvent &evt)
{
    onEnableDisable();
}

void DPDTest::onEnableDisable()
{

    int temp = 0;

    temp = TrainMode->GetSelection();

    if (temp == 0)
    {

        btnCapture->Enable(true);
        Button_TRAIN->Enable(true);
        Button_SEND->Enable(true);
        Button_TIMER->Enable(true);
        Button_READ->Enable(false);
        Button_SAVE->Enable(false);
        timer_enabled = false;
        m_timer->Stop();
        Button_END->Enable(true);
    }
    else
    {
        btnCapture->Enable(false);
        Button_TRAIN->Enable(false);
        Button_SEND->Enable(false);
        Button_TIMER->Enable(false);
        Button_READ->Enable(false);
        Button_SAVE->Enable(false);
        Button_END->Enable(false);
        timer_enabled = true;

        if (m_iThread == 0)
        {
            threadReadData();
            m_iThread = 1;
        }

        m_timer->Start(3000);
    }
}

double DPDTest::CalculateGainCorr(double p1, double p2, double g1, double g2, double p)
{

    double g = 0.0;
    if (p < p1)
        g = g1;
    else if (p > p2)
        g = g2;
    else
    {
        g = g1 + (g2 - g1) / (p2 - p1) * (p - p1);
    }
    return g;
}

double DPDTest::CalculateSignalPower(int Size, double *a, double *b, double Am)
{

    int offset = 100;
    int i = 0;
    double sum = 0;

    if (Size <= 0)
        return 0;

    for (i = offset; i < Size; i++)
        sum = sum + (a[i] * a[i] + b[i] * b[i]) / (Am * Am);
    sum = sum / Size;
    return sum;
}

void DPDTest::CalibrateND(int Size, double *a, double *b, int *pND)
{

    int ND_limit = 0;
    int offset = 0;
    int size = 0;

    int j = 0;
    int ND = 0;
    int iND = 0;

    double corr = 0.0;
    double max_corr = 0.0;

    if (Size <= 0)
        return;

    ND_limit = 300;
    offset = 200;
    size = Size - ND_limit;

    for (iND = 0; iND <= ND_limit; iND++)
    {
        corr = 0.0;
        for (j = offset; j < size; j++)
            corr = corr + abs(a[j] * b[j - iND]);
        if (corr > max_corr)
        {
            max_corr = corr;
            ND = iND;
        }
    }

    *pND = ND;
}

void DPDTest::OnbtnCalibrate(wxCommandEvent &event)
{

    long temp = 0;
    double tempd = 0.0;
    int i = 0;

    double *a = new double[32768];
    double *b = new double[32768];
    double *c = new double[32768];
    double *d = new double[32768];

    for (i = 0; i < 32768; i++)
    {
        a[i] = b[i] = c[i] = d[i] = 0.0;
    }

    int nd = 0;
    double pow_u = 0;
    double pow_y = 0;
    double gain = 1.000;
    double gainCorr = 1.0;
    TrainMode->SetSelection(0);
    CreateArrays();

    QADPD_N = spin_ADPD_N->GetValue();
    QADPD_M = spin_ADPD_M->GetValue();
    QADPD_AM = 14;

    wxString temps;
    temps = m_ADPD_txtFFT3->GetValue();
    ((wxString)temps).ToDouble(&tempd);
    QADPD_SPAN = tempd;
    QADPD_SKIP = 100;

    temp = 0;
    m_ADPD_txtTrain->GetValue().ToLong(&temp);
    if ((temp >= 0) && (temp <= 32768))
        QADPD_UPDATE = (int)temp;
    else
    {
        QADPD_UPDATE = 1;
        m_ADPD_txtTrain->SetValue("1");
    }

    QADPD_ND = 10;
    QADPD_GAIN = 1.0;
    temps.Printf(_T("%9.4f"), QADPD_GAIN);
    m_ADPD_txtGain->SetValue(temps);

    m_ADPD_txtLambda->GetValue().ToDouble(&tempd);
    QADPD_LAMBDA = tempd;
    QADPD_YPFPGA = true;

    Qadpd->init(QADPD_N, QADPD_M, 10, 1.0, QADPD_LAMBDA, (1 << (QADPD_AM - 1)), QADPD_SKIP);

    Qadpd->reset_coeff();
    send_coef();

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    readdata_QPCIe2();

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    run_QADPD();

    for (int i = 0; i < samplesReceived; i++)
    {
        a[i] = (double)y_samples[i].r;
        b[i] = (double)u_samples[i].r;
        c[i] = (double)y_samples[i].i;
        d[i] = (double)u_samples[i].i;
    }

    CalibrateND(samplesReceived, a, b, &nd);

    nd++;

    std::cout << "[INFO] " << "Calibration: ND=" << nd + 10 << std::endl;
    QADPD_ND = nd + 10;
    spin_ADPD_Delay->SetValue(QADPD_ND);
    SaveConfig();

    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;
}

int DPDTest::readdata_QPCIe()
{

    int status = 0;
    const int bytesToRead = 20 * 8192 * 2;
    unsigned char *buffer = new unsigned char[bytesToRead];
    memset(buffer, 0, bytesToRead);

    auto conn = ((LMS7_Device *)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        wxMessageBox(_("DPDviewer: Connection not initialized"), _("ERROR"));
        return 0;
    }
    else
    {

        int bytesReceived = 0;
        bytesReceived = ((LMS7_Device *)lmsControl)->GetConnection()->ReadDPDBuffer((char *)buffer, bytesToRead);
        if (bytesReceived == bytesToRead)
        {
            int samplesReceived = bytesReceived / 20; //one sample, 4 bytes * 3 channels

            int16_t value;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i]) & 0xFF;
                value |= (buffer[4 * i + 1] << 8) & 0xFF00;
                value >>= 2;

                if (chA == 1)
                {
                    xp_samples[i].r = value;
                    xp1_samples[i].r = value;
                }

                value = (buffer[4 * i + 2]) & 0xFF;
                value |= (buffer[4 * i + 3] << 8) & 0xFF00;
                value >>= 2;

                if (chA == 1)
                {
                    xp_samples[i].i = value;
                    xp1_samples[i].i = value;
                }
            }

            unsigned offset = bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    yp_samples[i].r = value;
                    yp1_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    yp_samples[i].i = value;
                    yp1_samples[i].i = value;
                }
            }

            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                x_samples[i].r = (int)(QADPD_GAIN * value);
                x1_samples[i].r = x_samples[i].r;
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                x_samples[i].i = (int)(QADPD_GAIN * value);
                x1_samples[i].i = x_samples[i].i;
            }

            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    xp_samples[i].r = value;
                    xp1_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    xp_samples[i].i = value;
                    xp1_samples[i].i = value;
                }
            }

            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    yp_samples[i].r = value;
                    yp1_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    yp_samples[i].i = value;
                    yp1_samples[i].i = value;
                }
            }

            for (int i = 0; i < samplesReceived; i++)
            {
                y_samples[i].r = 0;
                y_samples[i].i = 0;
                y1_samples[i].r = 0;
                y1_samples[i].i = 0;
                error_samples[i].r = 0;
                error_samples[i].i = 0;
                u_samples[i].r = 0;
                u_samples[i].i = 0;
            }
        }
        else
            std::cout << "[INFO] " << "DPD received samples " << bytesReceived << "/" << bytesToRead << std::endl;
    }

    samplesReceived = 2 * 8192;
    QADPD_SKIP = 0;
    delete buffer;
    buffer = NULL;
    if (status == -1)
        m_iThread = -1;
    wxCommandEvent evt;
    evt.SetEventObject(this);
    evt.SetId(ID_READING_FINISHED_EVENT);
    evt.SetEventType(wxEVT_COMMAND_THREAD);
    evt.SetString(status == 0 ? _("Reading Completed!") : _("Reading failed!\n"));
    wxPostEvent(this, evt);
    return status;
}

int DPDTest::readdata_QPCIe2()
{

    int status = 0;
    const int bytesToRead = 20 * 8192 * 2;
    unsigned char *buffer = new unsigned char[bytesToRead];
    memset(buffer, 0, bytesToRead);

    auto conn = ((LMS7_Device *)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        wxMessageBox(_("DPDviewer: Connection not initialized"), _("ERROR"));
        return 0;
    }
    else
    {
        int bytesReceived = 0;
        bytesReceived = ((LMS7_Device *)lmsControl)->GetConnection()->ReadDPDBuffer((char *)buffer, bytesToRead);
        if (bytesReceived == bytesToRead)
        {
            int samplesReceived = bytesReceived / 20;
            int16_t value;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i]) & 0xFF;
                value |= (buffer[4 * i + 1] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    xp_samples[i].r = value;
                    xp1_samples[i].r = value;
                }
                value = (buffer[4 * i + 2]) & 0xFF;
                value |= (buffer[4 * i + 3] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    xp_samples[i].i = value;
                    xp1_samples[i].i = value;
                }
            }

            unsigned offset = bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    yp_samples[i].r = value;
                    yp1_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    yp_samples[i].i = value;
                    yp1_samples[i].i = value;
                }
            }
            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                x_samples[i].r = (int)(QADPD_GAIN * value);
                x1_samples[i].r = x_samples[i].r;
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                x_samples[i].i = (int)(QADPD_GAIN * value);
                x1_samples[i].i = x_samples[i].i;
            }

            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    xp_samples[i].r = value;
                    xp1_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;

                if (chA != 1)
                {
                    xp_samples[i].i = value;
                    xp1_samples[i].i = value;
                }
            }
            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    yp_samples[i].r = value;
                    yp1_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    yp_samples[i].i = value;
                    yp1_samples[i].i = value;
                }
            }

            for (int i = 0; i < samplesReceived; i++)
            {
                y_samples[i].r = 0;
                y_samples[i].i = 0;
                y1_samples[i].r = 0;
                y1_samples[i].i = 0;
                error_samples[i].r = 0;
                error_samples[i].i = 0;
                u_samples[i].r = 0;
                u_samples[i].i = 0;
            }
        }
        else
            std::cout << "[INFO] " << "DPD received samples " << bytesReceived << "/" << bytesToRead << std::endl;
    }

    samplesReceived = 2 * 8192;
    QADPD_SKIP = 0;
    delete buffer;
    buffer = NULL;
    return status;
}

void DPDTest::OnbtnCalibrate2(wxCommandEvent &event)
{
    long temp = 0;
    double tempd = 0.0;
    int i = 0;

    double *a = new double[32768];
    double *b = new double[32768];
    double *c = new double[32768];
    double *d = new double[32768];

    int nd = 0;

    double pow_u = 0;
    double pow_y = 0;
    double gain = 1.000;
    double gainCorr = 1.0;

    CreateArrays();
    TrainMode->SetSelection(0);

    QADPD_N = spin_ADPD_N->GetValue();
    QADPD_M = spin_ADPD_M->GetValue();
    QADPD_AM = 14;

    wxString temps;
    temps = m_ADPD_txtFFT3->GetValue();
    ((wxString)temps).ToDouble(&tempd);
    QADPD_SPAN = tempd;

    mLmsNyquist_MHz = QADPD_SPAN / 2.0;
    QADPD_SKIP = 100;

    temp = 0;
    m_ADPD_txtTrain->GetValue().ToLong(&temp);
    if ((temp >= 0) && (temp <= 32768))
        QADPD_UPDATE = (int)temp;
    else
    {
        QADPD_UPDATE = 1;
        m_ADPD_txtTrain->SetValue("1");
    }

    QADPD_ND = 10;
    QADPD_GAIN = 1.0;
    temps.Printf(_T("%9.4f"), QADPD_GAIN);
    m_ADPD_txtGain->SetValue(temps);

    m_ADPD_txtLambda->GetValue().ToDouble(&tempd);
    QADPD_LAMBDA = tempd;
    QADPD_YPFPGA = true;
    SaveConfig();

    Qadpd->init(QADPD_N, QADPD_M, 10, 1.0, QADPD_LAMBDA, (1 << (QADPD_AM - 1)), QADPD_SKIP);

    Qadpd->reset_coeff();
    send_coef();
    readdata_QPCIe2();
    run_QADPD();

    for (i = 0; i < samplesReceived; i++)
    {
        a[i] = (double)y_samples[i].r;
        b[i] = (double)u_samples[i].r;
        c[i] = (double)y_samples[i].i;
        d[i] = (double)u_samples[i].i;
    }

    pow_y = CalculateSignalPower(samplesReceived, a, c, (1 << (QADPD_AM - 1)));
    pow_u = CalculateSignalPower(samplesReceived, b, d, (1 << (QADPD_AM - 1)));
    std::cout << "[INFO] " << "power of the input signal:" << pow_u << std::endl;

    gainCorr = CalculateGainCorr(0.03, 0.08, 1.0, 1.0, pow_u); // was 1.1
    gain = sqrt(pow_u / pow_y) * gainCorr;

    std::cout << "[INFO] " << "gainCorr=" << gainCorr << ", gain=" << gain << std::endl;
    temps.Printf(_T("gainCorr= %9.4f; gain= %9.4f"), gainCorr, gain);
    if ((gain > 6.0) || (gain < 0.333))
    {
        gain = 1.0;
        std::cout << "[INFO] " << "Gain cannot be calculated, set to default 1.0" << std::endl;
    }
    else
        std::cout << "[INFO] " << "Calibration: Gain=" << gain << std::endl;

    QADPD_GAIN = gain;
    temps.Printf(_T("%9.4f"), QADPD_GAIN);
    m_ADPD_txtGain->SetValue(temps);
    SaveConfig();

    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;
}

void DPDTest::OnbtnStartClick(wxCommandEvent &event)
{
    long temp = 0;
    double tempd = 0.0;

    CreateArrays();

    QADPD_N = spin_ADPD_N->GetValue();
    QADPD_M = spin_ADPD_M->GetValue();
    QADPD_ND = spin_ADPD_Delay->GetValue();
    QADPD_AM = 14;

    wxString temps;
    temps = m_ADPD_txtFFT3->GetValue();
    ((wxString)temps).ToDouble(&tempd);
    QADPD_SPAN = tempd;
    mLmsNyquist_MHz = QADPD_SPAN / 2.0;

    QADPD_SKIP = 100;
    temp = 0;
    m_ADPD_txtTrain->GetValue().ToLong(&temp);
    if ((temp >= 0) && (temp <= 32768))
        QADPD_UPDATE = (int)temp;
    else
    {
        QADPD_UPDATE = 1;
        m_ADPD_txtTrain->SetValue("1");
    }

    m_ADPD_txtGain->GetValue();
    QADPD_GAIN = tempd;
    m_ADPD_txtLambda->GetValue().ToDouble(&tempd);
    QADPD_LAMBDA = tempd;
    QADPD_YPFPGA = true;
    SaveConfig();
    Qadpd->init(QADPD_N, QADPD_M, QADPD_ND, 1.0, QADPD_LAMBDA, (1 << (QADPD_AM - 1)), QADPD_SKIP);

    std::string temps1;
    if (chA==1) temps1="chA";
    else temps1="chB";
   
    std::cout << "[INFO] " <<"DPD:" << temps1 <<",N=" << QADPD_N << ",M=" << QADPD_M << ",ND=" << QADPD_ND<< ",GAIN=" << QADPD_GAIN <<std::endl;
   
    Button_START->Enable(false);
    Button_TRAIN->Enable(true);
    Button_END->Enable(true);
    Button_SEND->Enable(true);
    spin_ADPD_N->Enable(false);
    spin_ADPD_M->Enable(false);
    spin_ADPD_Delay->Enable(false);
    m_ADPD_txtGain->Enable(true);
    m_ADPD_txtLambda->Enable(false);
    m_ADPD_txtTrain->Enable(false);
    TrainMode->Enable(true);
    CheckBox_Train->Enable(true);
    Button_CAL->Enable(false);
    Button_CAL2->Enable(false);
    rbChannelA->Enable(false);
    rbChannelB->Enable(false);
    onEnableDisable();

 #ifdef DEBUG_MODE  
    fp = fopen(fname.c_str(), "w");
 #endif   
}

int DPDTest::train()
{

    int samplesCount = samplesReceived;
    ind = 0;
    ind = prepare_train();
#ifdef DEBUG_MODE  
    fprintf(fp, "train ind: %d \n", ind);
#endif    
    
    if ((ind >= 4) || (ind <= -4))
        ind = 0;
    Qadpd->skiping = QADPD_N + QADPD_ND + 1;
    Qadpd->skiping += 100;

    QADPD_YPFPGA = true;
    Qadpd->updating = QADPD_UPDATE;
    //Qadpd->reset_matrix();
    int i = 6;
    int check = 0;
    double eps = 1e-3;

    int tempi, tempr;
    int am = 1 << (QADPD_AM - 1);

    ind = 0;

    while ((Qadpd->skiping >= 0) && (i < samplesCount - 6))
    {

        check = 3;
        if (check > 0)
            Qadpd->always(xp_samples[i].r, xp_samples[i].i, x_samples[i + ind].r,
                          x_samples[i + ind].i, yp_samples[i].r, yp_samples[i].i, QADPD_YPFPGA);
        i++;
    }
    int temp = 0;
    return temp;
}

void DPDTest::OnbtnTrainClick(wxCommandEvent &event)
{
    int temp = 0;
    train();
    temp = Qadpd->update_coeff(range);
    run_QADPD();
}

void DPDTest::OnbtnEndClick(wxCommandEvent &event)
{

    if (m_iThread == 1)
        mWorkerThread.join();
    else if (m_iThread == 0)
    {
        SaveConfig();
        Qadpd->release_memory();
    }

    Button_START->Enable(true);
    Button_TRAIN->Enable(false);
    Button_END->Enable(false);
    Button_SEND->Enable(false);
    spin_ADPD_N->Enable(true);
    spin_ADPD_M->Enable(true);
    spin_ADPD_Delay->Enable(true);
    m_ADPD_txtGain->Enable(true);
    m_ADPD_txtLambda->Enable(true);
    m_ADPD_txtTrain->Enable(true);
    TrainMode->Enable(false);
    btnCapture->Enable(false);
    Button_TRAIN->Enable(false);
    Button_SEND->Enable(false);
    Button_TIMER->Enable(false);
    Button_READ->Enable(true);

    if (updatedCoeff == 1)
        Button_SAVE->Enable(true);
    else
        Button_SAVE->Enable(false);

    CheckBox_Train->Enable(false);
    timer_enabled = false;
    m_timer->Stop();
    Button_CAL->Enable(true);
    Button_CAL2->Enable(true);
    rbChannelA->Enable(true);
    rbChannelB->Enable(true);

#ifdef DEBUG_MODE    
    fclose(fp);
#endif    
}

void DPDTest::send_coef()
{

    range = 16.0;
    double am = 32768.0 / range;
    int temp = 0;

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    SPI_write(lmsControl, 0x0004, 0x0000);

    double a1, b1, c1, d1 = 0.0;
    double oi1, oq1 = 0.0;

    short tempsh = 0;
    short tempsh2 = 0;
    double eps = 0.002;

    int i = 0;
    int j = 0;
    int LIM2 = 1;
    int temp_i = 0;

    double K = 0.0; //

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++) //
        {

            K = (double)pow(2, j);

            if ((i <= QADPD_N) && (j <= QADPD_M))
            {
                a1 = Qadpd->a[i][j];
                b1 = Qadpd->b[i][j];

                if (a1 >= range * K)
                {
                    a1 = (range * K - eps);
                    Qadpd->a[i][j] = a1;
                }
                if (a1 <= (-range * K))
                {
                    a1 = (-range * K + eps);
                    Qadpd->a[i][j] = a1;
                }
                if (b1 >= range * K)
                {
                    b1 = range * K - eps;
                    Qadpd->b[i][j] = b1;
                }
                if (b1 <= (-range * K))
                {
                    b1 = (-range * K + eps);
                    Qadpd->b[i][j] = b1;
                }
            }
            else
            {
                a1 = 0.0;
                b1 = 0.0;
            }

            if (chA == 1)
            {
                coeffa_chA[i][j] = a1;
                coeffb_chA[i][j] = b1;
            }
            else
            {
                coeffa_chB[i][j] = a1;
                coeffb_chB[i][j] = b1;
            }

            temp_i = (int)(am * a1 * 4.0 / K + 0.5); //
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);

            //WATCH
            temp = 0x1000 + tempsh2 + i * 16 + j;
            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; // delay

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; // delay

            temp_i = (int)(am * b1 * 4.0 / K + 0.5); //
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);

            temp = 0x2000 + tempsh2 + i * 16 + j;
            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; //delay

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; //delay

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; // delay
        }         // for (j = 0; j <= 3; j++)

    } // for (i = 0; i <= 6; i++)

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0xF000);
    else
        SPI_write(lmsControl, 0x0003, 0xF000);

    for (int k = 0; k <= LIM2 * 100; k++)
        ;

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;

    SPI_write(lmsControl, 0x0004, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;

    updatedCoeff = 1;
}

void DPDTest::update_coef()
{

    range = 16.0;
    double am = 32768.0 / range;
    int temp = 0;

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    SPI_write(lmsControl, 0x0004, 0x0000);

    double a1, b1, c1, d1 = 0.0;
    double oi1, oq1 = 0.0;

    short tempsh = 0;
    short tempsh2 = 0;
    double eps = 0.002;
    int i = 0;
    int j = 0;
    int LIM2 = 1;
    int temp_i = 0;

    double K = 0.0; //

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++) //
        {

            K = (double)pow(2, j);

            if (chA == 1)
            {
                a1 = coeffa_chA[i][j];
                b1 = coeffb_chA[i][j];
            }
            else
            {
                a1 = coeffa_chB[i][j];
                b1 = coeffb_chB[i][j];
            }

            temp_i = (int)(am * a1 * 4.0 / K + 0.5); //
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);

            //watch
            temp = 0x1000 + tempsh2 + i * 16 + j;

            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; // delay

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; // delay

            temp_i = (int)(am * b1 * 4.0 / K + 0.5); //
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);

            //watch
            temp = 0x2000 + tempsh2 + i * 16 + j;

            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; //delay

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; //delay

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ; // delay
        }         // for (j = 0; j <= 3; j++)
    }

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0xF000);
    else
        SPI_write(lmsControl, 0x0003, 0xF000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ; // delay

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ; // delay

    SPI_write(lmsControl, 0x0004, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ; // delay
}

void DPDTest::OnbtnSendClick(wxCommandEvent &event)
{

    auto conn = ((LMS7_Device *)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        wxMessageBox(_("DPDviewer: Connection not initialized"), _("ERROR"));
        return;
    }
    else
        send_coef();
}

void DPDTest::OnbtnPlot1ControlsClick(wxCommandEvent &event)
{
    wxString str;
    str = _T("");
    dlgADPDControls *m_ADPDControls = new dlgADPDControls(this, m_iValuePlot1, m_iYaxisTopPlot1, m_iYaxisBottomPlot1, m_iXaxisLeftPlot1, m_iXaxisRightPlot1);

    if (m_ADPDControls->ShowModal() == wxID_OK)
    {
        m_iValuePlot1 = m_ADPDControls->m_iValue;
        m_iXaxisLeftPlot1 = m_ADPDControls->m_iXaxisLeft;
        m_iXaxisRightPlot1 = m_ADPDControls->m_iXaxisRight;
        m_iYaxisTopPlot1 = m_ADPDControls->m_iYaxisTop;
        m_iYaxisBottomPlot1 = m_ADPDControls->m_iYaxisBottom;
    }
    OnPlot(m_iValuePlot1, m_plot_ADPD0, StaticText_PLOT1, m_iCenterFreqRatioPlot1, m_dCenterFreqRatioPlot1,
           m_iFreqSpanRatioPlot1, m_dFreqSpanRatioPlot1, m_iYaxisTopPlot1, m_iYaxisBottomPlot1, 0, m_iXaxisLeftPlot1, m_iXaxisRightPlot1);
    SaveConfig();
}

void DPDTest::OnPlot(int m_iValue, OpenGLGraph *plot, wxStaticText *text,
                     int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
                     int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot,
                     int m_iYaxisTopPlot, int m_iYaxisBottomPlot, int rb, int m_iXaxisLeftPlot, int m_iXaxisRightPlot)
{

    int m_iPlotType = 0;
    if ((m_iValue & 0x00003) == 0x00000)
        m_iPlotType = 0;
    else if ((m_iValue & 0x00003) == 0x00001)
        m_iPlotType = 1;
    else if ((m_iValue & 0x00003) == 0x00002)
        m_iPlotType = 2;
    changePlotType(plot, m_iPlotType, m_iXaxisLeftPlot, m_iXaxisRightPlot);
    wxString str;
    str = _T("");
    if (rb == 0)
        str = _T("1 - ");
    else if (rb == 1)
        str = _T("2 - ");
    else if (rb == 2)
        str = _T("3 - ");
    else if (rb == 3)
        str = _T("4 - ");

    wxString str2;
    if (m_iPlotType == 0)
        str2 = PlotTimeDomain(plot, m_iValue, m_iXaxisLeftPlot, m_iXaxisRightPlot);
    if (m_iPlotType == 1)
        str2 = PlotIQ(plot, m_iValue);
    if (m_iPlotType == 2)
        str2 = PlotFFT(plot, m_iValue, m_iCenterFreqRatioPlot, m_dCenterFreqRatioPlot,
                       m_iFreqSpanRatioPlot, m_dFreqSpanRatioPlot, m_iYaxisTopPlot, m_iYaxisBottomPlot);
    text->SetLabel(str2);
    plot->settings.title = str + str2;
}

void DPDTest::OnbtnPlot2ControlsClick(wxCommandEvent &event)
{
    wxString str;
    str = _T("");
    dlgADPDControls *m_ADPDControls = new dlgADPDControls(this, m_iValuePlot2, m_iYaxisTopPlot2, m_iYaxisBottomPlot2, m_iXaxisLeftPlot2, m_iXaxisRightPlot2);

    if (m_ADPDControls->ShowModal() == wxID_OK)
    {
        m_iValuePlot2 = m_ADPDControls->m_iValue;
        m_iXaxisLeftPlot2 = m_ADPDControls->m_iXaxisLeft;
        m_iXaxisRightPlot2 = m_ADPDControls->m_iXaxisRight;
        m_iYaxisTopPlot2 = m_ADPDControls->m_iYaxisTop;
        m_iYaxisBottomPlot2 = m_ADPDControls->m_iYaxisBottom;
    }
    OnPlot(m_iValuePlot2, m_plot_ADPD1, StaticText_PLOT2, m_iCenterFreqRatioPlot2, m_dCenterFreqRatioPlot2,
           m_iFreqSpanRatioPlot2, m_dFreqSpanRatioPlot2, m_iYaxisTopPlot2, m_iYaxisBottomPlot2, 1, m_iXaxisLeftPlot2, m_iXaxisRightPlot2);
    SaveConfig();
}

void DPDTest::OnbtnPlot3ControlsClick(wxCommandEvent &event)
{
    wxString str;
    str = _T("");
    dlgADPDControls *m_ADPDControls = new dlgADPDControls(this, m_iValuePlot3, m_iYaxisTopPlot3, m_iYaxisBottomPlot3, m_iXaxisLeftPlot3, m_iXaxisRightPlot3);

    if (m_ADPDControls->ShowModal() == wxID_OK)
    {
        m_iValuePlot3 = m_ADPDControls->m_iValue;
        m_iXaxisLeftPlot3 = m_ADPDControls->m_iXaxisLeft;
        m_iXaxisRightPlot3 = m_ADPDControls->m_iXaxisRight;
        m_iYaxisTopPlot3 = m_ADPDControls->m_iYaxisTop;
        m_iYaxisBottomPlot3 = m_ADPDControls->m_iYaxisBottom;
    }

    OnPlot(m_iValuePlot3, m_plot_ADPD2, StaticText_PLOT3, m_iCenterFreqRatioPlot3, m_dCenterFreqRatioPlot3,
           m_iFreqSpanRatioPlot3, m_dFreqSpanRatioPlot3, m_iYaxisTopPlot3, m_iYaxisBottomPlot3, 2, m_iXaxisLeftPlot3, m_iXaxisRightPlot3);
    SaveConfig();
}

void DPDTest::OnbtnPlot4ControlsClick(wxCommandEvent &event)
{
    wxString str;
    str = _T("");
    dlgADPDControls *m_ADPDControls = new dlgADPDControls(this, m_iValuePlot4, m_iYaxisTopPlot4, m_iYaxisBottomPlot4, m_iXaxisLeftPlot4, m_iXaxisRightPlot4);

    if (m_ADPDControls->ShowModal() == wxID_OK)
    {
        m_iValuePlot4 = m_ADPDControls->m_iValue;
        m_iXaxisLeftPlot4 = m_ADPDControls->m_iXaxisLeft;
        m_iXaxisRightPlot4 = m_ADPDControls->m_iXaxisRight;
        m_iYaxisTopPlot4 = m_ADPDControls->m_iYaxisTop;
        m_iYaxisBottomPlot4 = m_ADPDControls->m_iYaxisBottom;
    }
    OnPlot(m_iValuePlot4, m_plot_ADPD3, StaticText_PLOT4, m_iCenterFreqRatioPlot4, m_dCenterFreqRatioPlot4,
           m_iFreqSpanRatioPlot4, m_dFreqSpanRatioPlot4, m_iYaxisTopPlot4, m_iYaxisBottomPlot4, 3, m_iXaxisLeftPlot4, m_iXaxisRightPlot4);
    SaveConfig();
}

void DPDTest::Initialize(lms_device_t *dataPort)
{
    lmsControl = dataPort;
}

void DPDTest::changePlotType(OpenGLGraph *graph, int plotType, int m_iXaxisLeftPlot, int m_iXaxisRightPlot)
{

    graph->ResetConstraints();
    graph->DeletePendingEvents();
    graph->DeleteRelatedConstraints();

    switch (plotType)
    {
    case 0: // Time domain
        graph->settings.markersEnabled = false;
        graph->settings.useVBO = true;
        graph->SetInitialDisplayArea(m_iXaxisLeftPlot, m_iXaxisRightPlot, -8192, 8192);
        graph->SetDrawingMode(GLG_LINE);
        graph->series[0]->color = 0xFF0000FF;
        graph->series[1]->color = 0x00AF00FF;
        graph->series[2]->color = 0x0000FFFF;
        graph->series[3]->color = 0x2F2F2FFF;
        graph->settings.gridXlines = 15;
        graph->settings.marginTop = 0;
        graph->settings.marginLeft = 50;
        graph->settings.fontSize = 5;
        graph->settings.titleXaxis = "n";
        graph->settings.titleYaxis = "";
        graph->settings.xUnits = "";
        graph->settings.gridXprec = 1;
        graph->settings.staticGrid = false;
        break;
    case 1: // I versors Q
        graph->settings.useVBO = true;
        graph->series[0]->color = 0xFF0000FF;
        graph->series[1]->color = 0x00AF00FF;
        graph->series[2]->color = 0x0000FFFF;
        graph->series[3]->color = 0x2F2F2FFF;
        graph->SetInitialDisplayArea(-8192, 8192, -8192, 8192);
        graph->SetDrawingMode(GLG_POINTS);
        graph->settings.titleXaxis = "I";
        graph->settings.titleYaxis = "Q";
        graph->settings.gridXlines = 8;
        graph->settings.gridYlines = 8;
        graph->settings.marginTop = 0;
        graph->settings.marginLeft = 50;
        graph->settings.markersEnabled = false;
        graph->settings.staticGrid = false;
        graph->settings.gridXprec = 1;
        graph->settings.fontSize = 5;
        break;
    case 2: // FFT
        graph->settings.useVBO = true;
        graph->series[0]->color = 0xFF0000FF;
        graph->series[1]->color = 0x00AF00FF;
        graph->series[2]->color = 0x0000FFFF;
        graph->series[3]->color = 0x2F2F2FFF;
        graph->SetDrawingMode(GLG_LINE);
        graph->settings.gridXlines = 15;
        graph->SetInitialDisplayArea(-mLmsNyquist_MHz * 1000000, mLmsNyquist_MHz * 1000000, -100, 0);
        graph->settings.titleXaxis = "f(MHz)";
        graph->settings.titleYaxis = "Ampl.";
        graph->settings.xUnits = "";
        graph->settings.gridXprec = 1;
        //graph->settings.yUnits = "dB";
        graph->settings.markersEnabled = true;
        graph->settings.marginTop = 0;
        graph->settings.marginLeft = 50;
        graph->settings.staticGrid = true;
        graph->settings.fontSize = 5;
        break;
    default:
        break;
    }
}

int DPDTest::prepare_train()
{

    double error_nd_m5 = 0.0;
    double error_nd_m4 = 0.0;
    double error_nd_m3 = 0.0;
    double error_nd_m2 = 0.0;
    double error_nd_m1 = 0.0;
    double error_nd_p0 = 0.0;
    double error_nd_p1 = 0.0;
    double error_nd_p2 = 0.0;
    double error_nd_p3 = 0.0;
    double error_nd_p4 = 0.0;
    double error_nd_p5 = 0.0;

    int SKIP = 300;
    int LIMIT = samplesReceived - 100;

    int counter_e = 0;

    double xpr = 0.0;
    double xpi = 0.0;

    double ypr = 0.0;
    double ypi = 0.0;

    kiss_fft_cpx *xp;
    kiss_fft_cpx *yp;
    xp = xp_samples;
    yp = yp_samples;

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 5; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 5].r, x_samples[i + 5].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p5 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 4; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 4].r, x_samples[i + 4].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p4 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 3; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 3].r, x_samples[i + 3].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p3 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 2; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 2].r, x_samples[i + 2].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p2 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 1; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 1].r, x_samples[i + 1].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p1 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i].r, x_samples[i].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p0 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 1; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 1].r, x_samples[i - 1].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m1 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 2; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 2].r, x_samples[i - 2].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m2 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 3; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 3].r, x_samples[i - 3].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m3 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 4; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 4].r, x_samples[i - 4].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m4 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 5; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 5].r, x_samples[i - 5].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m5 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    int ind = 0;
    double minim = 0.0;

    if (error_nd_p5 < error_nd_p4)
    {
        ind = 5;
        minim = error_nd_p5;
    }
    else
    {
        ind = 4;
        minim = error_nd_p4;
    }

    if (minim > error_nd_p3)
    {
        ind = 3;
        minim = error_nd_p3;
    }
    if (minim > error_nd_p2)
    {
        ind = 2;
        minim = error_nd_p2;
    }

    if (minim > error_nd_p1)
    {
        ind = 1;
        minim = error_nd_p1;
    }

    if (minim > error_nd_p0)
    {
        ind = 0;
        minim = error_nd_p0;
    }

    if (minim > error_nd_m1)
    {
        ind = -1;
        minim = error_nd_m1;
    }

    if (minim > error_nd_m2)
    {
        ind = -2;
        minim = error_nd_m2;
    }

    if (minim > error_nd_m3)
    {
        ind = -3;
        minim = error_nd_m3;
    }

    if (minim > error_nd_m4)
    {
        ind = -4;
        minim = error_nd_m4;
    }

    if (minim > error_nd_m5)
    {
        ind = -5;
        minim = error_nd_m5;
    }
    return (ind);
}

void DPDTest::run_QADPD()
{

    Qadpd->start();
    Qadpd->prepare();
    for (int i = 3; i < samplesReceived - 3; i++)
    {

        Qadpd->oeval(xp_samples[i].r, xp_samples[i].i, x_samples[i + ind].r,
                     x_samples[i + ind].i, yp_samples[i].r, yp_samples[i].i, QADPD_YPFPGA);

        y_samples[i].r = Qadpd->yI;
        y_samples[i].i = Qadpd->yQ;
        y1_samples[i].r = Qadpd->yI;
        y1_samples[i].i = Qadpd->yQ;
        u_samples[i].r = Qadpd->uI;
        u_samples[i].i = Qadpd->uQ;
        error_samples[i].r = Qadpd->uI - Qadpd->yI;
        error_samples[i].i = Qadpd->uQ - Qadpd->yQ;
    }

    Qadpd->finish();

    double tempd = 0.0;
    wxString temps;
    QADPD_FFT1 = m_ADPD_txtFFT1->GetValue();
    QADPD_FFT2 = m_ADPD_txtFFT2->GetValue();

    temps = m_ADPD_txtFFT3->GetValue();
    ((wxString)temps).ToDouble(&tempd);
    QADPD_SPAN = tempd;
    QADPD_FFTSAMPLES = pow(2, QADPD_FFT1);
    wxString temps1;
    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(QADPD_FFTSAMPLES, 0, 0, 0);

    m_iWindowFunc = cmbWindowFunction->GetSelection();
    GenerateWindowCoefficients(m_iWindowFunc, QADPD_FFTSAMPLES);

    if (m_iWindowFunc > 0) //apply window function if it's selected
    {
        int index = 0;
        while (index < QADPD_FFTSAMPLES)
        {
            xp1_samples[index].r = xp_samples[index].r * windowFcoefs[index];
            xp1_samples[index].i = xp_samples[index].i * windowFcoefs[index];

            yp1_samples[index].r = yp_samples[index].r * windowFcoefs[index];
            yp1_samples[index].i = yp_samples[index].i * windowFcoefs[index];

            x1_samples[index].r = x_samples[index].r * windowFcoefs[index];
            x1_samples[index].i = x_samples[index].i * windowFcoefs[index];

            y1_samples[index].r = y_samples[index].r * windowFcoefs[index];
            y1_samples[index].i = y_samples[index].i * windowFcoefs[index];
            ++index;
        }
    }

    kiss_fft(m_fftCalcPlan, xp1_samples, xp_fft);
    kiss_fft(m_fftCalcPlan, yp1_samples, yp_fft);
    kiss_fft(m_fftCalcPlan, x1_samples, x_fft);
    kiss_fft(m_fftCalcPlan, y1_samples, y_fft);
    kiss_fft_free(m_fftCalcPlan);
    OnPlot(m_iValuePlot1, m_plot_ADPD0, StaticText_PLOT1, m_iCenterFreqRatioPlot1, m_dCenterFreqRatioPlot1,
           m_iFreqSpanRatioPlot1, m_dFreqSpanRatioPlot1, m_iYaxisTopPlot1, m_iYaxisBottomPlot1, 0, m_iXaxisLeftPlot1, m_iXaxisRightPlot1);
    OnPlot(m_iValuePlot2, m_plot_ADPD1, StaticText_PLOT2, m_iCenterFreqRatioPlot2, m_dCenterFreqRatioPlot2,
           m_iFreqSpanRatioPlot2, m_dFreqSpanRatioPlot2, m_iYaxisTopPlot2, m_iYaxisBottomPlot2, 1, m_iXaxisLeftPlot2, m_iXaxisRightPlot2);
    OnPlot(m_iValuePlot3, m_plot_ADPD2, StaticText_PLOT3, m_iCenterFreqRatioPlot3, m_dCenterFreqRatioPlot3,
           m_iFreqSpanRatioPlot3, m_dFreqSpanRatioPlot3, m_iYaxisTopPlot3, m_iYaxisBottomPlot3, 2, m_iXaxisLeftPlot3, m_iXaxisRightPlot3);
    OnPlot(m_iValuePlot4, m_plot_ADPD3, StaticText_PLOT4, m_iCenterFreqRatioPlot4, m_dCenterFreqRatioPlot4,
           m_iFreqSpanRatioPlot4, m_dFreqSpanRatioPlot4, m_iYaxisTopPlot4, m_iYaxisBottomPlot4, 3, m_iXaxisLeftPlot4, m_iXaxisRightPlot4);
}

void DPDTest::OnbtnCaptureClicked(wxCommandEvent &event)
{
    readdata_QPCIe2();
    run_QADPD();
}

void DPDTest::OnbtnTimerClick(wxCommandEvent &event)
{
    Qadpd->reset_coeff();
    send_coef();
}

void DPDTest::OnbtnTrain(wxCommandEvent &evt)
{

    if (CheckBox_Train->IsChecked() == true)
        m_bTrain = true;
    else
        m_bTrain = false;
}

void DPDTest::OnTimer(wxTimerEvent &event)
{
    int temp = 0;

    if (timer_enabled == true)
    {
        if (m_iThread == 0)
        {
            threadReadData();
            m_iThread = 1;
        }
    }
}

void DPDTest::CreateArrays()
{

    if (xp_samples != NULL)
        delete[] xp_samples;
    if (yp_samples != NULL)
        delete[] yp_samples;

    if (x_samples != NULL)
        delete[] x_samples;
    if (y_samples != NULL)
        delete[] y_samples;

    if (xp1_samples != NULL)
        delete[] xp1_samples;
    if (yp1_samples != NULL)
        delete[] yp1_samples;

    if (x1_samples != NULL)
        delete[] x1_samples;
    if (y1_samples != NULL)
        delete[] y1_samples;

    if (u_samples != NULL)
        delete[] u_samples;
    if (error_samples != NULL)
        delete[] error_samples;

    if (xp_fft != NULL)
        delete[] xp_fft;
    if (yp_fft != NULL)
        delete[] yp_fft;

    if (x_fft != NULL)
        delete[] x_fft;
    if (y_fft != NULL)
        delete[] y_fft;

    int samplesReceived = 32768;

    xp_samples = new kiss_fft_cpx[samplesReceived];
    yp_samples = new kiss_fft_cpx[samplesReceived];

    x_samples = new kiss_fft_cpx[samplesReceived];
    y_samples = new kiss_fft_cpx[samplesReceived];

    xp1_samples = new kiss_fft_cpx[samplesReceived];
    yp1_samples = new kiss_fft_cpx[samplesReceived];

    x1_samples = new kiss_fft_cpx[samplesReceived];
    y1_samples = new kiss_fft_cpx[samplesReceived];

    u_samples = new kiss_fft_cpx[samplesReceived];
    error_samples = new kiss_fft_cpx[samplesReceived];

    xp_fft = new kiss_fft_cpx[samplesReceived];
    yp_fft = new kiss_fft_cpx[samplesReceived];
    x_fft = new kiss_fft_cpx[samplesReceived];
    y_fft = new kiss_fft_cpx[samplesReceived];

    for (int i = 0; i < 32768; i++)
    {

        xp_samples[i].i = 0;
        xp_samples[i].r = 0;
        yp_samples[i].i = 0;
        yp_samples[i].r = 0;

        x_samples[i].i = 0;
        x_samples[i].r = 0;
        y_samples[i].i = 0;
        y_samples[i].r = 0;

        xp1_samples[i].i = 0;
        xp1_samples[i].r = 0;
        yp1_samples[i].i = 0;
        yp1_samples[i].r = 0;

        x1_samples[i].i = 0;
        x1_samples[i].r = 0;
        y1_samples[i].i = 0;
        y1_samples[i].r = 0;

        u_samples[i].i = 0;
        u_samples[i].r = 0;

        error_samples[i].i = 0;
        error_samples[i].r = 0;

        xp_fft[i].i = 0;
        xp_fft[i].r = 0;
        yp_fft[i].i = 0;
        yp_fft[i].r = 0;

        x_fft[i].i = 0;
        x_fft[i].r = 0;
        y_fft[i].i = 0;
        y_fft[i].r = 0;
    }
}

void DPDTest::SaveWfm()
{

    int i, q, k, j;
    std::string fname;
    FILE *fp;

    int Limit = 307200;

    fname = "xp.wfm";
    fp = fopen(fname.c_str(), "w");
    j = 0;
    k = 0;
    while (j < Limit)
    {
        i = (int)(xp_samples[k].r * 4.0);
        q = (int)(xp_samples[k].i * 4.0);
        fprintf(fp, "%c%c%c%c", 0xFF & (i >> 8), 0xFF & (i), 0xFF & (q >> 8), 0xFF & (q));
        if (k >= 16383)
            k = 0;
        else
            k++;
        j++;
    }
    fclose(fp);

    fname = "yp.wfm";
    fp = fopen(fname.c_str(), "w");
    j = 0;
    k = 0;
    while (j < Limit)
    {
        i = (int)(yp_samples[k].r * 4.0);
        q = (int)(yp_samples[k].i * 4.0);
        fprintf(fp, "%c%c%c%c", 0xFF & (i >> 8), 0xFF & (i), 0xFF & (q >> 8), 0xFF & (q));
        if (k >= 16383)
            k = 0;
        else
            k++;
        j++;
    }
    fclose(fp);

    fname = "x.wfm";
    fp = fopen(fname.c_str(), "w");
    j = 0;
    k = 0;
    while (j < Limit)
    {
        i = (int)(x_samples[k].r * 4.0);
        q = (int)(x_samples[k].i * 4.0);
        fprintf(fp, "%c%c%c%c", 0xFF & (i >> 8), 0xFF & (i), 0xFF & (q >> 8), 0xFF & (q));
        if (k >= 16383)
            k = 0;
        else
            k++;
        j++;
    }
    fclose(fp);
}

int DPDTest::LMS_ReadFPGARegs(lms_device_t *device, uint32_t *address, uint16_t *data, int num)
{

    int i = 0;
    uint32_t vals[100];
    uint32_t ret;

    for (i = 0; i < num; i++)
        data[i] = 0; // ini

    if (device == nullptr)
    {
        return -1;
    }
    LMS7_Device *lms = (LMS7_Device *)device;

    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        return -1;
    }

    ret = conn->ReadRegisters(address, vals, num);
    if (ret != LMS_SUCCESS)
        return ret;

    for (i = 0; i < num; i++)
        data[i] = vals[i];
    return LMS_SUCCESS;
}

void DPDTest::threadReadData()
{
    mWorkerThread = std::thread(&DPDTest::readdata_QPCIe, this);
}

void DPDTest::OnReadingFinished(wxCommandEvent &event)
{

    double *a = new double[32768];
    double *b = new double[32768];
    double *c = new double[32768];
    double *d = new double[32768];

    for (int i = 0; i < 32768; i++)
    {
        a[i] = b[i] = c[i] = d[i] = 0.0;
    }

    int nd = 0;
    double pow_u = 0;
    double pow_uy = 0;

    int temp = 0;
    mWorkerThread.join();
    if (m_iThread >= 0)
        m_iThread = 0;

    temp = train();
    temp = Qadpd->update_coeff(range);
    run_QADPD();
    for (int i = 0; i < samplesReceived; i++)
    {
        a[i] = (double)y_samples[i].r - u_samples[i].r;
        b[i] = (double)u_samples[i].r;
        c[i] = (double)y_samples[i].i - u_samples[i].i;
        d[i] = (double)u_samples[i].i;
    }

    pow_u = CalculateSignalPower(samplesReceived, b, d, (1 << (QADPD_AM - 1)));
    pow_uy = CalculateSignalPower(samplesReceived, a, c, (1 << (QADPD_AM - 1)));

    if (m_bTrain == true)
    {
        if (pow_u >= 0.03)
            send_coef();
    }
}

int DPDTest::SPI_write(lms_device_t *lmsControl, uint16_t address, uint16_t data)
{

    // LLLLLLLLHL
    // 0b00000000010 00000
    uint16_t maddress = 64;
    uint16_t tempa = address + maddress;
    int ret = 0;
    if (LMS_WriteFPGAReg(lmsControl, tempa, data) != 0)
        ret = -1;
    return ret;
}

int DPDTest::SPI_read(lms_device_t *lmsControl, uint16_t address, uint16_t *data)
{

    // LLLLLLLLHL
    // 0b00000000010 00000
    uint16_t maddress = 64;
    uint16_t tempa = address + maddress;
    int ret = 0;

    uint16_t regValue = 0x0000;
    LMS_ReadFPGAReg(lmsControl, tempa, &regValue);
    *data = regValue;
    return ret;
}

int DPDTest::SelectSource(int CHA)
{

    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;
    int lsb = 4;
    int msb = 6;
    uint16_t code = 0x0000;

    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= ~mask;
    if (CHA == 1)
        code = 1; // select CHA
    else
        code = 3; // select CHB
    regValue |= (code << lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, maddress, regValue) != 0)
        ret = -1;
    return ret;
}

void DPDTest::SetNyquist(float Nyquist_MHz)
{

    double mLmsNyquist_MHz1;
    if (LMS_GetSampleRate(lmsControl, LMS_CH_TX, 0, &mLmsNyquist_MHz1, nullptr) != 0)
        mLmsNyquist_MHz1 = 1;
    else
        mLmsNyquist_MHz1 /= 1e6;

    mLmsNyquist_MHz = (float)mLmsNyquist_MHz1;
    double ADCNyquistMHz;
    if (LMS_GetSampleRate(lmsControl, LMS_CH_RX, 4, &ADCNyquistMHz, nullptr) != 0)
        ADCNyquistMHz = 1;
    //mLmsNyquist_MHz = Nyquist_MHz;
    QADPD_SPAN = 2 * mLmsNyquist_MHz; // Nyquist_MHz;

    wxString temps;
    temps.Printf(_T("%9.4f"), QADPD_SPAN);

    m_ADPD_txtFFT3->SetValue(temps);
    OnPlot(m_iValuePlot1, m_plot_ADPD0, StaticText_PLOT1, m_iCenterFreqRatioPlot1, m_dCenterFreqRatioPlot1,
           m_iFreqSpanRatioPlot1, m_dFreqSpanRatioPlot1, m_iYaxisTopPlot1, m_iYaxisBottomPlot1, 0, m_iXaxisLeftPlot1, m_iXaxisRightPlot1);
    OnPlot(m_iValuePlot2, m_plot_ADPD1, StaticText_PLOT2, m_iCenterFreqRatioPlot2, m_dCenterFreqRatioPlot2,
           m_iFreqSpanRatioPlot2, m_dFreqSpanRatioPlot2, m_iYaxisTopPlot2, m_iYaxisBottomPlot2, 1, m_iXaxisLeftPlot2, m_iXaxisRightPlot2);
    OnPlot(m_iValuePlot3, m_plot_ADPD2, StaticText_PLOT3, m_iCenterFreqRatioPlot3, m_dCenterFreqRatioPlot3,
           m_iFreqSpanRatioPlot3, m_dFreqSpanRatioPlot3, m_iYaxisTopPlot3, m_iYaxisBottomPlot3, 2, m_iXaxisLeftPlot3, m_iXaxisRightPlot3);
    OnPlot(m_iValuePlot4, m_plot_ADPD3, StaticText_PLOT4, m_iCenterFreqRatioPlot4, m_dCenterFreqRatioPlot4,
           m_iFreqSpanRatioPlot4, m_dFreqSpanRatioPlot4, m_iYaxisTopPlot4, m_iYaxisBottomPlot4, 3, m_iXaxisLeftPlot4, m_iXaxisRightPlot4);
}

wxString DPDTest::PlotFFT(OpenGLGraph *plot, int m_iValue, int m_iCenterFreqRatioPlot, double m_dCenterFreqRatioPlot,
                          int m_iFreqSpanRatioPlot, double m_dFreqSpanRatioPlot, int m_iYaxisTopPlot, int m_iYaxisBottomPlot) //, const float nyquist_MHz)
{

    int plots = 0;
    wxString str;
    str = _T("FFT: ");
    wxString temps;
    double tempd = 0.0;

    temps = m_ADPD_txtFFT3->GetValue();
    ((wxString)temps).ToDouble(&tempd);
    QADPD_SPAN = tempd;

    QADPD_FFT1 = m_ADPD_txtFFT1->GetValue();
    QADPD_FFTSAMPLES = pow(2, QADPD_FFT1);
    int samplesCount = QADPD_FFTSAMPLES;

    float nyquist_MHz = mLmsNyquist_MHz;
    vector<float> freqAxis;
    freqAxis.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        freqAxis[i] = 1e6 * (-nyquist_MHz + 2 * (i + 1) * nyquist_MHz / samplesCount);

    kiss_fft_cpx *normalizedFFToutput = new kiss_fft_cpx[samplesCount];
    vector<float> outputs;
    outputs.resize(samplesCount, 0);

    vector<float> zeroes;
    zeroes.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        zeroes[i] = 100.0;

    int output_index = 0;

    double max = 0.0;
    double temp = 0.0;

    QADPD_FFT2 = m_ADPD_txtFFT2->GetValue();
    int Rb = pow(2, QADPD_FFT2);
    //Rb = 8;

    if (((m_iValue & 0x10000) == 0x10000) && (plots < 4))
    { // bit 16, xp

        for (int i = 0; i < samplesCount; ++i) // normalize FFT results
        {
            normalizedFFToutput[i].r = xp_fft[i].r / samplesCount;
            normalizedFFToutput[i].i = xp_fft[i].i / samplesCount;

            normalizedFFToutput[i].r *= mAmplitudeCorrectionCoef;
            normalizedFFToutput[i].i *= mAmplitudeCorrectionCoef;
        }
        output_index = 0;

#ifdef NOAVERAGE
        for (int i = samplesCount / 2 + 1; i < samplesCount; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
        for (int i = 0; i < samplesCount / 2 + 1; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
#endif
#ifdef AVERAGE
        for (int i = samplesCount / (2 * Rb); i < (samplesCount / Rb); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k - (samplesCount / 2)] = sqrt(temp / Rb);
        }

        for (int i = 0; i < (samplesCount / (2 * Rb)); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k + (samplesCount / 2)] = sqrt(temp / Rb);
        }
#endif

        for (int s = 0; s < samplesCount; ++s) //convert to dbFS
            outputs[s] = (outputs[s] != 0 ? (20 * log10(outputs[s])) - 66.2266 : -300);
        plot->series[plots]->AssignValues(&freqAxis[0], &outputs[0], outputs.size());
        if (plots == 0)
            str += _T("xp");
        else
            str += _T(", xp");
        plots++;
    }

    if (((m_iValue & 0x20000) == 0x20000) && (plots < 4))
    { // bit 17, yp

        for (int i = 0; i < samplesCount; ++i) // normalize FFT results
        {
            normalizedFFToutput[i].r = yp_fft[i].r / samplesCount;
            normalizedFFToutput[i].i = yp_fft[i].i / samplesCount;
            normalizedFFToutput[i].r *= mAmplitudeCorrectionCoef;
            normalizedFFToutput[i].i *= mAmplitudeCorrectionCoef;
        }
        output_index = 0;
        //shift fft negative frequencies for plotting, draw xp frequency domain
#ifdef NOAVERAGE
        for (int i = samplesCount / 2 + 1; i < samplesCount; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
        for (int i = 0; i < samplesCount / 2 + 1; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
#endif
#ifdef AVERAGE
        for (int i = samplesCount / (2 * Rb); i < (samplesCount / Rb); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
                //if (temp>max) max = temp;
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k - (samplesCount / 2)] = sqrt(temp / Rb);
        }

        for (int i = 0; i < (samplesCount / (2 * Rb)); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
                //if (temp>max) max = temp;
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k + (samplesCount / 2)] = sqrt(temp / Rb);
        }
#endif

        for (int s = 0; s < samplesCount; ++s)                                          //convert to dbFS
            outputs[s] = (outputs[s] != 0 ? (20 * log10(outputs[s])) - 66.2266 : -300); //69.2369

        plot->series[plots]->AssignValues(&freqAxis[0], &outputs[0], outputs.size());
        if (plots == 0)
            str += _T("yp");
        else
            str += _T(", yp");
        plots++;
    }

    if (((m_iValue & 0x40000) == 0x40000) && (plots < 4))
    { // bit 18, x

        for (int i = 0; i < samplesCount; ++i) // normalize FFT results
        {
            normalizedFFToutput[i].r = x_fft[i].r / samplesCount;
            normalizedFFToutput[i].i = x_fft[i].i / samplesCount;

            normalizedFFToutput[i].r *= mAmplitudeCorrectionCoef;
            normalizedFFToutput[i].i *= mAmplitudeCorrectionCoef;
        }
        output_index = 0;
        //shift fft negative frequencies for plotting, draw xp frequency domain
#ifdef NOAVERAGE

        for (int i = samplesCount / 2 + 1; i < samplesCount; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
        for (int i = 0; i < samplesCount / 2 + 1; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
#endif
#ifdef AVERAGE
        for (int i = samplesCount / (2 * Rb); i < (samplesCount / Rb); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
                //if (temp>max) max = temp;
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k - (samplesCount / 2)] = sqrt(temp / Rb);
        }
        for (int i = 0; i < (samplesCount / (2 * Rb)); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
                //if (temp>max) max = temp;
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k + (samplesCount / 2)] = sqrt(temp / Rb);
        }
#endif

        for (int s = 0; s < samplesCount; ++s) //convert to dbFS
            outputs[s] = (outputs[s] != 0 ? (20 * log10(outputs[s])) - 66.2266 : -300);
        plot->series[plots]->AssignValues(&freqAxis[0], &outputs[0], outputs.size());
        if (plots == 0)
            str += _T("x");
        else
            str += _T(", x");
        plots++;
    }

    if (((m_iValue & 0x80000) == 0x80000) && (plots < 4))
    { // bit 19, y

        for (int i = 0; i < samplesCount; ++i) // normalize FFT results
        {
            normalizedFFToutput[i].r = y_fft[i].r / samplesCount;
            normalizedFFToutput[i].i = y_fft[i].i / samplesCount;

            normalizedFFToutput[i].r *= mAmplitudeCorrectionCoef;
            normalizedFFToutput[i].i *= mAmplitudeCorrectionCoef;
        }
        output_index = 0;
        //shift fft negative frequencies for plotting, draw xp frequency domain
#ifdef NOAVERAGE
        for (int i = samplesCount / 2 + 1; i < samplesCount; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
        for (int i = 0; i < samplesCount / 2 + 1; ++i)
            outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
#endif
#ifdef AVERAGE

        for (int i = samplesCount / (2 * Rb); i < (samplesCount / Rb); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
                //if (temp>max) max = temp;
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k - (samplesCount / 2)] = sqrt(temp / Rb);
        }

        for (int i = 0; i < (samplesCount / (2 * Rb)); ++i)
        {
            max = 0.0;
            temp = 0.0;
            for (int k = 0; k < Rb; k++)
            {
                temp += (normalizedFFToutput[i * Rb + k].r * normalizedFFToutput[i * Rb + k].r + normalizedFFToutput[i * Rb + k].i * normalizedFFToutput[i * Rb + k].i);
                //if (temp>max) max = temp;
            }
            for (int k = 0; k < Rb; k++)
                outputs[i * Rb + k + (samplesCount / 2)] = sqrt(temp / Rb);
        }
#endif
        for (int s = 0; s < samplesCount; ++s) //convert to dbFS
            outputs[s] = (outputs[s] != 0 ? (20 * log10(outputs[s])) - 66.2266 : -300);
        plot->series[plots]->AssignValues(&freqAxis[0], &outputs[0], outputs.size());
        if (plots == 0)
            str += _T("y");
        else
            str += _T(", y");
        plots++;
    }

    for (int k = plots; k < 4; k++)
        plot->series[k]->AssignValues(&freqAxis[0], &zeroes[0], zeroes.size());

    plot->Refresh();
    plot->SetInitialDisplayArea(-nyquist_MHz * 1000000, nyquist_MHz * 1000000, m_iYaxisBottomPlot, m_iYaxisTopPlot);

    delete normalizedFFToutput;
    return str;
}

wxString DPDTest::PlotIQ(OpenGLGraph *plot, int m_iValue)
{

    kiss_fft_cpx *samples;
    int samplesCount = samplesReceived;

    vector<float> xaxis; //time domain x axis
    xaxis.resize(samplesCount, 0);
    vector<float> tempBuffer; //temporary buffer to pass samples for plotting
    tempBuffer.resize(samplesCount, 0);

    vector<float> zeroes;
    zeroes.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        zeroes[i] = 9000.0;
    int plots = 0;
    wxString str;
    str = _T("IQ: ");

    // bits 15-14: xp, yp, x, y
    if ((m_iValue & 0x0C000) == 0x00000)
    {

        samples = xp_samples;
        str += _T("xp");
    }
    else if ((m_iValue & 0x0C000) == 0x04000)
    {

        samples = yp_samples;
        str += _T("yp");
    }
    else if ((m_iValue & 0x0C000) == 0x08000)
    {
        samples = x_samples;
        str += _T("x");
    }
    else
    {
        samples = y_samples;
        str += _T("y");
    }

    for (int i = 0; i < samplesCount; ++i)
    {
        xaxis[i] = samples[i].r;
        tempBuffer[i] = samples[i].i;
    }

    plot->series[0]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());

    plots = 1;
    for (int k = plots; k < 4; k++)
        plot->series[k]->AssignValues(&xaxis[0], &zeroes[0], zeroes.size());

    plot->Refresh();
    return str;
}

wxString DPDTest::PlotTimeDomain(OpenGLGraph *plot, int m_iValue, int m_iXaxisLeftPlot, int m_iXaxisRightPlot) //, const kiss_fft_cpx* samples, const int samplesCount)
{

    int samplesCount = samplesReceived;
    vector<float> xaxis; //time domain x axis
    xaxis.resize(samplesCount, 0);

    for (int i = 0; i < samplesCount; ++i)
        xaxis[i] = i;

    vector<float> tempBuffer; //temporary buffer to pass samples for plotting
    tempBuffer.resize(samplesCount, 0);

    vector<float> zeroes;
    zeroes.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        zeroes[i] = 10000.0;

    int plots = 0;
    wxString str;
    str = _T("Time: ");

    if (((m_iValue & 0x00004) == 0x00004) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = xp_samples[i].r;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("xp_I");
        else
            str += _T(", xp_I");
        plots++;
    }

    if (((m_iValue & 0x00008) == 0x00008) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            //if (chA==1)
            tempBuffer[i] = xp_samples[i].i;
        //else  tempBuffer[i] = xp_samples_chB[i].i;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("xp_Q");
        else
            str += _T(", xp_Q");
        plots++;
    }

    if (((m_iValue & 0x00010) == 0x00010) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            //if (chA==1)
            tempBuffer[i] = yp_samples[i].r;
        //else tempBuffer[i] = yp_samples_chB[i].r;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("yp_I");
        else
            str += _T(", yp_I");
        plots++;
    }

    if (((m_iValue & 0x00020) == 0x00020) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            //if (chA==1)
            tempBuffer[i] = yp_samples[i].i;
        //else  tempBuffer[i] = yp_samples_chB[i].i;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("yp_Q");
        else
            str += _T(", yp_Q");
        plots++;
    }

    if (((m_iValue & 0x00040) == 0x00040) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = x_samples[i].r;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("x_I");
        else
            str += _T(", x_I");
        plots++;
    }

    if (((m_iValue & 0x00080) == 0x00080) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = x_samples[i].i;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("x_Q");
        else
            str += _T(", x_Q");
        plots++;
    }

    if (((m_iValue & 0x00100) == 0x00100) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = y_samples[i].r;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("y_I");
        else
            str += _T(", y_I");
        plots++;
    }

    if (((m_iValue & 0x00200) == 0x00200) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = y_samples[i].i;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("y_Q");
        else
            str += _T(", y_Q");
        plots++;
    }

    if (((m_iValue & 0x00400) == 0x00400) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = u_samples[i].r;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("u_I");
        else
            str += _T(", u_I");
        plots++;
    }

    if (((m_iValue & 0x00800) == 0x00800) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = u_samples[i].i;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("u_Q");
        else
            str += _T(", u_Q");
        plots++;
    }

    if (((m_iValue & 0x01000) == 0x01000) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = error_samples[i].r;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("u_I-y_I");
        else
            str += _T(", u_I-y_I");
        plots++;
    }

    if (((m_iValue & 0x02000) == 0x02000) && (plots < 4))
    {
        tempBuffer.empty();
        for (int i = 0; i < samplesCount; ++i)
            tempBuffer[i] = error_samples[i].i;
        plot->series[plots]->AssignValues(&xaxis[0], (float *)&tempBuffer[0], xaxis.size());
        if (plots == 0)
            str += _T("u_Q-y_Q");
        else
            str += _T(", u_Q-y_Q");
        plots++;
    }

    for (int k = plots; k < 4; k++)
        plot->series[k]->AssignValues(&xaxis[0], &zeroes[0], zeroes.size());

    plot->Refresh();
    return str;
}

void DPDTest::GenerateWindowCoefficients(int func, int fftsize)
{
    if (windowFcoefs)
        delete[] windowFcoefs;

    windowFcoefs = new float[fftsize];
    float a0 = 0.35875;
    float a1 = 0.48829;
    float a2 = 0.14128;
    float a3 = 0.01168;
    //float a4 = 1;
    int N = fftsize;
    float PI = 3.14159265359;
    switch (func)
    {
    case 0:
        mAmplitudeCorrectionCoef = 1;
        break;
    case 1: //blackman-harris
        for (int i = 0; i < N; ++i)
        {
            windowFcoefs[i] = a0 - a1 * cos((2 * PI * i) / (N - 1)) + a2 * cos((4 * PI * i) / (N - 1)) - a3 * cos((6 * PI * i) / (N - 1));
            mAmplitudeCorrectionCoef += windowFcoefs[i];
        }
        mAmplitudeCorrectionCoef = 1.0 / (mAmplitudeCorrectionCoef / N);
        break;
    case 2: //hamming
        mAmplitudeCorrectionCoef = 0;
        a0 = 0.54;
        for (int i = 0; i < N; ++i)
        {
            windowFcoefs[i] = a0 - (1 - a0) * cos((2 * PI * i) / (N));
            mAmplitudeCorrectionCoef += windowFcoefs[i];
        }
        mAmplitudeCorrectionCoef = 1.0 / (mAmplitudeCorrectionCoef / N);
        break;
    case 3: //hanning
        mAmplitudeCorrectionCoef = 0;
        for (int i = 0; i < N; ++i)
        {
            windowFcoefs[i] = 0.5 * (1 - cos((2 * PI * i) / (N)));
            mAmplitudeCorrectionCoef += windowFcoefs[i];
        }
        mAmplitudeCorrectionCoef = 1.0 / (mAmplitudeCorrectionCoef / N);
        break;
    default:
        mAmplitudeCorrectionCoef = 1;
    }
    //m_windowFunction = func;
}

void DPDTest::OnSwitchToChannelA(wxCommandEvent &event)
{
    chA = 1;
    SelectSource(chA);
    OpenConfig();
    updatedCoeff = 0;
    Button_SAVE->Enable(false);
    
    Button_START->Enable(true);
    Button_CAL->Enable(true);
    Button_CAL2->Enable(true);
    Button_READ->Enable(true);
}

void DPDTest::OnSwitchToChannelB(wxCommandEvent &event)
{
    chA = 0;
    SelectSource(chA);
    OpenConfig();
    updatedCoeff = 0;
    Button_SAVE->Enable(false);

    Button_START->Enable(true);
    Button_CAL->Enable(true);
    Button_CAL2->Enable(true);
    Button_READ->Enable(true);
}

void DPDTest::OnGainChanged(wxCommandEvent &event)
{

    double gain = 1.0;
    gain = m_ADPD_txtGain->GetValue();
    if ((gain >= 0.3) && (gain <= 3.0))
        QADPD_GAIN = gain;
}
