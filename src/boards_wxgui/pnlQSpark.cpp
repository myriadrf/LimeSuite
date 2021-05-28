#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/filedlg.h>

#include "pnlQSpark.h"
#include <ciso646>
#include <map>

#include <FPGA_common.h>
#include <lms7_device.h>
#include "lms7suiteEvents.h"
#include "iniParser.h"
#include "lms7002_dlgGFIR_Coefficients.h"

using namespace lime;
using namespace std;

BEGIN_EVENT_TABLE(pnlQSpark, wxPanel)
END_EVENT_TABLE()

pnlQSpark::Register::Register()
    : address(0), msb(0), lsb(0), defaultValue(0)
{
}

pnlQSpark::Register::Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue, unsigned short twocomplement)
    : address(address), msb(msb), lsb(lsb), defaultValue(defaultValue), twocomplement(twocomplement)
{
}

pnlQSpark::pnlQSpark(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, int style, wxString name)
{
    lmsControl = nullptr;

    wxFlexGridSizer *FlexGridSizer1;

    Create(parent, id, wxDefaultPosition, wxDefaultSize, style, _T("id"));
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer *mainSizer = new wxFlexGridSizer(0, 2, 10, 10);
    FlexGridSizer1 = new wxFlexGridSizer(0, 3, 10, 10);
    wxFlexGridSizer *FlexGridSizer2 = new wxFlexGridSizer(0, 1, 10, 10);
    SetSizer(mainSizer);

    wxStaticBoxSizer *ctrlGroup2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Controls"));
    wxFlexGridSizer *ctrlSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    {
        auto groupSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("External loopback controls")), wxVERTICAL);
        {
            auto controlsSizer = new wxFlexGridSizer(0, 1, 5, 5);
            chkLB_1A = new wxCheckBox(this, wxNewId(), _("RF loopback ch.A"));
            chkLB_1A->SetToolTip(_("[RFLB_A_EN] External RF loopback TxBAND2->RxLNAH channel A"));
            Connect(chkLB_1A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_1A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkLB_1B = new wxCheckBox(this, wxNewId(), _("RF loopback ch.B"));
            chkLB_1B->SetToolTip(_("[RFLB_B_EN] External RF loopback TxBAND2->RxLNAH channel B"));
            Connect(chkLB_1B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_1B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

            chkSH_1A = new wxCheckBox(this, wxNewId(), _("Ch.A shunt"));
            chkSH_1A->SetToolTip(_("[TX1_2_LB_SH]"));
            Connect(chkSH_1A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_1A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkSH_1B = new wxCheckBox(this, wxNewId(), _("Ch.B shunt"));
            chkSH_1B->SetToolTip(_("[TX2_2_LB_SH]"));
            Connect(chkSH_1B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_1B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_1A = new wxCheckBox(this, wxNewId(), _("Ch.A attenuator"));
            chkAT_1A->SetToolTip(_("[TX1_2_LB_AT]"));
            Connect(chkAT_1A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_1A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_1B = new wxCheckBox(this, wxNewId(), _("Ch.B attenuator"));
            chkAT_1B->SetToolTip(_("[TX2_2_LB_AT]"));
            Connect(chkAT_1B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_1B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            auto lms1sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("LMS#1")), wxVERTICAL);
            lms1sizer->Add(controlsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
            groupSizer->Add(lms1sizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
        }
        {
            auto controlsSizer = new wxFlexGridSizer(0, 1, 5, 5);
            chkLB_2A = new wxCheckBox(this, wxNewId(), _("RF loopback ch.A"));
            chkLB_2A->SetToolTip(_("[RFLB_A_EN] External RF loopback TxBAND2->RxLNAH channel A"));
            Connect(chkLB_2A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_2A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkLB_2B = new wxCheckBox(this, wxNewId(), _("RF loopback ch.B"));
            chkLB_2B->SetToolTip(_("[RFLB_B_EN] External RF loopback TxBAND2->RxLNAH channel B"));
            Connect(chkLB_2B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_2B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

            chkSH_2A = new wxCheckBox(this, wxNewId(), _("Ch.A shunt"));
            chkSH_2A->SetToolTip(_("[TX1_2_LB_SH]"));
            Connect(chkSH_2A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_2A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkSH_2B = new wxCheckBox(this, wxNewId(), _("Ch.B shunt"));
            chkSH_2B->SetToolTip(_("[TX2_2_LB_SH]"));
            Connect(chkSH_2B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_2B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_2A = new wxCheckBox(this, wxNewId(), _("Ch.A attenuator"));
            chkAT_2A->SetToolTip(_("[TX1_2_LB_AT]"));
            Connect(chkAT_2A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_2A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_2B = new wxCheckBox(this, wxNewId(), _("Ch.B attenuator"));
            chkAT_2B->SetToolTip(_("[TX2_2_LB_AT]"));
            Connect(chkAT_2B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_2B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            auto lms1sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("LMS#2")), wxVERTICAL);
            lms1sizer->Add(controlsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
            groupSizer->Add(lms1sizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
        }
        {

            btnUpdateAll = new wxButton(this, wxNewId(), _T("Refresh All"));
            Connect(btnUpdateAll->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlQSpark::OnbtnUpdateAll);
            btnLoadSettings = new wxButton(this, wxNewId(), _T("Read settings"));
            Connect(btnLoadSettings->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlQSpark::LoadQSparkSettings);
            btnSaveSettings = new wxButton(this, wxNewId(), _T("Save settings"));
            Connect(btnSaveSettings->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlQSpark::SaveQSparkSettings);
            chkResetN = new wxCheckBox(this, wxNewId(), _T("ResetN"));
            Connect(chkResetN->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

            ctrlSizer2->Add(btnLoadSettings, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 50);
            ctrlSizer2->Add(btnUpdateAll, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 50);
            ctrlSizer2->Add(btnSaveSettings, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 50);
            ctrlSizer2->Add(chkResetN, 1, wxALIGN_LEFT | wxALIGN_TOP, 50);
            ctrlGroup2->Add(ctrlSizer2, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 25);
            //groupSizer->Add(ctrlGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5); // only added
        }
        mainSizer->Add(groupSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    }

    wxSize freqTextfieldSize(64, -1);
    mPanelStreamPLL = new wxPanel(this, wxNewId());
    wxFlexGridSizer *sizerPllControls = new wxFlexGridSizer(0, 3, 5, 5);
    sizerPllControls->Add(new wxStaticText(mPanelStreamPLL, wxID_ANY, _("DAC :")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    txtPllFreqTxMHz = new wxTextCtrl(mPanelStreamPLL, wxNewId(), _("160"), wxDefaultPosition, freqTextfieldSize);
    sizerPllControls->Add(txtPllFreqTxMHz, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblRealFreqTx = new wxStaticText(mPanelStreamPLL, wxID_ANY, _("MHz"));
    sizerPllControls->Add(lblRealFreqTx, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    sizerPllControls->Add(new wxStaticText(mPanelStreamPLL, wxID_ANY, _("ADC :")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    txtPllFreqRxMHz = new wxTextCtrl(mPanelStreamPLL, wxNewId(), _("160"), wxDefaultPosition, freqTextfieldSize);
    sizerPllControls->Add(txtPllFreqRxMHz, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblRealFreqRx = new wxStaticText(mPanelStreamPLL, wxID_ANY, _("MHz"));
    sizerPllControls->Add(lblRealFreqRx, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    btnConfigurePLL = new wxButton(mPanelStreamPLL, wxNewId(), _("Configure"));
    sizerPllControls->Add(btnConfigurePLL, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Connect(btnConfigurePLL->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(pnlQSpark::OnConfigurePLL), NULL, this);

    wxStaticBoxSizer *streamPllGroup = new wxStaticBoxSizer(wxHORIZONTAL, mPanelStreamPLL, _T("Digital Interface Clock"));
    mPanelStreamPLL->SetSizer(streamPllGroup);
    streamPllGroup->Add(sizerPllControls, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    streamPllGroup->Fit(mPanelStreamPLL);
    FlexGridSizer2->Add(mPanelStreamPLL, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxStaticBoxSizer *loopbackGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Loopback controls"));
    wxFlexGridSizer *loopbackSizer = new wxFlexGridSizer(0, 2, 0, 0);
    loopbackGroup->Add(loopbackSizer, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);
    FlexGridSizer2->Add(loopbackGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);

    mPanelPACtrl = new wxPanel(this, wxNewId());
    wxFlexGridSizer *sizerPAMux = new wxFlexGridSizer(0, 2, 5, 5);
    sizerPAMux->Add(new wxStaticText(mPanelPACtrl, wxID_ANY, _("RX1_W source")), 5, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    wxString pa_choices[] = {wxT("None"), wxT("Ext. RF PA input 1"), wxT("PA1 coupler"), wxT("Ext. RF PA input 2"), wxT("PA2 coupler")};
    cmbPAsrc = new wxChoice(mPanelPACtrl, wxNewId(), wxDefaultPosition, wxDefaultSize, 5, pa_choices, 0);
    cmbPAsrc->SetSelection(0);
    Connect(cmbPAsrc->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    sizerPAMux->Add(cmbPAsrc, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    chkPA = new wxCheckBox(mPanelPACtrl, wxNewId(), _("PA Ch.A"));
    sizerPAMux->Add(chkPA, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkPA->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkPA_chB = new wxCheckBox(mPanelPACtrl, wxNewId(), _("PA Ch.B"));
    sizerPAMux->Add(chkPA_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkPA_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkDCDC = new wxCheckBox(mPanelPACtrl, wxNewId(), _("DC/DC Ch.A"));
    sizerPAMux->Add(chkDCDC, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkDCDC->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkDCDC_chB = new wxCheckBox(mPanelPACtrl, wxNewId(), _("DC/DC Ch.B"));
    sizerPAMux->Add(chkDCDC_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkDCDC_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    wxStaticBoxSizer *paGroup = new wxStaticBoxSizer(wxHORIZONTAL, mPanelPACtrl, _T("PA Controls"));
    mPanelPACtrl->SetSizer(paGroup);
    paGroup->Add(sizerPAMux, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    paGroup->Fit(mPanelPACtrl);
    FlexGridSizer2->Add(mPanelPACtrl, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    {
        wxStaticBoxSizer *ctrlGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("FIR filter"));
        wxFlexGridSizer *ctrlSizer = new wxFlexGridSizer(0, 4, 0, 0);

        ctrlSizer->Add(new wxStaticText(this, wxID_ANY, _("Ch.A ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        chkSLEEP_FIR = new wxCheckBox(this, wxNewId(), _("Sleep"));
        ctrlSizer->Add(chkSLEEP_FIR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
        Connect(chkSLEEP_FIR->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

        chkBYPASS_FIR = new wxCheckBox(this, wxNewId(), _("Bypass"));
        ctrlSizer->Add(chkBYPASS_FIR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
        Connect(chkBYPASS_FIR->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

        chkODD_FIR = new wxCheckBox(this, wxNewId(), _("Odd"));
        ctrlSizer->Add(chkODD_FIR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
        Connect(chkODD_FIR->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

        setFIR1 = new wxButton(this, wxNewId(), _T("Coeff."));
        Connect(setFIR1->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlQSpark::onbtnFIRCoefA);
        ctrlSizer->Add(setFIR1, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 50);

        ctrlSizer->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 5);
        ctrlSizer->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 5);
        ctrlSizer->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 5);

        ctrlSizer->Add(new wxStaticText(this, wxID_ANY, _("Ch.B ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        chkSLEEP_FIR_chB = new wxCheckBox(this, wxNewId(), _("Sleep"));
        ctrlSizer->Add(chkSLEEP_FIR_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
        Connect(chkSLEEP_FIR_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

        chkBYPASS_FIR_chB = new wxCheckBox(this, wxNewId(), _("Bypass"));
        ctrlSizer->Add(chkBYPASS_FIR_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
        Connect(chkBYPASS_FIR_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

        chkODD_FIR_chB = new wxCheckBox(this, wxNewId(), _("Odd"));
        ctrlSizer->Add(chkODD_FIR_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
        Connect(chkODD_FIR_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

        setFIR2 = new wxButton(this, wxNewId(), _T("Coeff."));
        Connect(setFIR2->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlQSpark::onbtnFIRCoefB);
        ctrlSizer->Add(setFIR2, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 50);

        ctrlGroup->Add(ctrlSizer, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 25);
        FlexGridSizer2->Add(ctrlGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5); 
    }

    FlexGridSizer1->Add(FlexGridSizer2, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxStaticBoxSizer *chBox = new wxStaticBoxSizer(wxVERTICAL, this, _T("ADC/DAC channel controls"));
    wxFlexGridSizer *chSizer = new wxFlexGridSizer(0, 1, 0, 0);
    wxFlexGridSizer *chSelect = new wxFlexGridSizer(0, 3, 0, 0);
    rbChannelA = new wxRadioButton(this, wxNewId(), wxT("A CHANNEL"), wxDefaultPosition, wxDefaultSize, 0);
    rbChannelA->SetValue(true);
    rbChannelA->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlQSpark::OnSwitchToChannelA), NULL, this);
    chSelect->Add(rbChannelA, 0, wxEXPAND, 5);

    rbChannelB = new wxRadioButton(this, wxNewId(), wxT("B CHANNEL"), wxDefaultPosition, wxDefaultSize, 0);
    chSelect->Add(rbChannelB, 0, wxEXPAND, 5);
    rbChannelB->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlQSpark::OnSwitchToChannelB), NULL, this);
    chSizer->Add(chSelect, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    chBox->Add(chSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    FlexGridSizer1->Add(chBox, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    wxSize spinBoxSize(64, -1);
    long spinBoxStyle = wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER;

    wxFlexGridSizer *cfrSizer = new wxFlexGridSizer(0, 4, 0, 4);
    wxStaticBoxSizer *cfrGroup = new wxStaticBoxSizer(wxVERTICAL, this, _T("Crest factor reduction, Hann windowing"));

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Ch.A ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _(" ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _(" ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    chkDEL_HB = new wxCheckBox(this, wxNewId(), _("HB del byp"));
    cfrSizer->Add(chkDEL_HB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkDEL_HB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkBYPASS_CFR = new wxCheckBox(this, wxNewId(), _("Bypass"));
    cfrSizer->Add(chkBYPASS_CFR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkBYPASS_CFR->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    chkSLEEP_CFR = new wxCheckBox(this, wxNewId(), _("Sleep"));
    cfrSizer->Add(chkSLEEP_CFR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkSLEEP_CFR->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    chkODD_CFR = new wxCheckBox(this, wxNewId(), _("Odd"));
    cfrSizer->Add(chkODD_CFR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkODD_CFR->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    chkBYPASSGAIN_CFR = new wxCheckBox(this, wxNewId(), _("Bypass gain"));
    cfrSizer->Add(chkBYPASSGAIN_CFR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkBYPASSGAIN_CFR->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("CFR order")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinCFR_ORDER = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 2, 40, 39);
    cfrSizer->Add(spinCFR_ORDER, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinCFR_ORDER->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::OnOrderChanged), NULL, this);
    Connect(spinCFR_ORDER->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::OnOrderChanged), NULL, this);
    Connect(spinCFR_ORDER->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::OnOrderChanged), NULL, this);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Gain")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    thresholdGain = new wxSpinCtrlDouble(this, wxNewId(), _("1.0"), wxDefaultPosition, wxSize(100, -1), 0, 0.0, 4.0, 1.0, 0.01);
    cfrSizer->Add(thresholdGain, 1, wxALIGN_LEFT | wxEXPAND, 5);
    Connect(thresholdGain->GetId(), wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(pnlQSpark::OnGainChanged), NULL, this);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Interpolation")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    wxString interpolation_choices[] = {wxT("0"), wxT("1")};
    cmbINTER_CFR = new wxChoice(this, wxNewId(), wxDefaultPosition, spinBoxSize, 2, interpolation_choices, 0);
    cmbINTER_CFR->SetSelection(0);
    Connect(cmbINTER_CFR->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::OnInterpolationChanged), NULL, this);
    cfrSizer->Add(cmbINTER_CFR, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Threshold")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    thresholdSpin = new wxSpinCtrlDouble(this, wxNewId(), _("1.0"), wxDefaultPosition, wxSize(100, -1), 0, 0.0, 1.0, 1.0, 0.01);
    cfrSizer->Add(thresholdSpin, 1, wxALIGN_LEFT | wxEXPAND, 5);
    Connect(thresholdSpin->GetId(), wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(pnlQSpark::OnThresholdChanged), NULL, this);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Ch.B ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _(" ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _(" ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    chkDEL_HB_chB = new wxCheckBox(this, wxNewId(), _("HB del byp"));
    cfrSizer->Add(chkDEL_HB_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkDEL_HB_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkBYPASS_CFR_chB = new wxCheckBox(this, wxNewId(), _("Bypass"));
    cfrSizer->Add(chkBYPASS_CFR_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkBYPASS_CFR_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkSLEEP_CFR_chB = new wxCheckBox(this, wxNewId(), _("Sleep"));
    cfrSizer->Add(chkSLEEP_CFR_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkSLEEP_CFR_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkODD_CFR_chB = new wxCheckBox(this, wxNewId(), _("Odd"));
    cfrSizer->Add(chkODD_CFR_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkODD_CFR_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkBYPASSGAIN_CFR_chB = new wxCheckBox(this, wxNewId(), _("Bypass gain"));
    cfrSizer->Add(chkBYPASSGAIN_CFR_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkBYPASSGAIN_CFR_chB->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("CFR order")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinCFR_ORDER_chB = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 2, 40, 39);
    cfrSizer->Add(spinCFR_ORDER_chB, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinCFR_ORDER_chB->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::OnOrderChanged), NULL, this);
    Connect(spinCFR_ORDER_chB->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::OnOrderChanged), NULL, this);
    Connect(spinCFR_ORDER_chB->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::OnOrderChanged), NULL, this);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Gain")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    thresholdGain_chB = new wxSpinCtrlDouble(this, wxNewId(), _("1.0"), wxDefaultPosition, wxSize(100, -1), 0, 0.0, 4.0, 1.0, 0.01);
    cfrSizer->Add(thresholdGain_chB, 1, wxALIGN_LEFT | wxEXPAND, 5);
    Connect(thresholdGain_chB->GetId(), wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(pnlQSpark::OnGainChanged), NULL, this);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Interpolation")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cmbINTER_CFR_chB = new wxChoice(this, wxNewId(), wxDefaultPosition, spinBoxSize, 2, interpolation_choices, 0);
    cmbINTER_CFR_chB->SetSelection(0);
    Connect(cmbINTER_CFR_chB->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::OnInterpolationChanged), NULL, this);
    cfrSizer->Add(cmbINTER_CFR_chB, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    cfrSizer->Add(new wxStaticText(this, wxID_ANY, _("Threshold")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    thresholdSpin_chB = new wxSpinCtrlDouble(this, wxNewId(), _("1.0"), wxDefaultPosition, wxSize(100, -1), 0, 0.0, 1.0, 1.0, 0.01);
    cfrSizer->Add(thresholdSpin_chB, 1, wxALIGN_LEFT | wxEXPAND, 5);
    //cfrSizer->Add(new wxStaticText(this, wxID_ANY, _(" ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Connect(thresholdSpin_chB->GetId(), wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(pnlQSpark::OnThresholdChanged), NULL, this);

    cfrGroup->Add(cfrSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chSizer->Add(cfrGroup, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chSizer->Add(ctrlGroup2, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5); // only added

    mainSizer->Add(FlexGridSizer1, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);

    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();

    controlsPtr2Registers.clear();

    controlsPtr2Registers[chkLB_1A] = Register(0x0017, 1, 0, 2, 0);
    controlsPtr2Registers[chkLB_1B] = Register(0x0017, 5, 4, 2, 0);
    controlsPtr2Registers[chkLB_2A] = Register(0x0017, 9, 8, 2, 0);
    controlsPtr2Registers[chkLB_2B] = Register(0x0017, 13, 12, 2, 0);
    controlsPtr2Registers[chkSH_1A] = Register(0x0017, 3, 3, 1, 0);
    controlsPtr2Registers[chkSH_1B] = Register(0x0017, 7, 7, 1, 0);
    controlsPtr2Registers[chkSH_2A] = Register(0x0017, 11, 11, 1, 0);
    controlsPtr2Registers[chkSH_2B] = Register(0x0017, 15, 15, 1, 0);
    controlsPtr2Registers[chkAT_1A] = Register(0x0017, 2, 2, 0, 0);
    controlsPtr2Registers[chkAT_1B] = Register(0x0017, 6, 6, 0, 0);
    controlsPtr2Registers[chkAT_2A] = Register(0x0017, 10, 10, 0, 0);
    controlsPtr2Registers[chkAT_2B] = Register(0x0017, 14, 14, 0, 0);

    controlsPtr2Registers[chkPA] = Register(0x0041, 2, 2, 0, 0);
    controlsPtr2Registers[chkPA_chB] = Register(0x0041, 3, 3, 0, 0);

    controlsPtr2Registers[chkDCDC] = Register(0x0041, 7, 7, 0, 0);
    controlsPtr2Registers[chkDCDC_chB] = Register(0x0041, 8, 8, 0, 0);

    controlsPtr2Registers[cmbPAsrc] = Register(0x0041, 6, 4, 0, 0);
    controlsPtr2Registers[chkSLEEP_CFR] = Register(0x0045, 0, 0, 0, 0);
    controlsPtr2Registers[chkBYPASS_CFR] = Register(0x0045, 1, 1, 0, 0);
    controlsPtr2Registers[chkODD_CFR] = Register(0x0045, 2, 2, 0, 0);
    controlsPtr2Registers[chkBYPASSGAIN_CFR] = Register(0x0045, 3, 3, 0, 0);
    controlsPtr2Registers[chkSLEEP_FIR] = Register(0x0045, 4, 4, 0, 0);
    controlsPtr2Registers[chkBYPASS_FIR] = Register(0x0045, 5, 5, 0, 0);
    controlsPtr2Registers[chkODD_FIR] = Register(0x0045, 6, 6, 0, 0);
    controlsPtr2Registers[chkDEL_HB] = Register(0x0045, 7, 7, 0, 0);
    controlsPtr2Registers[cmbINTER_CFR] = Register(0x0041, 13, 12, 0, 0);

    controlsPtr2Registers[chkSLEEP_CFR_chB] = Register(0x0045, 8, 8, 0, 0);
    controlsPtr2Registers[chkBYPASS_CFR_chB] = Register(0x0045, 9, 9, 0, 0);
    controlsPtr2Registers[chkODD_CFR_chB] = Register(0x0045, 10, 10, 0, 0);
    controlsPtr2Registers[chkBYPASSGAIN_CFR_chB] = Register(0x0045, 11, 11, 0, 0);
    controlsPtr2Registers[chkSLEEP_FIR_chB] = Register(0x0045, 12, 12, 0, 0);
    controlsPtr2Registers[chkBYPASS_FIR_chB] = Register(0x0045, 13, 13, 0, 0);
    controlsPtr2Registers[chkODD_FIR_chB] = Register(0x0045, 14, 14, 0, 0);
    controlsPtr2Registers[chkDEL_HB_chB] = Register(0x0045, 15, 15, 0, 0);
    controlsPtr2Registers[cmbINTER_CFR_chB] = Register(0x0041, 15, 14, 0, 0);

    //cfr_threshold
    controlsPtr2Registers[thresholdSpin] = Register(0x0046, 15, 0, 0, 0);     
    controlsPtr2Registers[thresholdSpin_chB] = Register(0x0047, 15, 0, 0, 0); 
    controlsPtr2Registers[thresholdGain] = Register(0x0048, 15, 0, 0, 1);     
    controlsPtr2Registers[thresholdGain_chB] = Register(0x0049, 15, 0, 0, 1); 
    controlsPtr2Registers[spinCFR_ORDER] = Register(0x004A, 7, 0, 0, 0);      
    controlsPtr2Registers[spinCFR_ORDER_chB] = Register(0x004A, 15, 8, 0, 0); 

    controlsPtr2Registers[chkResetN] = Register(0x0041, 11, 11, 0, 0);
    
    Bind(READ_ALL_VALUES, &pnlQSpark::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlQSpark::OnWriteAll, this, this->GetId());

    chkResetN->Enable(false);
    chkDEL_HB->Enable(false);
    chkDEL_HB_chB->Enable(false);
    chkODD_CFR->Enable(false);
    chkODD_CFR_chB->Enable(false);
    chkSLEEP_CFR->Enable(false);
    chkSLEEP_CFR_chB->Enable(false);
}

void pnlQSpark::OnInterpolationChanged(wxCommandEvent &event)
{

    int interpolation = 0;
    int order = 0;

    if (chA == 1)
    {
        interpolation = cmbINTER_CFR->GetSelection();
        order = spinCFR_ORDER->GetValue();
    }
    else
    {
        interpolation = cmbINTER_CFR_chB->GetSelection();
        order = spinCFR_ORDER_chB->GetValue();
    }

    if (order > 40)
        order = 40;
    if (order < 1)
        order = 1;

    Register reg = controlsPtr2Registers[event.GetEventObject()];
    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask
    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content
    regValue &= ~mask;
    regValue |= (interpolation << reg.lsb) & mask;
    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0) // register write
        wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));

    UpdateHannCoeff(lmsControl, order, chA, interpolation);
    wxCommandEvent evt;
    OnbtnUpdateAll(evt);
}

void pnlQSpark::OnOrderChanged(wxCommandEvent &event)
{
    int val = 0;
    int interpolation = 0;

    wxSpinCtrl *p = (wxSpinCtrl *)event.GetEventObject();
    val = p->GetValue();

    Register reg = controlsPtr2Registers[event.GetEventObject()];
    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content

    int order = val;
    if (order > 40)
        order = 40;
    if (order < 1)
        order = 1;

    regValue &= ~mask;
    regValue |= (order << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0) // register write
        wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));

    if (chA == 1)
        interpolation = cmbINTER_CFR->GetSelection();
    else
        interpolation = cmbINTER_CFR_chB->GetSelection();

    UpdateHannCoeff(lmsControl, order, chA, interpolation);
    wxCommandEvent evt;
    OnbtnUpdateAll(evt);
}

void pnlQSpark::SetRegValue(Register reg, uint16_t newValue)
{
    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content

    regValue &= ~mask;
    regValue |= (newValue << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0) // register write
        wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));
}

void pnlQSpark::OnThresholdChanged(wxCommandEvent &event)
{
    double threshold;

    wxSpinCtrlDouble *p = (wxSpinCtrlDouble *)event.GetEventObject();
    threshold = p->GetValue();

    Register reg = controlsPtr2Registers[event.GetEventObject()];
    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content

    int code = (int)(threshold * 65535);
    if (code >= 65535)
        code = 65535;
    if (code < 0)
        code = 0;

    regValue &= ~mask;
    regValue |= (code << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0) // register write
        wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));
}

void pnlQSpark::OnGainChanged(wxCommandEvent &event)
{
    double threshold;
    wxSpinCtrlDouble *p = (wxSpinCtrlDouble *)event.GetEventObject();
    threshold = p->GetValue();

    Register reg = controlsPtr2Registers[event.GetEventObject()];
    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue); // read bit content

    int code = (int)(threshold * 8192);
    if (code >= 32767)
        code = 32767;
    if (code < 0)
        code = 0;

    regValue &= ~mask;
    regValue |= (code << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0) // register write
        wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));
}

int pnlQSpark::SPI_write(lms_device_t *lmsControl, uint16_t address, uint16_t data)
{

    int ret = 0;
    if (LMS_WriteFPGAReg(lmsControl, address, data) != 0)
        ret = -1;
    return ret;
}

void pnlQSpark::UpdateHannCoeff(lms_device_t *lmsControl, uint16_t Filt_N, int chAA, int interpolation)
{
    Register reg, reg2, reg3, reg4;
    uint16_t msb, lsb = 0;
    uint16_t data = 0;
    uint16_t addr = 0;
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t offset = 0;
    uint16_t w[60];

    uint16_t maddressf0 = 0x07; //maddress==14
    uint16_t maddressf1 = 0x08; //maddress==16

    uint16_t NN = 3;
    uint16_t MaxFilt_N = 40;

    if (interpolation > 1)
        interpolation = 1; //limit
    else if (interpolation < 0)
        interpolation = 0; //limit

    if (interpolation == 2)
    {
        NN = 0;
        MaxFilt_N = 10;
    }
    else if (interpolation == 1)
    {
        NN = 1;
        MaxFilt_N = 20;
    }
    else
    {
        NN = 3;
        MaxFilt_N = 40;
    }

    if (Filt_N > MaxFilt_N)
        Filt_N = MaxFilt_N;

    for (i = 0; i < Filt_N; i++)
        w[i] = (uint16_t)(32768.0 * 0.25 * (1.0 - cos(2.0 * M_PI * i / (Filt_N - 1))));

    if (chAA == 1)
        reg = controlsPtr2Registers[chkSLEEP_CFR];
    else
        reg = controlsPtr2Registers[chkSLEEP_CFR_chB];

    if (chAA == 1)
        reg2 = controlsPtr2Registers[chkODD_CFR];
    else
        reg2 = controlsPtr2Registers[chkODD_CFR_chB];

    if (chAA == 1)
        reg4 = controlsPtr2Registers[chkDEL_HB];
    else
        reg4 = controlsPtr2Registers[chkDEL_HB_chB];

    reg3 = controlsPtr2Registers[chkResetN];

    SetRegValue(reg, 1); 

    msb = lsb = 0;
    data = 0;
    i = 0;

    SetRegValue(reg3, 0); 
    SetRegValue(reg3, 1);

    while (i < MaxFilt_N) //40
    {                    
        addr = (2 << 15) + (maddressf0 << 6) + (msb << 2) + lsb;
        SPI_write(lmsControl, addr, data);
        addr = (2 << 15) + (maddressf1 << 6) + (msb << 2) + lsb;
        SPI_write(lmsControl, addr, data);
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;
        i++;
    }

    msb = lsb = 0;
    i = j = 0;
    offset = 0;
    while (i <= (uint16_t)((Filt_N / 2) - 1))
    {
        addr = (2 << 15) + (maddressf1 << 6) + (msb << 2) + lsb;
        if (j >= offset)
            data = w[(uint16_t)((Filt_N + 1) / 2 + i)];
        else
            data = 0;
        SPI_write(lmsControl, addr, data);
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;

        if (j >= offset)
            i++;
        j++;
    }

    msb = lsb = 0;
    i = j = 0;
    offset = (MaxFilt_N / 2) - ((uint16_t)((Filt_N + 1) / 2));
    while (i < Filt_N)
    {
        addr = (2 << 15) + (maddressf0 << 6) + (msb << 2) + lsb;

        if (j >= offset)
            data = w[i];
        else
            data = 0;

        SPI_write(lmsControl, addr, data);
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;

        if (j >= offset)
            i++;
        j++;
    }
    if ((Filt_N % 2) == 1)
        SetRegValue(reg2, 1); // odd
    else
        SetRegValue(reg2, 0); // even

    if (interpolation == 1)
        SetRegValue(reg4, 1);
    else
        SetRegValue(reg4, 0);

    SetRegValue(reg, 0); 
    // software reset
    SetRegValue(reg3, 0); 
    SetRegValue(reg3, 1); 
}

void pnlQSpark::Initialize(lms_device_t *pControl)
{
    lmsControl = pControl;
    LMS_WriteFPGAReg(lmsControl, 0xFFFF, rbChannelB->GetValue() ? 0x2 : 0x1);
    double freqHz;
    LMS_GetSampleRate(lmsControl, LMS_CH_RX, 4, &freqHz, nullptr);
    txtPllFreqRxMHz->SetValue(wxString::Format("%1.3f", freqHz / 1e6));
    LMS_GetSampleRate(lmsControl, LMS_CH_TX, 4, &freqHz, nullptr);
    txtPllFreqTxMHz->SetValue(wxString::Format("%1.3f", freqHz / 1e6));
    chA = 1;
    wxCommandEvent evt;
    OnSwitchToChannelA(evt);
}

void pnlQSpark::RegisterParameterChangeHandler(wxCommandEvent &event)
{
    if (controlsPtr2Registers.find(event.GetEventObject()) == controlsPtr2Registers.end())
        return; //control not found in the table

    Register reg = controlsPtr2Registers[event.GetEventObject()];
    int mac = (reg.address != 0x17) && (rbChannelB->GetValue()) ? 0x2 : 0x1;
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, mac) != 0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl, reg.address, &regValue);

    regValue &= ~mask;
    int evtVal = event.GetInt();
    if (event.GetEventObject() == chkLB_1A || event.GetEventObject() == chkLB_1B || event.GetEventObject() == chkLB_2A || event.GetEventObject() == chkLB_2B)
        evtVal++;

    if ((event.GetEventObject() == cmbPAsrc) && (evtVal == 0))
        evtVal = 5;
    regValue |= (evtVal << reg.lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0)
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);

    //UpdateDegrees();
}

pnlQSpark::~pnlQSpark()
{
    mPanelStreamPLL->Disconnect(wxEVT_BUTTON, btnConfigurePLL->GetId(), wxCommandEventHandler(pnlQSpark::OnConfigurePLL), 0, this);
}

void pnlQSpark::OnbtnUpdateAll(wxCommandEvent &event)
{
    double tempd = 0.0;
    map<wxObject *, Register>::iterator iter;
    wxClassInfo *spinctr = wxClassInfo::FindClass("wxSpinCtrl");
    wxClassInfo *checkboxctr = wxClassInfo::FindClass("wxCheckBox");
    wxClassInfo *choicectr = wxClassInfo::FindClass("wxChoice");

    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, rbChannelB->GetValue() ? 0x2 : 0x1) != 0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    for (iter = controlsPtr2Registers.begin(); iter != controlsPtr2Registers.end(); ++iter)
    {
        Register reg = iter->second;
        unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask
        uint16_t value;

        LMS_ReadFPGAReg(lmsControl, reg.address, &value);
        value = value & mask;
        value = value >> reg.lsb;

        if ((iter->first == cmbPAsrc) && (value == 5))
            value = 0; 
        if (iter->first == chkLB_1A || iter->first == chkLB_1B || iter->first == chkLB_2A || iter->first == chkLB_2B)
            value = value == 2 ? 1 : 0;

        unsigned short mask2 = (~(~0 << (reg.msb - reg.lsb + 1)));
        short value2 = 0;
        if ((reg.twocomplement == 1) && (value > (mask2 >> 1)))
            value2 = value - (mask2 + 1);
        else
            value2 = value;

        if (iter->first->IsKindOf(spinctr))
            reinterpret_cast<wxSpinCtrl *>(iter->first)->SetValue(value2);
        else if (iter->first->IsKindOf(checkboxctr))
            reinterpret_cast<wxCheckBox *>(iter->first)->SetValue(value);
        else if (iter->first->IsKindOf(choicectr))
            reinterpret_cast<wxComboBox *>(iter->first)->SetSelection(value2); 
        else if ((iter->first == thresholdSpin) || (iter->first == thresholdSpin_chB))
        {
            tempd = value / 65535.0;
            if (tempd > 1.0)
                tempd = 1.0;
            else if (tempd < 0.0)
                tempd = 0.0;
            reinterpret_cast<wxSpinCtrlDouble *>(iter->first)->SetValue(tempd); 
        }
        else if ((iter->first == thresholdGain) || (iter->first == thresholdGain_chB))
        {
            tempd = value / (65535.0 / 8.0);
            if (tempd > 2.0)
                tempd = 2.0;
            else if (tempd < 0.0)
                tempd = 0.0;
            reinterpret_cast<wxSpinCtrlDouble *>(iter->first)->SetValue(tempd);
        }
    }

    uint16_t regValue = 0;
    Register reg1, reg2;

    reg1 = Register(0x004B, 15, 0, 0, 0); // dummy, txtPllFreqTxMHz
    reg2 = Register(0x004C, 15, 0, 0, 0); // dummy, txtPllFreqRxMHz

    LMS_ReadFPGAReg(lmsControl, reg1.address, &regValue); // read bit content
    txtPllFreqTxMHz->SetValue(wxString::Format("%1.3f", double(regValue) / 100.0));

    LMS_ReadFPGAReg(lmsControl, reg2.address, &regValue); // read bit content
    txtPllFreqRxMHz->SetValue(wxString::Format("%1.3f", double(regValue) / 100.0));
}

void pnlQSpark::OnConfigurePLL(wxCommandEvent &event)
{

    auto conn = ((LMS7_Device *)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        wxMessageBox(_("device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    double FreqTxMHz, FreqRxMHz;
    txtPllFreqTxMHz->GetValue().ToDouble(&FreqTxMHz);
    txtPllFreqRxMHz->GetValue().ToDouble(&FreqRxMHz);

    Register reg1, reg2;
    reg1 = Register(0x004B, 15, 0, 0, 0); //dummy, txtPllFreqTxMHz
    reg2 = Register(0x004C, 15, 0, 0, 0); //dummy, txtPllFreqRxMHz

    // save to FPGA
    if ((FreqTxMHz > 5.0) && (FreqTxMHz < 200.0))
        SetRegValue(reg1, (uint16_t)(FreqTxMHz * 100.0));
    if ((FreqRxMHz > 5.0) && (FreqRxMHz < 200.0))
        SetRegValue(reg2, (uint16_t)(FreqRxMHz * 100.0));
}

void pnlQSpark::OnReadAll(wxCommandEvent &event)
{
    OnbtnUpdateAll(event);
}

void pnlQSpark::OnWriteAll(wxCommandEvent &event)
{
    OnConfigurePLL(event);
}

void pnlQSpark::OnSwitchToChannelA(wxCommandEvent &event)
{
    chA = 1;
    chkBYPASS_CFR->Enable(true);
    chkBYPASSGAIN_CFR->Enable(true);
    chkSLEEP_FIR->Enable(true);
    chkBYPASS_FIR->Enable(true);
    chkODD_FIR->Enable(true);
    thresholdSpin->Enable(true);
    thresholdGain->Enable(true);
    spinCFR_ORDER->Enable(true);
    setFIR1->Enable(true);
    cmbINTER_CFR->Enable(true);
    chkBYPASS_CFR_chB->Enable(false);
    chkBYPASSGAIN_CFR_chB->Enable(false);
    chkSLEEP_FIR_chB->Enable(false);
    chkBYPASS_FIR_chB->Enable(false);
    chkODD_FIR_chB->Enable(false);
    thresholdSpin_chB->Enable(false);
    thresholdGain_chB->Enable(false);
    spinCFR_ORDER_chB->Enable(false);
    setFIR2->Enable(false);
    cmbINTER_CFR_chB->Enable(false);
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, 0x1) != 0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    wxCommandEvent evt;
    OnbtnUpdateAll(evt);
}

void pnlQSpark::OnSwitchToChannelB(wxCommandEvent &event)
{
    chA = 0;
    chkBYPASS_CFR->Enable(false);
    chkBYPASSGAIN_CFR->Enable(false);
    chkSLEEP_FIR->Enable(false);
    chkBYPASS_FIR->Enable(false);
    chkODD_FIR->Enable(false);
    thresholdSpin->Enable(false);
    thresholdGain->Enable(false);
    spinCFR_ORDER->Enable(false);
    setFIR1->Enable(false);
    cmbINTER_CFR->Enable(false);
    chkBYPASS_CFR_chB->Enable(true);
    chkBYPASSGAIN_CFR_chB->Enable(true);
    chkSLEEP_FIR_chB->Enable(true);
    chkBYPASS_FIR_chB->Enable(true);
    chkODD_FIR_chB->Enable(true);
    thresholdSpin_chB->Enable(true);
    thresholdGain_chB->Enable(true);
    spinCFR_ORDER_chB->Enable(true);
    setFIR2->Enable(true);
    cmbINTER_CFR_chB->Enable(true);
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, 0x2) != 0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    wxCommandEvent evt;
    OnbtnUpdateAll(evt);
}

void pnlQSpark::LoadQSparkSettings(wxCommandEvent &event)
{

    auto conn = ((LMS7_Device *)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        wxMessageBox(_("device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    wxFileDialog dlg(this, _("Open LimeSDR_QPCIe config file"), "", "", "Project-File (*.ini2)|*.ini2", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    wxString m_sConfigFilename = dlg.GetPath();
    const char *config_filename2 = m_sConfigFilename.mb_str();

    map<wxObject *, Register>::iterator iter;
    iniParser m_options;
    m_options.Open(config_filename2);
    m_options.SelectSection("LimeSDR_QPCIe");

    for (int kk = 1; kk <= 2; kk++)
    {

        if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, kk) != 0)
        {
            wxMessageBox(LMS_GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
            return;
        }

        for (iter = controlsPtr2Registers.begin(); iter != controlsPtr2Registers.end(); ++iter)
        {
            Register reg = iter->second;
            uint16_t regValue;

            regValue = m_options.Get(wxString::Format("Reg_%01X:%04X", kk, reg.address), 0); // read data from file
            if (LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0)
                wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));
           
        }
         
// load FIR filter coeffs
        const int maxCoefCount = 40;
        uint16_t maddressf0 = 0x240;
        uint16_t maddressf1 = 0x280;
        for (int i = 0; i < maxCoefCount; i++)
        {
                int16_t regValue = 0;
                regValue = m_options.Get(wxString::Format("Reg_%01X:%04X", kk, maddressf0 + i), 0); // read data from file
                if (LMS_WriteFPGAReg(lmsControl, maddressf0 + i, regValue) != 0)
                    wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));
                if (LMS_WriteFPGAReg(lmsControl, maddressf1 + i, regValue) != 0)
                    wxMessageBox(LMS_GetLastErrorMessage(), wxString::Format("%s", LMS_GetLastErrorMessage()));
        }

    }

    wxString temps;
    double FreqTxMHz, FreqRxMHz;
    Register reg1, reg2;
    reg1 = Register(0x004B, 15, 0, 0, 0); //dummy, txtPllFreqTxMHz
    reg2 = Register(0x004C, 15, 0, 0, 0); //dummy, txtPllFreqRxMHz

    temps = m_options.Get("FreqTxMHz", "30.72");
    txtPllFreqTxMHz->SetValue(temps);
    temps.ToDouble(&FreqTxMHz);

    temps = m_options.Get("FreqRxMHz", "30.72");
    txtPllFreqRxMHz->SetValue(temps);
    temps.ToDouble(&FreqRxMHz);

    // save to FPGA
    if ((FreqTxMHz > 5.0) && (FreqTxMHz < 200.0))
        SetRegValue(reg1, (uint16_t)(FreqTxMHz * 100.0));
    if ((FreqRxMHz > 5.0) && (FreqRxMHz < 200.0))
        SetRegValue(reg2, (uint16_t)(FreqRxMHz * 100.0));

    wxCommandEvent evt;
    OnbtnUpdateAll(evt);

    chA = 1;
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, 0x1) != 0)
    {
        wxMessageBox(LMS_GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    UpdateHannCoeff(lmsControl, spinCFR_ORDER->GetValue(), chA, cmbINTER_CFR->GetSelection());

    chA = 0;
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, 0x2) != 0)
    {
        wxMessageBox(LMS_GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    UpdateHannCoeff(lmsControl, spinCFR_ORDER_chB->GetValue(), chA, cmbINTER_CFR_chB->GetSelection());

    chA = 1;
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, 0x1) != 0)
    {
        wxMessageBox(LMS_GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
}

void pnlQSpark::SaveQSparkSettings(wxCommandEvent &event)
{

    int CFR_N_chA = 19;
    int CFR_N_chB = 19;
    int CFR_INT_chA = 1;
    int CFR_INT_chB = 1;

    /*
    if (!LMS_IsOpen(lmsControl, 1))
    {
        wxMessageBox(_("LimeSDR_QPCIe not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    */

    auto conn = ((LMS7_Device *)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        wxMessageBox(_("device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    wxFileDialog dlg(this, _("Save LimeSDR_QPCIe config file"), "", "", "Project-File (*.ini2)|*.ini2", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    wxString m_sConfigFilename = dlg.GetPath();
    const char *config_filename2 = m_sConfigFilename.mb_str();

    iniParser m_options;
    m_options.Open(config_filename2);
    m_options.SelectSection("LimeSDR_QPCIe");

    map<wxObject *, Register>::iterator iter;

    for (int kk = 1; kk <= 2; kk++)
    {

        if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, kk) != 0)
        {
            wxMessageBox(LMS_GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
            return;
        }

        uint16_t value = 0;
        for (iter = controlsPtr2Registers.begin(); iter != controlsPtr2Registers.end(); ++iter)
        {
            Register reg = iter->second;
            //unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask
            LMS_ReadFPGAReg(lmsControl, reg.address, &value);
            m_options.Set(wxString::Format("Reg_%01X:%04X", kk, reg.address), value);
        }

        // FIR filter coeffs
        const int maxCoefCount = 40;
        uint16_t maddressf0 = 0x240;
        for (int i = 0; i < maxCoefCount; i++)
        {
            int16_t regValue = 0;
            LMS_ReadFPGAReg(lmsControl, maddressf0 + i, (uint16_t *)&regValue);
            m_options.Set(wxString::Format("Reg_%01X:%04X", kk, maddressf0 + i), regValue);
        }
    }

    double FreqTxMHz, FreqRxMHz;
    wxString temps;

    txtPllFreqTxMHz->GetValue().ToDouble(&FreqTxMHz);
    temps.Printf(_T("%7.2f"), FreqTxMHz);
    m_options.Set(wxString::Format("FreqTxMHz"), temps.ToAscii());

    txtPllFreqRxMHz->GetValue().ToDouble(&FreqRxMHz);
    temps.Printf(_T("%7.2f"), FreqRxMHz);
    m_options.Set(wxString::Format("FreqRxMHz"), temps.ToAscii());

    CFR_N_chA = spinCFR_ORDER->GetValue();
    temps.Printf(_T("%3d"), CFR_N_chA);
    m_options.Set(wxString::Format("CFR_N_chA"), temps.ToAscii());

    CFR_INT_chA = cmbINTER_CFR->GetSelection();
    temps.Printf(_T("%3d"), CFR_INT_chA);
    m_options.Set(wxString::Format("CFR_INT_chA"), temps.ToAscii());

    CFR_N_chB = spinCFR_ORDER_chB->GetValue();
    temps.Printf(_T("%3d"), CFR_N_chB);
    m_options.Set(wxString::Format("CFR_N_chB"), temps.ToAscii());

    CFR_INT_chB = cmbINTER_CFR_chB->GetSelection();
    temps.Printf(_T("%3d"), CFR_INT_chB);
    m_options.Set(wxString::Format("CFR_INT_chB"), temps.ToAscii());

    m_options.Save(config_filename2);
}

void pnlQSpark::onbtnFIRCoefA(wxCommandEvent &event)
{
    onbtnFIRCoef(1, cmbINTER_CFR->GetSelection());
}

void pnlQSpark::onbtnFIRCoefB(wxCommandEvent &event)
{
    onbtnFIRCoef(2, cmbINTER_CFR_chB->GetSelection());
}

void pnlQSpark::onbtnFIRCoef(int ch, int interpolation)
{
    Register reg, reg2;
    uint16_t chVal = 0;
    LMS_ReadFPGAReg(lmsControl, 0xFFFF, &chVal);
    LMS_WriteFPGAReg(lmsControl, 0xFFFF, ch);
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);

    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    uint16_t maddressf0 = 0x240;
    uint16_t maddressf1 = 0x280;
    int16_t regValue = 0;
    uint16_t Filt_N = 0;
    uint16_t NN = 0;
    uint16_t addr = 0;
    uint16_t data = 0;
    uint16_t msb = 0;
    uint16_t lsb = 0;
    uint16_t i = 0;

    if (interpolation == 1)
    {
        Filt_N = 20;
        NN = 1;
    }
    else
    {
        Filt_N = 40;
        NN = 3;
    }
    coefficients.resize(Filt_N, 0);

    if (ch == 1)
        reg = controlsPtr2Registers[chkSLEEP_FIR];
    else
        reg = controlsPtr2Registers[chkSLEEP_FIR_chB];

    if (ch == 1)
        reg2 = controlsPtr2Registers[chkODD_FIR];
    else
        reg2 = controlsPtr2Registers[chkODD_FIR_chB];

    for (i = 0; i < Filt_N; i++) //maxCoefCount
    {
        coefficients[i] = 0.0;
    }
    // read coeffs
    msb = lsb = i = 0;
    while (i <= (uint16_t)((Filt_N)-1))
    {
        addr = maddressf0 + (msb << 2) + lsb;
        LMS_ReadFPGAReg(lmsControl, addr, (uint16_t *)&regValue);
        coefficients[i] = (double)regValue;
        if (lsb >= NN) // 3
        {
            lsb = 0;
            msb++;
        }
        else
            lsb++;
        i++;
    }
    dlg->SetCoefficients(coefficients);

    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();

        SetRegValue(reg, 1); // sleep <= '1';
       
        for (i = 0; i < maxCoefCount; i++)
        {
            addr = maddressf0 + i;
            LMS_WriteFPGAReg(lmsControl, addr, 0);
            addr = maddressf1 + i;
            LMS_WriteFPGAReg(lmsControl, addr, 0);
        }
       
        msb = lsb = i = 0;
        while (i <= (uint16_t)((Filt_N)-1))
        {
            addr = maddressf0 + (msb << 2) + lsb;
            data = (uint16_t)coefficients[i];
            SPI_write(lmsControl, addr, data);
            addr = maddressf1 + (msb << 2) + lsb;
            data = (uint16_t)coefficients[i];
            SPI_write(lmsControl, addr, data);
            if (lsb >= NN) // 3
            {
                lsb = 0;
                msb++;
            }
            else
                lsb++;
            i++;
        }

        if ((Filt_N % 2) == 1)
            SetRegValue(reg2, 1); // odd
        else
            SetRegValue(reg2, 0); // even

        SetRegValue(reg, 0); // sleep <= '0';
    }
    dlg->Destroy();
    LMS_WriteFPGAReg(lmsControl, 0xFFFF, chVal);
}
