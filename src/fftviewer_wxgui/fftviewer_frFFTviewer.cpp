#include "fftviewer_frFFTviewer.h"
#include <wx/timer.h>
#include <vector>
#include "OpenGLGraph.h"
#include <LMSBoards.h>
#include "kiss_fft.h"
#include "IConnection.h"
#include "dataTypes.h"
#include "LMS7002M.h"

using namespace std;
using namespace lime;

void fftviewer_frFFTviewer::Initialize(lms_device_t* pDataPort)
{
    lmsControl = pDataPort;
}

fftviewer_frFFTviewer::fftviewer_frFFTviewer( wxWindow* parent )
:
frFFTviewer(parent), lmsControl(nullptr), mStreamRunning(false)
{
    chkCaptureToFile->Disable();
    spinCaptureCount->Disable();
    cmbWindowFunc->Disable();
#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif
    SetSize(800, 600);
    mFFTpanel->settings.useVBO = true;
    mFFTpanel->AddSerie(new cDataSerie());
    mFFTpanel->AddSerie(new cDataSerie());
    mFFTpanel->series[0]->color = 0xFF0000FF;
    mFFTpanel->series[1]->color = 0x0000FFFF;
    mFFTpanel->SetDrawingMode(GLG_LINE);
    mFFTpanel->settings.gridXlines = 15;
    mFFTpanel->SetInitialDisplayArea(-16000000, 16000000, -100, 0);

    mFFTpanel->settings.title = "FFT";
    mFFTpanel->settings.titleXaxis = "Frequency(MHz)";
    mFFTpanel->settings.titleYaxis = "Amplitude(dBFS)";
    mFFTpanel->settings.xUnits = "";
    mFFTpanel->settings.gridXprec = 3;
    //mFFTpanel->settings.yUnits = "dB";
    mFFTpanel->settings.markersEnabled = true;

    mFFTpanel->settings.marginLeft = 40;
    mFFTpanel->settings.staticGrid = true;

    mTimeDomainPanel->settings.useVBO = true;
    mTimeDomainPanel->AddSerie(new cDataSerie());
    mTimeDomainPanel->AddSerie(new cDataSerie());
    mTimeDomainPanel->AddSerie(new cDataSerie());
    mTimeDomainPanel->AddSerie(new cDataSerie());
    mTimeDomainPanel->SetInitialDisplayArea(0, 1024, -2050, 2050);
    mTimeDomainPanel->settings.title = "IQ samples";
    mTimeDomainPanel->series[0]->color = 0xFF0000FF;
    mTimeDomainPanel->series[1]->color = 0x0000FFFF;
    mTimeDomainPanel->series[2]->color = 0xFF00FFFF;
    mTimeDomainPanel->series[3]->color = 0x00FFFFFF;
    mTimeDomainPanel->settings.marginLeft = 40;

    mConstelationPanel->settings.useVBO = true;
    mConstelationPanel->AddSerie(new cDataSerie());
    mConstelationPanel->AddSerie(new cDataSerie());
    mConstelationPanel->series[0]->color = 0xFF0000FF;
    mConstelationPanel->series[1]->color = 0x0000FFFF;
    mConstelationPanel->SetInitialDisplayArea(-2050, 2050, -2050, 2050);
    mConstelationPanel->SetDrawingMode(GLG_POINTS);
    mConstelationPanel->settings.title = "I versus Q";
    mConstelationPanel->settings.titleXaxis = "I";
    mConstelationPanel->settings.titleYaxis = "Q";
    mConstelationPanel->settings.gridXlines = 8;
    mConstelationPanel->settings.gridYlines = 8;
    mConstelationPanel->settings.marginLeft = 40;

    mGUIupdater = new wxTimer(this, wxID_ANY); //timer for updating plots
    Connect(wxEVT_TIMER, wxTimerEventHandler(fftviewer_frFFTviewer::OnUpdatePlots), NULL, this);

    wxCommandEvent evt;
    //show only A channel at startup
    evt.SetInt(0);
    OnChannelVisibilityChange(evt);
}

fftviewer_frFFTviewer::~fftviewer_frFFTviewer()
{
    if (mStreamRunning == true)
        StopStreaming();
}

void fftviewer_frFFTviewer::OnFFTsamplesCountChanged( wxSpinEvent& event )
{
// TODO: Implement OnFFTsamplesCountChanged
}

void fftviewer_frFFTviewer::OnWindowFunctionChanged( wxCommandEvent& event )
{
// TODO: Implement OnWindowFunctionChanged
}

void fftviewer_frFFTviewer::OnbtnStartStop( wxCommandEvent& event )
{
    if (mStreamRunning == false)
        StartStreaming();
    else
        StopStreaming();
}

void fftviewer_frFFTviewer::StartStreaming()
{
    if(!LMS_IsOpen(lmsControl,1))
    {
        wxMessageBox(_("FFTviewer: Connection not initialized"), _("ERROR"));
        return;
    }

    txtNyquistFreqMHz->Disable();
    cmbStreamType->Disable();
    spinFFTsize->Disable();

    if (mStreamRunning.load() == true)
        return;
    stopProcessing.store(false);

    if (threadProcessing.joinable())
        threadProcessing.join();

    switch (cmbStreamType->GetSelection())
    {
    case 0: //SISO
        threadProcessing = std::thread(StreamingLoop, this, spinFFTsize->GetValue(), 1, 0);
        break;
    case 1: //MIMO
        threadProcessing = std::thread(StreamingLoop, this, spinFFTsize->GetValue(), 2, 0);
        break;
    }

    btnStartStop->SetLabel(_("STOP"));
    mGUIupdater->Start(50);
}

void fftviewer_frFFTviewer::StopStreaming()
{
    txtNyquistFreqMHz->Enable();
    mGUIupdater->Stop();
    if (mStreamRunning.load() == false)
        return;
    stopProcessing.store(true);
    threadProcessing.join();
	btnStartStop->SetLabel(_("START"));
	cmbStreamType->Enable();
	spinFFTsize->Enable();
}

void fftviewer_frFFTviewer::OnUpdatePlots(wxTimerEvent& event)
{
    float RxFilled = 0;
    float TxFilled = 0;
    float RxRate = 0;
    float TxRate = 0;

    if (mStreamRunning.load() == false)
        return;

    lms_stream_status_t rxStats;
    lms_stream_status_t txStats;
    LMS_GetStreamStatus(&rxStreams[0],&rxStats);
    LMS_GetStreamStatus(&txStreams[0],&txStats);
    RxFilled = 100*(float)rxStats.fifoFilledCount/rxStats.fifoSize;
    TxFilled = 100*(float)txStats.fifoFilledCount/txStats.fifoSize;

    RxRate = rxStats.linkRate;
    TxRate = txStats.linkRate;

    if (streamData.fftBins_dbFS[0].size() > 0)
    {
        std::unique_lock<std::mutex> lck(graphsDataLock);
        std::vector<float> freqs;
        freqs.reserve(streamData.fftBins_dbFS[0].size());
        double nyquistMHz;
        txtNyquistFreqMHz->GetValue().ToDouble(&nyquistMHz);
                        const float step = 2*nyquistMHz / streamData.samplesI[0].size();
        for (unsigned i = 0; i < streamData.fftBins_dbFS[0].size(); ++i)
            freqs.push_back(1000000*(-nyquistMHz + (i+1)*step));
        vector<float> indexes;
        indexes.reserve(streamData.samplesI[0].size());
        for (unsigned i = 0; i < streamData.samplesI[0].size(); ++i)
            indexes.push_back(i);
        if (chkFreezeTimeDomain->IsChecked() == false)
        {
            mTimeDomainPanel->series[0]->AssignValues(&indexes[0], &streamData.samplesI[0][0], streamData.samplesI[0].size());
            mTimeDomainPanel->series[1]->AssignValues(&indexes[0], &streamData.samplesQ[0][0], streamData.samplesQ[0].size());
            mTimeDomainPanel->series[2]->AssignValues(&indexes[0], &streamData.samplesI[1][0], streamData.samplesI[1].size());
            mTimeDomainPanel->series[3]->AssignValues(&indexes[0], &streamData.samplesQ[1][0], streamData.samplesQ[1].size());
        }
        if (chkFreezeConstellation->IsChecked() == false)
        {
            mConstelationPanel->series[0]->AssignValues(&streamData.samplesI[0][0], &streamData.samplesQ[0][0], streamData.samplesQ[0].size());
            mConstelationPanel->series[1]->AssignValues(&streamData.samplesI[1][0], &streamData.samplesQ[1][0], streamData.samplesQ[1].size());
        }
        if (chkFreezeFFT->IsChecked() == false)
        {
            mFFTpanel->series[0]->AssignValues(&freqs[0], &streamData.fftBins_dbFS[0][0], streamData.fftBins_dbFS[0].size());
            mFFTpanel->series[1]->AssignValues(&freqs[0], &streamData.fftBins_dbFS[1][0], streamData.fftBins_dbFS[1].size());
        }
    }


    if (chkFreezeTimeDomain->IsChecked() == false)
    {
        mTimeDomainPanel->Refresh();
        mTimeDomainPanel->Draw();
    }

    if (chkFreezeConstellation->IsChecked() == false)
    {
        mConstelationPanel->Refresh();
        mConstelationPanel->Draw();
    }

    if (chkFreezeFFT->IsChecked() == false)
    {
        mFFTpanel->Refresh();
        mFFTpanel->Draw();
    }

    gaugeRxBuffer->SetValue((int)RxFilled);
    gaugeTxBuffer->SetValue((int)TxFilled);
    lblRxDataRate->SetLabel(printDataRate(RxRate));
    lblTxDataRate->SetLabel(printDataRate(TxRate));
}

void fftviewer_frFFTviewer::StreamingLoop(fftviewer_frFFTviewer* pthis, const unsigned int fftSize, const int channelsCount, const uint32_t format)
{
    const int cMaxChCount = 2;
    const int test_count = 680*26;//4096*16;
    lime::complex16_t** buffers;

    DataToGUI localDataResults;
    localDataResults.nyquist_Hz = 7.68e6;
    localDataResults.samplesI[0].resize(fftSize, 0);
    localDataResults.samplesI[1].resize(fftSize, 0);
    localDataResults.samplesQ[0].resize(fftSize, 0);
    localDataResults.samplesQ[1].resize(fftSize, 0);
    localDataResults.fftBins_dbFS[0].resize(fftSize, 0);
    localDataResults.fftBins_dbFS[1].resize(fftSize, 0);
    buffers = new lime::complex16_t*[channelsCount];
    for (int i = 0; i < channelsCount; ++i)
        buffers[i] = new complex16_t[test_count];

    for(int i=0; i<channelsCount; ++i)
    {
        pthis->rxStreams[i].channel = i;
        pthis->rxStreams[i].fifoSize = test_count*40;
        pthis->rxStreams[i].isTx = false;
        pthis->rxStreams[i].dataFmt = lms_stream_t::LMS_FMT_I16;
        pthis->rxStreams[i].throughputVsLatency = 1;
        LMS_SetupStream(pthis->lmsControl, &pthis->rxStreams[i]);

        pthis->txStreams[i].channel = i;
        pthis->txStreams[i].fifoSize = test_count*40;
        pthis->txStreams[i].isTx = true;
        pthis->txStreams[i].dataFmt = lms_stream_t::LMS_FMT_I16;
        pthis->txStreams[i].throughputVsLatency = 1;
        LMS_SetupStream(pthis->lmsControl, &pthis->txStreams[i]);
    }

    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];
    unsigned updateCounter = 0;

    for(int i=0; i<channelsCount; ++i)
    {
        LMS_StartStream(&pthis->rxStreams[i]);
        LMS_StartStream(&pthis->txStreams[i]);
    }

    pthis->mStreamRunning.store(true);
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    lms_stream_meta_t meta;
    meta.waitForTimestamp = true;

    while (pthis->stopProcessing.load() == false)
    {
        ++updateCounter;
        uint32_t samplesPopped[2] = {0, 0};
        uint64_t ts[2];
        for(int i=0; i<channelsCount; ++i)
        {
            samplesPopped[i] += LMS_RecvStream(&pthis->rxStreams[i], &buffers[i][samplesPopped[i]], test_count, &meta, 1000);
            ts[i] = meta.timestamp + 1024 * 1024*2;
        }

        uint32_t samplesPushed[2] = {0, 0};
        for(int i=0; i<channelsCount; ++i)
        {
            meta.timestamp = ts[i];
            meta.waitForTimestamp = true;
            samplesPushed[i] += LMS_SendStream(&pthis->txStreams[i], &buffers[i][samplesPushed[i]], test_count, &meta, 1000);
        }

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 1000)
        {
            t1 = t2;
            lms_stream_status_t rxStats;
            LMS_GetStreamStatus(&pthis->rxStreams[0], &rxStats);
            lms_stream_status_t txStats;
            LMS_GetStreamStatus(&pthis->txStreams[0], &txStats);
            printf("Rx FIFO %2.1f%% \t\t Tx FIFO %2.1f%%\n", 100.0*rxStats.fifoFilledCount/rxStats.fifoSize, 100.0*txStats.fifoFilledCount/txStats.fifoSize);
        }

        if (updateCounter & 0x40)
        {
            for (int ch = 0; ch < channelsCount; ++ch)
            {

                for (unsigned i = 0; i < fftSize; ++i)
                {
                    localDataResults.samplesI[ch][i] = m_fftCalcIn[i].r = buffers[ch][i].i;
                    localDataResults.samplesQ[ch][i] = m_fftCalcIn[i].i = buffers[ch][i].q;
                }
                kiss_fft(m_fftCalcPlan, m_fftCalcIn, m_fftCalcOut);

                for(unsigned int i=0; i<fftSize; ++i)
                {
                    m_fftCalcOut[i].r /= fftSize;
                    m_fftCalcOut[i].i /= fftSize;
                }

                int output_index = 0;
                for (unsigned i = fftSize / 2 + 1; i < fftSize; ++i)
                    localDataResults.fftBins_dbFS[ch][output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
                for (unsigned i = 0; i < fftSize / 2 + 1; ++i)
                    localDataResults.fftBins_dbFS[ch][output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
                for (unsigned s = 0; s < fftSize; ++s)
                    localDataResults.fftBins_dbFS[ch][s] = (localDataResults.fftBins_dbFS[ch][s] != 0 ? (20 * log10(localDataResults.fftBins_dbFS[ch][s])) - 69.2369 : -300);
            }
            {
                std::unique_lock<std::mutex> lck(pthis->graphsDataLock);
                pthis->streamData = localDataResults;
            }
            updateCounter = 0;
        }
    }
    kiss_fft_free(m_fftCalcPlan);
    pthis->stopProcessing.store(true);
    pthis->mStreamRunning.store(false);
    for(int i=0; i<channelsCount; ++i)
    {
        LMS_StopStream(&pthis->txStreams[i]);
        LMS_StopStream(&pthis->rxStreams[i]);
    }
    for(int i=0; i<channelsCount; ++i)
    {
        LMS_DestroyStream(pthis->lmsControl, &pthis->txStreams[i]);
        LMS_DestroyStream(pthis->lmsControl, &pthis->rxStreams[i]);
    }
    for (int i = 0; i < channelsCount; ++i)
        delete [] buffers[i];
    delete [] buffers;
    delete [] m_fftCalcIn;
    delete [] m_fftCalcOut;
}

wxString fftviewer_frFFTviewer::printDataRate(float dataRate)
{
    if (dataRate > 1000000)
        return wxString::Format(_("%.3f MB/s"), dataRate / 1000000.0);
    else if (dataRate > 1000)
        return wxString::Format(_("%.3f KB/s"), dataRate / 1000.0);
    else
        return wxString::Format(_("%.0f B/s"), dataRate / 1000.0);
}

void fftviewer_frFFTviewer::SetNyquistFrequency(float freqHz)
{
    txtNyquistFreqMHz->SetValue(wxString::Format(_("%f"), freqHz / 1e6));
    mFFTpanel->SetInitialDisplayArea(-freqHz, freqHz, -100, 0);
}

void fftviewer_frFFTviewer::OnChannelVisibilityChange(wxCommandEvent& event)
{
    const int channelCount = 2;
    bool visibilities[channelCount];

    switch(event.GetInt())
    {
    case 0:
        visibilities[0] = true;
        visibilities[1] = false;
        break;
    case 1:
        visibilities[0] = false;
        visibilities[1] = true;
        break;
    case 2:
        visibilities[0] = true;
        visibilities[1] = true;
        break;
    }
    mTimeDomainPanel->series[0]->visible = visibilities[0];
    mTimeDomainPanel->series[1]->visible = visibilities[0];
    mTimeDomainPanel->series[2]->visible = visibilities[1];
    mTimeDomainPanel->series[3]->visible = visibilities[1];
    mConstelationPanel->series[0]->visible = visibilities[0];
    mConstelationPanel->series[1]->visible = visibilities[1];
    mFFTpanel->series[0]->visible = visibilities[0];
    mFFTpanel->series[1]->visible = visibilities[1];
}

void fftviewer_frFFTviewer::OnbtnCaptureClick(wxCommandEvent& event)
{

}
