#include "fftviewer_frFFTviewer.h"
#include "mathplot.h"
#include "StreamerLTE.h"
#include <wx/timer.h>
#include <vector>

void fftviewer_frFFTviewer::Initialize(LMScomms* pDataPort)
{
    assert(pDataPort != nullptr);
    mDataPort = pDataPort;
}

fftviewer_frFFTviewer::fftviewer_frFFTviewer( wxWindow* parent )
:
frFFTviewer(parent), mLTEstreamer(nullptr), mDataPort(nullptr)
{
    SetSize(800, 600);
    wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    //FFT plot
    wxPen fftpen(*wxRED, 1, wxSOLID);
    mFFTdata = new mpPolygon();
    mFFTdata->SetPen(fftpen);
    mFFTdata->SetDrawOutsideMargins(false);
    mpScaleX* xaxis = new mpScaleX(wxT("Frequency(kHz)"), mpALIGN_BOTTOM, false, mpX_NORMAL);
    mpScaleY* yaxis = new mpScaleY(wxT("Amplitude(dBFS)"), mpALIGN_LEFT, false);
    xaxis->SetFont(graphFont);
    yaxis->SetFont(graphFont);
    xaxis->SetDrawOutsideMargins(false);
    yaxis->SetDrawOutsideMargins(false);
    mFFTpanel->SetMargins(0, 0, 24, 75);
    mFFTpanel->AddLayer(mFFTdata);
    mFFTpanel->AddLayer(xaxis);
    mFFTpanel->AddLayer(yaxis);    

    mFFTpanel->EnableDoubleBuffer(true);
    mFFTpanel->Fit(-16000, 16000, -100, 0);

    //time domain plot
    wxPen ipen(*wxRED, 1, wxSOLID);
    wxPen qpen(*wxBLUE, 1, wxSOLID);
    mTimeDomainIdata = new mpPolygon();
    mTimeDomainQdata = new mpPolygon();
    mTimeDomainIdata->SetPen(ipen);
    mTimeDomainQdata->SetPen(qpen);
    mTimeDomainIdata->SetDrawOutsideMargins(false);
    mTimeDomainQdata->SetDrawOutsideMargins(false);
    xaxis = new mpScaleX(wxT("sample"), mpALIGN_BOTTOM, false, mpX_NORMAL);
    yaxis = new mpScaleY(wxT(""), mpALIGN_LEFT, false);
    xaxis->SetFont(graphFont);
    yaxis->SetFont(graphFont);
    xaxis->SetDrawOutsideMargins(false);
    yaxis->SetDrawOutsideMargins(false);
    mTimeDomainPanel->SetMargins(0, 0, 24, 75);
    mTimeDomainPanel->AddLayer(mTimeDomainIdata);
    mTimeDomainPanel->AddLayer(mTimeDomainQdata);
    mTimeDomainPanel->AddLayer(xaxis);
    mTimeDomainPanel->AddLayer(yaxis);
    mTimeDomainPanel->EnableDoubleBuffer(true);
    mTimeDomainPanel->Fit(0, 16384, -2050, 2050);

    //Constellation plot
    wxPen constellationPen(*wxBLACK, 3, wxSOLID);
    mConstelationData = new mpFXYVector();
    mConstelationData->SetPen(constellationPen);
    mConstelationData->SetDrawOutsideMargins(false);
    xaxis = new mpScaleX(wxT("I"), mpALIGN_BOTTOM, false, mpX_NORMAL);
    yaxis = new mpScaleY(wxT("Q"), mpALIGN_LEFT, false);
    xaxis->SetFont(graphFont);
    yaxis->SetFont(graphFont);
    xaxis->SetDrawOutsideMargins(false);
    yaxis->SetDrawOutsideMargins(false);
    mConstelationPanel->SetMargins(0, 0, 24, 75);
    mConstelationPanel->AddLayer(xaxis);
    mConstelationPanel->AddLayer(yaxis);
    mConstelationPanel->AddLayer(mConstelationData);
    mConstelationPanel->EnableDoubleBuffer(true);
    mConstelationPanel->Fit(-2050,2050,-2050,2050);

    mGUIupdater = new wxTimer(this, wxID_ANY); //timer for updating plots
    Connect(wxEVT_TIMER, wxTimerEventHandler(fftviewer_frFFTviewer::OnUpdatePlots), NULL, this);
}

fftviewer_frFFTviewer::~fftviewer_frFFTviewer()
{
    if (mLTEstreamer != nullptr)
    {
        mLTEstreamer->StopStreaming();
        delete mLTEstreamer;
        mLTEstreamer = nullptr;
    }
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
    if (mLTEstreamer == nullptr)
        StartStreaming();
    else
        StopStreaming();
}

void fftviewer_frFFTviewer::StartStreaming()
{
    if (mLTEstreamer == nullptr)
    {
        txtNyquistFreqMHz->Disable();
        mLTEstreamer = new StreamerLTE(mDataPort);
        mLTEstreamer->StartStreaming(spinFFTsize->GetValue());
        btnStartStop->SetLabel(_("STOP"));
        mGUIupdater->Start(50);
    }
}

void fftviewer_frFFTviewer::StopStreaming()
{
    if (mLTEstreamer != nullptr)
    {
        txtNyquistFreqMHz->Enable();
        mGUIupdater->Stop();
        mLTEstreamer->StopStreaming();
        delete mLTEstreamer;
        mLTEstreamer = nullptr;
        btnStartStop->SetLabel(_("START"));
    }
}

void fftviewer_frFFTviewer::OnUpdatePlots(wxTimerEvent& event)
{
    StreamerLTE::DataToGUI data = mLTEstreamer->GetIncomingData();
    if (chkFreezeTimeDomain->IsChecked() == false)
    {
        std::vector<double> sampleIndexes;
        sampleIndexes.reserve(data.samplesI.size());
        for (int i = 0; i < data.samplesI.size(); ++i)
            sampleIndexes.push_back(i);
        mTimeDomainIdata->setPoints(sampleIndexes, data.samplesI, false);
        mTimeDomainQdata->setPoints(sampleIndexes, data.samplesQ, false);
        mTimeDomainPanel->UpdateAll();
    }

    if (chkFreezeConstellation->IsChecked() == false)
    {
        mConstelationData->SetData(data.samplesI, data.samplesQ);
        mConstelationPanel->UpdateAll();
    }

    if (chkFreezeFFT->IsChecked() == false)
    {
        std::vector<double> freqs;
        freqs.reserve(data.fftBins_dbFS.size());
        double nyquistMHz;
        txtNyquistFreqMHz->GetValue().ToDouble(&nyquistMHz);
        for (int i = 0; i < data.fftBins_dbFS.size(); ++i)
            freqs.push_back(-nyquistMHz*1000 + i * 2000 * nyquistMHz / data.samplesI.size());
        mFFTdata->setPoints(freqs, data.fftBins_dbFS, false);
        mFFTpanel->UpdateAll();
    }

    StreamerLTE::ProgressStats stats = mLTEstreamer->GetStats();
    if(stats.RxFIFOfilled > 100)
        stats.RxFIFOfilled = 100;
    if(stats.TxFIFOfilled > 100)
        stats.TxFIFOfilled = 100;
    gaugeRxBuffer->SetValue((int)stats.RxFIFOfilled);
    gaugeTxBuffer->SetValue((int)stats.TxFIFOfilled);
    lblRxDataRate->SetLabel(printDataRate(stats.RxRate_Bps));
    lblTxDataRate->SetLabel(printDataRate(stats.TxRate_Bps));
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

void fftviewer_frFFTviewer::SetNyquistFrequency(float freqMHz)
{
    txtNyquistFreqMHz->SetValue(wxString::Format(_("%f"), freqMHz));
}
