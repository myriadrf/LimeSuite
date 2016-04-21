#include "lms7002_pnlCLKGEN_view.h"
#include "LMS7002M.h"
#include "ErrorReporting.h"
#include <map>
#include <wx/msgdlg.h>
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include "lms7002_dlgVCOfrequencies.h"
using namespace lime;

lms7002_pnlCLKGEN_view::lms7002_pnlCLKGEN_view( wxWindow* parent )
:
pnlCLKGEN_view( parent )
{

}

lms7002_pnlCLKGEN_view::lms7002_pnlCLKGEN_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlCLKGEN_view(parent, id, pos, size, style), lmsControl(nullptr)
{
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

void lms7002_pnlCLKGEN_view::Initialize(LMS7002M* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);

    txtFrequency->SetValue(wxString::Format(_("%.3f"), lmsControl->GetFrequencyCGEN() / 1e6));
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"), lmsControl->GetReferenceClk_SX(LMS7002M::Rx) / 1e6));
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

    lmsControl->Modify_SPI_Reg_bits(parameter, event.GetInt());

    if (LMS7ParameterCompare(parameter,LMS7param(EN_ADCCLKH_CLKGN))==0)
        UpdateInterfaceFrequencies();
    else if (LMS7ParameterCompare(parameter ,LMS7param(CSW_VCO_CGEN))==0)
    {
        OnbtnReadComparators(event);
    }
    else if (LMS7ParameterCompare(parameter,LMS7param(CLKH_OV_CLKL_CGEN))==0)
    {
        double cgenFreq;
        txtFrequency->GetValue().ToDouble(&cgenFreq);
        txtFrequencyCLKL->SetValue(wxString::Format(_("%.3f"), cgenFreq / pow(2.0, cmbCLKH_OV_CLKL_CGEN->GetSelection())));
        UpdateInterfaceFrequencies();
        UpdateCLKL();
    }
}

void lms7002_pnlCLKGEN_view::onbtnCalculateClick( wxCommandEvent& event )
{
    double freqMHz;
    txtFrequency->GetValue().ToDouble(&freqMHz);
    int status = lmsControl->SetFrequencyCGEN(freqMHz * 1e6, true);
    if (status != 0)
        wxMessageBox(wxString::Format(_("Set frequency CGEN: %s"), wxString::From8BitData(GetLastErrorMessage())));
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), lmsControl->GetFrequencyCGEN() / 1e6));
    UpdateGUI();
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    wxCommandEvent cmd;
    cmd.SetString(_("CGEN frequency set to ") + lblRealOutFrequency->GetLabel() + _(" MHz"));
    cmd.SetEventType(LOG_MESSAGE);
    wxPostEvent(this, cmd);
}

void lms7002_pnlCLKGEN_view::onbtnTuneClick( wxCommandEvent& event )
{
    int status = lmsControl->TuneVCO(LMS7002M::VCO_CGEN);
    if (status != 0)
        wxMessageBox(wxString::Format(_("CLKGEN Tune: %s"), wxString::From8BitData(GetLastErrorMessage())));
    cmbCSW_VCO_CGEN->SetValue(lmsControl->Get_SPI_Reg_bits(LMS7param(CSW_VCO_CGEN)));
    OnbtnReadComparators(event);
}

void lms7002_pnlCLKGEN_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    wxCommandEvent evt;
    OnbtnReadComparators(evt);
    UpdateInterfaceFrequencies();
    UpdateCLKL();
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), lmsControl->GetFrequencyCGEN() / 1e6));
    txtFrequency->SetValue(wxString::Format(_("%.3f"), lmsControl->GetFrequencyCGEN() / 1e6));
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"), lmsControl->GetReferenceClk_SX(LMS7002M::Rx) / 1e6 ));
    int fracValue = (lmsControl->Get_SPI_Reg_bits(LMS7param(FRAC_SDM_CGEN_MSB), false) << 16) | lmsControl->Get_SPI_Reg_bits(LMS7param(FRAC_SDM_CGEN_LSB), false);
    lblFRAC_SDM_CGEN->SetLabel(wxString::Format("%i", fracValue));
    lblDivider->SetLabel(wxString::Format("2^%i", lmsControl->Get_SPI_Reg_bits(LMS7param(DIV_OUTCH_CGEN))+1));
}

void lms7002_pnlCLKGEN_view::UpdateInterfaceFrequencies()
{
    lblRxTSPfreq->SetLabel(wxString::Format(_("%.3f"), lmsControl->GetReferenceClk_TSP(LMS7002M::Rx) / 1e6));
    lblTxTSPfreq->SetLabel(wxString::Format(_("%.3f"), lmsControl->GetReferenceClk_TSP(LMS7002M::Tx) / 1e6));
}

void lms7002_pnlCLKGEN_view::OnbtnReadComparators(wxCommandEvent& event)
{
    int param;
    param = lmsControl->Get_SPI_Reg_bits(LMS7param(VCO_CMPHO_CGEN));
    lblVCO_CMPHO_CGEN->SetLabel(wxString::Format(_("%i"), param));
    if (param == 1)
        lblVCO_CMPHO_CGEN->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPHO_CGEN->SetBackgroundColour(*wxRED);

    param = lmsControl->Get_SPI_Reg_bits(LMS7param(VCO_CMPLO_CGEN));
    lblVCO_CMPLO_CGEN->SetLabel(wxString::Format(_("%i"), param));
    if (param == 0)
        lblVCO_CMPLO_CGEN->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPLO_CGEN->SetBackgroundColour(*wxRED);
}

void lms7002_pnlCLKGEN_view::OnbtnUpdateCoarse(wxCommandEvent& event)
{
    int param;
    param = lmsControl->Get_SPI_Reg_bits(LMS7param(COARSE_STEPDONE));
    lblCOARSE_STEPDONE_CGEN->SetLabel(wxString::Format(_("%i"), param));

    param = lmsControl->Get_SPI_Reg_bits(LMS7param(COARSEPLL_COMPO_CGEN));
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
