#include "DPDTest.h"
#include <wx/timer.h>
#include <vector>
#include "../oglGraph/OpenGLGraph.h"
#include "../lime/LimeSuite.h"

DPDTest::DPDTest( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
, lmsControl(nullptr)
{
    mLmsNyquist_MHz = 1;
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif

    wxFlexGridSizer* sizerPlots;
    sizerPlots = new wxFlexGridSizer( 6, 2, 0, 0 );
    sizerPlots->AddGrowableCol(0);
    sizerPlots->AddGrowableCol(1);
    sizerPlots->AddGrowableRow(0);
    sizerPlots->AddGrowableRow(1);
    sizerPlots->AddGrowableRow(2);
    sizerPlots->AddGrowableRow(3);
    sizerPlots->AddGrowableRow(4);

    int GLCanvasAttributes_1[] = {
        WX_GL_RGBA,
        WX_GL_DOUBLEBUFFER,
        WX_GL_DEPTH_SIZE, 16,
        WX_GL_STENCIL_SIZE, 0,
        0, 0 };

    mFFT_xp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mFFT_xp, 1, wxEXPAND, 5);
    mTime_xp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_xp, 1, wxEXPAND, 5);
    mFFT_yp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mFFT_yp, 1, wxEXPAND, 5);
    mTime_yp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_yp, 1, wxEXPAND, 5);
    mFFT_xp1 = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add( mFFT_xp1, 1, wxEXPAND, 5 );
    mTime_xp1 = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_xp1, 1, wxEXPAND, 5);
    mFFT_yp1 = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add( mFFT_yp1, 1, wxEXPAND, 5 );
    mTime_yp1 = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_yp1, 1, wxEXPAND, 5);
    mFFT_x = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add( mFFT_x, 1, wxEXPAND, 5 );
    mTime_x = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_x, 1, wxEXPAND, 5);
    btnCapture = new wxButton(this, wxNewId(), _("Capture"));
    btnCapture->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DPDTest::OnbtnCaptureClicked), NULL, this);
    sizerPlots->Add(btnCapture, 1, wxEXPAND, 5);
   /* cmbLmsDevice = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LMS_CHOICE"));
    cmbLmsDevice->Append(_T("LMS 1"));
    cmbLmsDevice->SetSelection(cmbLmsDevice->Append(_T("LMS 2")));
    sizerPlots->Add(cmbLmsDevice, 1, wxALIGN_RIGHT, 5);*/

    OpenGLGraph* plots[] = { mFFT_xp, mFFT_yp, mFFT_xp1, mFFT_yp1, mFFT_x };
    const char* signalNames[] = { "xp0", "yp0", "xp1", "yp1", "x" };
    for (int i = 0; i < 5; ++i)
    {
        plots[i]->settings.useVBO = true;
        plots[i]->AddSerie(new cDataSerie());
        plots[i]->series[0]->color = 0xFF0000FF;
        plots[i]->SetDrawingMode(GLG_LINE);
        plots[i]->settings.gridXlines = 15;
        plots[i]->SetInitialDisplayArea(-16000000, 16000000, -100, 0);

        plots[i]->settings.title = std::string(signalNames[i]) + " FFT";
        plots[i]->settings.titleXaxis = "Frequency(MHz)";
        plots[i]->settings.titleYaxis = "Amplitude(dBFS)";
        plots[i]->settings.xUnits = "";
        plots[i]->settings.gridXprec = 2;
        plots[i]->settings.markersEnabled = true;

        plots[i]->settings.marginLeft = 40;
        plots[i]->settings.staticGrid = true;
    }

    OpenGLGraph* plotst[] = { mTime_xp, mTime_yp, mTime_xp1, mTime_yp1, mTime_x };
    for (int i = 0; i < 5; ++i)
    {
        plotst[i]->settings.markersEnabled = false;
        plotst[i]->settings.useVBO = true;
        plotst[i]->AddSerie(new cDataSerie());
        plotst[i]->AddSerie(new cDataSerie());
        plotst[i]->series[0]->color = 0xFF0000FF;
        plotst[i]->series[1]->color = 0x00FF00FF;
        plotst[i]->SetDrawingMode(GLG_LINE);
        plotst[i]->settings.gridXlines = 15;
        plotst[i]->SetInitialDisplayArea(0, 1024, -2048, 2048);

        plotst[i]->settings.title = std::string(signalNames[i]) + " Time";
        plotst[i]->settings.titleXaxis = "";
        plotst[i]->settings.titleYaxis = "";
        plotst[i]->settings.xUnits = "";
        plotst[i]->settings.gridXprec = 2;
        plotst[i]->settings.markersEnabled = true;

        plotst[i]->settings.marginLeft = 40;
        plotst[i]->settings.staticGrid = false;
    }

    SetSizer(sizerPlots);
    Layout();
    sizerPlots->Fit(this);
    SetSize(1280, 720);
}

DPDTest::~DPDTest()
{
}

void DPDTest::Initialize(lms_device_t* dataPort)
{
    lmsControl = dataPort;
}

void DPDTest::OnbtnCaptureClicked(wxCommandEvent& event)
{
    if (!LMS_IsOpen(lmsControl, 1))
    {
        wxMessageBox("Not connected");
        return;
    }
    const int bytesToRead = 20*8192;
    unsigned char *buffer = new unsigned char[bytesToRead];
    memset(buffer, 0, bytesToRead);

    int bytesReceived = ((LMS7_Device*)lmsControl)->GetConnection()->ReadDPDBuffer((char*)buffer, bytesToRead);
    if (bytesReceived == bytesToRead)
    {
        int samplesReceived = bytesReceived / 20; //one sample, 4 bytes * 3 channels
        //allocate memory for FFT
        kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(samplesReceived, 0, 0, 0);
        kiss_fft_cpx* xp_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* yp_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* xp1_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* yp1_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* x_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* fftCalcOut = new kiss_fft_cpx[samplesReceived];

        int16_t value; //temporary value for samples forming
        // deinterleave xp and yp samples
/*
        for (unsigned i = 0; i < bytesReceived; i++)
        {
            printf("%02X ",buffer[i]);
            if (i % 32 == 31)
                printf("\n");
            if (i % (bytesReceived / 3) == (bytesReceived/3-1))
                printf("\n");
        }
        printf("\nEND\n");
*/

        for (int i = 0; i < samplesReceived; i++)
        {
            value = (buffer[4 * i])&0xFF;
            value |= (buffer[4 * i + 1]<<8)&0xFF00;
            value >>= 2;
            xp_samples[i].r = value;
            value = (buffer[4 * i+2])&0xFF;
            value |= (buffer[4 * i + 3]<<8)&0xFF00;
            value >>= 2;
            xp_samples[i].i = value;
        }

        unsigned offset = bytesReceived / 5;
        for (int i = 0; i < samplesReceived; i++)
        {
            value = (buffer[4 * i + offset])&0xFF;
            value |= (buffer[4 * i + 1 + offset]<<8)&0xFF00;
            value >>= 2;
            yp_samples[i].r = value;
            value = (buffer[4 * i + 2 + offset])&0xFF;
            value |= (buffer[4 * i + 3 + offset]<<8)&0xFF00;
            value >>= 2;
            yp_samples[i].i = value;
        }

        offset += bytesReceived / 5;
        for (int i = 0; i < samplesReceived; i++)
        {
            value = (buffer[4 * i + offset])&0xFF;
            value |= (buffer[4 * i + 1 + offset]<<8)&0xFF00;
            value >>= 2;
            x_samples[i].r = value;
            value = (buffer[4 * i + 2 + offset])&0xFF;
            value |= (buffer[4 * i + 3 + offset]<<8)&0xFF00;
            value >>= 2;
            x_samples[i].i = value;
        }

        offset += bytesReceived / 5;
        for (int i = 0; i < samplesReceived; i++)
        {
            value = (buffer[4 * i + offset]) & 0xFF;
            value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
            value >>= 2;
            xp1_samples[i].r = value;
            value = (buffer[4 * i + 2 + offset]) & 0xFF;
            value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
            value >>= 2;
            xp1_samples[i].i = value;
        }

        offset += bytesReceived / 5;
        for (int i = 0; i < samplesReceived; i++)
        {
            value = (buffer[4 * i + offset]) & 0xFF;
            value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
            value >>= 2;
            yp1_samples[i].r = value;
            value = (buffer[4 * i + 2 + offset]) & 0xFF;
            value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
            value >>= 2;
            yp1_samples[i].i = value;
        }

        //draw xp, yp, x samples in time domain
        PlotTimeDomain(mTime_xp, xp_samples, samplesReceived);
        PlotTimeDomain(mTime_yp, yp_samples, samplesReceived);
        PlotTimeDomain(mTime_xp1, xp1_samples, samplesReceived);
        PlotTimeDomain(mTime_yp1, yp1_samples, samplesReceived);
        PlotTimeDomain(mTime_x, x_samples, samplesReceived);

        //calculate xp FFT
        kiss_fft(m_fftCalcPlan, xp_samples, fftCalcOut);
        PlotFFT(mFFT_xp, fftCalcOut, samplesReceived, mLmsNyquist_MHz);

        //calculate yp FFT
        kiss_fft(m_fftCalcPlan, yp_samples, fftCalcOut);
        PlotFFT(mFFT_yp, fftCalcOut, samplesReceived, mLmsNyquist_MHz);

        //calculate xp FFT
        kiss_fft(m_fftCalcPlan, xp1_samples, fftCalcOut);
        PlotFFT(mFFT_xp1, fftCalcOut, samplesReceived, mLmsNyquist_MHz);

        //calculate yp FFT
        kiss_fft(m_fftCalcPlan, yp1_samples, fftCalcOut);
        PlotFFT(mFFT_yp1, fftCalcOut, samplesReceived, mLmsNyquist_MHz);

        //calculate x FFT
        kiss_fft(m_fftCalcPlan, x_samples, fftCalcOut);
        double ADCNyquistMHz;
        if (LMS_GetSampleRate(lmsControl,LMS_CH_RX,4,&ADCNyquistMHz,nullptr)!=0)
            ADCNyquistMHz = 1;
        else
            ADCNyquistMHz /= 2*2e6;
        PlotFFT(mFFT_x, fftCalcOut, samplesReceived, ADCNyquistMHz);

        //free allocated memory
        kiss_fft_free(m_fftCalcPlan);
        delete xp_samples;
        delete yp_samples;
        delete xp1_samples;
        delete yp1_samples;
        delete x_samples;
        delete fftCalcOut;
    }
    else
        printf("DPD received bytes %d/%d", bytesReceived, bytesToRead);
    delete buffer;
}

void DPDTest::SetNyquist(float LmsNyquistMHz)
{
    mLmsNyquist_MHz = LmsNyquistMHz;
    double ADCNyquistMHz;
    if (LMS_GetSampleRate(lmsControl,LMS_CH_RX,4,&ADCNyquistMHz,nullptr)!=0)
        ADCNyquistMHz = 1;
    mFFT_xp->SetInitialDisplayArea(-LmsNyquistMHz*1000000, LmsNyquistMHz*1000000, -100, 0);
    mFFT_yp->SetInitialDisplayArea(-LmsNyquistMHz*1000000, LmsNyquistMHz*1000000, -100, 0);
    mFFT_xp1->SetInitialDisplayArea(-LmsNyquistMHz * 1000000, LmsNyquistMHz * 1000000, -100, 0);
    mFFT_yp1->SetInitialDisplayArea(-LmsNyquistMHz * 1000000, LmsNyquistMHz * 1000000, -100, 0);
    mFFT_x->SetInitialDisplayArea(-ADCNyquistMHz*1000000, ADCNyquistMHz*1000000/2, -100, 0);
}

/** @brief Normalizes the fft output and displays results in given graph
*/
void DPDTest::PlotFFT(OpenGLGraph* plot, const kiss_fft_cpx* fftOutput, const int samplesCount, const float nyquist_MHz)
{
    std::vector<float> freqAxis; //frequency domain x axis
    freqAxis.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        freqAxis[i] = 1e6*(-nyquist_MHz + 2 * (i + 1)*nyquist_MHz / samplesCount);

    kiss_fft_cpx *normalizedFFToutput = new kiss_fft_cpx[samplesCount];
    for (int i = 0; i < samplesCount; ++i) // normalize FFT results
    {
        normalizedFFToutput[i].r = fftOutput[i].r / samplesCount;
        normalizedFFToutput[i].i = fftOutput[i].i / samplesCount;
    }

    std::vector<float> outputs;
    outputs.resize(samplesCount, 0);
    int output_index = 0;
    //shift fft negative frequencies for plotting, draw xp frequency domain
    for (int i = samplesCount / 2 + 1; i < samplesCount; ++i)
        outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
    for (int i = 0; i < samplesCount / 2 + 1; ++i)
        outputs[output_index++] = sqrt(normalizedFFToutput[i].r * normalizedFFToutput[i].r + normalizedFFToutput[i].i * normalizedFFToutput[i].i);
    for (int s = 0; s < samplesCount; ++s) //convert to dbFS
        outputs[s] = (outputs[s] != 0 ? (20 * log10(outputs[s])) - 69.2369 : -300);
    plot->series[0]->AssignValues(&freqAxis[0], &outputs[0], outputs.size());
    plot->Refresh();
    plot->SetInitialDisplayArea(-nyquist_MHz * 1000000, nyquist_MHz * 1000000, -100, 0);
    delete normalizedFFToutput;
}

void DPDTest::PlotTimeDomain(OpenGLGraph* plot, const kiss_fft_cpx* samples, const int samplesCount)
{
    std::vector<float> xaxis; //time domain x axis
    xaxis.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        xaxis[i] = i;

    std::vector<float> tempBuffer; //temporary buffer to pass samples for plotting
    tempBuffer.resize(samplesCount, 0);

    //draw xp, yp time domain
    for (int i = 0; i < samplesCount; ++i)
        tempBuffer[i] = samples[i].r;
    plot->series[0]->AssignValues(&xaxis[0], (float*)&tempBuffer[0], xaxis.size());
    for (int i = 0; i < samplesCount; ++i)
        tempBuffer[i] = samples[i].i;
    plot->series[1]->AssignValues(&xaxis[0], (float*)&tempBuffer[0], xaxis.size());
    plot->Refresh();
}