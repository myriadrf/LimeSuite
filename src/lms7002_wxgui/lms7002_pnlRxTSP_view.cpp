#include "lms7002_pnlRxTSP_view.h"
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7002_dlgGFIR_Coefficients.h"
#include "lms7_device.h"
#include "lms7suiteAppFrame.h"

using namespace lime;
using namespace LMS7002_WXGUI;
static indexValueMap hbd_ovr_rxtsp_IndexValuePairs;
static indexValueMap tsgfcw_rxtsp_IndexValuePairs;
indexValueMap cmix_gain_rxtsp_IndexValuePairs;

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
    wndId2Enum[chkDC_LOOP_RXTSP] = LMS7_DCLOOP_STOP;
    wndId2Enum[chkCAPSEL_ADC_RXTSP] = LMS7_CAPSEL_ADC;

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
    wndId2Enum[cmbHBD_DLY] = LMS7param(HBD_DLY);
    wndId2Enum[cmbDCCORR_AVG] = LMS7param(DCCORR_AVG_RXTSP);

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
    for(int i=0; i<7; ++i)
        temp.push_back(wxString::Format(_("2^%i"), i+12));
    cmbDCCORR_AVG->Set(temp);

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

    cmix_gain_rxtsp_IndexValuePairs.push_back(indexValuePair(0, 2));
    cmix_gain_rxtsp_IndexValuePairs.push_back(indexValuePair(1, 2));
    cmix_gain_rxtsp_IndexValuePairs.push_back(indexValuePair(2, 0));
    cmix_gain_rxtsp_IndexValuePairs.push_back(indexValuePair(3, 0));
    cmix_gain_rxtsp_IndexValuePairs.push_back(indexValuePair(4, 1));

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
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_RX, ch, LMS_GFIR1, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        LMS_SetGFIRCoeff(lmsControl,LMS_CH_RX,ch,LMS_GFIR1,&coefficients[0],coefficients.size());
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlRxTSP_view::onbtnGFIR2Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_RX, ch, LMS_GFIR2, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        LMS_SetGFIRCoeff(lmsControl,LMS_CH_RX,ch,LMS_GFIR2,&coefficients[0],coefficients.size());
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlRxTSP_view::onbtnGFIR3Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<double> coefficients;
    const int maxCoefCount = 120;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_RX, ch, LMS_GFIR3, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        LMS_SetGFIRCoeff(lmsControl,LMS_CH_RX,ch,LMS_GFIR3,&coefficients[0],coefficients.size());
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlRxTSP_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
         value = 1;
    chkDC_LOOP_RXTSP->Enable(value);
    cmbHBD_DLY->Enable(value);
    wxArrayString temp;
    temp.clear();
    temp.push_back("-6 dB");
    temp.push_back(value ? "-3 dB":"-6 dB");
    temp.push_back("0 dB");
    temp.push_back(value ? "+3 dB":"+6 dB");
    temp.push_back("+6 dB");
    cmbCMIX_GAIN_RXTSP->Set(temp);
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
    if(event.GetEventObject() == cmbIQCORR_RXTSP)
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
    else if(event.GetEventObject() == cmbCMIX_GAIN_RXTSP)
    {
        LMS_WriteParam(lmsControl, LMS7_CMIX_GAIN_RXTSP_R3, value % 0x2);
        value = index2value(value, cmix_gain_rxtsp_IndexValuePairs);
    }
    LMS_WriteParam(lmsControl,parameter,value);

    if(event.GetEventObject() == rgrMODE_RX)
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
    unsigned valrez = 0;

    //Read ADCI, ADCQ
    LMS_WriteParam(lmsControl,LMS7param(CAPSEL),1);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),1);
    LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);

    LMS_ReadLMSReg(lmsControl,0x040E,&value);
    LMS_ReadLMSReg(lmsControl,0x040F,&value2);

    if (chkCAPSEL_ADC_RXTSP->GetValue())
    {
        lblADCI->SetLabel(wxString::Format("0x%0.4X", value));
        lblADCQ->SetLabel(wxString::Format("0x%0.4X", value2));
        lblRSSI->SetLabel(wxString::Format(" ----- "));
    }
    else
    {
        lblADCI->SetLabel(wxString::Format("0x%0.3X", value & 0x3ff));
        lblADCQ->SetLabel(wxString::Format("0x%0.3X", value2 & 0x3ff));
        //Read RSSI
        //LMS_WriteParam(lmsControl,LMS7param(CAPSEL_ADC),0);
        LMS_WriteParam(lmsControl,LMS7param(CAPSEL),0);
        LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);
        LMS_WriteParam(lmsControl,LMS7param(CAPTURE),1);
        LMS_WriteParam(lmsControl,LMS7param(CAPTURE),0);
        LMS_ReadLMSReg(lmsControl,0x040E,&value);
        LMS_ReadLMSReg(lmsControl,0x040F,&value2);
        valrez = ((value & 0x3) | (value2 << 2)) & 0x3FFFF;
        lblRSSI->SetLabel(wxString::Format("0x%0.5X", valrez));
    }

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
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    if (rgrMODE_RX->GetSelection() == 0)
    {
        double nco_freq[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_freq[i]);
            nco_freq[i] *= 1e6;
        }
        double value;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&value);
        LMS_SetNCOFrequency(lmsControl,LMS_CH_RX,ch,nco_freq,value);
    }
    else
    {
        double nco_phase[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_phase[i]);
        }
        double freq_MHz;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq_MHz);
        LMS_SetNCOPhase(lmsControl, LMS_CH_RX, ch, nco_phase, freq_MHz);
    }
    UpdateGUI();// API changes nco selection
}

void lms7002_pnlRxTSP_view::OnbtnSetLPFClick( wxCommandEvent& event )
{
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    double bw;
    txtLPFBW->GetValue().ToDouble(&bw);
    if (LMS_SetGFIRLPF(lmsControl, LMS_CH_RX, ch, true, bw*1e6)!=0)
        wxMessageBox(_("GFIR configuration failed"), _("Error"));
    
    UpdateGUI();// API changes nco selection
}

void lms7002_pnlRxTSP_view::UpdateNCOinputs()
{
    assert(txtNCOinputs.size() == 16);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    if (rgrMODE_RX->GetSelection() == 0) //FCW mode
    {
        double freq[16] = {0};
        double pho=0;
        LMS_GetNCOFrequency(lmsControl, LMS_CH_RX, ch, freq, &pho);
        for (size_t i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), freq[i]/1e6));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.3f"), pho));
        lblFCWPHOmodeName->SetLabel(_("PHO (deg)"));
        tableTitleCol1->SetLabel(_("FCW (MHz)"));
        tableTitleCol2->SetLabel(_("PHO (deg)"));
    }
    else //PHO mode
    {
        double phase[16] = {0};
        double fcw = 0;
        LMS_GetNCOPhase(lmsControl, LMS_CH_RX, ch, phase, &fcw);
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

void lms7002_pnlRxTSP_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
    double freq = lms->GetReferenceClk_TSP(false);
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

    uint16_t g_cmix;
    LMS_ReadParam(lmsControl,LMS7param(CMIX_GAIN_RXTSP),&g_cmix);
    value = value2index(g_cmix, cmix_gain_rxtsp_IndexValuePairs);
    LMS_ReadParam(lmsControl,LMS7param(CMIX_GAIN_RXTSP_R3),&g_cmix);
    if (g_cmix)
        value |= 1;
    else
        value &= ~1;
    cmbCMIX_GAIN_RXTSP->SetSelection(value);
    
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    double sr = 0;
    LMS_GetSampleRate(lmsControl, LMS_CH_RX, ch , &sr, nullptr);
    txtRateVal->SetLabel(wxString::Format("%3.3f MHz", sr/1e6));

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
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    // Write values for NCO phase or frequency each time they change - to ease the tuning of these values in measurements
    if (rgrMODE_RX->GetSelection() == 0)
    {
        double angle;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
        LMS_SetNCOFrequency(lmsControl,LMS_CH_RX,ch,nullptr,angle);
    }
    else //PHO mode
    {
        double freq;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq);
        LMS_SetNCOPhase(lmsControl, LMS_CH_RX, ch, nullptr, freq*1e6);
    }

    assert(lblNCOangles.size() == 16);
    if (rgrMODE_RX->GetSelection() == 1)
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

void lms7002_pnlRxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel(wxMouseEvent& event){
    double angle = 0;
    txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
    int change = event.GetWheelRotation()/120;
    angle += change*0.1;
    txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.1f"), angle > 0 ? angle : 0));
}
