#include "SDRConfiguration_view.h"

#include "limesuite/SDRDevice.h"
#include <wx/msgdlg.h>

using namespace std;
using namespace lime;

SOCConfig_view::SOCConfig_view(
    wxWindow* parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
    , socIndex(0)
{
    gui.titledBox = new wxStaticBox( this, wxID_ANY, wxT("RF SOC") );
    wxStaticBoxSizer* sbSizer = new wxStaticBoxSizer(gui.titledBox , wxHORIZONTAL);
    wxWindow* base = sbSizer->GetStaticBox();

    wxSizerFlags titleFlags(0);
    titleFlags = titleFlags.Center();
    wxSizerFlags ctrlFlags(0);
    ctrlFlags = ctrlFlags.Center();

    wxFlexGridSizer *rxGrid = new wxFlexGridSizer(5, 4, 4);
    {
        const vector<string> titles = {"Enable", "RxAntenna", "RxGain", "RxLPF (MHz)", "RxNCO (MHz)"};
        for (const auto& name : titles)
            rxGrid->Add(new wxStaticText(base, wxID_ANY, name.c_str()), titleFlags);

        for (int i=0; i<2; ++i)
        {
            ChannelConfigGUI& fields = gui.rx[i];
            fields.enable = new wxCheckBox(base, wxNewId(), (i == 0) ? "RxA" : "RxB");
            fields.enable->SetValue(true);
            rxGrid->Add(fields.enable, ctrlFlags);

            fields.path = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize);
            rxGrid->Add(fields.path, ctrlFlags);

            fields.gain = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize);
            fields.gain->Hide(); // not implemented yet
            rxGrid->Add(fields.gain, ctrlFlags);

            fields.lpf = new wxTextCtrl(base, wxNewId(), wxT("0"));
            rxGrid->Add(fields.lpf, ctrlFlags);

            fields.nco = new wxTextCtrl(base, wxNewId(), wxT("0"));;
            rxGrid->Add(fields.nco, ctrlFlags);
        }
    }

    wxFlexGridSizer *centerGrid = new wxFlexGridSizer(1, 4, 4);
    {
        wxFlexGridSizer *LOgrid = new wxFlexGridSizer(3, 0, 0);
        LOgrid->Add(new wxStaticText(base, wxID_ANY, "RxLO (MHz)"), titleFlags);
        LOgrid->Add(new wxStaticText(base, wxID_ANY, ""), titleFlags);
        LOgrid->Add(new wxStaticText(base, wxID_ANY, "TxLO (MHz)"), titleFlags);

        gui.rxLO = new wxTextCtrl(base, wxNewId(), wxT("1000"));;
        LOgrid->Add(gui.rxLO, ctrlFlags);

        gui.tdd = new wxCheckBox(base, wxNewId(), wxT("TDD"));
        LOgrid->Add(gui.tdd, ctrlFlags);

        gui.txLO = new wxTextCtrl(base, wxNewId(), wxT("1000"));;
        LOgrid->Add(gui.txLO, ctrlFlags);
        centerGrid->Add(LOgrid);

        wxFlexGridSizer *samplingGrid = new wxFlexGridSizer(2, 0, 0);

        samplingGrid->Add(new wxStaticText(base, wxID_ANY, wxT("Nyquist (MHz):")), titleFlags);
        gui.nyquist = new wxTextCtrl(base, wxNewId(), wxT("10"));
        samplingGrid->Add(gui.nyquist, ctrlFlags);
        centerGrid->Add(samplingGrid, wxSizerFlags().Center());

        wxFlexGridSizer *oversamplingGrid = new wxFlexGridSizer(4, 0, 0);
        oversamplingGrid->Add(new wxStaticText(base, wxID_ANY, "Decimate:"), titleFlags);
        wxArrayString oversampleNames;
        oversampleNames.Add(wxT("max"));
        for(int i=0; i<5; ++i)
            oversampleNames.Add(wxString::Format("%i", 1 << i));
        gui.decimation = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize, oversampleNames);
        gui.decimation->SetSelection(2);
        oversamplingGrid->Add(gui.decimation, ctrlFlags);

        oversamplingGrid->Add(new wxStaticText(base, wxID_ANY, "Interpolate:"), titleFlags);
        gui.interpolation = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize, oversampleNames);
        gui.interpolation->SetSelection(2);
        oversamplingGrid->Add(gui.interpolation, ctrlFlags);

        centerGrid->Add(oversamplingGrid, wxSizerFlags().Center());
    }

    wxFlexGridSizer *txGrid = new wxFlexGridSizer(5, 4, 4);
    {
        const vector<string> titles = {"TxNCO (MHz)", "TxLPF (MHz)", "TxGain", "TxAntenna", "Enable"};
        for (auto name : titles)
            txGrid->Add(new wxStaticText(base, wxID_ANY, name.c_str()), titleFlags);

        for (int i=0; i<2; ++i)
        {
            ChannelConfigGUI& fields = gui.tx[i];
            fields.nco = new wxTextCtrl(base, wxNewId(), wxT("0"));;
            txGrid->Add(fields.nco, ctrlFlags);

            fields.lpf = new wxTextCtrl(base, wxNewId(), wxT("0"));;
            txGrid->Add(fields.lpf, ctrlFlags);

            fields.gain = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize);
            txGrid->Add(fields.gain, ctrlFlags);
            fields.gain->Hide(); // not implemented yet

            fields.path = new wxChoice(base, wxNewId(), wxDefaultPosition, wxDefaultSize);
            txGrid->Add(fields.path, ctrlFlags);

            fields.enable = new wxCheckBox(base, wxNewId(), (i == 0) ? "TxA" : "TxB");
            fields.enable->SetValue(true);
            txGrid->Add(fields.enable, ctrlFlags);
        }
    }

    sbSizer->Add(rxGrid);
    sbSizer->Add(centerGrid, wxSizerFlags().Border(wxALL, 5));
    sbSizer->Add(txGrid);
    wxButton* btnSubmit = new wxButton(base, id, wxT("Submit"));
    btnSubmit->Bind(wxEVT_BUTTON, &SOCConfig_view::SubmitConfig, this);
    sbSizer->Add(btnSubmit, ctrlFlags);
    SetSizerAndFit(sbSizer);
}

void SOCConfig_view::Setup(SDRDevice *device, int index)
{
    sdrDevice = device;
    const SDRDevice::RFSOCDescripion &descriptor = device->GetDescriptor().rfSOC.at(index);
    socIndex = index;
    gui.titledBox->SetLabel(descriptor.name.c_str());
    wxArrayString rxPathNames;
    for (const auto& name : descriptor.rxPathNames)
        rxPathNames.Add(name.c_str());
    wxArrayString txPathNames;
    for (const auto& name : descriptor.txPathNames)
        txPathNames.Add(name.c_str());
    for (int i=0; i<descriptor.channelCount; ++i)
    {
        gui.rx[i].path->Set(rxPathNames);
        gui.rx[i].path->SetSelection(rxPathNames.size() > 0 ? 1 : 0);

        gui.tx[i].path->Set(txPathNames);
        gui.tx[i].path->SetSelection(txPathNames.size() > 0 ? 1 : 0);
    }
}

void SOCConfig_view::UpdateGUI(const lime::SDRDevice::SDRConfig &config)
{

}

void SOCConfig_view::SubmitConfig(wxCommandEvent &event)
{
    if (!sdrDevice)
        return;

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

        ch.rxCenterFrequency = parseGuiValue(gui.rxLO->GetValue()) * multiplier;
        ch.txCenterFrequency = parseGuiValue(gui.txLO->GetValue()) * multiplier;

        if(gui.tdd->IsChecked())
            ch.rxCenterFrequency = ch.txCenterFrequency;

        ch.rxNCOoffset = parseGuiValue(gui.rx[i].nco->GetValue()) * multiplier;
        ch.txNCOoffset = parseGuiValue(gui.tx[i].nco->GetValue()) * multiplier;
        ch.rxSampleRate = parseGuiValue(gui.nyquist->GetValue()) * multiplier;
        ch.txSampleRate = ch.rxSampleRate;
        // ch.rxGain = parseGuiValue(gui.rx[i].gain->GetSelection());
        // ch.txGain = parseGuiValue(gui.tx[i].gain->GetSelection());
        ch.rxPath = gui.rx[i].path->GetSelection();
        ch.txPath = gui.tx[i].path->GetSelection();
        ch.rxLPF = parseGuiValue(gui.rx[i].lpf->GetValue()) * multiplier;
        ch.txLPF = parseGuiValue(gui.tx[i].lpf->GetValue()) * multiplier;
        int oversampleIndex = gui.decimation->GetSelection();
        ch.rxOversample = oversampleIndex > 0 ? (1<<(oversampleIndex-1)) : 0;

        oversampleIndex = gui.interpolation->GetSelection();
        ch.txOversample = oversampleIndex > 0 ? (1<<(oversampleIndex-1)) : 0;
        ch.rxEnabled = gui.rx[i].enable->IsChecked();
        ch.txEnabled = gui.tx[i].enable->IsChecked();
        // ch.rxCalibrate;
        // ch.txCalibrate;
        // ch.rxTestSignal;
        // ch.txTestSignal;
    }

    try {
        sdrDevice->Init();
        sdrDevice->Configure(config, socIndex);
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

SDRConfiguration_view::SDRConfiguration_view(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                                     const wxSize &size, long style)
    : wxPanel(parent, id, pos, size, style), sdrDevice(nullptr)
{
    wxFlexGridSizer *mainSizer;
    mainSizer = new wxFlexGridSizer(4, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->SetFlexibleDirection(wxBOTH);
    mainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxSizerFlags ctrlFlags(0);
    ctrlFlags = ctrlFlags.Left().Top();

    const int rowCount = 4;
    for (int i=0; i<rowCount; ++i)
    {
        SOCConfig_view* row = new SOCConfig_view(this, wxNewId());
        mainSizer->Add(row, ctrlFlags);
        socGUI.push_back(row);
        if (i>0)
            row->Hide();
    }

   this->SetSizerAndFit(mainSizer);
}

void SDRConfiguration_view::Setup(lime::SDRDevice *device)
{
    sdrDevice = device;
    if (!sdrDevice)
        return;
    const SDRDevice::Descriptor &desc = device->GetDescriptor();
    for (size_t i=0; i<socGUI.size(); ++i)
    {
        if (i < desc.rfSOC.size())
        {
            socGUI[i]->Setup(sdrDevice, i);
            socGUI[i]->Show();

        }
        else
            socGUI[i]->Hide();
    }
    Fit();
}
