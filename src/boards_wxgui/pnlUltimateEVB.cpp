#include "pnlUltimateEVB.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"

#include <ciso646>
#include <vector>


using namespace std;

BEGIN_EVENT_TABLE(pnlUltimateEVB, wxPanel)
END_EVENT_TABLE()

pnlUltimateEVB::pnlUltimateEVB(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    SetSizer(mainSizer);

    auto powerBox = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Voltage/Current/Power") ), wxVERTICAL );
    auto sizerPowerRd = new wxFlexGridSizer( 0, 4, 2, 2 );
    sizerPowerRd->SetFlexibleDirection( wxBOTH );
    sizerPowerRd->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Name")), 1, wxALL, 5);
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Voltage")), 1, wxALL, 5);
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Current")), 1, wxALL, 5);
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Power")), 1, wxALL, 5);

    const std::vector<std::string> names = {"VCC1P25_LMS_DIG", "VCC1P25_LMS", "VCC1P4_LMS", "VCC1P8_LMS", "VDIO_LMS", "USER"};

    for (unsigned i = 0; i<names.size(); i++)
    {
        const int offset = 35+i;
        pwrParams.push_back(ADCparam(offset,names[i]));
        pwrParams.push_back(ADCparam(offset+names.size(),names[i]));
        pwrParams.push_back(ADCparam(offset+2*names.size(),names[i]));
    }

    for (unsigned i = 0; i<pwrParams.size(); i++)
    {
        if (i%3 == 0)
        {
            pwrParams[i].title = new wxStaticText(this, wxID_ANY, pwrParams[i].name);
            sizerPowerRd->Add(pwrParams[i].title, 1, wxALL, 5);
        }
        pwrParams[i].value = new wxStaticText(this, wxID_ANY, _("- - - - -"));
        sizerPowerRd->Add(pwrParams[i].value, 1, wxALL, 5);
    }
    powerBox->Add( sizerPowerRd, 1, wxEXPAND, 5 );
    auto sizertotalPwr = new wxFlexGridSizer( 1, 2, 10, 10 );
    totalPwr = new wxStaticText(this, wxID_ANY, _("- - - W"));
    sizertotalPwr->Add(new wxStaticText(this, wxID_ANY, _("Total LMS power:")), 1, wxALL, 5);
    sizertotalPwr->Add( totalPwr, 1, wxALL, 5 );
    powerBox->Add( sizertotalPwr, 1, wxEXPAND, 5 );

    auto voltageBox = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Voltages") ), wxVERTICAL );
    auto sizerVoltageRd = new wxFlexGridSizer( 0, 4, 2, 2 );
    sizerVoltageRd->SetFlexibleDirection( wxBOTH );
    sizerVoltageRd->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _("Name")), 1, wxALL, 5);
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _(" Voltage")), 1, wxALL, 5);
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _("Name")), 1, wxALL, 5);
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _("Voltage")), 1, wxALL, 5);

    const std::vector<std::string> voltages = {
        "AMUX_VDD_AFE", "AMUX_VDD12_RXBUF", "AMUX_VDD12_VCO_SXR", "AMUX_DVDD_SXR",
        "AMUX_VDD_DIV_SXR", "AMUX_VDD_CP_SXR", "AMUX_VDD18_SXR", "AMUX_VDD_MXLOBUF_RFE",
        "AMUX_VDD12_TIA_RFE", "AMUX_VDD14_LNA_RFE", "AMUX_VDD14_TIA_RFE", "AMUX_DIGPRVDD1_1",
        "AMUX_VDD12_LNA_RFE", "AMUX_VDD14_RBB", "AMUX_DVDD_SXT", "AMUX_VDD_TBB",
        "AMUX_VDD_TPAD_TRF", "AMUX_VDD12_DIG", "AMUX_VDD_CP_SXT", "AMUX_VDD12_VCO_SXT",
        "AMUX_DIGPRVDD1_2", "AMUX_DVDD_CGEN", "AMUX_VDD_DIV_CGEN", "AMUX_VDD_SPI_BUF",
        "AMUX_VDD12_TXBUF", "AMUX_VDD_DIV_SXT", "AMUX_VDD_TLOBUF_TRF", "AMUX_VDD14_VCO_CGEN",
        "AMUX_VDD_CP_CGEN", "AMUX_LMS_TSTAO", "AMUX_LMS_TSTDO1", "AMUX_LMS_TSTDO0"
    };

    for (unsigned i = 0; i<voltages.size(); i++)
        vltgParams.push_back(ADCparam(2+i,voltages[i]));


    for (unsigned i = 0; i<vltgParams.size(); i++)
    {
        vltgParams[i].title = new wxStaticText(this, wxID_ANY, vltgParams[i].name);
        sizerVoltageRd->Add(vltgParams[i].title, 1, wxALL, 5);
        vltgParams[i].value = new wxStaticText(this, wxID_ANY, _("- - - mV"));
        sizerVoltageRd->Add(vltgParams[i].value, 1, wxALL, 5);
    }
    voltageBox->Add( sizerVoltageRd, 1, wxEXPAND, 5 );

    mainSizer->Add( voltageBox, 1, wxEXPAND, 5 );
    mainSizer->Add( powerBox, 1, wxEXPAND, 5 );
    mainSizer->Fit(this);
    //mainSizer->SetSizeHints(this);
    Layout();
    Fit();
    Bind(READ_ALL_VALUES, &pnlUltimateEVB::OnReadAll, this, this->GetId());
}

void pnlUltimateEVB::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
}

pnlUltimateEVB::~pnlUltimateEVB()
{
}

void pnlUltimateEVB::OnReadAll(wxCommandEvent &event)
{
    double sumPwr = 0;
    float_type value;
    lms_name_t units;
    for (size_t i = 0; i < pwrParams.size(); ++i)
    {
        int status = LMS_ReadCustomBoardParam(lmsControl,pwrParams[i].channel,&value,units);
        if (status != 0)
        {
            wxMessageBox(LMS_GetLastErrorMessage(), _("Warning"));
            return;
        }
        if (pwrParams[i].channel >= 47 && pwrParams[i].channel < 52)
            sumPwr += value;
        pwrParams[i].value->SetLabel(wxString::Format(_("%1.0f %s"), value, units));
    }
    totalPwr->SetLabel(wxString::Format(_("%1.0f mW"), sumPwr));

    for (size_t i = 0; i < vltgParams.size(); ++i)
    {
        int status = LMS_ReadCustomBoardParam(lmsControl,vltgParams[i].channel,&value,units);
        if (status != 0)
        {
            wxMessageBox(LMS_GetLastErrorMessage(), _("Warning"));
            return;
        }
        vltgParams[i].value->SetLabel(wxString::Format(_("%1.0f %s"), value, units));
    }
    this->GetParent()->Layout();
    this->GetParent()->Fit();
}

void pnlUltimateEVB::OnWriteAll(wxCommandEvent &event)
{
}
