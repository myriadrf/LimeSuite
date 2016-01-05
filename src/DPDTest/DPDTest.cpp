#include "DPDTest.h"
#include <wx/timer.h>
#include <vector>
#include "lmsComms.h"
#include "OpenGLGraph.h"
#include "kiss_fft.h"

DPDTest::DPDTest( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
, mDataPort(nullptr)
{
    mNyquist_MHz = 1;
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif

    wxFlexGridSizer* sizerPlots;
    sizerPlots = new wxFlexGridSizer( 4, 2, 0, 0 );
    sizerPlots->AddGrowableCol(0);
    sizerPlots->AddGrowableCol(1);
    sizerPlots->AddGrowableRow(0);
    sizerPlots->AddGrowableRow(1);
    sizerPlots->AddGrowableRow(2);
    
    int GLCanvasAttributes_1[] = {
        WX_GL_RGBA,
        WX_GL_DOUBLEBUFFER,
        WX_GL_DEPTH_SIZE, 16,
        WX_GL_STENCIL_SIZE, 0,
        0, 0 };
    mFFT_xp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add( mFFT_xp, 1, wxEXPAND, 5 );
    mTime_xp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_xp, 1, wxEXPAND, 5);
    mFFT_yp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add( mFFT_yp, 1, wxEXPAND, 5 );
    mTime_yp = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_yp, 1, wxEXPAND, 5);
    mFFT_x = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add( mFFT_x, 1, wxEXPAND, 5 );
    mTime_x = new OpenGLGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _(""), GLCanvasAttributes_1);
    sizerPlots->Add(mTime_x, 1, wxEXPAND, 5);
    btnCapture = new wxButton(this, wxNewId(), _("Capture"));
    btnCapture->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DPDTest::OnbtnCaptureClicked), NULL, this);
    sizerPlots->Add(btnCapture, 1, wxEXPAND, 5);

    OpenGLGraph* plots[] = { mFFT_xp, mFFT_yp, mFFT_x };
    const char* signalNames[] = {"xp", "yp", "x"};
    for (int i = 0; i < 3; ++i)
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
        plots[i]->settings.gridXprec = 3;
        plots[i]->settings.markersEnabled = true;

        plots[i]->settings.marginLeft = 40;
        plots[i]->settings.staticGrid = true;
    }

    OpenGLGraph* plotst[] = { mTime_xp, mTime_yp, mTime_x };
    for (int i = 0; i < 3; ++i)
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
        plotst[i]->settings.gridXprec = 3;
        plotst[i]->settings.markersEnabled = true;

        plotst[i]->settings.marginLeft = 40;
        plotst[i]->settings.staticGrid = false;
    }

    SetSizer(sizerPlots);
    Layout();
    sizerPlots->Fit(this);
    SetSize(800, 600);
}

DPDTest::~DPDTest()
{   
}

void DPDTest::Initialize(LMScomms* dataPort)
{
    mDataPort = dataPort;
}

void DPDTest::OnbtnCaptureClicked(wxCommandEvent& event)
{
    if (mDataPort->IsOpen() == false)
    {
        wxMessageBox("Not connected");
        return;
    }
    const int bytesToRead = 184320;
    unsigned char *buffer = new unsigned char[bytesToRead];
    memset(buffer, 0, bytesToRead);

    SPI_write(mDataPort, 0x0005, 0xb); //disable FPGA Rx

    //Reset USB FIFO
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_USB_FIFO_RST;
    ctrPkt.outBuffer.push_back(0);
    mDataPort->TransferPacket(ctrPkt);

    SPI_write(mDataPort, 0x0005, 0xf); //enable FPGA Rx

    SPI_write(mDataPort, 0x0010, 0x0); //0x1-burst, 0x3-continuos
    SPI_write(mDataPort, 0x0010, 0x1); //0x1-burst, 0x3-continuos

    int handle = mDataPort->BeginDataReading((char*)buffer, bytesToRead);
    if (mDataPort->WaitForReading(handle, 1000) != 1)
    {
        wxMessageBox("Failed to receive data");
    }
    long btr = bytesToRead;
    int bytesReceived = mDataPort->FinishDataReading((char*)buffer, btr, handle);
    if (bytesReceived > 0)
    {
        int samplesReceived = bytesReceived / 9; //one sample, 3 bytes * 3 channels
        //allocate memory for FFT
        kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(samplesReceived, 0, 0, 0);
        kiss_fft_cpx* xp_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* yp_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* x_samples = new kiss_fft_cpx[samplesReceived];
        kiss_fft_cpx* fftCalcOut = new kiss_fft_cpx[samplesReceived];

        int index = 0;
        int16_t value; //temporary value for samples forming
        // deinterleave xp and yp samples
        for (int i = 0; i < samplesReceived*3*2; i+=6)
        {   
            //xp i
            value = (buffer[i + 1] & 0x0F) << 8;
            value |= buffer[i] & 0xFF;
            value <<= 4;
            value >>= 4;
            xp_samples[index].r = value;
            
            //xp q
            value = (buffer[i + 1] >> 4) + ((unsigned int)buffer[i + 2] << 4);
            value <<= 4;
            value >>= 4;
            xp_samples[index].i = value;

            //yp i
            value = (buffer[i + 3]&0xFF) + ((buffer[i + 4] & 0x0F) << 8);
            value <<= 4;
            value >>= 4;
            yp_samples[index].r = value;

            //yp q
            value = (buffer[i + 4] >> 4) + ((unsigned int)buffer[i + 5] << 4);
            value <<= 4;
            value >>= 4;
            yp_samples[index].i = value;
            ++index;
        }
        
        index = 0;
        //extract x samples
        for (int i = samplesReceived*3*2; i < bytesReceived;  i+=3)
        {
            value = (buffer[i]&0xFF) | ((buffer[i + 1] & 0x0F) << 8);
            value <<= 4;
            value >>= 4;
            x_samples[index].r = value;

            value = (int)(buffer[i + 1] >> 4) + ((int)buffer[i + 2] << 4);
            value <<= 4;
            value >>= 4;
            x_samples[index].i = value;
            ++index;
        }

        //draw xp, yp, x samples in time domain
        PlotTimeDomain(mTime_xp, xp_samples, samplesReceived);
        PlotTimeDomain(mTime_yp, yp_samples, samplesReceived);
        PlotTimeDomain(mTime_x, x_samples, samplesReceived);

        //calculate xp FFT
        kiss_fft(m_fftCalcPlan, xp_samples, fftCalcOut);
        PlotFFT(mFFT_xp, fftCalcOut, samplesReceived, mNyquist_MHz);

        //calculate yp FFT
        kiss_fft(m_fftCalcPlan, yp_samples, fftCalcOut);
        PlotFFT(mFFT_yp, fftCalcOut, samplesReceived, mNyquist_MHz);

        //calculate x FFT
        kiss_fft(m_fftCalcPlan, x_samples, fftCalcOut);
        PlotFFT(mFFT_x, fftCalcOut, samplesReceived, mNyquist_MHz);

        //free allocated memory
        kiss_fft_free(m_fftCalcPlan);
        delete xp_samples;
        delete yp_samples;
        delete x_samples;
        delete fftCalcOut;
    }
    delete buffer;
}

int DPDTest::SPI_write(LMScomms* dataPort, uint16_t address, uint16_t data)
{
    assert(dataPort != nullptr);
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_BRDSPI_WR;
    ctrPkt.outBuffer.push_back((address >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(address & 0xFF);
    ctrPkt.outBuffer.push_back((data >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(data & 0xFF);
    dataPort->TransferPacket(ctrPkt);
    return ctrPkt.status == 1 ? 0 : 1;
}
uint16_t DPDTest::SPI_read(LMScomms* dataPort, uint16_t address)
{
    assert(dataPort != nullptr);
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_BRDSPI_RD;
    ctrPkt.outBuffer.push_back((address >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(address & 0xFF);
    dataPort->TransferPacket(ctrPkt);
    if (ctrPkt.inBuffer.size() > 4)
        return ctrPkt.inBuffer[2] * 256 + ctrPkt.inBuffer[3];
    else
        return 0;
}

void DPDTest::SetNyquist(float Nyquist_MHz)
{
    mNyquist_MHz = Nyquist_MHz;
    OpenGLGraph* plots[] = { mFFT_xp, mFFT_yp, mFFT_x };
    for (int i = 0; i < 3; ++i)
    {   
        plots[i]->SetInitialDisplayArea(-Nyquist_MHz*1000000, Nyquist_MHz*1000000, -100, 0);
    }
}

/** @brief Normalizes the fft output and displays results in given graph
*/
void DPDTest::PlotFFT(OpenGLGraph* plot, const kiss_fft_cpx* fftOutput, const int samplesCount, const float nyquist_MHz)
{
    vector<float> freqAxis; //frequency domain x axis
    freqAxis.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        freqAxis[i] = 1e6*(-nyquist_MHz + 2 * (i + 1)*nyquist_MHz / samplesCount);

    kiss_fft_cpx *normalizedFFToutput = new kiss_fft_cpx[samplesCount];
    for (int i = 0; i < samplesCount; ++i) // normalize FFT results
    {
        normalizedFFToutput[i].r = fftOutput[i].r / samplesCount;
        normalizedFFToutput[i].i = fftOutput[i].i / samplesCount;
    }

    vector<float> outputs;
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
    vector<float> xaxis; //time domain x axis
    xaxis.resize(samplesCount, 0);
    for (int i = 0; i < samplesCount; ++i)
        xaxis[i] = i;

    vector<float> tempBuffer; //temporary buffer to pass samples for plotting
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