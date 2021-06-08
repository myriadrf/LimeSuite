#include "pnlQSpark.h"
#include "LMS64CProtocol.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>

#include <ciso646>
#include <map>

#include <IConnection.h>
#include <ErrorReporting.h>
#include <FPGA_common.h>

//#include "lms7suiteAppFrame.h"

using namespace lime;
using namespace std;

BEGIN_EVENT_TABLE(pnlQSpark, wxFrame)
END_EVENT_TABLE()

pnlQSpark::Register::Register()
: address(0), msb(0), lsb(0), defaultValue(0), twocomplement(0)
{
}


pnlQSpark::Register::Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue, unsigned short twocomplement)
	: address(address), msb(msb), lsb(lsb), defaultValue(defaultValue), twocomplement(twocomplement)
{
}

pnlQSpark::pnlQSpark(LMS7SuiteAppFrame * parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, int style, wxString name)
{
    m_serPort = nullptr;

	parent =  parent; //(LMS7SuiteAppFrame *)

    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, title, wxDefaultPosition, wxDefaultSize, style, _T("id"));
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 5);

    SetSizer(mainSizer);

    wxSize freqTextfieldSize(64, -1);
    mPanelStreamPLL = new wxPanel(this, wxNewId());
    wxFlexGridSizer* sizerPllControls = new wxFlexGridSizer(0, 3, 5, 5);
    sizerPllControls->Add(new wxStaticText(mPanelStreamPLL, wxID_ANY, _("DAC (MHz):")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    txtPllFreqTxMHz = new wxTextCtrl(mPanelStreamPLL, wxNewId(), _("160"), wxDefaultPosition, freqTextfieldSize);
    sizerPllControls->Add(txtPllFreqTxMHz, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblRealFreqTx = new wxStaticText(mPanelStreamPLL, wxID_ANY, _("Real: ? MHz"));
    sizerPllControls->Add(lblRealFreqTx, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    sizerPllControls->Add(new wxStaticText(mPanelStreamPLL, wxID_ANY, _("ADC (MHz):")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    txtPllFreqRxMHz = new wxTextCtrl(mPanelStreamPLL, wxNewId(), _("160"), wxDefaultPosition, freqTextfieldSize);
    sizerPllControls->Add(txtPllFreqRxMHz, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblRealFreqRx = new wxStaticText(mPanelStreamPLL, wxID_ANY, _("Real: ? MHz"));
    sizerPllControls->Add(lblRealFreqRx, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    btnConfigurePLL = new wxButton(mPanelStreamPLL, wxNewId(), _("Configure"));
    sizerPllControls->Add(btnConfigurePLL, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Connect(btnConfigurePLL->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(pnlQSpark::OnConfigurePLL), NULL, this);

    wxStaticBoxSizer* streamPllGroup = new wxStaticBoxSizer(wxHORIZONTAL, mPanelStreamPLL, _T("Digital Interface Clock"));
    mPanelStreamPLL->SetSizer(streamPllGroup);
    streamPllGroup->Add(sizerPllControls, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    streamPllGroup->Fit(mPanelStreamPLL);
    FlexGridSizer1->Add(mPanelStreamPLL, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxFlexGridSizer* moduleEnables = new wxFlexGridSizer(0, 2, 0, 0);
    chkEN_RXTSP = new wxCheckBox(this, wxNewId(), _("Enable RxTSP"));
    moduleEnables->Add(chkEN_RXTSP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkEN_RXTSP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    chkEN_TXTSP = new wxCheckBox(this, wxNewId(), _("Enable TxTSP"));
    moduleEnables->Add(chkEN_TXTSP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkEN_TXTSP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    FlexGridSizer1->Add(moduleEnables, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);

    wxStaticBoxSizer* bypassesGroup = new wxStaticBoxSizer(wxVERTICAL, this, _T("Bypass"));
    wxFlexGridSizer* bypasses = new wxFlexGridSizer(0, 2, 0, 0);
    chkRX_GCORR_BYP = new wxCheckBox(this, wxNewId(), _("Rx Gain corrector"));
    bypasses->Add(chkRX_GCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkRX_GCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkTX_GCORR_BYP = new wxCheckBox(this, wxNewId(), _("Tx Gain corrector"));
    bypasses->Add(chkTX_GCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkTX_GCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkRX_PHCORR_BYP = new wxCheckBox(this, wxNewId(), _("Rx Phase corrector"));
    bypasses->Add(chkRX_PHCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkRX_PHCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkTX_PHCORR_BYP = new wxCheckBox(this, wxNewId(), _("Tx Phase corrector"));
    bypasses->Add(chkTX_PHCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkTX_PHCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkRX_DCCORR_BYP = new wxCheckBox(this, wxNewId(), _("Rx DC corrector"));
    bypasses->Add(chkRX_DCCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkRX_DCCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkTX_DCCORR_BYP = new wxCheckBox(this, wxNewId(), _("Tx DC corrector"));
    bypasses->Add(chkTX_DCCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkTX_DCCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    bypassesGroup->Add(bypasses , 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer1->Add(bypassesGroup, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxFlexGridSizer* tx_correctorsSizer = new wxFlexGridSizer(0, 2, 0, 5);
    wxSize spinBoxSize(64, -1);
    long spinBoxStyle = wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER;
    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx GCORRQ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_GCORRQ = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    tx_correctorsSizer->Add(spinTX_GCORRQ, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_GCORRQ->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRQ->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRQ->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx GCORRI")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_GCORRI = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    tx_correctorsSizer->Add(spinTX_GCORRI, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_GCORRI->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRI->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRI->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx PHCORR")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_PHCORR = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -2048, 2047, 0);
    tx_correctorsSizer->Add(spinTX_PHCORR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_PHCORR->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_PHCORR->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_PHCORR->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx DCCORRI")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_DCCORRI = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -128, 127, 0);
    tx_correctorsSizer->Add(spinTX_DCCORRI, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_DCCORRI->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRI->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRI->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx DCCORRQ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_DCCORRQ = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -128, 127, 0);
    tx_correctorsSizer->Add(spinTX_DCCORRQ, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_DCCORRQ->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRQ->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRQ->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);


    wxFlexGridSizer* rx_correctorsSizer = new wxFlexGridSizer(0, 2, 0, 5);
    rx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Rx GCORRQ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinRX_GCORRQ = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    rx_correctorsSizer->Add(spinRX_GCORRQ, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinRX_GCORRQ->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRQ->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRQ->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    rx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Rx GCORRI")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinRX_GCORRI = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    rx_correctorsSizer->Add(spinRX_GCORRI, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinRX_GCORRI->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRI->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRI->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    rx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Rx PHCORR")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinRX_PHCORR = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -2048, 2047, 0);
    rx_correctorsSizer->Add(spinRX_PHCORR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinRX_PHCORR->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_PHCORR->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_PHCORR->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    wxFlexGridSizer* correctorsSizer = new wxFlexGridSizer(0, 2, 0, 5);
    correctorsSizer->Add(rx_correctorsSizer, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    correctorsSizer->Add(tx_correctorsSizer, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxStaticBoxSizer* correctorsGroup = new wxStaticBoxSizer(wxVERTICAL, this, _T("Correctors"));
    correctorsGroup->Add(correctorsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer1->Add(correctorsGroup, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    txtNcoFreq = new wxTextCtrl(this, wxNewId(), _("1.0"), wxDefaultPosition, freqTextfieldSize);
    wxArrayString insel_choices;
    insel_choices.push_back(_("Waveform"));
    insel_choices.push_back(_("FPGA NCO"));
    cmbInsel = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, insel_choices, 1);
    cmbInsel->SetSelection(0);
    Connect(cmbInsel->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    wxStaticBoxSizer* loopbackGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Loopback controls"));
    wxFlexGridSizer* loopbackSizer = new wxFlexGridSizer(0, 2, 0, 0);
    loopbackSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx chain input source:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    loopbackSizer->Add(cmbInsel, 1, wxALIGN_LEFT | wxEXPAND, 5);
    loopbackSizer->Add(new wxStaticText(this, wxID_ANY, _("NCO (MHz):")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    loopbackSizer->Add(txtNcoFreq, 1, wxALIGN_LEFT | wxEXPAND, 5);
    Connect(txtNcoFreq->GetId(), wxEVT_TEXT, wxCommandEventHandler(pnlQSpark::OnNcoFrequencyChanged), NULL, this);
    Connect(txtNcoFreq->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::OnNcoFrequencyChanged), NULL, this);

    loopbackGroup->Add(loopbackSizer, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);
    FlexGridSizer1->Add(loopbackGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);

	wxArrayString pasw_choices1;
	pasw_choices1.push_back(_("Off"));
	pasw_choices1.push_back(_("PA1 in1"));
	pasw_choices1.push_back(_("PA1 in2"));
	
	wxArrayString pasw_choices2;
	pasw_choices2.push_back(_("Off"));
	pasw_choices2.push_back(_("PA2 in1"));
	pasw_choices2.push_back(_("PA2 in2"));

	cmbPa1Sw = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, pasw_choices1, 1);
	cmbPa1Sw ->SetSelection(0);
	Connect(cmbPa1Sw->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

	cmbPa2Sw = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, pasw_choices2, 1);
	cmbPa2Sw->SetSelection(0);
	Connect(cmbPa2Sw->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
	
	wxArrayString pamo_choices;
	pamo_choices.push_back(_("Off"));
	pamo_choices.push_back(_("On"));
	//pamo_choices.push_back(_("Low Pwr"));
	//pamo_choices.push_back(_("Med. Pwr"));
	//pamo_choices.push_back(_("High Pwr"));

	cmbPa1Mo = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, pamo_choices, 1);
	cmbPa1Mo->SetSelection(0);
	Connect(cmbPa1Mo->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

	cmbPa2Mo = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, pamo_choices, 1);
	cmbPa2Mo->SetSelection(0);
	Connect(cmbPa2Mo->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);



	wxArrayString rf_choices;
	rf_choices.push_back(_("All off"));
	rf_choices.push_back(_("EXT CPL in1"));
	rf_choices.push_back(_("PA1 CPL out"));
	rf_choices.push_back(_("EXT CPL in2"));
	rf_choices.push_back(_("PA2 CPL out"));

	cmbRf = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, rf_choices, 1);
	cmbRf->SetSelection(0);
	Connect(cmbRf->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);


	wxStaticBoxSizer* paGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Power amplifier 1 and 2 controls"));
	wxFlexGridSizer* paSizer = new wxFlexGridSizer(0, 7, 0, 0);
	
	txtVccPa1 = new wxTextCtrl(this, wxNewId(), _("4.2"), wxDefaultPosition, freqTextfieldSize);
	
	paSizer->Add(new wxStaticText(this, wxID_ANY, _("PA1 input:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
	paSizer->Add(cmbPa1Sw, 1, wxALIGN_LEFT | wxEXPAND, 5);
	paSizer->Add(new wxStaticText(this, wxID_ANY, _(" switch:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
	paSizer->Add(cmbPa1Mo, 1, wxALIGN_LEFT | wxEXPAND, 5);
	paSizer->Add(new wxStaticText(this, wxID_ANY, _("voltage[V]:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);	
	paSizer->Add(txtVccPa1, 1, wxALIGN_LEFT | wxEXPAND, 5);
	lblPa1 = new wxStaticText(this, wxID_ANY, _(" Value:0x00"));
	paSizer->Add(lblPa1, 1, wxALIGN_LEFT | wxEXPAND, 5);

	Connect(txtVccPa1->GetId(), wxEVT_TEXT, wxCommandEventHandler(pnlQSpark::OnVccPa1Changed), NULL, this);
	Connect(txtVccPa1->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::OnVccPa1Changed), NULL, this);


	txtVccPa2 = new wxTextCtrl(this, wxNewId(), _("4.2"), wxDefaultPosition, freqTextfieldSize);
	paSizer->Add(new wxStaticText(this, wxID_ANY, _("PA2 input:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
	paSizer->Add(cmbPa2Sw, 1, wxALIGN_LEFT | wxEXPAND, 5);
	paSizer->Add(new wxStaticText(this, wxID_ANY, _(" switch:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
	paSizer->Add(cmbPa2Mo, 1, wxALIGN_LEFT | wxEXPAND, 5);
	paSizer->Add(new wxStaticText(this, wxID_ANY, _("voltage[V]:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
	paSizer->Add(txtVccPa2, 1, wxALIGN_LEFT | wxEXPAND, 5);
	lblPa2 = new wxStaticText(this, wxID_ANY, _(" Value:0x00"));
	paSizer->Add(lblPa2, 1, wxALIGN_LEFT | wxEXPAND, 5);

	Connect(txtVccPa2->GetId(), wxEVT_TEXT, wxCommandEventHandler(pnlQSpark::OnVccPa2Changed), NULL, this);
	Connect(txtVccPa2->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::OnVccPa2Changed), NULL, this);

	paSizer->Add(new wxStaticText(this, wxID_ANY, _("RXRF1_W input:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
	paSizer->Add(cmbRf, 1, wxALIGN_LEFT | wxEXPAND, 5);
	paGroup->Add(paSizer, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);
	FlexGridSizer1->Add(paGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);


    btnUpdateAll = new wxButton(this, wxNewId(), _T("Refresh All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Connect(btnUpdateAll->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&pnlQSpark::OnbtnUpdateAll);
    FlexGridSizer1->Add(btnUpdateAll, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(FlexGridSizer1, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();

    controlsPtr2Registers.clear();
    controlsPtr2Registers[chkEN_RXTSP] = Register(0x0080, 0, 0, 0, 0);
    controlsPtr2Registers[chkEN_TXTSP] = Register(0x0060, 0, 0, 0, 0);

    controlsPtr2Registers[chkRX_DCCORR_BYP] = Register(0x008C, 2, 2, 0, 0);
    controlsPtr2Registers[chkRX_PHCORR_BYP] = Register(0x008C, 0, 0, 0, 0);
    controlsPtr2Registers[chkRX_GCORR_BYP] = Register(0x008C, 1, 1, 0, 0);
    controlsPtr2Registers[chkTX_PHCORR_BYP] = Register(0x0068, 0, 0, 0, 0);
    controlsPtr2Registers[chkTX_GCORR_BYP] = Register(0x0068, 1, 1, 0, 0);
    controlsPtr2Registers[chkTX_DCCORR_BYP] = Register(0x0068, 3, 3, 0, 0);

    controlsPtr2Registers[spinTX_GCORRQ] = Register(0x0061, 10, 0, 2047, 0);
    controlsPtr2Registers[spinTX_GCORRI] = Register(0x0062, 10, 0, 2047, 0);
    controlsPtr2Registers[spinTX_PHCORR] = Register(0x0063, 11, 0, 0, 1);  // ovde
    controlsPtr2Registers[spinTX_DCCORRI] = Register(0x0064, 15, 8, 0, 1); // mozda
    controlsPtr2Registers[spinTX_DCCORRQ] = Register(0x0064, 7, 0, 0, 1);  // mozda

    controlsPtr2Registers[spinRX_GCORRQ] = Register(0x0081, 10, 0, 2047, 0);
    controlsPtr2Registers[spinRX_GCORRI] = Register(0x0082, 10, 0, 2047, 0);
    controlsPtr2Registers[spinRX_PHCORR] = Register(0x0083, 11, 0, 0, 1);  //ovde

    controlsPtr2Registers[cmbInsel] = Register(0x0060, 2, 2, 0, 0);

	//
	//rfsw(2:0)&"000"&pa2mode(2:0)&pa2sw(1:0)&pa1mode(2:0)&pa1sw(1:0)
	controlsPtr2Registers[cmbPa1Sw] = Register(0x0051, 1, 0, 0, 0);
	controlsPtr2Registers[cmbPa1Mo] = Register(0x0051, 4, 2, 0, 0);
	controlsPtr2Registers[cmbPa2Sw] = Register(0x0051, 6, 5, 0, 0);
	controlsPtr2Registers[cmbPa2Mo] = Register(0x0051, 9, 7, 0, 0);
	controlsPtr2Registers[cmbRf] = Register(0x0051, 15, 13, 0, 0);
}

void pnlQSpark::Initialize(IConnection* pControl)
{
    m_serPort = pControl;
}

void pnlQSpark::UpdatePanel()
{
    wxCommandEvent evt;
    OnbtnUpdateAll(evt);
}

void pnlQSpark::RegisterParameterChangeHandler(wxCommandEvent& event)
{
    if (controlsPtr2Registers.find(event.GetEventObject()) == controlsPtr2Registers.end())
        return; //control not found in the table
	/*
	switch (event.GetInt()) {
	case 0: wxMessageBox(GetLastErrorMessage(), "Event0"); break;
	case 1: wxMessageBox(GetLastErrorMessage(), "Event1"); break;
	case 2: wxMessageBox(GetLastErrorMessage(), "Event2"); break;
	case 3: wxMessageBox(GetLastErrorMessage(), "Event3"); break;
	case 4: wxMessageBox(GetLastErrorMessage(), "Event4"); break;
	}
	wxMessageBox(GetLastErrorMessage(), "Event");
	*/

    if (not m_serPort || not m_serPort->IsOpen())
    {
        wxMessageBox(_("device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    Register reg = controlsPtr2Registers[event.GetEventObject()];
    unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    unsigned short regValue;
    m_serPort->ReadRegister(reg.address, regValue);

    regValue &= ~mask;
    regValue |= (event.GetInt() << reg.lsb) & mask;

    if(m_serPort->WriteRegister(reg.address, regValue) != 0)
        wxMessageBox(GetLastErrorMessage(), "Error");
}

pnlQSpark::~pnlQSpark()
{
    mPanelStreamPLL->Disconnect(wxEVT_BUTTON, btnConfigurePLL->GetId(), wxCommandEventHandler(pnlQSpark::OnConfigurePLL), 0, this);
}

void pnlQSpark::OnbtnUpdateAll(wxCommandEvent& event)
{
    if(m_serPort == nullptr)
    {
        wxMessageBox(_("Update GUI: device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    map<wxObject*, Register>::iterator iter;
    wxClassInfo* spinctr = wxClassInfo::FindClass("wxSpinCtrl");
    wxClassInfo* checkboxctr = wxClassInfo::FindClass("wxCheckBox");
    wxClassInfo* choicectr = wxClassInfo::FindClass("wxChoice");
    for (iter = controlsPtr2Registers.begin(); iter != controlsPtr2Registers.end(); ++iter)
    {
        Register reg = iter->second;
        unsigned short mask = (~(~0 << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask
        unsigned short value;

        m_serPort->ReadRegister(reg.address, value);

        value = value & mask;
        value = value >> reg.lsb;

		unsigned short mask2 = (~(~0 << (reg.msb - reg.lsb + 1)));
		short value2 = 0;
		if ((reg.twocomplement == 1) && (value > (mask2 >> 1)))
			  value2 = value - (mask2 + 1);
		else  value2 = value;

        if (iter->first->IsKindOf(spinctr))
            reinterpret_cast<wxSpinCtrl*>(iter->first)->SetValue(value2);
        else if(iter->first->IsKindOf(checkboxctr))
            reinterpret_cast<wxCheckBox*>(iter->first)->SetValue(value);
        else if(iter->first->IsKindOf(choicectr))
            reinterpret_cast<wxComboBox*>(iter->first)->SetSelection(value2);
    }
}

void pnlQSpark::OnConfigurePLL(wxCommandEvent &event)
{
    double FreqTxMHz, FreqRxMHz;
    txtPllFreqTxMHz->GetValue().ToDouble(&FreqTxMHz);
    txtPllFreqRxMHz->GetValue().ToDouble(&FreqRxMHz);
    double phaseOffset = 180;

    lime::fpga::FPGA_PLL_clock clocks[2];
    //ADC
    clocks[0].bypass = false;
    clocks[0].index = 0;
    clocks[0].outFrequency = FreqRxMHz*1e6;
    clocks[0].phaseShift_deg = 0;

    //DAC
    clocks[1].bypass = false;
    clocks[1].index = 1;
    clocks[1].outFrequency = FreqTxMHz*1e6;
    clocks[1].phaseShift_deg = 0;

	//LMS7SuiteAppFrame * ptr = (LMS7SuiteAppFrame *) parent;

	if (lime::fpga::SetPllFrequency(m_serPort, 2, 30.72e6, clocks, 2) != 0) {
		wxMessageBox(GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
		//parent->SetDPDNyquist(FreqRxMHz*1e6 / 2);
	}
    else
    {
        OnNcoFrequencyChanged(event);
		
        lblRealFreqTx->SetLabel(wxString::Format("Real: %g MHz", clocks[1].rd_actualFrequency / 1e6));
        lblRealFreqRx->SetLabel(wxString::Format("Real: %g MHz", clocks[0].rd_actualFrequency / 1e6));

		//parent->SetDPDNyquist(FreqRxMHz*1e6 / 2.0);
    }
}

void pnlQSpark::OnNcoFrequencyChanged(wxCommandEvent& event)
{
    double refClk_MHz, ncoFreq_MHz;
    txtPllFreqTxMHz->GetValue().ToDouble(&refClk_MHz);
    txtNcoFreq->GetValue().ToDouble(&ncoFreq_MHz);
    uint32_t fcw = (uint32_t)((ncoFreq_MHz / refClk_MHz) * 4294967296);
    vector<uint32_t> addrs = { 0x006E, 0x006F };
    vector<uint32_t> values = { (fcw >> 16) & 0xFFFF, fcw & 0xFFFF };
    if(m_serPort && m_serPort->WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
        wxMessageBox(GetLastErrorMessage(), _("Error"), wxICON_ERROR | wxOK);
}

void pnlQSpark::OnVccPa1Changed(wxCommandEvent& event)
{
	double vccPa1_V;
	int val = 0;

	txtVccPa1->GetValue().ToDouble(&vccPa1_V);
	if (vccPa1_V <= 3.3){
		vccPa1_V = 3.3;
		txtVccPa1->SetValue(wxString::Format("%3.1f", 3.3));
	}

	if (vccPa1_V >= 4.5){
		vccPa1_V = 4.5;
		txtVccPa1->SetValue(wxString::Format("%3.1f", 4.5));
	}
	VoltageSetup(vccPa1_V, 0, &val);

	lblPa1->SetLabel(wxString::Format(" Value:0x%02X", val));

}



void pnlQSpark::OnVccPa2Changed(wxCommandEvent& event)
{

	double vccPa2_V;
	int val = 0;

	txtVccPa2->GetValue().ToDouble(&vccPa2_V);
	if (vccPa2_V <= 3.3){
		vccPa2_V = 3.3;
		txtVccPa2->SetValue(wxString::Format("%3.1f", 3.3));
	}

	if (vccPa2_V >= 4.5){
		vccPa2_V = 4.5;
		txtVccPa2->SetValue(wxString::Format("%3.1f", 4.5));
	}
	VoltageSetup(vccPa2_V, 1, &val);
	lblPa2->SetLabel(wxString::Format(" Value:0x%02X", val));
}

void pnlQSpark::VoltageSetup(double vccPa_V, unsigned char addr, int  * pValue){
	// addr is 0 for PA1, 1 for PA2


	*pValue=0;
	if (m_serPort == nullptr)
	{
		wxMessageBox("Not connected");
		return;
	}

	//unsigned char dataByte = 0;
	//dataByte = (unsigned char)(256.0 - (128.0 / vccPa_V));
	double value = (256.0 - (128.0 / vccPa_V));

	uint8_t id = 2;
	if (addr == 0) id = 2; //PA1
	else if (addr == 1) id = 3;  // PA2
	string units = " ";

	int status = m_serPort->CustomParameterWrite(&id, &value, 1, &units);
	if (status != 0)
	{
		wxMessageBox(_("Failed to write value"), _("Warning"));
		return;
	}

	status = m_serPort->CustomParameterRead(&id, &value, 1, &units);
	if (status != 0)
	{
		wxMessageBox(_("Failed to read value"), _("Warning"));
		return;
	}

	*pValue = (int)(value);
}
