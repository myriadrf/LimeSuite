/**
@file 	dlgADPDControls.cpp
@author Lime Microsystems, Borisav Jovanovic
@brief	dialog for various controls for ADPD related plots
*/
#include "dlgADPDControls.h"
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/button.h>

BEGIN_EVENT_TABLE(dlgADPDControls, wxDialog)
//(*EventTable(dlgADPDControls)
//*)
END_EVENT_TABLE()

dlgADPDControls::dlgADPDControls(wxWindow *parent, int value,
								 int m_iYaxisTopPlot, int m_iYaxisBottomPlot, int m_iXaxisLeftPlot, int m_iXaxisRightPlot,
								 wxWindowID id, const wxPoint &pos, const wxSize &size)
{
	m_iValue = value;
	m_iYaxisTop = m_iYaxisTopPlot;
	m_iYaxisBottom = m_iYaxisBottomPlot;
	m_iXaxisLeft = m_iXaxisLeftPlot;
	m_iXaxisRight = m_iXaxisRightPlot;
	BuildContent(parent, id, pos, size);
}

const long dlgADPDControls::ID_PLOTTYPE = wxNewId();
const long dlgADPDControls::ID_TDXPI = wxNewId();
const long dlgADPDControls::ID_TDXPQ = wxNewId();
const long dlgADPDControls::ID_TDYPI = wxNewId();
const long dlgADPDControls::ID_TDYPQ = wxNewId();
const long dlgADPDControls::ID_TDXI = wxNewId();
const long dlgADPDControls::ID_TDXQ = wxNewId();
const long dlgADPDControls::ID_TDYI = wxNewId();
const long dlgADPDControls::ID_TDYQ = wxNewId();
const long dlgADPDControls::ID_UI = wxNewId();
const long dlgADPDControls::ID_UQ = wxNewId();
const long dlgADPDControls::ID_ERROR = wxNewId();
const long dlgADPDControls::ID_PHASEERROR = wxNewId();
const long dlgADPDControls::ID_IVSQ = wxNewId();
const long dlgADPDControls::ID_FFTXP = wxNewId();
const long dlgADPDControls::ID_FFTYP = wxNewId();
const long dlgADPDControls::ID_FFTX = wxNewId();
const long dlgADPDControls::ID_FFTY = wxNewId();
const long dlgADPDControls::ID_BUTTON3 = wxNewId();
const long dlgADPDControls::ID_BUTTON4 = wxNewId();
const long dlgADPDControls::ID_STATICTEXT18 = wxNewId();
const long dlgADPDControls::ID_TEXTCTRL6 = wxNewId();
const long dlgADPDControls::ID_STATICTEXT19 = wxNewId();
const long dlgADPDControls::ID_TEXTCTRL5 = wxNewId();
const long dlgADPDControls::ID_STATICTEXT_X1 = wxNewId();
const long dlgADPDControls::ID_STATICTEXT_X2 = wxNewId();
const long dlgADPDControls::ID_TEXTCTRL_X1 = wxNewId();
const long dlgADPDControls::ID_TEXTCTRL_X2 = wxNewId();

void dlgADPDControls::BuildContent(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
{
	Create(parent, id, _T("Plot options"), wxDefaultPosition, wxSize(300, 450), wxDEFAULT_DIALOG_STYLE | wxCLOSE_BOX, _T("Plot options"));
	Move(wxDefaultPosition);
	SetMinSize(wxSize(300, 450));

	m_controlsSizer = new wxFlexGridSizer(0, 1, 0, 0);
	m_controlsSizer->AddGrowableCol(0);
	SetSizer(m_controlsSizer);

	StaticBox_main = new wxStaticBoxSizer(wxVERTICAL, this, _T("Plot type"));
	Sizer_main = new wxFlexGridSizer(0, 1, 0, 0);
	Sizer_main->AddGrowableCol(0);
	StaticBox_main->Add(Sizer_main);
	m_controlsSizer->Add(StaticBox_main, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);

	StaticBox_td = new wxStaticBoxSizer(wxVERTICAL, this, _T("Time domain options"));
	Sizer_td = new wxFlexGridSizer(0, 4, 0, 0);
	Sizer_td->AddGrowableCol(0);
	StaticBox_td->Add(Sizer_td);

	FlexGridSizer_x = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer_x->AddGrowableCol(0);
	StaticBox_td->Add(FlexGridSizer_x);
	m_controlsSizer->Add(StaticBox_td, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);

	Sizer_IQ = new wxFlexGridSizer(0, 4, 0, 0);
	Sizer_IQ->AddGrowableCol(0);

	m_controlsSizer->Add(Sizer_IQ, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);

	//StaticBox_fft = new wxStaticBoxSizer(wxVERTICAL, this, _T(""));
	//Sizer_fft = new wxFlexGridSizer(0, 2, 0, 0);
	//Sizer_fft->AddGrowableCol(0);
	//StaticBox_fft->Add(Sizer_fft);

	createControls();
	m_controlsSizer->Add(StaticBoxSizer5, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);

	cancelButton = new wxButton(this, wxID_ANY, "Cancel");
	okButton = new wxButton(this, wxID_ANY, "OK");
	//StaticText17 = new wxStaticText(this, wxID_ANY, _T(""));

	Sizer_OC = new wxFlexGridSizer(0, 2, 0, 0);
	//Sizer_OC->AddGrowableCol(0);
	//Sizer_OC->AddGrowableRow(0);
	Sizer_OC->Add(okButton, 0, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 10);
	//Sizer_OC->Add(StaticText17, 0, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 10);
	Sizer_OC->Add(cancelButton, 0, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 10);
	m_controlsSizer->Add(Sizer_OC, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 10);

	Connect(cancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onCancel);
	Connect(okButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onOk);
}

void dlgADPDControls::onCancel(wxCommandEvent &WXUNUSED(pEvent))
{
	EndModal(wxID_CANCEL);
	Destroy();
}

void dlgADPDControls::onOk(wxCommandEvent &WXUNUSED(pEvent))
{

	long templ = 0;
	txtYaxisTop->GetValue().ToLong(&templ);
	m_iYaxisTop = (int)templ;

	templ = 0;
	txtYaxisBottom->GetValue().ToLong(&templ);
	m_iYaxisBottom = (int)templ;

	templ = 0;
	txtXaxisLeft->GetValue().ToLong(&templ);
	m_iXaxisLeft = (int)templ;

	templ = 0;
	txtXaxisRight->GetValue().ToLong(&templ);
	m_iXaxisRight = (int)templ;

	EndModal(wxID_OK);
	Destroy();
}

dlgADPDControls::~dlgADPDControls()
{
}

void dlgADPDControls::createControls()
{
	int m_iPlotOpt = 0;
	wxString __wxPlotType[3] = {_("Time domain"), _("I versus Q"), _("FFT")};
	plotType = new wxRadioBox(this, ID_PLOTTYPE, _(""), wxDefaultPosition, wxDefaultSize, 3, __wxPlotType, 0, wxHORIZONTAL, wxDefaultValidator, "Plot type");
	if ((m_iValue & 0x00003) == 0x00000)
	{
		plotType->SetSelection(0);
		m_iPlotOpt = 0;
	}
	else if ((m_iValue & 0x00003) == 0x00001)
	{
		plotType->SetSelection(1);
		m_iPlotOpt = 1;
	}
	else if ((m_iValue & 0x00003) == 0x00002)
	{
		plotType->SetSelection(2);
		m_iPlotOpt = 2;
	}
	plotType->Enable(true);
	Sizer_main->Add(plotType, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Connect(ID_PLOTTYPE, wxEVT_COMMAND_RADIOBOX_SELECTED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_xpI = new wxCheckBox(this, ID_TDXPI, "xp_I", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "xp_I");
	if (m_iPlotOpt == 0)
		td_xpI->Enable(true);
	else
		td_xpI->Enable(false);
	if ((m_iValue & 0x00004) == 0x00004)
		td_xpI->SetValue(true); // bit 2
	else
		td_xpI->SetValue(false);
	Connect(ID_TDXPI, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_xpQ = new wxCheckBox(this, ID_TDXPQ, "xp_Q", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "xp_Q");
	if (m_iPlotOpt == 0)
		td_xpQ->Enable(true);
	else
		td_xpQ->Enable(false);
	if ((m_iValue & 0x00008) == 0x00008)
		td_xpQ->SetValue(true); // bit 3
	else
		td_xpQ->SetValue(false);
	Connect(ID_TDXPQ, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_ypI = new wxCheckBox(this, ID_TDYPI, "yp_I", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "yp_I");
	if (m_iPlotOpt == 0)
		td_ypI->Enable(true);
	else
		td_ypI->Enable(false);
	if ((m_iValue & 0x00010) == 0x00010)
		td_ypI->SetValue(true); // bit 4
	else
		td_ypI->SetValue(false);
	Connect(ID_TDYPI, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_ypQ = new wxCheckBox(this, ID_TDYPQ, "yp_Q", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "yp_Q");
	if (m_iPlotOpt == 0)
		td_ypQ->Enable(true);
	else
		td_ypQ->Enable(false);
	if ((m_iValue & 0x00020) == 0x00020)
		td_ypQ->SetValue(true); // bit 5
	else
		td_ypQ->SetValue(false);
	Connect(ID_TDYPQ, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_xI = new wxCheckBox(this, ID_TDXI, "x_I", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "x_I");
	if (m_iPlotOpt == 0)
		td_xI->Enable(true);
	else
		td_xI->Enable(false);
	if ((m_iValue & 0x00040) == 0x00040)
		td_xI->SetValue(true); // bit 6
	else
		td_xI->SetValue(false);
	Connect(ID_TDXI, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_xQ = new wxCheckBox(this, ID_TDXQ, "x_Q", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "x_Q");
	if (m_iPlotOpt == 0)
		td_xQ->Enable(true);
	else
		td_xQ->Enable(false);
	if ((m_iValue & 0x00080) == 0x00080)
		td_xQ->SetValue(true); // bit 7
	else
		td_xQ->SetValue(false);
	Connect(ID_TDXQ, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_yI = new wxCheckBox(this, ID_TDYI, "y_I", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "y_I");
	if (m_iPlotOpt == 0)
		td_yI->Enable(true);
	else
		td_yI->Enable(false);
	if ((m_iValue & 0x00100) == 0x00100)
		td_yI->SetValue(true); // bit 8
	else
		td_yI->SetValue(false);
	Connect(ID_TDYI, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_yQ = new wxCheckBox(this, ID_TDYQ, "y_Q", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "y_Q");
	if (m_iPlotOpt == 0)
		td_yQ->Enable(true);
	else
		td_yQ->Enable(false);
	if ((m_iValue & 0x00200) == 0x00200)
		td_yQ->SetValue(true); // bit 9
	else
		td_yQ->SetValue(false);
	Connect(ID_TDYQ, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_uI = new wxCheckBox(this, ID_UI, "u_I", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "u_I");
	td_uI->Show(true);
	if (m_iPlotOpt == 0)
		td_uI->Enable(true);
	else
		td_uI->Enable(false);
	if ((m_iValue & 0x00400) == 0x00400)
		td_uI->SetValue(true); // bit 10
	else
		td_uI->SetValue(false);
	Connect(ID_UI, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_uQ = new wxCheckBox(this, ID_UQ, "u_Q", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "u_Q");
	if (m_iPlotOpt == 0)
		td_uQ->Enable(true);
	else
		td_uQ->Enable(false);
	if ((m_iValue & 0x00800) == 0x00800)
		td_uQ->SetValue(true); // bit 11
	else
		td_uQ->SetValue(false);
	Connect(ID_UQ, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_error = new wxCheckBox(this, ID_ERROR, "u_I-y_I", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "u_I-y_I");
	if (m_iPlotOpt == 0)
		td_error->Enable(true);
	else
		td_error->Enable(false);
	if ((m_iValue & 0x01000) == 0x01000)
		td_error->SetValue(true); // bit 12
	else
		td_error->SetValue(false);
	Connect(ID_ERROR, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	td_phase_error = new wxCheckBox(this, ID_PHASEERROR, "u_Q-y_Q", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "u_Q-y_Q");
	if (m_iPlotOpt == 0)
		td_phase_error->Enable(true);
	else
		td_phase_error->Enable(false);
	if ((m_iValue & 0x02000) == 0x02000)
		td_phase_error->SetValue(true); // bit 13
	else
		td_phase_error->SetValue(false);
	Connect(ID_PHASEERROR, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	Sizer_td->Add(td_xpI, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_ypI, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_xI, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_yI, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);

	Sizer_td->Add(td_xpQ, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_ypQ, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_xQ, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_yQ, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);

	Sizer_td->Add(td_uI, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_error, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 2);
	Sizer_td->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 2);

	Sizer_td->Add(td_uQ, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(td_phase_error, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 2);
	Sizer_td->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 2);
	Sizer_td->Add(new wxStaticText(this, wxID_ANY, _("")), 1, wxALIGN_LEFT | wxEXPAND, 2);

	wxString temps;

	StaticText_x1 = new wxStaticText(this, ID_STATICTEXT_X1, _T("min X:"), wxDefaultPosition, wxDefaultSize, 0, _T(" ID_STATICTEXT_X1"));
	FlexGridSizer_x->Add(StaticText_x1, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 5);
	txtXaxisLeft = new wxTextCtrl(this, ID_TEXTCTRL_X1, _T("0"), wxDefaultPosition, wxSize(48, -1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_X1"));
	temps.Printf(_T("%d"), m_iXaxisLeft);
	txtXaxisLeft->SetValue(temps);
	FlexGridSizer_x->Add(txtXaxisLeft, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);

	StaticText_x2 = new wxStaticText(this, ID_STATICTEXT_X2, _T("max X:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_X2"));
	FlexGridSizer_x->Add(StaticText_x2, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 5);
	txtXaxisRight = new wxTextCtrl(this, ID_TEXTCTRL_X2, _T("8191"), wxDefaultPosition, wxSize(48, -1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_X2"));
	temps.Printf(_T("%d"), m_iXaxisRight);
	txtXaxisRight->SetValue(temps);
	FlexGridSizer_x->Add(txtXaxisRight, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);

	wxString __wxValue[4] = {_("xp"), _("yp"), _("x"), _("y")};
	IvsQ = new wxRadioBox(this, ID_IVSQ, _("I versus Q options"), wxDefaultPosition, wxDefaultSize, 4, __wxValue, 0, wxHORIZONTAL, wxDefaultValidator, "IvsQ");
	if (m_iPlotOpt == 1)
		IvsQ->Enable(true);
	else
		IvsQ->Enable(false);
	if ((m_iValue & 0x0C000) == 0x00000)
		IvsQ->SetSelection(0); // bits 15-14
	else if ((m_iValue & 0x0C000) == 0x04000)
		IvsQ->SetSelection(1);
	else if ((m_iValue & 0x0C000) == 0x08000)
		IvsQ->SetSelection(2);
	else
		IvsQ->SetSelection(3);
	Sizer_IQ->Add(IvsQ, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
	Connect(ID_IVSQ, wxEVT_COMMAND_RADIOBOX_SELECTED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	fft_xp = new wxCheckBox(this, ID_FFTXP, "xp", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "xp");
	if (m_iPlotOpt == 2)
		fft_xp->Enable(true);
	else
		fft_xp->Enable(false);
	if ((m_iValue & 0x10000) == 0x10000)
		fft_xp->SetValue(true); // bit 16
	else
		fft_xp->SetValue(false);
	//Sizer_fft->Add(fft_xp, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
	Connect(ID_FFTXP, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	fft_yp = new wxCheckBox(this, ID_FFTYP, "yp", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "yp");
	if (m_iPlotOpt == 2)
		fft_yp->Enable(true);
	else
		fft_yp->Enable(false);
	if ((m_iValue & 0x20000) == 0x20000)
		fft_yp->SetValue(true); // bit 17
	else
		fft_yp->SetValue(false);
	//Sizer_fft->Add(fft_yp, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
	Connect(ID_FFTYP, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	fft_x = new wxCheckBox(this, ID_FFTX, "x", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "x");
	if (m_iPlotOpt == 2)
		fft_x->Enable(true);
	else
		fft_x->Enable(false);
	if ((m_iValue & 0x40000) == 0x40000)
		fft_x->SetValue(true); // bit 18
	else
		fft_x->SetValue(false);
	//Sizer_fft->Add(fft_x, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
	Connect(ID_FFTX, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	fft_y = new wxCheckBox(this, ID_FFTY, "y", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "y");
	if (m_iPlotOpt == 2)
		fft_y->Enable(true);
	else
		fft_y->Enable(false);
	if ((m_iValue & 0x80000) == 0x80000)
		fft_y->SetValue(true); // bit 19
	else
		fft_y->SetValue(false);
	//Sizer_fft->Add(fft_y, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
	Connect(ID_FFTY, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&dlgADPDControls::onParamChange);

	StaticBoxSizer5 = new wxStaticBoxSizer(wxVERTICAL, this, _T("FFT plot setup"));
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);

	FlexGridSizer16 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer16->Add(fft_xp, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);
	FlexGridSizer16->Add(fft_yp, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);
	FlexGridSizer16->Add(fft_x, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);
	FlexGridSizer16->Add(fft_y, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);

	StaticText15 = new wxStaticText(this, 1, _T("max Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
	FlexGridSizer16->Add(StaticText15, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);
	txtYaxisTop = new wxTextCtrl(this, ID_TEXTCTRL6, _T("0"), wxDefaultPosition, wxSize(48, -1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL6"));
	temps.Printf(_T("%d"), m_iYaxisTop);
	txtYaxisTop->SetValue(temps);
	FlexGridSizer16->Add(txtYaxisTop, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);

	StaticText16 = new wxStaticText(this, ID_STATICTEXT19, _T("min Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
	FlexGridSizer16->Add(StaticText16, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);
	txtYaxisBottom = new wxTextCtrl(this, ID_TEXTCTRL5, _T("-100"), wxDefaultPosition, wxSize(48, -1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	temps.Printf(_T("%d"), m_iYaxisBottom);
	txtYaxisBottom->SetValue(temps);
	FlexGridSizer16->Add(txtYaxisBottom, 1, wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxALL, 2);

	StaticBoxSizer5->Add(FlexGridSizer9, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 5);
	StaticBoxSizer5->Add(FlexGridSizer16, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND | wxALL, 5);

	onEnableDisable();
}

void dlgADPDControls::onParamChange(wxCommandEvent &evt)
{
	onEnableDisable();
}

void dlgADPDControls::onEnableDisable()
{
	int temp = 0;

	temp = plotType->GetSelection(); //bits 1-0
	m_iValue &= 0xFFFFC;			 // reset
	m_iValue |= temp;

	if (temp == 0)
	{ // time domain options enabled
		td_xpI->Enable(true);
		td_xpQ->Enable(true);
		td_ypI->Enable(true);
		td_ypQ->Enable(true);
		td_xI->Enable(true);
		td_xQ->Enable(true);
		td_yI->Enable(true);
		td_yQ->Enable(true);
		td_uI->Enable(true);
		td_uQ->Enable(true);
		td_error->Enable(true);
		td_phase_error->Enable(true);
		IvsQ->Enable(false);
		fft_xp->Enable(false);
		fft_yp->Enable(false);
		fft_x->Enable(false);
		fft_y->Enable(false);

		if (td_xpI->IsChecked())
			m_iValue |= 0x00004; // bit 2
		else
			m_iValue &= 0xFFFFB;
		if (td_xpQ->IsChecked())
			m_iValue |= 0x00008; // bit 3
		else
			m_iValue &= 0xFFFF7;
		if (td_ypI->IsChecked())
			m_iValue |= 0x00010; // bit 4
		else
			m_iValue &= 0xFFFEF;
		if (td_ypQ->IsChecked())
			m_iValue |= 0x00020; // bit 5
		else
			m_iValue &= 0xFFFDF;
		if (td_xI->IsChecked())
			m_iValue |= 0x00040; // bit 6
		else
			m_iValue &= 0xFFFBF;
		if (td_xQ->IsChecked())
			m_iValue |= 0x00080; // bit 7
		else
			m_iValue &= 0xFFF7F;
		if (td_yI->IsChecked())
			m_iValue |= 0x00100; // bit 8
		else
			m_iValue &= 0xFFEFF;
		if (td_yQ->IsChecked())
			m_iValue |= 0x00200; // bit 9
		else
			m_iValue &= 0xFFDFF;
		if (td_uI->IsChecked())
			m_iValue |= 0x00400; // bit 10
		else
			m_iValue &= 0xFFBFF;
		if (td_uQ->IsChecked())
			m_iValue |= 0x00800; // bit 11
		else
			m_iValue &= 0xFF7FF;
		if (td_error->IsChecked())
			m_iValue |= 0x01000; // bit 12
		else
			m_iValue &= 0xFEFFF;
		if (td_phase_error->IsChecked())
			m_iValue |= 0x02000; // bit 13
		else
			m_iValue &= 0xFDFFF;

		StaticText15->Enable(false);
		txtYaxisTop->Enable(false);
		StaticText16->Enable(false);
		txtYaxisBottom->Enable(false);
	}
	else if (temp == 1)
	{ // IQ constellation options  enabled
		td_xpI->Enable(false);
		td_xpQ->Enable(false);
		td_ypI->Enable(false);
		td_ypQ->Enable(false);
		td_xI->Enable(false);
		td_xQ->Enable(false);
		td_yI->Enable(false);
		td_yQ->Enable(false);
		td_uI->Enable(false);
		td_uQ->Enable(false);
		td_error->Enable(false);
		td_phase_error->Enable(false);
		IvsQ->Enable(true);
		fft_xp->Enable(false);
		fft_yp->Enable(false);
		fft_x->Enable(false);
		fft_y->Enable(false);
		temp = IvsQ->GetSelection(); // bits 15-14
		m_iValue &= 0xF3FFF;		 // reset
		m_iValue |= (temp << 14);

		StaticText15->Enable(false);
		txtYaxisTop->Enable(false);
		StaticText16->Enable(false);
		txtYaxisBottom->Enable(false);
	}
	else
	{ // FFT options enabled
		td_xpI->Enable(false);
		td_xpQ->Enable(false);
		td_ypI->Enable(false);
		td_ypQ->Enable(false);
		td_xI->Enable(false);
		td_xQ->Enable(false);
		td_yI->Enable(false);
		td_yQ->Enable(false);
		td_uI->Enable(false);
		td_uQ->Enable(false);
		td_error->Enable(false);
		td_phase_error->Enable(false);
		IvsQ->Enable(false);
		fft_xp->Enable(true);
		fft_yp->Enable(true);
		fft_x->Enable(true);
		fft_y->Enable(true);

		if (fft_xp->IsChecked())
			m_iValue |= 0x10000; // bit 16
		else
			m_iValue &= 0xEFFFF;
		if (fft_yp->IsChecked())
			m_iValue |= 0x20000; // bit 17
		else
			m_iValue &= 0xDFFFF;
		if (fft_x->IsChecked())
			m_iValue |= 0x40000; // bit 18
		else
			m_iValue &= 0xBFFFF;
		if (fft_y->IsChecked())
			m_iValue |= 0x80000; // bit 19
		else
			m_iValue &= 0x7FFFF;

		StaticText15->Enable(true);
		txtYaxisTop->Enable(true);
		StaticText16->Enable(true);
		txtYaxisBottom->Enable(true);
	}
}
