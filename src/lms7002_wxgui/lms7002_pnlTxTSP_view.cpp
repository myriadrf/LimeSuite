#include "lms7002_pnlTxTSP_view.h"
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7002_dlgGFIR_Coefficients.h"
#include "lms7suiteAppFrame.h"

using namespace lime;
using namespace LMS7002_WXGUI;

indexValueMap hbi_ovr_txtsp_IndexValuePairs;
indexValueMap tsgfcw_txtsp_IndexValuePairs;
indexValueMap cmix_gain_txtsp_IndexValuePairs;

lms7002_pnlTxTSP_view::lms7002_pnlTxTSP_view( wxWindow* parent )
:
pnlTxTSP_view( parent )
{

}

lms7002_pnlTxTSP_view::lms7002_pnlTxTSP_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlTxTSP_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[rgrMODE_TX] = LMS7param(MODE_TX);
    wndId2Enum[chkBSTART_TXTSP] = LMS7param(BSTART_TXTSP);
    wndId2Enum[chkCMIX_BYP_TXTSP] = LMS7param(CMIX_BYP_TXTSP);
    wndId2Enum[cmbCMIX_GAIN_TXTSP] = LMS7param(CMIX_GAIN_TXTSP);
    wndId2Enum[cmbDCCORRI_TXTSP] = LMS7param(DCCORRI_TXTSP);
    wndId2Enum[cmbDCCORRQ_TXTSP] = LMS7param(DCCORRQ_TXTSP);
    wndId2Enum[chkDC_BYP_TXTSP] = LMS7param(DC_BYP_TXTSP);
    wndId2Enum[chkEN_TXTSP] = LMS7param(EN_TXTSP);
    wndId2Enum[cmbGCORRI_TXTSP] = LMS7param(GCORRI_TXTSP);
    wndId2Enum[cmbGCORRQ_TXTSP] = LMS7param(GCORRQ_TXTSP);
    wndId2Enum[chkGC_BYP_TXTSP] = LMS7param(GC_BYP_TXTSP);
    wndId2Enum[chkGFIR1_BYP_TXTSP] = LMS7param(GFIR1_BYP_TXTSP);
    wndId2Enum[cmbGFIR1_L_TXTSP] = LMS7param(GFIR1_L_TXTSP);
    wndId2Enum[cmbGFIR1_N_TXTSP] = LMS7param(GFIR1_N_TXTSP);
    wndId2Enum[chkGFIR2_BYP_TXTSP] = LMS7param(GFIR2_BYP_TXTSP);
    wndId2Enum[cmbGFIR2_L_TXTSP] = LMS7param(GFIR2_L_TXTSP);
    wndId2Enum[cmbGFIR2_N_TXTSP] = LMS7param(GFIR2_N_TXTSP);
    wndId2Enum[chkGFIR3_BYP_TXTSP] = LMS7param(GFIR3_BYP_TXTSP);
    wndId2Enum[cmbGFIR3_L_TXTSP] = LMS7param(GFIR3_L_TXTSP);
    wndId2Enum[cmbGFIR3_N_TXTSP] = LMS7param(GFIR3_N_TXTSP);
    wndId2Enum[cmbHBI_OVR_TXTSP] = LMS7param(HBI_OVR_TXTSP);
    wndId2Enum[cmbIQCORR_TXTSP] = LMS7param(IQCORR_TXTSP);
    wndId2Enum[chkISINC_BYP_TXTSP] = LMS7param(ISINC_BYP_TXTSP);
    wndId2Enum[chkPH_BYP_TXTSP] = LMS7param(PH_BYP_TXTSP);
    wndId2Enum[cmbCMIX_SC_TXTSP] = LMS7param(CMIX_SC_TXTSP);

    wndId2Enum[rgrTSGFCW_TXTSP] = LMS7param(TSGFCW_TXTSP);
    wndId2Enum[chkTSGSWAPIQ_TXTSP] = LMS7param(TSGSWAPIQ_TXTSP);
    wndId2Enum[rgrTSGMODE_TXTSP] = LMS7param(TSGMODE_TXTSP);
    wndId2Enum[rgrINSEL_TXTSP] = LMS7param(INSEL_TXTSP);
    wndId2Enum[rgrTSGFC_TXTSP] = LMS7param(TSGFC_TXTSP);
    wndId2Enum[cmbDTHBIT_TX] = LMS7param(DTHBIT_TX);

    wndId2Enum[rgrSEL0] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL01] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL02] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL03] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL04] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL05] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL06] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL07] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL08] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL09] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL10] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL11] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL12] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL13] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL14] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL15] = LMS7param(SEL_TX);

    lblNCOangles.push_back(txtAnglePHO0);
    lblNCOangles.push_back(txtAnglePHO01);
    lblNCOangles.push_back(txtAnglePHO02);
    lblNCOangles.push_back(txtAnglePHO03);
    lblNCOangles.push_back(txtAnglePHO04);
    lblNCOangles.push_back(txtAnglePHO05);
    lblNCOangles.push_back(txtAnglePHO06);
    lblNCOangles.push_back(txtAnglePHO07);
    lblNCOangles.push_back(txtAnglePHO08);
    lblNCOangles.push_back(txtAnglePHO09);
    lblNCOangles.push_back(txtAnglePHO10);
    lblNCOangles.push_back(txtAnglePHO11);
    lblNCOangles.push_back(txtAnglePHO12);
    lblNCOangles.push_back(txtAnglePHO13);
    lblNCOangles.push_back(txtAnglePHO14);
    lblNCOangles.push_back(txtAnglePHO15);

    rgrNCOselections.push_back(rgrSEL0);
    rgrNCOselections.push_back(rgrSEL01);
    rgrNCOselections.push_back(rgrSEL02);
    rgrNCOselections.push_back(rgrSEL03);
    rgrNCOselections.push_back(rgrSEL04);
    rgrNCOselections.push_back(rgrSEL05);
    rgrNCOselections.push_back(rgrSEL06);
    rgrNCOselections.push_back(rgrSEL07);
    rgrNCOselections.push_back(rgrSEL08);
    rgrNCOselections.push_back(rgrSEL09);
    rgrNCOselections.push_back(rgrSEL10);
    rgrNCOselections.push_back(rgrSEL11);
    rgrNCOselections.push_back(rgrSEL12);
    rgrNCOselections.push_back(rgrSEL13);
    rgrNCOselections.push_back(rgrSEL14);
    rgrNCOselections.push_back(rgrSEL15);

    txtNCOinputs.push_back(txtFCWPHO0);
    txtNCOinputs.push_back(txtFCWPHO01);
    txtNCOinputs.push_back(txtFCWPHO02);
    txtNCOinputs.push_back(txtFCWPHO03);
    txtNCOinputs.push_back(txtFCWPHO04);
    txtNCOinputs.push_back(txtFCWPHO05);
    txtNCOinputs.push_back(txtFCWPHO06);
    txtNCOinputs.push_back(txtFCWPHO07);
    txtNCOinputs.push_back(txtFCWPHO08);
    txtNCOinputs.push_back(txtFCWPHO09);
    txtNCOinputs.push_back(txtFCWPHO10);
    txtNCOinputs.push_back(txtFCWPHO11);
    txtNCOinputs.push_back(txtFCWPHO12);
    txtNCOinputs.push_back(txtFCWPHO13);
    txtNCOinputs.push_back(txtFCWPHO14);
    txtNCOinputs.push_back(txtFCWPHO15);

    wxArrayString temp;

    temp.clear();
    for(int i=0; i<8; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbGFIR1_L_TXTSP->Set(temp);
    cmbGFIR2_L_TXTSP->Set(temp);
    cmbGFIR3_L_TXTSP->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbDTHBIT_TX->Set(temp);

    temp.clear();
    temp.push_back("2^1");
    temp.push_back("2^2");
    temp.push_back("2^3");
    temp.push_back("2^4");
    temp.push_back("2^5");
    temp.push_back("Bypass");
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(0, 0));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(1, 1));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(2, 2));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(3, 3));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(4, 4));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(5, 7));
    cmbHBI_OVR_TXTSP->Set(temp);

    tsgfcw_txtsp_IndexValuePairs.push_back(indexValuePair(0, 1));
    tsgfcw_txtsp_IndexValuePairs.push_back(indexValuePair(1, 2));

    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(0, 2));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(1, 2));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(2, 0));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(3, 0));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(4, 1));

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlTxTSP_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
         value = 1;
    wxArrayString temp;
    temp.clear();
    temp.push_back("-6 dB");
    temp.push_back(value ? "-3 dB":"-6 dB");
    temp.push_back("0 dB");
    temp.push_back(value ? "+3 dB":"+6 dB");
    temp.push_back("+6 dB");
    cmbCMIX_GAIN_TXTSP->Set(temp);
}

void lms7002_pnlTxTSP_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlTxTSP_view::ParameterChangeHandler( wxCommandEvent& event )
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
    long value = event.GetInt();
    if(event.GetEventObject() == cmbIQCORR_TXTSP)
    {
        float angle = atan(value / 2048.0) * 180 / 3.141596;
        txtPhaseAlpha->SetLabel(wxString::Format("%.3f", angle));
    }
    else if (event.GetEventObject() == rgrTSGFCW_TXTSP)
    {
        value = index2value(value, tsgfcw_txtsp_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbHBI_OVR_TXTSP)
    {
        value = index2value(value, hbi_ovr_txtsp_IndexValuePairs);
    }
    else if(event.GetEventObject() == cmbCMIX_GAIN_TXTSP)
    {
        LMS_WriteParam(lmsControl, LMS7_CMIX_GAIN_TXTSP_R3, value % 0x2);
        value = index2value(value, cmix_gain_txtsp_IndexValuePairs);
    }

    LMS_WriteParam(lmsControl,parameter,value);

    if(event.GetEventObject() == rgrMODE_TX)
        UpdateNCOinputs();
}

void lms7002_pnlTxTSP_view::OnNCOSelectionChange(wxCommandEvent& event)
{
    wxRadioButton* btn = reinterpret_cast<wxRadioButton*>(event.GetEventObject());
    int value = 0;
    for (size_t i = 0; i < rgrNCOselections.size(); ++i)
        if (btn == rgrNCOselections[i])
        {
            value = i;
            break;
        }
    LMS_WriteParam(lmsControl,LMS7param(SEL_TX),value);
}

void lms7002_pnlTxTSP_view::onbtnReadBISTSignature( wxCommandEvent& event )
{
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(BSTATE_TXTSP),&value);
    lblBSTATE_TXTSP->SetLabel(wxString::Format("%i", value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGI_TXTSP),&value);
    lblBSIGI_TXTSP->SetLabel(wxString::Format("0x%0.6X", value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGQ_TXTSP),&value);
    lblBSIGQ_TXTSP->SetLabel(wxString::Format("0x%0.6X", value));
}

void lms7002_pnlTxTSP_view::OnbtnLoadDCIClick( wxCommandEvent& event )
{
    long value = 0;
    txtDC_REG_TXTSP->GetValue().ToLong(&value, 16);
    LMS_WriteParam(lmsControl,LMS7param(DC_REG_TXTSP),value);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_TXTSP),0);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_TXTSP),1);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_TXTSP),0);
}

void lms7002_pnlTxTSP_view::OnbtnLoadDCQClick( wxCommandEvent& event )
{
    long value = 0;
    txtDC_REG_TXTSP->GetValue().ToLong(&value, 16);
    LMS_WriteParam(lmsControl,LMS7param(DC_REG_TXTSP),value);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_TXTSP),0);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_TXTSP),1);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_TXTSP),0);
}

void lms7002_pnlTxTSP_view::onbtnGFIR1Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR1, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Failed to read GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        status = LMS_SetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR1, &coefficients[0],coefficients.size());
        if (status != 0)
            wxMessageBox(_("Failed to set GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlTxTSP_view::onbtnGFIR2Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR2, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Failed to read GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }
    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        status = LMS_SetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR2, &coefficients[0],coefficients.size());
        if (status != 0)
            wxMessageBox(_("Failed to set GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlTxTSP_view::onbtnGFIR3Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 120;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR3, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Failed to read GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }
    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        status = LMS_SetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR3, &coefficients[0],coefficients.size());
        if (status != 0)
            wxMessageBox(_("Failed to set GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlTxTSP_view::OnbtnUploadNCOClick( wxCommandEvent& event )
{
    LMS_WriteParam(lmsControl,LMS7param(MODE_TX),rgrMODE_TX->GetSelection());
    assert(txtNCOinputs.size() == 16);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    if (rgrMODE_TX->GetSelection() == 0)
    {
        float_type nco_freq[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_freq[i]);
            nco_freq[i] *= 1e6;
        }
        long value;
        txtFCWPHOmodeAdditional->GetValue().ToLong(&value);
        LMS_SetNCOFrequency(lmsControl,LMS_CH_TX,ch,nco_freq,value);
    }
    else //PHO mode
    {
        float_type nco_phase[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_phase[i]);
        }
        double freq_MHz;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq_MHz);
        LMS_SetNCOPhase(lmsControl, LMS_CH_TX, ch-1, nco_phase, freq_MHz);
    }
    UpdateGUI();// API changes nco selection
}

void lms7002_pnlTxTSP_view::OnbtnSetLPFClick( wxCommandEvent& event )
{
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    double bw;
    txtLPFBW->GetValue().ToDouble(&bw);
    if (LMS_SetGFIRLPF(lmsControl, LMS_CH_TX, ch, true, bw*1e6)!=0)
        wxMessageBox(_("GFIR configuration failed"), _("Error"));
    UpdateGUI();// API changes nco selection
}

void lms7002_pnlTxTSP_view::UpdateNCOinputs()
{
    assert(txtNCOinputs.size() == 16);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    if (rgrMODE_TX->GetSelection() == 0) //FCW mode
    {
        float_type freq[16] = { 0 };
        float_type pho=0;
        LMS_GetNCOFrequency(lmsControl, LMS_CH_TX, ch, freq, &pho);
        for (size_t i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), freq[i]/1e6));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%3.3f"), pho));
        lblFCWPHOmodeName->SetLabel(_("PHO (deg)"));
        tableTitleCol1->SetLabel(_("FCW (MHz)"));
        tableTitleCol2->SetLabel(_("PHO (deg)"));
    }
    else //PHO mode
    {
        float_type phase[16] = { 0 };
        float_type fcw = 0;
        LMS_GetNCOPhase(lmsControl, LMS_CH_TX, ch, phase, &fcw);
        for (size_t i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.3f"), phase[i]));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.6f"), fcw/1e6));
        lblFCWPHOmodeName->SetLabel(_("FCW (MHz)"));
        tableTitleCol2->SetLabel(_("FCW (MHz)"));
        tableTitleCol1->SetLabel(_("PHO (deg)"));
    }
}

void lms7002_pnlTxTSP_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_TXTSP,&freq);
    lblRefClk->SetLabel(wxString::Format(_("%3.3f"), freq/1e6));

    uint16_t hbi;
    LMS_ReadParam(lmsControl,LMS7param(HBI_OVR_TXTSP),&hbi);
    cmbHBI_OVR_TXTSP->SetSelection(value2index(hbi, hbi_ovr_txtsp_IndexValuePairs));

    int16_t value;
    LMS_ReadParam(lmsControl,LMS7param(TSGFCW_TXTSP),(uint16_t*)&value);

    rgrTSGFCW_TXTSP->SetSelection(value2index(value, tsgfcw_txtsp_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(IQCORR_TXTSP),(uint16_t*)&value);
    int bitsToShift = (15 - LMS7param(IQCORR_TXTSP).msb - LMS7param(IQCORR_TXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_TXTSP->SetValue(value);

    LMS_ReadParam(lmsControl,LMS7param(SEL_TX),(uint16_t*)&value);
    assert(rgrNCOselections.size() == 16);
    rgrNCOselections[value & 0xF]->SetValue(true);
    UpdateNCOinputs();

    LMS_ReadParam(lmsControl,LMS7param(DCCORRI_TXTSP),(uint16_t*)&value);
    int8_t dccorr = value;
    cmbDCCORRI_TXTSP->SetValue(dccorr);
    LMS_ReadParam(lmsControl,LMS7param(DCCORRQ_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRQ_TXTSP->SetValue(dccorr);

    uint16_t g_cmix;
    LMS_ReadParam(lmsControl,LMS7param(CMIX_GAIN_TXTSP),&g_cmix);
    value = value2index(g_cmix, cmix_gain_txtsp_IndexValuePairs);
    LMS_ReadParam(lmsControl,LMS7param(CMIX_GAIN_TXTSP_R3),&g_cmix);
    if (g_cmix)
        value |= 1;
    else
        value &= ~1;
    cmbCMIX_GAIN_TXTSP->SetSelection(value);
    
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    
    double sr = 0;
    LMS_GetSampleRate(lmsControl, LMS_CH_TX, ch , &sr, nullptr);
    txtRATEVAL->SetLabel(wxString::Format("%3.3f MHz", sr/1e6));
    //check if B channel is enabled
    LMS_ReadParam(lmsControl,LMS7param(MAC),(uint16_t*)&value);
    if (value >= 2)
    {
        LMS_ReadParam(lmsControl,LMS7param(MIMO_SISO),(uint16_t*)&value);
        if (value != 0)
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
    }
}

void lms7002_pnlTxTSP_view::PHOinputChanged(wxCommandEvent& event)
{
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    // Write values for NCO phase or frequency each time they change - to ease the tuning of these values in measurements
    if (rgrMODE_TX->GetSelection() == 0)
    {
        double angle;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
        LMS_SetNCOFrequency(lmsControl,LMS_CH_TX,ch,nullptr,angle);
    }
    else //PHO mode
    {
        double freq;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq);
        LMS_SetNCOPhase(lmsControl, LMS_CH_TX, ch, nullptr, freq*1e6);
    }

    assert(lblNCOangles.size() == 16);
    if (rgrMODE_TX->GetSelection() == 1)
    {
        double freq;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq);
        for (int i = 0; i < 16; ++i){
            lblNCOangles[i]->SetLabel(wxString::Format("%3.3f", freq));
        }
    }
    else
    {
        double angle;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
        for (int i = 0; i < 16; ++i){
            lblNCOangles[i]->SetLabel(wxString::Format("%3.3f", angle));
        }
    }
}

void lms7002_pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel(wxMouseEvent& event){
    double angle = 0;
    txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
    int change = event.GetWheelRotation()/120;
    angle += change*0.1;
    txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.1f"), angle > 0 ? angle : 0));
}
