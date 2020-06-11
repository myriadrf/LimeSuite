#include "lms7002_pnlCLKGEN_view.h"
#include <map>
#include <wx/msgdlg.h>
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include "lms7002_dlgVCOfrequencies.h"
#include "lms7_device.h"
#include "FPGA_common.h"
#include "Logger.h"
using namespace lime;

lms7002_pnlCLKGEN_view::lms7002_pnlCLKGEN_view( wxWindow* parent )
:
pnlCLKGEN_view( parent )
{

}

lms7002_pnlCLKGEN_view::lms7002_pnlCLKGEN_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlCLKGEN_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    sizerR3->Add(new wxStaticText(this, wxID_ANY, _("CMPLO_CTRL:")), 1, wxEXPAND, 0);
    cmbCMPLO_CTRL = new wxComboBox(this, wxID_ANY);
    cmbCMPLO_CTRL->Append(_("Low threshold is set to 0.18V"));
    cmbCMPLO_CTRL->Append(_("Low threshold is set to 0.1V"));
    cmbCMPLO_CTRL->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlCLKGEN_view::ParameterChangeHandler), NULL, this);
    sizerR3->Add(cmbCMPLO_CTRL, 0, 0, 5);
    wndId2Enum[cmbCMPLO_CTRL] = LMS7_CMPLO_CTRL_CGEN;

    wndId2Enum[cmbCLKH_OV_CLKL_CGEN] = LMS7param(CLKH_OV_CLKL_CGEN);
    wndId2Enum[chkCOARSE_START_CGEN] = LMS7param(COARSE_START_CGEN);
    wndId2Enum[cmbCP2_CGEN] = LMS7param(CP2_CGEN);
    wndId2Enum[cmbCP3_CGEN] = LMS7param(CP3_CGEN);
    wndId2Enum[cmbCSW_VCO_CGEN] = LMS7param(CSW_VCO_CGEN);
    wndId2Enum[cmbCZ_CGEN] = LMS7param(CZ_CGEN);
    wndId2Enum[chkEN_COARSE_CKLGEN] = LMS7param(EN_COARSE_CKLGEN);
    wndId2Enum[cmbEN_ADCCLKH_CLKGN] = LMS7param(EN_ADCCLKH_CLKGN);
    wndId2Enum[chkEN_G_CGEN] = LMS7param(EN_G_CGEN);
    wndId2Enum[chkEN_INTONLY_SDM_CGEN] = LMS7param(EN_INTONLY_SDM_CGEN);
    wndId2Enum[chkEN_SDM_CLK_CGEN] = LMS7param(EN_SDM_CLK_CGEN);
    wndId2Enum[cmbICT_VCO_CGEN] = LMS7param(ICT_VCO_CGEN);
    wndId2Enum[lblINT_SDM_CGEN] = LMS7param(INT_SDM_CGEN);
    wndId2Enum[cmbIOFFSET_CP_CGEN] = LMS7param(IOFFSET_CP_CGEN);
    wndId2Enum[cmbIPULSE_CP_CGEN] = LMS7param(IPULSE_CP_CGEN);
    wndId2Enum[chkPD_CP_CGEN] = LMS7param(PD_CP_CGEN);
    wndId2Enum[chkPD_FDIV_FB_CGEN] = LMS7param(PD_FDIV_FB_CGEN);
    wndId2Enum[chkPD_FDIV_O_CGEN] = LMS7param(PD_FDIV_O_CGEN);
    wndId2Enum[chkPD_SDM_CGEN] = LMS7param(PD_SDM_CGEN);
    wndId2Enum[chkPD_VCO_CGEN] = LMS7param(PD_VCO_CGEN);
    wndId2Enum[chkPD_VCO_COMP_CGEN] = LMS7param(PD_VCO_COMP_CGEN);
    wndId2Enum[chkRESET_N_CGEN] = LMS7param(RESET_N_CGEN);
    wndId2Enum[chkREVPH_PFD_CGEN] = LMS7param(REVPH_PFD_CGEN);
    wndId2Enum[chkREV_CLKADC_CGEN] = LMS7param(REV_CLKADC_CGEN);
    wndId2Enum[chkREV_CLKDAC_CGEN] = LMS7param(REV_CLKDAC_CGEN);
    wndId2Enum[chkREV_SDMCLK_CGEN] = LMS7param(REV_SDMCLK_CGEN);
    wndId2Enum[rgrSEL_SDMCLK_CGEN] = LMS7param(SEL_SDMCLK_CGEN);
    wndId2Enum[chkSPDUP_VCO_CGEN] = LMS7param(SPDUP_VCO_CGEN);
    wndId2Enum[chkSX_DITHER_EN_CGEN] = LMS7param(SX_DITHER_EN_CGEN);
    wndId2Enum[cmbTST_CGEN] = LMS7param(TST_CGEN);

    wxArrayString temp;
    temp.clear();
    temp.push_back(_("1"));
    temp.push_back(_("2"));
    temp.push_back(_("4"));
    temp.push_back(_("8"));
    cmbCLKH_OV_CLKL_CGEN->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        temp.push_back(wxString::Format(_("%.3f pF"), (i * 6 * 63.2) / 1000.0));
    }
    cmbCP2_CGEN->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        temp.push_back(wxString::Format(_("%.3f pF"), (i*248.0) / 1000.0));
    }
    cmbCP3_CGEN->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        temp.push_back(wxString::Format(_("%.0f pF"), (i * 8 * 365.0) / 1000.0));
    }
    cmbCZ_CGEN->Set(temp);

    temp.clear();
    temp.push_back(_("TST disabled"));
    temp.push_back(_("TST[0]=ADC clk; TST[1]=DAC clk; TSTA=Hi Z"));
    temp.push_back(_("TST[0]=SDM clk; TST[1]=FBD output; TSTA=Vtune@60kOhm"));
    temp.push_back(_("TST[0]=Ref clk; TST[1]=FBD output; TSTA=Vtune@10kOhm"));
    temp.push_back(_("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Hi Z"));
    temp.push_back(_("TST[0]=CP Down offset; TST[1]=CP Up offset; TSTA=Hi Z"));
    temp.push_back(_("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Vtune@60kOhm"));
    temp.push_back(_("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Vtune@10kOhm"));
    cmbTST_CGEN->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlCLKGEN_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    double freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_CGEN,&freq);
    txtFrequency->SetValue(wxString::Format(_("%.3f"), freq));
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"), freq));
}

void lms7002_pnlCLKGEN_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlCLKGEN_view::ParameterChangeHandler(wxCommandEvent& event)
{
    assert(lmsControl != nullptr);
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }

    LMS_WriteParam(lmsControl,parameter,event.GetInt());

    if(event.GetEventObject() == cmbEN_ADCCLKH_CLKGN)
        UpdateInterfaceFrequencies();
    else if(event.GetEventObject() == cmbCSW_VCO_CGEN)
    {
        OnbtnReadComparators(event);
    }
    else if(event.GetEventObject() == cmbCLKH_OV_CLKL_CGEN)
    {
        double cgenFreq;
        txtFrequency->GetValue().ToDouble(&cgenFreq);
        txtFrequencyCLKL->SetValue(wxString::Format(_("%.3f"), cgenFreq / pow(2.0, cmbCLKH_OV_CLKL_CGEN->GetSelection())));
        UpdateInterfaceFrequencies();
        UpdateCLKL();
    }
}

 void lms7002_pnlCLKGEN_view::OnAutoPhase(wxCommandEvent& event)
 {
     bool disabled = this->chkAutoPhase->GetValue();
     this->txPhase->Enable(!disabled);
     this->rxPhase->Enable(!disabled);
 }

void lms7002_pnlCLKGEN_view::onbtnCalculateClick(wxSpinEvent& event)
{
    double freqMHz;
    auto device = ((LMS7_Device*)lmsControl);
    txtFrequency->GetValue().ToDouble(&freqMHz);
    LMS7002M* lms = device->GetLMS();
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
    int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    if (lms->SetInterfaceFrequency(freqMHz*1e6, interp, decim))
    {
        wxMessageBox(_("CLKGEN: failed to set interface frequency"));
        return;
    }
    device->SetFPGAInterfaceFreq(interp, decim, txPhase->GetValue(), rxPhase->GetValue());

    auto freq = lms->GetFrequencyCGEN();
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
    UpdateGUI();
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    wxCommandEvent cmd;
    cmd.SetString(_("CGEN frequency set to ") + lblRealOutFrequency->GetLabel() + _(" MHz"));
    cmd.SetEventType(LOG_MESSAGE);
    cmd.SetInt(lime::LOG_LEVEL_INFO);
    wxPostEvent(this, cmd);
}

void lms7002_pnlCLKGEN_view::onbtnCalculateClick( wxCommandEvent& event )
{
    double freqMHz;
    auto device = ((LMS7_Device*)lmsControl);
    txtFrequency->GetValue().ToDouble(&freqMHz);
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
    int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    if (lms->SetInterfaceFrequency(freqMHz*1e6, interp, decim))
    {
        wxMessageBox(_("CLKGEN: failed to set interface frequency"));
        auto freq = lms->GetFrequencyCGEN();
        lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
        UpdateGUI();
        return ;
    }

    int status;
    if (this->chkAutoPhase->GetValue())
        status = device->SetFPGAInterfaceFreq(interp, decim);
    else
        status = device->SetFPGAInterfaceFreq(interp, decim, txPhase->GetValue(), rxPhase->GetValue());
    if (status != 0)
        wxMessageBox(_("CLKGEN: failed to set interface frequency"));

    auto freq = lms->GetFrequencyCGEN();
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
    UpdateGUI();
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    wxCommandEvent cmd;
    cmd.SetString(_("CGEN frequency set to ") + lblRealOutFrequency->GetLabel() + _(" MHz"));
    cmd.SetEventType(LOG_MESSAGE);
    cmd.SetInt(lime::LOG_LEVEL_INFO);
    wxPostEvent(this, cmd);
}

void lms7002_pnlCLKGEN_view::onbtnTuneClick( wxCommandEvent& event )
{
    auto device = ((LMS7_Device*)lmsControl);
    LMS7002M* lms = device->GetLMS();
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
    if (lms->TuneVCO(lime::LMS7002M::VCO_CGEN)!=0)
    {
        wxMessageBox(wxString(_("CLKGEN VCO Tune failed")));
        return ;
    }

    int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));

    int status;
    if (this->chkAutoPhase->GetValue())
        status = device->SetFPGAInterfaceFreq(interp, decim);
    else
        status = device->SetFPGAInterfaceFreq(interp, decim, txPhase->GetValue(), rxPhase->GetValue());
    if (status != 0)
        wxMessageBox(_("CLKGEN VCO Tune: failed to set interface frequency"));

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(CSW_VCO_CGEN),&value);
    cmbCSW_VCO_CGEN->SetValue(value);
    OnbtnReadComparators(event);
}

void lms7002_pnlCLKGEN_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    wxCommandEvent evt;
    OnbtnReadComparators(evt);
    UpdateInterfaceFrequencies();
    UpdateCLKL();
    double freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_CGEN,&freq);
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
    txtFrequency->SetValue(wxString::Format(_("%.3f"), freq / 1e6));
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"),freq / 1e6 ));
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(FRAC_SDM_CGEN_MSB),&value);
    int fracValue = value << 16;
    LMS_ReadParam(lmsControl,LMS7param(FRAC_SDM_CGEN_LSB),&value);
    fracValue |= value;
    lblFRAC_SDM_CGEN->SetLabel(wxString::Format("%i", fracValue));
    LMS_ReadParam(lmsControl,LMS7param(DIV_OUTCH_CGEN),&value);
    lblDivider->SetLabel(wxString::Format("2*%i", value+1));
}

void lms7002_pnlCLKGEN_view::UpdateInterfaceFrequencies()
{
    double freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_RXTSP,&freq);
    lblRxTSPfreq->SetLabel(wxString::Format(_("%.3f"), freq / 1e6));
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_TXTSP,&freq);
    lblTxTSPfreq->SetLabel(wxString::Format(_("%.3f"), freq / 1e6));
}

void lms7002_pnlCLKGEN_view::OnbtnReadComparators(wxCommandEvent& event)
{

    uint16_t param;
    LMS_ReadParam(lmsControl,LMS7param(VCO_CMPHO_CGEN),&param);

    lblVCO_CMPHO_CGEN->SetLabel(wxString::Format(_("%i"), param));
    if (param == 1)
        lblVCO_CMPHO_CGEN->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPHO_CGEN->SetBackgroundColour(*wxRED);

    LMS_ReadParam(lmsControl,LMS7param(VCO_CMPLO_CGEN),&param);

    lblVCO_CMPLO_CGEN->SetLabel(wxString::Format(_("%i"), param));
    if (param == 0)
        lblVCO_CMPLO_CGEN->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPLO_CGEN->SetBackgroundColour(*wxRED);
}

void lms7002_pnlCLKGEN_view::OnbtnUpdateCoarse(wxCommandEvent& event)
{
    uint16_t param;
    LMS_ReadParam(lmsControl,LMS7param(COARSE_STEPDONE),&param);
    lblCOARSE_STEPDONE_CGEN->SetLabel(wxString::Format(_("%i"), param));
    LMS_ReadParam(lmsControl,LMS7param(COARSEPLL_COMPO_CGEN),&param);
    lblCOARSEPLL_COMPO_CGEN->SetLabel(wxString::Format(_("%i"), param));
}

void lms7002_pnlCLKGEN_view::UpdateCLKL()
{
    int dMul;
    dMul = cmbCLKH_OV_CLKL_CGEN->GetSelection();
    double cgenFreq;
    txtFrequency->GetValue().ToDouble(&cgenFreq);
    double clklfreq = cgenFreq / pow(2.0, dMul);
    txtFrequencyCLKL->SetLabel(wxString::Format("%.3f", clklfreq));
}

void lms7002_pnlCLKGEN_view::OnShowVCOclicked(wxCommandEvent& event)
{
    lms7002_dlgVCOfrequencies* dlg = new lms7002_dlgVCOfrequencies(this, lmsControl);
    dlg->ShowModal();
    dlg->Destroy();
}
