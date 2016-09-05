#include "lms7002_pnlRxTSP_view.h"
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7002_dlgGFIR_Coefficients.h"

using namespace lime;
using namespace LMS7002_WXGUI;
static indexValueMap hbd_ovr_rxtsp_IndexValuePairs;
static indexValueMap tsgfcw_rxtsp_IndexValuePairs;

lms7002_pnlRxTSP_view::lms7002_pnlRxTSP_view( wxWindow* parent )
:
pnlRxTSP_view( parent )
{

}

lms7002_pnlRxTSP_view::lms7002_pnlRxTSP_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : pnlRxTSP_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[rgrMODE_RX] = LMS7param(MODE_RX);
    wndId2Enum[chkBSTART_RXTSP] = LMS7param(BSTART_RXTSP);
    wndId2Enum[chkCMIX_BYP_RXTSP] = LMS7param(CMIX_BYP_RXTSP);
    wndId2Enum[cmbCMIX_GAIN_RXTSP] = LMS7param(CMIX_GAIN_RXTSP);
    wndId2Enum[chkDC_BYP_RXTSP] = LMS7param(DC_BYP_RXTSP);
    wndId2Enum[chkEN_RXTSP] = LMS7param(EN_RXTSP);
    wndId2Enum[cmbGCORRI_RXTSP] = LMS7param(GCORRI_RXTSP);
    wndId2Enum[cmbGCORRQ_RXTSP] = LMS7param(GCORRQ_RXTSP);
    wndId2Enum[chkGC_BYP_RXTSP] = LMS7param(GC_BYP_RXTSP);
    wndId2Enum[chkGFIR1_BYP_RXTSP] = LMS7param(GFIR1_BYP_RXTSP);
    wndId2Enum[cmbGFIR1_L_RXTSP] = LMS7param(GFIR1_L_RXTSP);
    wndId2Enum[cmbGFIR1_N_RXTSP] = LMS7param(GFIR1_N_RXTSP);
    wndId2Enum[chkGFIR2_BYP_RXTSP] = LMS7param(GFIR2_BYP_RXTSP);
    wndId2Enum[cmbGFIR2_L_RXTSP] = LMS7param(GFIR2_L_RXTSP);
    wndId2Enum[cmbGFIR2_N_RXTSP] = LMS7param(GFIR2_N_RXTSP);
    wndId2Enum[chkGFIR3_BYP_RXTSP] = LMS7param(GFIR3_BYP_RXTSP);
    wndId2Enum[cmbGFIR3_L_RXTSP] = LMS7param(GFIR3_L_RXTSP);
    wndId2Enum[cmbGFIR3_N_RXTSP] = LMS7param(GFIR3_N_RXTSP);
    wndId2Enum[cmbHBD_OVR_RXTSP] = LMS7param(HBD_OVR_RXTSP);
    wndId2Enum[cmbIQCORR_RXTSP] = LMS7param(IQCORR_RXTSP);
    wndId2Enum[chkAGC_BYP_RXTSP] = LMS7param(AGC_BYP_RXTSP);
    wndId2Enum[chkPH_BYP_RXTSP] = LMS7param(PH_BYP_RXTSP);
    wndId2Enum[cmbCMIX_SC_RXTSP] = LMS7param(CMIX_SC_RXTSP);

    wndId2Enum[cmbAGC_MODE_RXTSP] = LMS7param(AGC_MODE_RXTSP);
    wndId2Enum[cmbAGC_AVG_RXTSP] = LMS7param(AGC_AVG_RXTSP);
    wndId2Enum[cmbAGC_ADESIRED_RXTSP] = LMS7param(AGC_ADESIRED_RXTSP);
    wndId2Enum[spinAGC_K_RXTSP] = LMS7param(AGC_K_RXTSP);

    wndId2Enum[rgrTSGFCW_RXTSP] = LMS7param(TSGFCW_RXTSP);
    wndId2Enum[chkTSGSWAPIQ_RXTSP] = LMS7param(TSGSWAPIQ_RXTSP);
    wndId2Enum[rgrTSGMODE_RXTSP] = LMS7param(TSGMODE_RXTSP);
    wndId2Enum[rgrINSEL_RXTSP] = LMS7param(INSEL_RXTSP);
    wndId2Enum[rgrTSGFC_RXTSP] = LMS7param(TSGFC_RXTSP);
    wndId2Enum[cmbDTHBIT_RX] = LMS7param(DTHBIT_RX);

    wndId2Enum[rgrSEL0] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL01] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL02] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL03] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL04] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL05] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL06] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL07] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL08] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL09] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL10] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL11] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL12] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL13] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL14] = LMS7param(SEL_RX);
    wndId2Enum[rgrSEL15] = LMS7param(SEL_RX);

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
    for (int i = 0; i<8; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbGFIR1_L_RXTSP->Set(temp);
    cmbGFIR2_L_RXTSP->Set(temp);
    cmbGFIR3_L_RXTSP->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbDTHBIT_RX->Set(temp);

    temp.clear();
    temp.push_back("2^1");
    temp.push_back("2^2");
    temp.push_back("2^3");
    temp.push_back("2^4");
    temp.push_back("2^5");
    temp.push_back("Bypass");
    hbd_ovr_rxtsp_IndexValuePairs.push_back(indexValuePair(0, 0));
    hbd_ovr_rxtsp_IndexValuePairs.push_back(indexValuePair(1, 1));
    hbd_ovr_rxtsp_IndexValuePairs.push_back(indexValuePair(2, 2));
    hbd_ovr_rxtsp_IndexValuePairs.push_back(indexValuePair(3, 3));
    hbd_ovr_rxtsp_IndexValuePairs.push_back(indexValuePair(4, 4));
    hbd_ovr_rxtsp_IndexValuePairs.push_back(indexValuePair(5, 7));
    cmbHBD_OVR_RXTSP->Set(temp);

    tsgfcw_rxtsp_IndexValuePairs.push_back(indexValuePair(0, 1));
    tsgfcw_rxtsp_IndexValuePairs.push_back(indexValuePair(1, 2));

    UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlRxTSP_view::onbtnGFIR1Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);

    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_RX, ch-1, LMS_GFIR1, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        LMS_SetGFIRCoeff(lmsControl,LMS_CH_RX,ch-1,LMS_GFIR1,&coefficients[0],coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlRxTSP_view::onbtnGFIR2Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_RX, ch-1, LMS_GFIR2, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        LMS_SetGFIRCoeff(lmsControl,LMS_CH_RX,ch-1,LMS_GFIR2,&coefficients[0],coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlRxTSP_view::onbtnGFIR3Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 120;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_RX, ch-1, LMS_GFIR3, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        LMS_SetGFIRCoeff(lmsControl,LMS_CH_RX,ch-1,LMS_GFIR3,&coefficients[0],coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlRxTSP_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlRxTSP_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlRxTSP_view::ParameterChangeHandler(wxCommandEvent& event)
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
    if (0==LMS7ParameterCompare(parameter,LMS7param(IQCORR_RXTSP)))
    {
        float angle = atan(value / 2048.0) * 180 / 3.141596;
        txtPhaseAlpha->SetLabel(wxString::Format("%.3f", angle));
    }
    else if (event.GetEventObject() == cmbHBD_OVR_RXTSP)
    {
        value = index2value(value, hbd_ovr_rxtsp_IndexValuePairs);
    }
    else if (event.GetEventObject() == rgrTSGFCW_RXTSP)
    {
        value = index2value(value, tsgfcw_rxtsp_IndexValuePairs);
    }
    LMS_WriteParam(lmsControl,parameter,value);

    if (LMS7ParameterCompare(parameter,LMS7param(MODE_RX))==0)
        UpdateNCOinputs();
}

void lms7002_pnlRxTSP_view::OnNCOSelectionChange(wxCommandEvent& event)
{
    wxRadioButton* btn = reinterpret_cast<wxRadioButton*>(event.GetEventObject());
    int value = 0;
    for (size_t i = 0; i < rgrNCOselections.size(); ++i)
        if (btn == rgrNCOselections[i])
        {
            value = i;
            break;
        }
    LMS_WriteParam(lmsControl,LMS7param(SEL_RX),value);
}

void lms7002_pnlRxTSP_view::OnbtnReadBISTSignature(wxCommandEvent& event)
{
    //Read BISTI BSTATE
    LMS_WriteParam(lmsControl,LMS7param(CAPSEL),2);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),1);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);
    uint16_t value;
    LMS_ReadLMSReg(lmsControl,0x040E,&value);
    uint16_t value2;
    LMS_ReadLMSReg(lmsControl,0x040F,&value2);
    int valrez = ((value2 << 15) | (value >> 1)) & 0x7FFFFF;
    lblBISTI->SetLabel(wxString::Format("0x%0.6X", valrez));
    lblBSTATE_I->SetLabel(wxString::Format("0x%0.1X", value & 0x1));

    //Read BISTI BSTATE
    LMS_WriteParam(lmsControl,LMS7param(CAPSEL),3);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),1);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);
    LMS_ReadLMSReg(lmsControl,0x040E,&value);
    LMS_ReadLMSReg(lmsControl,0x040F,&value2);
    valrez = ((value2 << 15) | (value >> 1)) & 0x7FFFFF;
    lblBISTQ->SetLabel(wxString::Format("0x%0.6X", valrez));
    lblBSTATE_Q->SetLabel(wxString::Format("0x%0.1X", value & 0x1));
}

void lms7002_pnlRxTSP_view::OnbtnReadRSSI(wxCommandEvent& event)
{
    uint16_t value = 0;
    uint16_t value2 = 0;
    long valrez = 0;
    //Read RSSI
    LMS_WriteParam(lmsControl,LMS7param(CAPSEL),0);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),1);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);
    LMS_ReadLMSReg(lmsControl,0x040E,&value);
    LMS_ReadLMSReg(lmsControl,0x040F,&value2);
    valrez = ((value & 0x3) | (value2 << 2)) & 0x3FFFF;
    lblRSSI->SetLabel(wxString::Format("0x%0.5X", valrez));

    //Read ADCI, ADCQ
    LMS_WriteParam(lmsControl,LMS7param(CAPSEL),1);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),1);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);

    LMS_ReadLMSReg(lmsControl,0x040E,&value);
    LMS_ReadLMSReg(lmsControl,0x040F,&value2);
    lblADCI->SetLabel(wxString::Format("0x%0.3X", value & 0x3ff));
    lblADCQ->SetLabel(wxString::Format("0x%0.3X", value2 & 0x3ff));
}

void lms7002_pnlRxTSP_view::OnbtnLoadDCIClick(wxCommandEvent& event)
{
    long value = 0;
    txtDC_REG_RXTSP->GetValue().ToLong(&value, 16);
    LMS_WriteParam(lmsControl,LMS7param(DC_REG_RXTSP),value);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_RXTSP),0);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_RXTSP),1);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_RXTSP),0);
}

void lms7002_pnlRxTSP_view::OnbtnLoadDCQClick(wxCommandEvent& event)
{
    long value = 0;
    txtDC_REG_RXTSP->GetValue().ToLong(&value, 16);
    LMS_WriteParam(lmsControl,LMS7param(DC_REG_RXTSP),value);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_RXTSP),0);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_RXTSP),1);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_RXTSP),0);
}

void lms7002_pnlRxTSP_view::OnbtnUploadNCOClick(wxCommandEvent& event)
{
    LMS_WriteParam(lmsControl,LMS7param(MODE_RX),rgrMODE_RX->GetSelection());
    assert(txtNCOinputs.size() == 16);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    if (rgrMODE_RX->GetSelection() == 0)
    {
        float_type nco_freq[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_freq[i]);
            nco_freq[i] *= 1e6;
        }
        long value;
        txtFCWPHOmodeAdditional->GetValue().ToLong(&value);
        LMS_SetNCOFrequency(lmsControl,LMS_CH_RX,ch-1,nco_freq,value);
    }
    else
    {
        float_type nco_phase[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_phase[i]);
        }
        double freq_MHz;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq_MHz);
        LMS_SetNCOPhase(lmsControl, LMS_CH_RX, ch-1, nco_phase, freq_MHz);
    }
}

void lms7002_pnlRxTSP_view::UpdateNCOinputs()
{
    assert(txtNCOinputs.size() == 16);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC), &ch);
    if (rgrMODE_RX->GetSelection() == 0) //FCW mode
    {
        float_type freq[16];
        float_type pho;
        LMS_GetNCOFrequency(lmsControl,LMS_CH_RX,ch-1,freq,&pho);
        for (size_t i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), freq[i]/1e6));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%f"), pho));
        lblFCWPHOmodeName->SetLabel(_("PHO"));
    }
    else //PHO mode
    {
        float_type phase[16];
        float_type fcw;
        LMS_GetNCOFrequency(lmsControl,LMS_CH_RX,ch-1,phase,&fcw);
        for (size_t i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), (65536.0 / 360.0)*phase[i]));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.6f"), fcw/1e6));
        lblFCWPHOmodeName->SetLabel(_("FCW(MHz)"));
    }
}

void lms7002_pnlRxTSP_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_RXTSP,&freq);
    lblRefClk->SetLabel(wxString::Format(_("%3.3f"), freq/1e6));

    int16_t iqcorr_value;
    LMS_ReadParam(lmsControl,LMS7param(IQCORR_RXTSP),(uint16_t*)&iqcorr_value);
    int bitsToShift = (15 - LMS7param(IQCORR_RXTSP).msb - LMS7param(IQCORR_RXTSP).lsb);
    iqcorr_value = iqcorr_value << bitsToShift;
    iqcorr_value = iqcorr_value >> bitsToShift;
    cmbIQCORR_RXTSP->SetValue(iqcorr_value);

    int16_t value;
    LMS_ReadParam(lmsControl,LMS7param(HBD_OVR_RXTSP),(uint16_t*)&value);
    cmbHBD_OVR_RXTSP->SetSelection(value2index(value, hbd_ovr_rxtsp_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(TSGFCW_RXTSP),(uint16_t*)&value);
    rgrTSGFCW_RXTSP->SetSelection(value2index(value, tsgfcw_rxtsp_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(SEL_RX),(uint16_t*)&value);
    assert(rgrNCOselections.size() == 16);
    rgrNCOselections[value & 0xF]->SetValue(true);
    UpdateNCOinputs();

    //check if B channel is enabled
    LMS_ReadParam(lmsControl,LMS7param(MAC),(uint16_t*)&value);
    if (value >= 2)
    {
        LMS_ReadParam(lmsControl,LMS7param(MIMO_SISO),(uint16_t*)&value);
        if (value != 0)
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
    }
}

void lms7002_pnlRxTSP_view::PHOinputChanged(wxCommandEvent& event)
{
    assert(lblNCOangles.size() == 16);
    if (rgrMODE_RX->GetSelection() == 1)
        for (int i = 0; i < 16; ++i)
        {
            long phoVal = 0;
            txtNCOinputs[i]->GetValue().ToLong(&phoVal);
            lblNCOangles[i]->SetLabel(wxString::Format("%3.3f", 2.0 * 180 * phoVal / (65536.0)));
        }
    else
    {
        long phoVal = 0;
        txtFCWPHOmodeAdditional->GetValue().ToLong(&phoVal);
        for (int i = 0; i < 16; ++i)
            lblNCOangles[i]->SetLabel(wxString::Format("%3.3f", 2.0 * 180 * phoVal / (65536.0)));
    }
}
