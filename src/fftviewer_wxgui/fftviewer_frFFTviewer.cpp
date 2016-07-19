#include "fftviewer_frFFTviewer.h"
#include "StreamerLTE.h"
#include <wx/timer.h>
#include <vector>
#include "LMS_StreamBoard.h"
#include "StreamerNovena.h"
#include "IConnection.h"
#include "OpenGLGraph.h"
#include <LMSBoards.h>

using namespace std;
using namespace lime;

void fftviewer_frFFTviewer::Initialize(IConnection* pDataPort)
{
    mDataPort = pDataPort;
}

fftviewer_frFFTviewer::fftviewer_frFFTviewer( wxWindow* parent )
:
frFFTviewer(parent), mLTEstreamer(nullptr), mDataPort(nullptr), mStreamBrd(nullptr), mStreamRunning(false)
{
#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif
    SetSize(1000, 700);
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
    mTimeDomainPanel->SetInitialDisplayArea(0, 1024, -2048, 2048);
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
    mConstelationPanel->SetInitialDisplayArea(-2048, 2048, -2048, 2048);
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
    if(!mDataPort)
    {
        wxMessageBox(_("FFTviewer: Connection not initialized"), _("ERROR"));
        return;
    }
    txtNyquistFreqMHz->Disable();
    cmbStreamType->Disable();
    spinFFTsize->Disable();
    chkCaptureToFile->Disable();
    mStreamRunning = true;
    spinCaptureCount->Disable();
    cmbWindowFunc->Disable();
    bool capturing = chkCaptureToFile->IsChecked();
    string filename = "";
    if(capturing)
    {
        wxFileDialog dlg(this, _("Save samples file"), "", "", "Text (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() == wxID_CANCEL)
            capturing = false;
        else
            filename = dlg.GetPath().To8BitData();
    }
    switch (cmbStreamType->GetSelection())
    {
    case 0:
        assert(mStreamBrd == nullptr);
        if (mDataPort->GetDeviceInfo().deviceName == GetDeviceName(LMS_DEV_NOVENA))
            mStreamBrd = new StreamerNovena(mDataPort);
        else
            mStreamBrd = new LMS_StreamBoard(mDataPort);
        mStreamBrd->SetWidowFunction(cmbWindowFunc->GetSelection());
        mStreamBrd->SetCaptureToFile(capturing, filename.c_str(), spinCaptureCount->GetValue());
        mStreamBrd->StartReceiving(spinFFTsize->GetValue());
        break;
    case 1: //SISO
        assert(mLTEstreamer == nullptr);
        mLTEstreamer = new StreamerLTE(mDataPort);
        mLTEstreamer->SetWidowFunction(cmbWindowFunc->GetSelection());
        mLTEstreamer->SetCaptureToFile(capturing, filename.c_str(), spinCaptureCount->GetValue());
        mLTEstreamer->StartStreaming(spinFFTsize->GetValue(), 1, StreamerLTE::STREAM_12_BIT_COMPRESSED);
        break;
    case 2: //MIMO
        assert(mLTEstreamer == nullptr);
        mLTEstreamer = new StreamerLTE(mDataPort);
        mLTEstreamer->SetWidowFunction(cmbWindowFunc->GetSelection());
        mLTEstreamer->SetCaptureToFile(capturing, filename.c_str(), spinCaptureCount->GetValue());
        mLTEstreamer->StartStreaming(spinFFTsize->GetValue(), 2, StreamerLTE::STREAM_12_BIT_COMPRESSED);
        break;
    case 3: //SISO uncompressed samples
        assert(mLTEstreamer == nullptr);
        mLTEstreamer = new StreamerLTE(mDataPort);
        mLTEstreamer->SetWidowFunction(cmbWindowFunc->GetSelection());
        mLTEstreamer->StartStreaming(spinFFTsize->GetValue(), 2, StreamerLTE::STREAM_12_BIT_IN_16);
        break;
    }
    btnStartStop->SetLabel(_("STOP"));
    mGUIupdater->Start(50);
}

void fftviewer_frFFTviewer::StopStreaming()
{
    txtNyquistFreqMHz->Enable();
    mGUIupdater->Stop();
    switch (cmbStreamType->GetSelection())
    {
    case 0:
        if (mStreamBrd)
        {
            mStreamBrd->StopReceiving();
            delete mStreamBrd;
            mStreamBrd = nullptr;
        }
        break;
    case 1:
    case 2:
    case 3:
        if (mLTEstreamer)
        {
            mLTEstreamer->StopStreaming();
            delete mLTEstreamer;
            mLTEstreamer = nullptr;
        }
        break;
    }
    mStreamRunning = false;
    btnStartStop->SetLabel(_("START"));
    cmbStreamType->Enable();
    spinFFTsize->Enable();
    chkCaptureToFile->Enable();
    spinCaptureCount->Enable();
    cmbWindowFunc->Enable();
}

void fftviewer_frFFTviewer::OnUpdatePlots(wxTimerEvent& event)
{
    float RxFilled = 0;
    float TxFilled = 0;
    float RxRate = 0;
    float TxRate = 0;

    switch (cmbStreamType->GetSelection())
    {
        case 0:
        {
            assert(mStreamBrd != nullptr);
            LMS_StreamBoard::DataToGUI data = mStreamBrd->GetIncomingData();
            LMS_StreamBoard::ProgressStats stats = mStreamBrd->GetStats();
            RxFilled = stats.RxFIFOfilled;
            TxFilled = stats.TxFIFOfilled;
            RxRate = stats.RxRate_Bps;
            TxRate = stats.TxRate_Bps;

			std::vector<float> freqs;
			freqs.reserve(data.fftBins_dbFS.size());
			double nyquistMHz;
			txtNyquistFreqMHz->GetValue().ToDouble(&nyquistMHz);
			const float step = 2*nyquistMHz / data.samplesI.size();
			for (int i = 0; i < data.fftBins_dbFS.size(); ++i)
				freqs.push_back(1000000*(-nyquistMHz + (i+1)*step));
			vector<float> indexes;
			indexes.reserve(data.samplesI.size());
			for (int i = 0; i < data.samplesI.size(); ++i)
				indexes.push_back(i);
            if (chkFreezeTimeDomain->IsChecked() == false)
            {
                mTimeDomainPanel->series[0]->AssignValues(&indexes[0], &data.samplesI[0], data.samplesI.size());
                mTimeDomainPanel->series[1]->AssignValues(&indexes[0], &data.samplesQ[0], data.samplesQ.size());
            }
            if (chkFreezeConstellation->IsChecked() == false)
            {
                mConstelationPanel->series[0]->AssignValues(&data.samplesI[0], &data.samplesQ[0], data.samplesQ.size());
            }
            if (chkFreezeFFT->IsChecked() == false)
            {
                mFFTpanel->series[0]->AssignValues(&freqs[0], &data.fftBins_dbFS[0], data.fftBins_dbFS.size());
            }
            break;
        }
        case 1:
        case 2:
        case 3:
        {
            assert(mLTEstreamer != nullptr);
            StreamerLTE::DataToGUI data = mLTEstreamer->GetIncomingData();
            StreamerLTE::Stats info = mLTEstreamer->GetStats();
            RxFilled = 100.0*info.rxBufFilled / info.rxBufSize;
            TxFilled = 100.0*info.txBufFilled / info.txBufSize;
            RxRate = data.rxDataRate_Bps;
            TxRate = data.txDataRate_Bps;

            if (data.fftBins_dbFS[0].size() > 0)
            {
                std::vector<float> freqs;
                freqs.reserve(data.fftBins_dbFS[0].size());
                double nyquistMHz;
                txtNyquistFreqMHz->GetValue().ToDouble(&nyquistMHz);
                const float step = 2*nyquistMHz / data.samplesI[0].size();
                for (int i = 0; i < data.fftBins_dbFS[0].size(); ++i)
                    freqs.push_back(1000000*(-nyquistMHz + (i+1)*step));
                vector<float> indexes;
                indexes.reserve(data.samplesI[0].size());
                for (int i = 0; i < data.samplesI[0].size(); ++i)
                    indexes.push_back(i);
                if (chkFreezeTimeDomain->IsChecked() == false)
                {
                    mTimeDomainPanel->series[0]->AssignValues(&indexes[0], &data.samplesI[0][0], data.samplesI[0].size());
                    mTimeDomainPanel->series[1]->AssignValues(&indexes[0], &data.samplesQ[0][0], data.samplesQ[0].size());
                    mTimeDomainPanel->series[2]->AssignValues(&indexes[0], &data.samplesI[1][0], data.samplesI[1].size());
                    mTimeDomainPanel->series[3]->AssignValues(&indexes[0], &data.samplesQ[1][0], data.samplesQ[1].size());
                }
                if (chkFreezeConstellation->IsChecked() == false)
                {
                    mConstelationPanel->series[0]->AssignValues(&data.samplesI[0][0], &data.samplesQ[0][0], data.samplesQ[0].size());
                    mConstelationPanel->series[1]->AssignValues(&data.samplesI[1][0], &data.samplesQ[1][0], data.samplesQ[1].size());
                }
                if (chkFreezeFFT->IsChecked() == false)
                {
                    mFFTpanel->series[0]->AssignValues(&freqs[0], &data.fftBins_dbFS[0][0], data.fftBins_dbFS[0].size());
                    mFFTpanel->series[1]->AssignValues(&freqs[0], &data.fftBins_dbFS[1][0], data.fftBins_dbFS[1].size());
                }

                float chPwr[2];
                double cFreq[2] = {0, 0};
                txtCenterOffset1->GetValue().ToDouble(&cFreq[0]);
                cFreq[0] *= 1000000;
                txtCenterOffset2->GetValue().ToDouble(&cFreq[1]);
                cFreq[1] *= 1000000;
                double bw[2] = {1e6, 1e6};
                txtBW1->GetValue().ToDouble(&bw[0]);
                bw[0] *= 1000000;
                txtBW2->GetValue().ToDouble(&bw[1]);
                bw[1] *= 1000000;
                char ctemp[512];
                for(int c=0; c<2; ++c)
                {
                    float f0 = cFreq[c]-bw[c]/2;
                    float fn = cFreq[c]+bw[c]/2;
                    float sum = 0;
                    int bins = 0;
                    const int fftSize = data.fftBins[0].size();
                    for(int i=0; i<fftSize; ++i)
                        if(f0 <= freqs[i] && freqs[i] <= fn)
                        {
                            double val = data.fftBins[0][i];
                            sum += val;
                            ++bins;
                        }
                    chPwr[c] = sum;
                }

                float pwr1  = (chPwr[0] != 0 ? (20 * log10(chPwr[0])) - 69.2369 : -300);
                lblPower1->SetLabel(wxString::Format("%.3f", pwr1));
                float pwr2 = (chPwr[1] != 0 ? (20 * log10(chPwr[1])) - 69.2369 : -300);
                lblPower2->SetLabel(wxString::Format("%.3f", pwr2));
                lbldBc->SetLabel(wxString::Format("%.3f", 20*log10(chPwr[1]/chPwr[0])));
            }
            break;
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

    if(RxFilled > 100)
        RxFilled = 100;
    if(TxFilled > 100)
        TxFilled = 100;
    gaugeRxBuffer->SetValue((int)RxFilled);
    gaugeTxBuffer->SetValue((int)TxFilled);
    lblRxDataRate->SetLabel(printDataRate(RxRate));
    lblTxDataRate->SetLabel(printDataRate(TxRate));
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
