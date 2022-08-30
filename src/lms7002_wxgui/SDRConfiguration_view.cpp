#include "SDRConfiguration_view.h"

#include "SDRDevice.h"
#include <wx/msgdlg.h>

using namespace std;
using namespace lime;

wxPanel* SDRConfiguration_view::CreateChannelPanel(SDRConfigGUI &config ,wxWindow *parent, wxWindowID id)
{ 
    wxPanel *basePanel = new wxPanel(parent, id);

    wxStaticBoxSizer* sbSizer = new wxStaticBoxSizer( new wxStaticBox( basePanel, wxID_ANY, wxT("Channel A") ), wxHORIZONTAL);
        
    wxWindow* base = sbSizer->GetStaticBox();

    wxSizerFlags columnFlags = wxALIGN_CENTER_VERTICAL;
    wxSizerFlags titleFlags(0); 
    titleFlags = titleFlags.Center();
    wxSizerFlags ctrlFlags(0);
    ctrlFlags = ctrlFlags.Center();

    wxFlexGridSizer *rxGrid = new wxFlexGridSizer(5, 4, 4);
    {
        const vector<string> titles = {"Enable", "Antenna", "Gain", "LPF (MHz)", "NCO (MHz)"};
        for (auto name : titles)
            rxGrid->Add(new wxStaticText(base, wxID_ANY, name.c_str()), titleFlags);

        for (int i=0; i<2; ++i)
        {
            wxCheckBox *chkEnable = new wxCheckBox(base, wxNewId(), (i == 0) ? "RxA" : "RxB");
            config.rx[i].enable = chkEnable;
            rxGrid->Add(chkEnable, ctrlFlags);

            wxArrayString pathNames;
            pathNames.Add("LNA_L");
            pathNames.Add("LNA_W");
            pathNames.Add("LNA_H");
            wxChoice* cmbRxPath = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize, pathNames);
            config.rx[i].path = cmbRxPath;
            rxGrid->Add(cmbRxPath, ctrlFlags);

            wxArrayString gains;
            gains.Add("0db");
            gains.Add("-1db");
            wxChoice* cmbRxGain = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize, gains);
            config.rx[i].gain = cmbRxGain;
            rxGrid->Add(cmbRxGain, ctrlFlags);

            wxTextCtrl* txtRxLPF = new wxTextCtrl(base, wxNewId(), wxT("0"));
            config.rx[i].lpf = txtRxLPF;
            rxGrid->Add(txtRxLPF, ctrlFlags);

            wxTextCtrl* txtRxNCO = new wxTextCtrl(base, wxNewId(), wxT("0"));
            config.rx[i].nco = txtRxNCO;
            rxGrid->Add(txtRxNCO, ctrlFlags);
        }
    }

    wxFlexGridSizer *centerGrid = new wxFlexGridSizer(1, 4, 4);
    {
        wxFlexGridSizer *LOgrid = new wxFlexGridSizer(3, 0, 0);
        LOgrid->Add(new wxStaticText(base, wxID_ANY, "RxLO (MHz)"), titleFlags);
        LOgrid->Add(new wxStaticText(base, wxID_ANY, ""), titleFlags);
        LOgrid->Add(new wxStaticText(base, wxID_ANY, "TxLO (MHz)"), titleFlags);

        wxTextCtrl* txtRxLO = new wxTextCtrl(base, wxNewId(), wxT("1000"));
        config.rxLO = txtRxLO;
        LOgrid->Add(txtRxLO, ctrlFlags);

        wxCheckBox* chkTDD = new wxCheckBox(base, wxNewId(), wxT("TDD"));
        config.tdd = chkTDD;
        LOgrid->Add(chkTDD, ctrlFlags);

        wxTextCtrl* txtTxLO = new wxTextCtrl(base, wxNewId(), wxT("1000"));
        config.txLO = txtTxLO;
        LOgrid->Add(txtTxLO, ctrlFlags);
        centerGrid->Add(LOgrid);

        wxFlexGridSizer *samplingGrid = new wxFlexGridSizer(2, 0, 0);

        samplingGrid->Add(new wxStaticText(base, wxID_ANY, "Nyquist (MHz):"), titleFlags);
        wxTextCtrl* txtSampleRate = new wxTextCtrl(base, wxNewId(), wxT("10"));
        config.nyquist = txtSampleRate;
        samplingGrid->Add(txtSampleRate, ctrlFlags);

        samplingGrid->Add(new wxStaticText(base, wxID_ANY, "Oversample:"), titleFlags);
        wxArrayString oversampleNames;
        oversampleNames.Add(wxT("max"));
        for(int i=0; i<5; ++i)
            oversampleNames.Add(wxString::Format("%i", 1 << i));
        wxChoice* cmbOversample = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize, oversampleNames);
        cmbOversample->SetSelection(0);
        config.oversample = cmbOversample;
        samplingGrid->Add(cmbOversample, ctrlFlags);

        centerGrid->Add(samplingGrid, wxSizerFlags().Center());
    }

    wxFlexGridSizer *txGrid = new wxFlexGridSizer(5, 4, 4);
    {
        const vector<string> titles = {"NCO (MHz)", "LPF (MHz)", "Gain", "Antenna", "Enable"};
        for (auto name : titles)
            txGrid->Add(new wxStaticText(base, wxID_ANY, name.c_str()), titleFlags);

        for (int i=0; i<2; ++i)
        {
            wxTextCtrl* txtTxNCO = new wxTextCtrl(base, wxNewId(), wxT("0"));
            config.tx[i].nco = txtTxNCO;
            txGrid->Add(txtTxNCO, ctrlFlags);

            wxTextCtrl* txtTxLPF = new wxTextCtrl(base, wxNewId(), wxT("0"));
            config.tx[i].lpf = txtTxLPF;
            txGrid->Add(txtTxLPF, ctrlFlags);

            wxArrayString gains;
            gains.Add("0db");
            gains.Add("-1db");
            wxChoice* cmbTxGain = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize, gains);
            cmbTxGain->SetSelection(0);
            config.tx[i].gain = cmbTxGain;
            txGrid->Add(cmbTxGain, ctrlFlags);

            wxArrayString pathNames;
            pathNames.Add("BAND_1");
            pathNames.Add("BAND_2");
            wxChoice* cmbTxPath = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize, pathNames);
            cmbTxPath->SetSelection(0);
            config.tx[i].path = cmbTxPath;
            txGrid->Add(cmbTxPath, ctrlFlags);

            wxCheckBox* chkEnable = new wxCheckBox(base, wxNewId(), (i == 0) ? "TxA" : "TxB");
            config.tx[i].enable = chkEnable;
            txGrid->Add(chkEnable, ctrlFlags);
        }
    }

    sbSizer->Add(rxGrid);
    sbSizer->Add(centerGrid, wxSizerFlags().Border(wxALL, 5));
    sbSizer->Add(txGrid);
    wxButton* btnSubmit = new wxButton(base, id, wxT("Submit"));
    btnSubmit->Bind(wxEVT_BUTTON, &SDRConfiguration_view::SubmitConfiguration, this);
    sbSizer->Add(btnSubmit, ctrlFlags);
    basePanel->SetSizerAndFit(sbSizer);
    return basePanel;
}

SDRConfiguration_view::SDRConfiguration_view(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                                     const wxSize &size, long style)
    : wxPanel(parent, id, pos, size, style), sdrDevice(nullptr)
{
    wxFlexGridSizer *mainSizer;
    mainSizer = new wxFlexGridSizer(3, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(1);
    mainSizer->SetFlexibleDirection(wxBOTH);
    mainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


    const int rowCount = 1;//sdrDevice->GetDescriptor().rfSOC_count;
    for (int i=0; i<rowCount; ++i)
    {
        wxWindowID cfgId = wxNewId();
        mConfigControls[cfgId] = SDRConfigGUI();
        wxPanel *cfgRow = CreateChannelPanel(mConfigControls[cfgId], this, cfgId);
        mainSizer->Add(cfgRow);
    }

   this->SetSizerAndFit(mainSizer);
}

void SDRConfiguration_view::Initialize(lime::SDRDevice *device)
{
    sdrDevice = device;
}

void SDRConfiguration_view::SubmitConfiguration(wxCommandEvent &event)
{
    if (!sdrDevice)
        return;

    int id = event.GetId();
    const SDRConfigGUI &gui = mConfigControls[id];
    SDRDevice::SDRConfig config;

    config.referenceClockFreq = 30.72e6;

    auto parseGuiValue = [](const wxString& str)
    {
        double value = 0;
        str.ToDouble(&value);
        return value;
    };

    for(int i=0; i<2; ++i)
    {
        const double multiplier = 1e6; // convert from GUI MHz to Hz
        SDRDevice::ChannelConfig &ch = config.channel[i];

        ch.rxCenterFrequency = parseGuiValue(gui.rxLO->GetValue())*multiplier;
        ch.txCenterFrequency = parseGuiValue(gui.txLO->GetValue())*multiplier;

        if(gui.tdd->IsChecked())
            ch.rxCenterFrequency = ch.txCenterFrequency;

        ch.rxNCOoffset = parseGuiValue(gui.rx[i].nco->GetValue())*multiplier;
        ch.txNCOoffset = parseGuiValue(gui.tx[i].nco->GetValue())*multiplier;
        ch.rxSampleRate = parseGuiValue(gui.nyquist->GetValue())*multiplier;
        ch.txSampleRate = ch.rxSampleRate;
        // ch.rxGain = parseGuiValue(gui.rx[i].gain->GetSelection());
        // ch.txGain = parseGuiValue(gui.tx[i].gain->GetSelection());
        ch.rxPath = gui.rx[i].path->GetSelection();
        ch.txPath = gui.tx[i].path->GetSelection();
        ch.rxLPF = parseGuiValue(gui.rx[i].lpf->GetValue())*multiplier;
        ch.txLPF = parseGuiValue(gui.tx[i].lpf->GetValue())*multiplier;
        const int oversampleIndex = gui.oversample->GetSelection();
        ch.rxOversample = oversampleIndex > 0 ? (1<<(oversampleIndex-1)) : 0;
        ch.txOversample = ch.rxOversample;
        ch.rxEnabled = gui.rx[i].enable->IsChecked();
        ch.txEnabled = gui.tx[i].enable->IsChecked();
        // ch.rxCalibrate;
        // ch.txCalibrate;
        // ch.rxTestSignal;
        // ch.txTestSignal;
    }

    try {
        sdrDevice->Configure(config, 0);
    }
    catch (std::logic_error &e) // settings problem
    {
        wxMessageBox(wxString::Format("Configure failed: %s", e.what()), _("Warning"));
        return;
    }
    catch (std::runtime_error &e) // communications problem
    {
        wxMessageBox(wxString::Format("Configure failed: %s", e.what()), _("Warning"));
        return;
    }
}