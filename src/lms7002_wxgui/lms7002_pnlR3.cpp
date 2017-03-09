#include "lms7002_pnlR3.h"

#include <wx/wx.h>
#include <wx/stattext.h>
#include "LMS7002M_parameters.h"
#include "lms7002_gui_utilities.h"
#include <chrono>
#include <thread>
#include "mcu_programs.h"

#include <vector>

using namespace std;

lms7002_pnlR3_view::lms7002_pnlR3_view( wxWindow* parent ) :
    wxPanel( parent)
{

}

lms7002_pnlR3_view::lms7002_pnlR3_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
    wxPanel( parent, id, pos, size, style )
{
    lmsControl = nullptr;
    wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 0, 1, 5, 5 );
    {
        wxFlexGridSizer* rowGroup = new wxFlexGridSizer(0, 4, 5, 5);
        { //LimeLight controls
            wxStaticBoxSizer* limeLight = new wxStaticBoxSizer(wxVERTICAL, this, wxT("LimeLight"));
            const wxString delays[] = {_("No delay"), _("1x delay"), _("2x delay"), _("3x delay")};
            const wxArrayString delaysArray(sizeof(delays) / sizeof(wxString), delays);

            wxFlexGridSizer* lmlDelaysSizer = new wxFlexGridSizer(0, 4, 0, 0);
            lmlDelaysSizer->Add(new wxStaticText(limeLight->GetStaticBox(), wxID_ANY, wxT("FCLK1_DLY:")), 0, wxALIGN_CENTER_VERTICAL, 5);
            cmbFCLK1_DLY = new wxComboBox(limeLight->GetStaticBox(), wxID_ANY, wxT("No Delay"));
            cmbFCLK1_DLY->Append(delaysArray);
            lmlDelaysSizer->Add(cmbFCLK1_DLY, 1, wxLEFT, 5);

            lmlDelaysSizer->Add(new wxStaticText(limeLight->GetStaticBox(), wxID_ANY, wxT("FCLK2_DLY:")), 0, wxALIGN_CENTER_VERTICAL, 5);
            cmbFCLK2_DLY = new wxComboBox(limeLight->GetStaticBox(), wxID_ANY, wxT("No Delay"));
            cmbFCLK2_DLY->Append(delaysArray);
            lmlDelaysSizer->Add(cmbFCLK2_DLY, 1, wxLEFT, 5);

            lmlDelaysSizer->Add(new wxStaticText(limeLight->GetStaticBox(), wxID_ANY, wxT("MCLK1_DLY:")), 0, wxALIGN_CENTER_VERTICAL, 5);
            cmbMCLK1_DLY = new wxComboBox(limeLight->GetStaticBox(), wxID_ANY, wxT("No Delay"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
            cmbMCLK1_DLY->Append(delaysArray);
            lmlDelaysSizer->Add(cmbMCLK1_DLY, 1, wxLEFT, 5);

            lmlDelaysSizer->Add(new wxStaticText(limeLight->GetStaticBox(), wxID_ANY, wxT("MCLK2_DLY:")), 0, wxALIGN_CENTER_VERTICAL, 5);
            cmbMCLK2_DLY = new wxComboBox(limeLight->GetStaticBox(), wxID_ANY, wxT("No Delay"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
            cmbMCLK2_DLY->Append(delaysArray);
            lmlDelaysSizer->Add(cmbMCLK2_DLY, 1, wxLEFT, 5);

            limeLight->Add(lmlDelaysSizer, 1, 0, 5);

            cmbFCLK1_DLY->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            cmbFCLK2_DLY->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            cmbMCLK1_DLY->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            cmbMCLK2_DLY->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);

            wxFlexGridSizer* lmlCheckboxes = new wxFlexGridSizer(0, 2, 0, 0);
            chkMCLK1_INV = new wxCheckBox(limeLight->GetStaticBox(), wxID_ANY, wxT("MCLK1 Invert"));
            chkMCLK2_INV = new wxCheckBox(limeLight->GetStaticBox(), wxID_ANY, wxT("MCLK2 invert"));
            chkLML1_TRXIQPULSE = new wxCheckBox(limeLight->GetStaticBox(), wxID_ANY, wxT("LML1_TRXIQPULSE mode"));
            chkLML2_TRXIQPULSE = new wxCheckBox(limeLight->GetStaticBox(), wxID_ANY, wxT("LML2_TRXIQPULSE mode"));
            chkLML1_SISODDR = new wxCheckBox(limeLight->GetStaticBox(), wxID_ANY, wxT("LML1_SISODDR mode"));
            chkLML2_SISODDR = new wxCheckBox(limeLight->GetStaticBox(), wxID_ANY, wxT("LML2_SISODDR mode"));
            wxWindow* items[] = {chkMCLK1_INV, chkMCLK2_INV, chkLML1_TRXIQPULSE, chkLML2_TRXIQPULSE, chkLML1_SISODDR, chkLML2_SISODDR};
            for(auto i : items)
            {
                i->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                lmlCheckboxes->Add(i, 0, wxEXPAND, 5);
            }
            limeLight->Add(lmlCheckboxes, 0, 0, 5);
            rowGroup->Add(limeLight, 1, 0, 5);
        }
        {
            wxStaticBoxSizer* sbSizer140;
            sbSizer140 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("TRX_GAIN")), wxVERTICAL);

            chkTRX_GAIN_SRC = new wxCheckBox(sbSizer140->GetStaticBox(), wxID_ANY, wxT("Alternative TRX gain source"), wxDefaultPosition, wxDefaultSize, 0);
            sbSizer140->Add(chkTRX_GAIN_SRC, 0, wxLEFT, 5);

            wxFlexGridSizer* fgSizer256;
            fgSizer256 = new wxFlexGridSizer(0, 2, 0, 0);
            fgSizer256->SetFlexibleDirection(wxBOTH);
            fgSizer256->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

            wxStaticBoxSizer* sbSizer137;
            sbSizer137 = new wxStaticBoxSizer(new wxStaticBox(sbSizer140->GetStaticBox(), wxID_ANY, wxT("TBB")), wxVERTICAL);

            wxFlexGridSizer* fgSizer254;
            fgSizer254 = new wxFlexGridSizer(0, 2, 0, 0);
            fgSizer254->AddGrowableCol(1);
            fgSizer254->SetFlexibleDirection(wxBOTH);
            fgSizer254->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

            fgSizer254->Add(new wxStaticText(sbSizer137->GetStaticBox(), wxID_ANY, wxT("CG_IAMP:")), 0, 0, 5);

            spinCG_IAMP_TBB = new NumericSlider(sbSizer137->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0);
            fgSizer254->Add(spinCG_IAMP_TBB, 0, wxEXPAND, 5);

            fgSizer254->Add(new wxStaticText(sbSizer137->GetStaticBox(), wxID_ANY, wxT("LOSS_LIN_TXPAD")), 0, 0, 5);

            spinLOSS_LIN_TXPAD_TRF = new NumericSlider(sbSizer137->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0);
            fgSizer254->Add(spinLOSS_LIN_TXPAD_TRF, 0, wxEXPAND, 5);

            fgSizer254->Add(new wxStaticText(sbSizer137->GetStaticBox(), wxID_ANY, wxT("LOSS_MAIN_TXPAD")), 0, 0, 5);

            spinLOSS_MAIN_TXPAD_TRF = new NumericSlider(sbSizer137->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0);
            fgSizer254->Add(spinLOSS_MAIN_TXPAD_TRF, 0, wxEXPAND, 5);


            sbSizer137->Add(fgSizer254, 1, wxEXPAND, 5);

            chkR5_LPF_BYP_TBB = new wxCheckBox(sbSizer137->GetStaticBox(), wxID_ANY, wxT("Bypass LPFS5 low pass real-pole filter capacitor banks"), wxDefaultPosition, wxDefaultSize, 0);
            chkR5_LPF_BYP_TBB->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sbSizer137->Add(chkR5_LPF_BYP_TBB, 0, 0, 5);


            fgSizer256->Add(sbSizer137, 1, 0, 5);


            wxStaticBoxSizer* sbSizer138;
            sbSizer138 = new wxStaticBoxSizer(new wxStaticBox(sbSizer140->GetStaticBox(), wxID_ANY, wxT("RX")), wxVERTICAL);

            wxFlexGridSizer* fgSizer255;
            fgSizer255 = new wxFlexGridSizer(0, 2, 0, 0);
            fgSizer255->SetFlexibleDirection(wxBOTH);
            fgSizer255->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

            fgSizer255->Add(new wxStaticText(sbSizer138->GetStaticBox(), wxID_ANY, wxT("C_CTL_PGA_RBB")), 0, wxALIGN_CENTER_VERTICAL, 5);

            spinC_CTL_PGA_RBB = new NumericSlider(sbSizer138->GetStaticBox(), wxID_ANY, wxT("2"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 4, 2);
            fgSizer255->Add(spinC_CTL_PGA_RBB, 0, 0, 5);

            {
                fgSizer255->Add(new wxStaticText(sbSizer138->GetStaticBox(), wxID_ANY, wxT("G_PGA_RBB")), 0, wxALIGN_CENTER_VERTICAL, 5);
                cmbG_PGA_RBB = new wxComboBox(sbSizer138->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
                wxArrayString temp;
                for(int i = 0; i < 32; ++i)
                    temp.push_back(wxString::Format(_("%i dB"), -12 + i));
                cmbG_PGA_RBB->Append(temp);
                fgSizer255->Add(cmbG_PGA_RBB, 0, wxEXPAND, 5);
            }
        {
            fgSizer255->Add(new wxStaticText(sbSizer138->GetStaticBox(), wxID_ANY, wxT("G_LNA")), 0, wxALIGN_CENTER_VERTICAL, 5);
            cmbG_LNA_RFE = new wxComboBox(sbSizer138->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
            const wxString items[] =
            {_("Gmax-30")
            , _("Gmax-27")
            , _("Gmax-24")
            , _("Gmax-21")
            , _("Gmax-18")
            , _("Gmax-15")
            , _("Gmax-12")
            , _("Gmax-9")
            , _("Gmax-6")
            , _("Gmax-5")
            , _("Gmax-4")
            , _("Gmax-3")
            , _("Gmax-2")
            , _("Gmax-1")
            , _("Gmax")};
            cmbG_LNA_RFE->Append(wxArrayString(sizeof(items) / sizeof(wxString), items));
            fgSizer255->Add(cmbG_LNA_RFE, 0, wxEXPAND, 5);
        }

        {
            fgSizer255->Add(new wxStaticText(sbSizer138->GetStaticBox(), wxID_ANY, wxT("G_TIA")), 0, wxALIGN_CENTER_VERTICAL, 5);
            cmbG_TIA_RFE = new wxComboBox(sbSizer138->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
            const wxString items[] =
            {_("Gmax-12")
            , _("Gmax-3")
            , _("Gmax")};
            cmbG_TIA_RFE->Append(wxArrayString(sizeof(items) / sizeof(wxString), items));
            fgSizer255->Add(cmbG_TIA_RFE, 0, wxEXPAND, 5);
        }

        sbSizer138->Add(fgSizer255, 1, wxEXPAND, 5);


        fgSizer256->Add(sbSizer138, 1, wxEXPAND, 5);


        sbSizer140->Add(fgSizer256, 1, wxEXPAND, 5);


        rowGroup->Add(sbSizer140, 1, wxEXPAND, 5);
        }
        mainSizer->Add(rowGroup, 1, wxEXPAND, 5);
    }

    {
        wxStaticBoxSizer* dcCalibGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("DC calibration"));
        wxWindow* panel = dcCalibGroup->GetStaticBox();
        {
            wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 1, 0, 5);
            std::vector<const LMS7Parameter*> params = {
                &LMS7_PD_DCDAC_RXB,
                &LMS7_PD_DCDAC_RXA,
                &LMS7_PD_DCDAC_TXB,
                &LMS7_PD_DCDAC_TXA,
                &LMS7_PD_DCCMP_RXB,
                &LMS7_PD_DCCMP_RXA,
                &LMS7_PD_DCCMP_TXB,
                &LMS7_PD_DCCMP_TXA
            };
            int row = 0;
            for(auto i : params)
            {
                sizer->AddGrowableRow(row++);
                wxCheckBox* chkbox = new wxCheckBox(dcCalibGroup->GetStaticBox(), wxNewId(), i->name);
                chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                sizer->Add(chkbox, 1, wxEXPAND, 5);
                wndId2Enum[chkbox] = *i;
            }
            dcCalibGroup->Add(sizer, 0, wxEXPAND, 5);
        }
        {
            wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 2, 0, 0);
            std::vector<const LMS7Parameter*> paramsRx = {
                &LMS7_DCWR_RXBQ, &LMS7_DCRD_RXBQ, &LMS7_DC_RXBQ,
                &LMS7_DCWR_RXBI, &LMS7_DCRD_RXBI, &LMS7_DC_RXBI,
                &LMS7_DCWR_RXAQ, &LMS7_DCRD_RXAQ, &LMS7_DC_RXAQ,
                &LMS7_DCWR_RXAI, &LMS7_DCRD_RXAI, &LMS7_DC_RXAI
            };
            for(size_t i = 0; i<paramsRx.size(); i += 3)
            {
                wxButton* btnReadDC = new wxButton(dcCalibGroup->GetStaticBox(), wxNewId(), _("Read"));
                btnReadDC ->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::OnReadDC), NULL, this);
                sizer->Add(btnReadDC , 1, wxEXPAND, 5);
                NumericSlider* slider = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0);
                cmbDCControlsRx.push_back(slider);
                slider->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::OnWriteRxDC), NULL, this);
                sizer->Add(slider, 1, wxEXPAND, 5);
                wndId2Enum[slider] = *paramsRx[i + 2];
            }
            std::vector<const LMS7Parameter*> paramsTx = {
                &LMS7_DCWR_TXBQ, &LMS7_DCRD_TXBQ, &LMS7_DC_TXBQ,
                &LMS7_DCWR_TXBI, &LMS7_DCRD_TXBI, &LMS7_DC_TXBI,
                &LMS7_DCWR_TXAQ, &LMS7_DCRD_TXAQ, &LMS7_DC_TXAQ,
                &LMS7_DCWR_TXAI, &LMS7_DCRD_TXAI, &LMS7_DC_TXAI
            };
            for(size_t i = 0; i<paramsTx.size(); i += 3)
            {
                wxButton* btnReadDC = new wxButton(dcCalibGroup->GetStaticBox(), wxNewId(), _("Read"));
                btnReadDC ->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::OnReadDC), NULL, this);
                sizer->Add(btnReadDC , 1, wxEXPAND, 5);
                NumericSlider* slider = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1023, 1023, 0);
                cmbDCControlsTx.push_back(slider);
                slider->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::OnWriteTxDC), NULL, this);
                sizer->Add(slider, 1, wxEXPAND, 5);
                wndId2Enum[slider] = *paramsTx[i + 2];
            }
            dcCalibGroup->Add(sizer, 0, 0, 5);
        }
        {
            const std::vector<wxString> names = {_("RXBQ"), _("RXBI"), _("RXAQ"), _("RXAI"), _("TXBQ"), _("TXBI"), _("TXAQ"), _("TXAI")};
            const std::vector<LMS7Parameter> cmpcfg = {(LMS7_DCCAL_CMPCFG_RXBQ), (LMS7_DCCAL_CMPCFG_RXBI), (LMS7_DCCAL_CMPCFG_RXAQ), (LMS7_DCCAL_CMPCFG_RXAI), (LMS7_DCCAL_CMPCFG_TXBQ), (LMS7_DCCAL_CMPCFG_TXBI), (LMS7_DCCAL_CMPCFG_TXAQ), (LMS7_DCCAL_CMPCFG_TXAI)};
            const std::vector<LMS7Parameter> cmpstatus ={(LMS7_DCCAL_CMPSTATUS_RXBQ), (LMS7_DCCAL_CMPSTATUS_RXBI), (LMS7_DCCAL_CMPSTATUS_RXAQ), (LMS7_DCCAL_CMPSTATUS_RXAI), (LMS7_DCCAL_CMPSTATUS_TXBQ), (LMS7_DCCAL_CMPSTATUS_TXBI), (LMS7_DCCAL_CMPSTATUS_TXAQ), (LMS7_DCCAL_CMPSTATUS_TXAI)};
            const std::vector<LMS7Parameter> calstatus = {(LMS7_DCCAL_CALSTATUS_RXBQ), (LMS7_DCCAL_CALSTATUS_RXBI), (LMS7_DCCAL_CALSTATUS_RXAQ), (LMS7_DCCAL_CALSTATUS_RXAI), (LMS7_DCCAL_CALSTATUS_TXBQ), (LMS7_DCCAL_CALSTATUS_TXBI), (LMS7_DCCAL_CALSTATUS_TXAQ), (LMS7_DCCAL_CMPSTATUS_TXAI)};
            const std::vector<LMS7Parameter> start = {(LMS7_DCCAL_START_RXBQ), (LMS7_DCCAL_START_RXBI), (LMS7_DCCAL_START_RXAQ), (LMS7_DCCAL_START_RXAI), (LMS7_DCCAL_START_TXBQ), (LMS7_DCCAL_START_TXBI), (LMS7_DCCAL_START_TXAQ), (LMS7_DCCAL_START_TXAI)};

            wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 5, 0, 5);
            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("Name:")), 1, wxEXPAND, 0);
            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("START:")), 1, wxEXPAND, 0);
            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("CMP invert:")), 1, wxEXPAND, 0);
            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("CMP:")), 1, wxEXPAND, 0);
            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("Status:")), 1, wxEXPAND, 0);
            for(size_t i = 0; i<names.size(); ++i)
            {
                sizer->AddGrowableRow(i);
                wxCheckBox* chkbox;
                sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, names[i]), 1, wxEXPAND, 0);

                chkbox = new wxCheckBox(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString);
                chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                wndId2Enum[chkbox] = start[i];
                sizer->Add(chkbox, 0, wxALIGN_CENTER_HORIZONTAL, 0);

                chkbox = new wxCheckBox(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString);
                chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                wndId2Enum[chkbox] = cmpcfg[i];
                sizer->Add(chkbox, 0, wxALIGN_CENTER_HORIZONTAL, 0);

                dccal_cmpstatuses[i] = new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("0"));
                wndId2Enum[dccal_cmpstatuses[i]] = cmpstatus[i];
                sizer->Add(dccal_cmpstatuses[i], 1, wxEXPAND, 0);
                dccal_statuses[i] = new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("Not running"));
                sizer->Add(dccal_statuses[i], 1, wxEXPAND, 0);
                wndId2Enum[dccal_statuses[i]] = calstatus[i];
            }
            for(int i = 0; i < 4; ++i)
                sizer->Add(new wxFlexGridSizer(0, 0, 0, 0));
            wxButton* btnUpdateCMP = new wxButton(panel, wxNewId(), _("Read"));
            btnUpdateCMP->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::OnReadDCCMP), NULL, this);
            sizer->Add(btnUpdateCMP);
            dcCalibGroup->Add(sizer, 0, 0, 5);
        }
        {
            wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 2, 0, 5);
            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("DC_RXCDIV")), 1, wxEXPAND, 0);
            NumericSlider* ctrl = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0);
            ctrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(ctrl, 1, wxEXPAND, 5);
            wndId2Enum[ctrl] = LMS7_DC_RXCDIV;
            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("DC_TXCDIV")), 1, wxEXPAND, 0);
            ctrl = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0);
            ctrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(ctrl, 1, wxEXPAND, 5);
            wndId2Enum[ctrl] = LMS7_DC_TXCDIV;

            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("HYSCMP_RXB")), 1, wxEXPAND, 0);
            ctrl = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 0);
            ctrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(ctrl, 1, wxEXPAND, 5);
            wndId2Enum[ctrl] = LMS7_HYSCMP_RXB;

            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("HYSCMP_RXA")), 1, wxEXPAND, 0);
            ctrl = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 0);
            ctrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(ctrl, 1, wxEXPAND, 5);
            wndId2Enum[ctrl] = LMS7_HYSCMP_RXA;

            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("HYSCMP_TXB")), 1, wxEXPAND, 0);
            ctrl = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 0);
            ctrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(ctrl, 1, wxEXPAND, 5);
            wndId2Enum[ctrl] = LMS7_HYSCMP_TXB;

            sizer->Add(new wxStaticText(dcCalibGroup->GetStaticBox(), wxID_ANY, _("HYSCMP_TXA")), 1, wxEXPAND, 0);
            ctrl = new NumericSlider(dcCalibGroup->GetStaticBox(), wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 0);
            ctrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(ctrl, 1, wxEXPAND, 5);
            wndId2Enum[ctrl] = LMS7_HYSCMP_TXA;
            dcCalibGroup->Add(sizer, 0, wxLEFT, 5);

            wxCheckBox* chkbox = new wxCheckBox(dcCalibGroup->GetStaticBox(), wxNewId(), "Automatic DC calibration mode");
            chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            wndId2Enum[chkbox] = LMS7_DCMODE;
            sizer->Add(chkbox, 0, wxALIGN_CENTER_HORIZONTAL, 0);
        }
        mainSizer->Add(dcCalibGroup, 1, wxEXPAND, 5);
    }
    {
        wxFlexGridSizer* rowGroup = new wxFlexGridSizer(0, 4, 0, 5);
        {
            wxFlexGridSizer* rowGroupTSP = new wxFlexGridSizer(0, 1, 0, 0);
            {
                wxStaticBoxSizer* sbSizer139 = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("LDO"));
                sbSizer139->Add(new wxStaticText(sbSizer139->GetStaticBox(), wxID_ANY, wxT("ISINK_SPI_BUFF")), 0, wxALIGN_CENTER_VERTICAL, 5);
                cmbISINK_SPIBUFF = new wxComboBox(sbSizer139->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
                cmbISINK_SPIBUFF->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);

                sbSizer139->Add(cmbISINK_SPIBUFF, 0, wxLEFT, 5);
                const wxString items[] = {_("Off"), _("10 kOhm"), _("2.5 kOhm"), _("2 kOhm"), _("625 Ohm"), _("588 Ohm"), _("500 Ohm"), _("476 Ohm")};
                cmbISINK_SPIBUFF->Append(wxArrayString(sizeof(items) / sizeof(wxString), items));
                rowGroupTSP->Add(sbSizer139, 1, 0, 5);
            }
            {
                wxStaticBoxSizer* txtspSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("TxTSP"));
                txtspSizer->Add(new wxStaticText(txtspSizer->GetStaticBox(), wxID_ANY, wxT("CMIX_GAIN:")), 0, wxALIGN_CENTER_VERTICAL);
                cmbCMIX_GAIN_TXTSP = new wxComboBox(txtspSizer->GetStaticBox(), wxID_ANY);
                cmbCMIX_GAIN_TXTSP->Append(_("0 dB"));
                cmbCMIX_GAIN_TXTSP->Append(_("+6 dB"));
                cmbCMIX_GAIN_TXTSP->Append(_("-6 dB"));
                cmbCMIX_GAIN_TXTSP->Append(_("-6 dB"));
                cmbCMIX_GAIN_TXTSP->Append(_("+3 dB"));
                cmbCMIX_GAIN_TXTSP->Append(_("-3 dB"));
                cmbCMIX_GAIN_TXTSP->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                txtspSizer->Add(cmbCMIX_GAIN_TXTSP, 0, 0, 5);
                rowGroupTSP->Add(txtspSizer, 0, 0, 5);
            }

            {
                wxStaticBoxSizer* rxtspGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("RxTSP"));
                wxFlexGridSizer* rxtspSizer = new wxFlexGridSizer(0, 2, 0, 0);

                wxCheckBox* chkbox = new wxCheckBox(rxtspGroup->GetStaticBox(), wxNewId(), _("DCLOOP_STOP"));
                chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                rxtspSizer->Add(chkbox);
                wndId2Enum[chkbox] = LMS7_DCLOOP_STOP;

                rxtspSizer->Add(new wxFlexGridSizer(0,1,0,0));

                rxtspSizer->Add(new wxStaticText(rxtspGroup->GetStaticBox(), wxID_ANY, wxT("CMIX_GAIN:")), 0, wxALIGN_CENTER_VERTICAL);
                cmbCMIX_GAIN_RXTSP = new wxComboBox(rxtspGroup->GetStaticBox(), wxID_ANY);
                cmbCMIX_GAIN_RXTSP->Append(_("0 dB"));
                cmbCMIX_GAIN_RXTSP->Append(_("+6 dB"));
                cmbCMIX_GAIN_RXTSP->Append(_("-6 dB"));
                cmbCMIX_GAIN_TXTSP->Append(_("-6 dB"));
                cmbCMIX_GAIN_RXTSP->Append(_("+3 dB"));
                cmbCMIX_GAIN_RXTSP->Append(_("-3 dB"));
                cmbCMIX_GAIN_RXTSP->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                rxtspSizer->Add(cmbCMIX_GAIN_RXTSP, 0, 0, 5);

                rxtspSizer->Add(new wxStaticText(rxtspGroup->GetStaticBox(), wxID_ANY, wxT("HBD delay:")), 0, wxALIGN_CENTER_VERTICAL);
                cmbHBD_DLY = new wxComboBox(rxtspGroup->GetStaticBox(), wxID_ANY, wxT("No delay"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
                cmbHBD_DLY->Append(wxT("No delay"));
                cmbHBD_DLY->Append(wxT("Delay 1 clock cycle"));
                cmbHBD_DLY->Append(wxT("Delay 2 clock cycles"));
                cmbHBD_DLY->Append(wxT("Delay 3 clock cycles"));
                cmbHBD_DLY->Append(wxT("Delay 4 clock cycles"));
                cmbHBD_DLY->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                rxtspSizer->Add(cmbHBD_DLY, 1, wxEXPAND, 5);
                rxtspGroup->Add(rxtspSizer, 1, wxEXPAND, 5);

                rxtspSizer->Add(new wxStaticText(rxtspGroup->GetStaticBox(), wxID_ANY, wxT("ADCI_o:")), 0, wxALIGN_CENTER_VERTICAL);
                lblADCI_o = new wxStaticText(rxtspGroup->GetStaticBox(), wxID_ANY, wxT("???"));
                rxtspSizer->Add(lblADCI_o, 0, wxALIGN_CENTER_VERTICAL);
                rxtspSizer->Add(new wxStaticText(rxtspGroup->GetStaticBox(), wxID_ANY, wxT("ADCQ_o:")), 0, wxALIGN_CENTER_VERTICAL);
                lblADCQ_o = new wxStaticText(rxtspGroup->GetStaticBox(), wxID_ANY, wxT("???"));
                rxtspSizer->Add(lblADCQ_o, 0, wxALIGN_CENTER_VERTICAL);
                btnReadADC = new wxButton(rxtspGroup->GetStaticBox(), wxNewId(), _("Read"));
                rxtspSizer->Add(btnReadADC, 0, wxALIGN_CENTER_VERTICAL);
                btnReadADC->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::OnReadADC), NULL, this);

                chkbox = new wxCheckBox(rxtspGroup->GetStaticBox(), wxNewId(), _("CAPSEL_ADC"));
                chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                rxtspSizer->Add(chkbox);
                wndId2Enum[chkbox] = LMS7_CAPSEL_ADC;

                rowGroupTSP->Add(rxtspGroup, 1, wxEXPAND, 5);
            }
        rowGroup->Add(rowGroupTSP, 1, wxEXPAND, 5);
        }

        {
            wxStaticBoxSizer* RSSIPDETGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("RSSI, PDET, TEMP"));
            wxWindow* panel = RSSIPDETGroup->GetStaticBox();

            wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 2, 0, 5);

            wxCheckBox* chkbox = new wxCheckBox(panel, wxNewId(), _("Power down modules"));
            chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(chkbox);
            wndId2Enum[chkbox] = LMS7_RSSI_PD;

            chkbox = new wxCheckBox(panel, wxNewId(), _("Manual operation mode"));
            chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(chkbox);
            wndId2Enum[chkbox] = LMS7_RSSI_RSSIMODE;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("DAC_CLKDIV")), 1, wxEXPAND, 0);
            NumericSlider* spnCtrl = new NumericSlider(panel, wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0);
            spnCtrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(spnCtrl);
            wndId2Enum[spnCtrl] = LMS7_DAC_CLKDIV;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("Reference bias current to test ADC")), 1, wxEXPAND, 0);
            spnCtrl = new NumericSlider(panel, wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0);
            spnCtrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(spnCtrl);
            wndId2Enum[spnCtrl] = LMS7_RSSI_BIAS;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("HYSCMP")), 1, wxEXPAND, 0);
            spnCtrl = new NumericSlider(panel, wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 0);
            spnCtrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(spnCtrl);
            wndId2Enum[spnCtrl] = LMS7_RSSI_HYSCMP;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("DAC_VAL")), 1, wxEXPAND, 0);
            spnCtrl = new NumericSlider(panel, wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0);
            spnCtrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
            sizer->Add(spnCtrl);
            wndId2Enum[spnCtrl] = LMS7_RSSI_DAC_VAL;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("PDET2_VAL")), 1, wxEXPAND, 0);
            pdet_vals[1] = new wxStaticText(panel, wxID_ANY, _("????"));
            sizer->Add(pdet_vals[1]);
            wndId2Enum[pdet_vals[1]] = LMS7_RSSI_PDET2_VAL;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("PDET1_VAL")), 1, wxEXPAND, 0);
            pdet_vals[0] = new wxStaticText(panel, wxID_ANY, _("????"));
            sizer->Add(pdet_vals[0]);
            wndId2Enum[pdet_vals[0]] = LMS7_RSSI_PDET1_VAL;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("RSSI2_VAL")), 1, wxEXPAND, 0);
            rssi_vals[1] = new wxStaticText(panel, wxID_ANY, _("????"));
            sizer->Add(rssi_vals[1]);
            wndId2Enum[rssi_vals[1]] = LMS7_INTADC_CMPSTATUS_RSSI2;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("RSSI1_VAL")), 1, wxEXPAND, 0);
            rssi_vals[0] = new wxStaticText(panel, wxID_ANY, _("????"));
            sizer->Add(rssi_vals[0]);
            wndId2Enum[rssi_vals[0]] = LMS7_INTADC_CMPSTATUS_RSSI1;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("TREF_VAL")), 1, wxEXPAND, 0);
            tref_val = new wxStaticText(panel, wxID_ANY, _("????"));
            sizer->Add(tref_val);
            wndId2Enum[tref_val] = LMS7_INTADC_CMPSTATUS_TEMPREF;

            sizer->Add(new wxStaticText(panel, wxID_ANY, _("TVPTAT_VAL")), 1, wxEXPAND, 0);
            tvptat_val = new wxStaticText(panel, wxID_ANY, _("????"));
            wndId2Enum[tvptat_val] = LMS7_INTADC_CMPSTATUS_TEMPVPTAT;
            sizer->Add(tvptat_val);

            RSSIPDETGroup->Add(sizer);

            wxFlexGridSizer* sizerCMP = new wxFlexGridSizer(0, 2, 0, 5);
            std::vector<LMS7Parameter> paramStatus= {LMS7_INTADC_CMPSTATUS_TEMPREF,
LMS7_INTADC_CMPSTATUS_TEMPVPTAT,
LMS7_INTADC_CMPSTATUS_RSSI2,
LMS7_INTADC_CMPSTATUS_RSSI1,
LMS7_INTADC_CMPSTATUS_PDET2,
LMS7_INTADC_CMPSTATUS_PDET1};
std::vector<LMS7Parameter> params= {LMS7_INTADC_CMPCFG_TEMPREF,
LMS7_INTADC_CMPCFG_TEMPVPTAT,
LMS7_INTADC_CMPCFG_RSSI2,
LMS7_INTADC_CMPCFG_RSSI1,
LMS7_INTADC_CMPCFG_PDET2,
LMS7_INTADC_CMPCFG_PDET1};
            sizerCMP->Add( new wxStaticText(panel, wxID_ANY, _("Invert:")));
            sizerCMP->Add(new wxStaticText(panel, wxID_ANY, _("CMP:")));

            for(int i = 0; i < 6; ++i)
            {
                rssiCMPCFG[i] = new wxCheckBox(panel, wxNewId(), wxString::Format("%s", params[i].name));
                sizerCMP->Add(rssiCMPCFG[i]);
                wndId2Enum[rssiCMPCFG[i]] = params[i];
                rssiCMPCFG[i]->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
                rssiCMPSTATUS[i] = new wxStaticText(panel, wxNewId(), _("?"));
                sizerCMP->Add(rssiCMPSTATUS[i]);
                wndId2Enum[rssiCMPSTATUS[i]] = paramStatus[i];
            }
            sizerCMP->Add(new wxFlexGridSizer(0, 0, 0, 0));
            wxButton* btnUpdateRSSICMP = new wxButton(panel, wxNewId(), _("Read"));
            btnUpdateRSSICMP->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::OnReadRSSICMP), NULL, this);
            sizerCMP->Add(btnUpdateRSSICMP);
            RSSIPDETGroup->Add(sizerCMP);

            rowGroup->Add(RSSIPDETGroup);
        }
    {
        wxStaticBoxSizer* RSSIGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("RSSI DC Calibration"));
        wxWindow* panel = RSSIGroup->GetStaticBox();

        wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 2, 0, 5);

        sizer->Add(new wxFlexGridSizer(0, 1, 0, 0), 0, 0, 0);

        wxCheckBox* chkbox = new wxCheckBox(panel, wxNewId(), _("Power down modules"));
        chkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
        sizer->Add(chkbox);
        wndId2Enum[chkbox] = LMS7_RSSIDC_PD;
        chkRSSI_PD = chkbox;

        sizer->Add(new wxStaticText(panel, wxID_ANY, _("HYSCMP")), 1, wxEXPAND, 0);
        NumericSlider* spnCtrl = new NumericSlider(panel, wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 7, 0);
        cmbRSSIDC_HYSCMP = spnCtrl;
        spnCtrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandler), NULL, this);
        sizer->Add(spnCtrl);
        wndId2Enum[spnCtrl] = LMS7_RSSIDC_HYSCMP;

        sizer->Add(new wxStaticText(panel, wxID_ANY, _("DCO2")), 1, wxEXPAND, 0);
        spnCtrl = new NumericSlider(panel, wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 127, 0);
        spinDCO2 = spnCtrl;
        spnCtrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandlerCMPRead), NULL, this);
        sizer->Add(spnCtrl);
        wndId2Enum[spnCtrl] = LMS7_RSSIDC_DCO2;

        sizer->Add(new wxStaticText(panel, wxID_ANY, _("DCO1")), 1, wxEXPAND, 0);
        spnCtrl = new NumericSlider(panel, wxNewId(), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 127, 0);
        spinDCO1 = spnCtrl;
        spnCtrl->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandlerCMPRead), NULL, this);
        sizer->Add(spnCtrl);
        wndId2Enum[spnCtrl] = LMS7_RSSIDC_DCO1;

        wxArrayString rselArray;
        float value_mV = 800;
        int i = 0;
        for(; i < 4; ++i)
        {
            rselArray.push_back(wxString::Format("%.1f mV", value_mV));
            value_mV -= 50;
        }
        for(; i < 12; ++i)
        {
            rselArray.push_back(wxString::Format("%.1f mV", value_mV));
            value_mV -= 21.5;
        }
        for(; i <= 31; ++i)
        {
            rselArray.push_back(wxString::Format("%.1f mV", value_mV));
            value_mV -= 10;
        }
        sizer->Add(new wxStaticText(panel, wxID_ANY, _("RSEL:")), 1, wxEXPAND, 0);
        cmbRSEL = new wxComboBox(panel, wxID_ANY);
        cmbRSEL->Append(rselArray);
        cmbRSEL->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlR3_view::ParameterChangeHandlerCMPRead), NULL, this);
        sizer->Add(cmbRSEL);
        wndId2Enum[cmbRSEL] = LMS7_RSSIDC_RSEL;

        sizer->Add(new wxStaticText(panel, wxID_ANY, _("CMPSTATUS")), 1, wxEXPAND, 0);
        rssidc_cmpstatus = new wxStaticText(panel, wxID_ANY, _("?"));
        sizer->Add(rssidc_cmpstatus);

        wxButton* calibrateAnalogRSSIDC = new wxButton(panel, wxNewId(), _("Calibrate Analog RSSI"));
        calibrateAnalogRSSIDC->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(lms7002_pnlR3_view::OnCalibrateAnalogRSSI), NULL, this);
        sizer->Add(calibrateAnalogRSSIDC);

        RSSIGroup->Add(sizer);
        rowGroup->Add(RSSIGroup);
    }
    mainSizer->Add(rowGroup);
    }



	this->SetSizer( mainSizer );
	this->Layout();
	mainSizer->Fit( this );

	// Connect Events
	chkTRX_GAIN_SRC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinCG_IAMP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinCG_IAMP_TBB->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbG_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbG_LNA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbG_TIA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );

	wndId2Enum[cmbFCLK1_DLY] = LMS7param(FCLK1_DLY);
	wndId2Enum[cmbFCLK2_DLY] = LMS7param(FCLK2_DLY);
	wndId2Enum[cmbMCLK1_DLY] = LMS7param(MCLK1_DLY);
	wndId2Enum[cmbMCLK2_DLY] = LMS7param(MCLK2_DLY);
	wndId2Enum[chkMCLK1_INV] = LMS7param(MCLK1_INV);
	wndId2Enum[chkMCLK2_INV] = LMS7param(MCLK2_INV);
	wndId2Enum[chkLML1_TRXIQPULSE] = LMS7param(LML1_TRXIQPULSE);
	wndId2Enum[chkLML2_TRXIQPULSE] = LMS7param(LML2_TRXIQPULSE);
	wndId2Enum[chkLML1_SISODDR] = LMS7param(LML1_SISODDR);
	wndId2Enum[chkLML2_SISODDR] = LMS7param(LML2_SISODDR);
	wndId2Enum[cmbHBD_DLY] = LMS7param(HBD_DLY);
	wndId2Enum[cmbISINK_SPIBUFF] = LMS7param(ISINK_SPIBUFF);
	wndId2Enum[chkTRX_GAIN_SRC] = LMS7param(TRX_GAIN_SRC);
	wndId2Enum[spinCG_IAMP_TBB] = LMS7param(CG_IAMP_TBB_R3);
	wndId2Enum[spinLOSS_LIN_TXPAD_TRF] = LMS7param(LOSS_LIN_TXPAD_R3);
	wndId2Enum[spinLOSS_MAIN_TXPAD_TRF] = LMS7param(LOSS_MAIN_TXPAD_R3);
	wndId2Enum[chkR5_LPF_BYP_TBB] = LMS7param(R5_LPF_BYP_TBB);
	wndId2Enum[spinC_CTL_PGA_RBB] = LMS7param(C_CTL_PGA_RBB_R3);
	wndId2Enum[cmbG_PGA_RBB] = LMS7param(G_PGA_RBB_R3);
	wndId2Enum[cmbG_LNA_RFE] = LMS7param(G_LNA_RFE_R3);
	wndId2Enum[cmbG_TIA_RFE] = LMS7param(G_TIA_RFE_R3);
	wndId2Enum[cmbCMIX_GAIN_TXTSP] = LMS7param(CMIX_GAIN_TXTSP_R3);
    wndId2Enum[cmbCMIX_GAIN_RXTSP] = LMS7param(CMIX_GAIN_RXTSP_R3);



    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

lms7002_pnlR3_view::~lms7002_pnlR3_view()
{
    // Disconnect Events
	cmbFCLK1_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbFCLK2_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbMCLK1_DLY->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	chkLML1_TRXIQPULSE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	chkLML2_TRXIQPULSE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	chkLML1_SISODDR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	chkLML2_SISODDR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbCMIX_GAIN_TXTSP->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	chkTRX_GAIN_SRC->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinCG_IAMP_TBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinCG_IAMP_TBB->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_LIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_LIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_MAIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinLOSS_MAIN_TXPAD_TRF->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinC_CTL_PGA_RBB->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	spinC_CTL_PGA_RBB->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbG_PGA_RBB->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbG_LNA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
	cmbG_TIA_RFE->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlR3_view::ParameterChangeHandler ), NULL, this );
}

void lms7002_pnlR3_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
}

void lms7002_pnlR3_view::UpdateGUI()
{
    LMS_Synchronize(lmsControl, false);
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);

    wxCommandEvent evt;
    OnReadADC(evt);

    for(size_t i = 0; i<cmbDCControlsRx.size(); ++i)
    {
        uint16_t value = 0;
        auto parameter = wndId2Enum[cmbDCControlsRx[i]];
        LMS_WriteLMSReg(lmsControl, parameter.address, 0);
        LMS_WriteLMSReg(lmsControl, parameter.address, 0x4000);
        LMS_ReadParam(lmsControl, wndId2Enum[cmbDCControlsRx[i]], &value);
        LMS_WriteLMSReg(lmsControl, parameter.address, value & ~0xC000);
        int absval = (value & 0x3F);
        if(value&0x40)
            absval *= -1;
        cmbDCControlsRx[i]->SetValue(absval);
    }
    for(size_t i = 0; i<cmbDCControlsTx.size(); ++i)
    {
        uint16_t value = 0;
        auto parameter = wndId2Enum[cmbDCControlsTx[i]];
        LMS_WriteLMSReg(lmsControl, parameter.address, 0);
        LMS_WriteLMSReg(lmsControl, parameter.address, 0x4000);
        LMS_ReadParam(lmsControl, wndId2Enum[cmbDCControlsTx[i]], &value);
        LMS_WriteLMSReg(lmsControl, parameter.address, value & ~0xC000);
        int absval = (value & 0x3FF);
        if(value&0x400)
            absval *= -1;
        cmbDCControlsTx[i]->SetValue(absval);
    }

    uint16_t value;
    uint16_t rxtsp_cmix_gain = 0;
    LMS_ReadParam(lmsControl, LMS7_CMIX_GAIN_RXTSP, &value);
    rxtsp_cmix_gain |= value;
    LMS_ReadParam(lmsControl, LMS7_CMIX_GAIN_RXTSP_R3, &value);
    rxtsp_cmix_gain |= value << 2;
    cmbCMIX_GAIN_RXTSP->SetSelection(rxtsp_cmix_gain);

    uint16_t txtsp_cmix_gain = 0;
    LMS_ReadParam(lmsControl, LMS7_CMIX_GAIN_TXTSP, &value);
    txtsp_cmix_gain |= value;
    LMS_ReadParam(lmsControl, LMS7_CMIX_GAIN_TXTSP_R3, &value);
    txtsp_cmix_gain |= value << 2;
    cmbCMIX_GAIN_TXTSP->SetSelection(txtsp_cmix_gain);

    UpdateGUISlow();
}

void lms7002_pnlR3_view::MCU_RunProcedure(uint8_t id)
{
    uint16_t temp;
    LMS_ReadLMSReg(lmsControl, 0x0002, &temp);

    const uint16_t x0002reg = temp & 0xFF;
    const uint16_t interupt6 = 0x0008;
    const uint16_t addrs[5] = {0x0006, 0x0, 0x0002, 0x0002, 0x0002};
    const uint16_t values[5] = {
        (uint16_t)(id != 0),
        (uint16_t)(id),
        (uint16_t)(x0002reg & ~interupt6),
        (uint16_t)(x0002reg | interupt6),
        (uint16_t)(x0002reg & ~interupt6)};
    for(int i=0; i<5; ++i)
        LMS_WriteLMSReg(lmsControl, addrs[i], values[i]);
}

uint8_t lms7002_pnlR3_view::MCU_WaitForStatus(uint16_t timeout_ms)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    uint16_t value = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    do {
        //value = SPI_read(0x0001) & 0xFF;
        LMS_ReadLMSReg(lmsControl, 0x0001, &value);
        value &= 0xFF;
        if (value != 0xFF) //working
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        t2 = std::chrono::high_resolution_clock::now();
    }while (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms);
    LMS_WriteLMSReg(lmsControl, 0x0006, 0);
    //SPI_write(0x0006, 0); //return SPI control to PC
    return value & 0x7F;
}

void lms7002_pnlR3_view::ParameterChangeHandler(wxCommandEvent& event)
{
    assert(lmsControl != nullptr);
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
        if(parameter.address == LMS7_CMIX_GAIN_RXTSP_R3.address || parameter.name == LMS7_CMIX_GAIN_RXTSP_R3.name)
        {
            uint16_t value = event.GetInt();
            LMS_WriteParam(lmsControl, LMS7_CMIX_GAIN_RXTSP, value & 0x3);
            LMS_WriteParam(lmsControl, LMS7_CMIX_GAIN_RXTSP_R3, value > 0x2);
            return;
        }
        if(parameter.address == LMS7_CMIX_GAIN_TXTSP_R3.address || parameter.name == LMS7_CMIX_GAIN_TXTSP_R3.name)
        {
            uint16_t value = event.GetInt();
            LMS_WriteParam(lmsControl, LMS7_CMIX_GAIN_TXTSP, value & 0x3);
            LMS_WriteParam(lmsControl, LMS7_CMIX_GAIN_TXTSP_R3, value > 0x2);
            return;
        }
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }

    if(parameter.address == 0x0640 || parameter.address == 0x0641)
    {
        MCU_RunProcedure(MCU_FUNCTION_GET_PROGRAM_ID);
        if(MCU_WaitForStatus(100) != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
            LMS_ProgramLMSMCU(lmsControl, (const char*)mcu_program_lms7_dc_iq_calibration_bin, sizeof(mcu_program_lms7_dc_iq_calibration_bin), LMS_TARGET_RAM, nullptr);

        //run mcu write
        LMS_WriteLMSReg(lmsControl, 0x002D, parameter.address);
        uint16_t wrVal = 0;

        //read reg
        MCU_RunProcedure(8);
        uint16_t rdVal = 0;
        MCU_WaitForStatus(100);
        LMS_ReadLMSReg(lmsControl, 0x040B, &rdVal);

        uint16_t mask = (~0) << (parameter.msb-parameter.lsb+1);
        mask = ~mask;
        mask <<= parameter.lsb;
        rdVal &= ~mask;
        wrVal = rdVal | (event.GetInt() << parameter.lsb);

        LMS_WriteLMSReg(lmsControl, 0x020C, wrVal);
        MCU_RunProcedure(7);
        MCU_WaitForStatus(100);

        //check if correct
        MCU_RunProcedure(8);

        MCU_WaitForStatus(100);
        LMS_ReadLMSReg(lmsControl, 0x040B, &rdVal);
        if(rdVal != wrVal)
        {
            printf("Reg 0x%04X value mismatch, written 0x%04X, got 0x%04X\n",
                   parameter.address, wrVal, rdVal);
        }
    }
    else
        LMS_WriteParam(lmsControl,parameter,event.GetInt());
}

void lms7002_pnlR3_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlR3_view::OnReadADC( wxCommandEvent& event )
{
    //LMS_WriteParam(lmsControl, LMS7param(CAPSEL_ADC), 1);
    LMS_WriteParam(lmsControl, LMS7param(CAPTURE), 0);
    LMS_WriteParam(lmsControl, LMS7param(CAPTURE), 1);

    uint16_t adci = 0;
    LMS_ReadLMSReg(lmsControl, 0x040E, &adci);
    uint16_t adcq = 0;
    LMS_ReadLMSReg(lmsControl, 0x040F, &adcq);

    lblADCI_o->SetLabel(wxString::Format("0x%04X", adci));
    lblADCQ_o->SetLabel(wxString::Format("0x%04X", adcq));

    //LMS_WriteParam(lmsControl, LMS7param(CAPSEL_ADC), 0);
}


void lms7002_pnlR3_view::OnDCCMPCFGRead()
{
    UpdateGUI();
}

void lms7002_pnlR3_view::OnReadRSSICMP(wxCommandEvent& event)
{
    UpdateGUI();
}

void lms7002_pnlR3_view::OnReadDCCMP(wxCommandEvent& event)
{
    UpdateGUI();
}

void lms7002_pnlR3_view::OnWriteTxDC(wxCommandEvent& event)
{
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
        uint16_t regVal = 0;
        LMS_ReadLMSReg(lmsControl, parameter.address, &regVal);
        regVal &= 0xF800;
        int dcVal = event.GetInt();
        if(dcVal < 0)
        {
            --dcVal;
            regVal |= 0x0400;
        }
        regVal |= (abs(dcVal+0x400) & 0x3FF);
        LMS_WriteLMSReg(lmsControl, parameter.address, regVal);
        LMS_WriteLMSReg(lmsControl, parameter.address, regVal | 0x8000);
        LMS_WriteLMSReg(lmsControl, parameter.address, regVal);
        return;
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }
}

void lms7002_pnlR3_view::OnWriteRxDC(wxCommandEvent& event)
{
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
        uint16_t regVal = 0;
        LMS_ReadLMSReg(lmsControl, parameter.address, &regVal);
        regVal &= 0xFF80;
        int dcVal = event.GetInt();
        if(dcVal < 0)
        {
            --dcVal;
            regVal |= 0x0040;
        }
        regVal |= (abs(dcVal+0x40) & 0x3F);
        LMS_WriteLMSReg(lmsControl, parameter.address, regVal & ~0x8000);
        LMS_WriteLMSReg(lmsControl, parameter.address, regVal | 0x8000);
        LMS_WriteLMSReg(lmsControl, parameter.address, regVal);
        return;
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }
}

void lms7002_pnlR3_view::OnReadDC( wxCommandEvent& event )
{
    UpdateGUI();
}

void lms7002_pnlR3_view::ParameterChangeHandlerCMPRead( wxCommandEvent& event )
{
    ParameterChangeHandler(event);
    uint16_t value = 0;
    //LMS_ReadParam(lmsControl, LMS7_RSSIDC_CMPSTATUS, &value);
    UpdateGUISlow();
    //rssidc_cmpstatus->SetLabel(wxString::Format("%i", value & 1));
}

void lms7002_pnlR3_view::UpdateGUISlow()
{
    vector<uint16_t> addrs = {0x0640, 0x0641};
    vector<uint16_t> rez;
    for(auto i : addrs)
    {
        LMS_WriteLMSReg(lmsControl, 0x002D, i);
        uint16_t value = 0;
        MCU_RunProcedure(8);
        MCU_WaitForStatus(100);
        LMS_ReadLMSReg(lmsControl, 0x040B, &value);
        rez.push_back(value);
    }
    rssidc_cmpstatus->SetLabel(wxString::Format("%i", (rez[0]>>15) ));
    cmbRSEL->SetSelection((rez[0] >> 4) & 0x1F);
    cmbRSSIDC_HYSCMP->SetValue((rez[0] >> 1) & 0x7);
    chkRSSI_PD->SetValue(rez[0] & 0x1);
    spinDCO2->SetValue((rez[1] >> 7) & 0x7F);
    spinDCO1->SetValue((rez[1] >> 0) & 0x7F);

}

void lms7002_pnlR3_view::OnCalibrateAnalogRSSI( wxCommandEvent& event )
{
    LMS_CalibrateAnalogRSSIDC(lmsControl);
    UpdateGUI();
}
