#include "lms7002_pnlRxTSP_view.h"
#include "lms7002_gui_utilities.h"
#include "LMS7002M.h"
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
    wndId2Enum[rgrMODE_RX] = MODE_RX;
    wndId2Enum[chkBSTART_RXTSP] = BSTART_RXTSP;
    wndId2Enum[chkCMIX_BYP_RXTSP] = CMIX_BYP_RXTSP;
    wndId2Enum[cmbCMIX_GAIN_RXTSP] = CMIX_GAIN_RXTSP;    
    wndId2Enum[chkDC_BYP_RXTSP] = DC_BYP_RXTSP;
    wndId2Enum[chkEN_RXTSP] = EN_RXTSP;
    wndId2Enum[cmbGCORRI_RXTSP] = GCORRI_RXTSP;
    wndId2Enum[cmbGCORRQ_RXTSP] = GCORRQ_RXTSP;
    wndId2Enum[chkGC_BYP_RXTSP] = GC_BYP_RXTSP;
    wndId2Enum[chkGFIR1_BYP_RXTSP] = GFIR1_BYP_RXTSP;
    wndId2Enum[cmbGFIR1_L_RXTSP] = GFIR1_L_RXTSP;
    wndId2Enum[cmbGFIR1_N_RXTSP] = GFIR1_N_RXTSP;
    wndId2Enum[chkGFIR2_BYP_RXTSP] = GFIR2_BYP_RXTSP;
    wndId2Enum[cmbGFIR2_L_RXTSP] = GFIR2_L_RXTSP;
    wndId2Enum[cmbGFIR2_N_RXTSP] = GFIR2_N_RXTSP;
    wndId2Enum[chkGFIR3_BYP_RXTSP] = GFIR3_BYP_RXTSP;
    wndId2Enum[cmbGFIR3_L_RXTSP] = GFIR3_L_RXTSP;
    wndId2Enum[cmbGFIR3_N_RXTSP] = GFIR3_N_RXTSP;
    wndId2Enum[cmbHBD_OVR_RXTSP] = HBD_OVR_RXTSP;
    wndId2Enum[cmbIQCORR_RXTSP] = IQCORR_RXTSP;
    wndId2Enum[chkAGC_BYP_RXTSP] = AGC_BYP_RXTSP;
    wndId2Enum[chkPH_BYP_RXTSP] = PH_BYP_RXTSP;
    wndId2Enum[cmbCMIX_SC_RXTSP] = CMIX_SC_RXTSP;

    wndId2Enum[cmbAGC_MODE_RXTSP] = AGC_MODE_RXTSP;
    wndId2Enum[cmbAGC_AVG_RXTSP] = AGC_AVG_RXTSP;
    wndId2Enum[cmbAGC_ADESIRED_RXTSP] = AGC_ADESIRED_RXTSP;
    wndId2Enum[spinAGC_K_RXTSP] = AGC_K_RXTSP;

    wndId2Enum[rgrTSGFCW_RXTSP] = TSGFCW_RXTSP;
    wndId2Enum[chkTSGSWAPIQ_RXTSP] = TSGSWAPIQ_RXTSP;
    wndId2Enum[rgrTSGMODE_RXTSP] = TSGMODE_RXTSP;
    wndId2Enum[rgrINSEL_RXTSP] = INSEL_RXTSP;
    wndId2Enum[rgrTSGFC_RXTSP] = TSGFC_RXTSP;
    wndId2Enum[cmbDTHBIT_RX] = DTHBIT_RX;

    wndId2Enum[rgrSEL0] = SEL_RX;
    wndId2Enum[rgrSEL01] = SEL_RX;
    wndId2Enum[rgrSEL02] = SEL_RX;
    wndId2Enum[rgrSEL03] = SEL_RX;
    wndId2Enum[rgrSEL04] = SEL_RX;
    wndId2Enum[rgrSEL05] = SEL_RX;
    wndId2Enum[rgrSEL06] = SEL_RX;
    wndId2Enum[rgrSEL07] = SEL_RX;
    wndId2Enum[rgrSEL08] = SEL_RX;
    wndId2Enum[rgrSEL09] = SEL_RX;
    wndId2Enum[rgrSEL10] = SEL_RX;
    wndId2Enum[rgrSEL11] = SEL_RX;
    wndId2Enum[rgrSEL12] = SEL_RX;
    wndId2Enum[rgrSEL13] = SEL_RX;
    wndId2Enum[rgrSEL14] = SEL_RX;
    wndId2Enum[rgrSEL15] = SEL_RX;

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
    std::vector<int16_t> coefficients;
    const int gfirIndex = 0;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    liblms7_status status = lmsControl->GetGFIRCoefficients(false, gfirIndex, &coefficients[0], maxCoefCount);
    if (status != LIBLMS7_SUCCESS)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(liblms7_status2string(status)), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        lmsControl->SetGFIRCoefficients(false, gfirIndex, &coefficients[0], coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlRxTSP_view::onbtnGFIR2Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<int16_t> coefficients;
    const int gfirIndex = 1;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    liblms7_status status = lmsControl->GetGFIRCoefficients(false, gfirIndex, &coefficients[0], maxCoefCount);
    if (status != LIBLMS7_SUCCESS)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(liblms7_status2string(status)), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        lmsControl->SetGFIRCoefficients(false, gfirIndex, &coefficients[0], coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlRxTSP_view::onbtnGFIR3Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<int16_t> coefficients;
    const int gfirIndex = 2;
    const int maxCoefCount = 120;
    coefficients.resize(maxCoefCount, 0);
    liblms7_status status = lmsControl->GetGFIRCoefficients(false, gfirIndex, &coefficients[0], maxCoefCount);
    if (status != LIBLMS7_SUCCESS)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(liblms7_status2string(status)), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        lmsControl->SetGFIRCoefficients(false, gfirIndex, &coefficients[0], coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlRxTSP_view::Initialize(LMS7002M* pControl)
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
    if (parameter == IQCORR_RXTSP)
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
    lmsControl->Modify_SPI_Reg_bits(parameter, value);

    if (parameter == MODE_RX)
        UpdateNCOinputs();
}

void lms7002_pnlRxTSP_view::OnNCOSelectionChange(wxCommandEvent& event)
{
    wxRadioButton* btn = reinterpret_cast<wxRadioButton*>(event.GetEventObject());
    int value = 0;
    for (int i = 0; i < rgrNCOselections.size(); ++i)
        if (btn == rgrNCOselections[i])
        {
            value = i;
            break;
        }
    lmsControl->Modify_SPI_Reg_bits(SEL_RX, value);
}

void lms7002_pnlRxTSP_view::OnbtnReadBISTSignature(wxCommandEvent& event)
{
    //Read BISTI BSTATE
    lmsControl->Modify_SPI_Reg_bits(CAPSEL, 2);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 1);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 0);
    int value = lmsControl->SPI_read(0x040E);
    int value2 = lmsControl->SPI_read(0x040F);
    int valrez = ((value2 << 15) | (value >> 1)) & 0x7FFFFF;
    lblBISTI->SetLabel(wxString::Format("0x%0.6X", valrez));
    lblBSTATE_I->SetLabel(wxString::Format("0x%0.1X", value & 0x1));

    //Read BISTI BSTATE
    lmsControl->Modify_SPI_Reg_bits(CAPSEL, 3);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 1);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 0);
    value = lmsControl->SPI_read(0x040E);
    value2 = lmsControl->SPI_read(0x040F);
    valrez = ((value2 << 15) | (value >> 1)) & 0x7FFFFF;
    lblBISTQ->SetLabel(wxString::Format("0x%0.6X", valrez));
    lblBSTATE_Q->SetLabel(wxString::Format("0x%0.1X", value & 0x1));
}

void lms7002_pnlRxTSP_view::OnbtnReadRSSI(wxCommandEvent& event)
{
    long value = 0;
    long value2 = 0;
    long valrez = 0;
    //Read RSSI
    lmsControl->Modify_SPI_Reg_bits(CAPSEL, 0);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 0);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 1);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 0);
    value = lmsControl->SPI_read(0x040E);
    value2 = lmsControl->SPI_read(0x040F);
    valrez = ((value & 0x3) | (value2 << 2)) & 0x3FFFF;
    lblRSSI->SetLabel(wxString::Format("0x%0.5X", valrez));

    //Read ADCI, ADCQ
    lmsControl->Modify_SPI_Reg_bits(CAPSEL, 1);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 1);
    lmsControl->Modify_SPI_Reg_bits(CAPTURE, 0);
    value = lmsControl->SPI_read(0x040E);
    value2 = lmsControl->SPI_read(0x040F);
    lblADCI->SetLabel(wxString::Format("0x%0.3X", value & 0x3ff));
    lblADCQ->SetLabel(wxString::Format("0x%0.3X", value2 & 0x3ff));
}

void lms7002_pnlRxTSP_view::OnbtnLoadDCIClick(wxCommandEvent& event)
{
    long value = 0;
    txtDC_REG_RXTSP->GetValue().ToLong(&value, 16);
    lmsControl->Modify_SPI_Reg_bits(DC_REG_RXTSP, value);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDI_RXTSP, 0);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDI_RXTSP, 1);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDI_RXTSP, 0);
}

void lms7002_pnlRxTSP_view::OnbtnLoadDCQClick(wxCommandEvent& event)
{
    long value = 0;
    txtDC_REG_RXTSP->GetValue().ToLong(&value, 16);
    lmsControl->Modify_SPI_Reg_bits(DC_REG_RXTSP, value);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDQ_RXTSP, 0);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDQ_RXTSP, 1);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDQ_RXTSP, 0);
}

void lms7002_pnlRxTSP_view::OnbtnUploadNCOClick(wxCommandEvent& event)
{
    lmsControl->Modify_SPI_Reg_bits(MODE_RX, rgrMODE_RX->GetSelection());
    assert(txtNCOinputs.size() == 16);
    if (rgrMODE_RX->GetSelection() == 0)
    {
        for (int i = 0; i < 16; ++i)
        {
            double freq;
            txtNCOinputs[i]->GetValue().ToDouble(&freq);
            lmsControl->SetNCOFrequency(LMS7002M::Rx, i, freq);
        }
        long value;
        txtFCWPHOmodeAdditional->GetValue().ToLong(&value);
        lmsControl->SPI_write(0x0441, value);
    }
    else
    {
        long value;
        for (int i = 0; i < 16; ++i)
        {

            txtNCOinputs[i]->GetValue().ToLong(&value);
            lmsControl->SPI_write(0x0444 + i, value);
        }
        double freq;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq);
        lmsControl->SetNCOFrequency(LMS7002M::Rx, 0, freq);
    }
}

void lms7002_pnlRxTSP_view::UpdateNCOinputs()
{
    bool fromChip = false;
    assert(txtNCOinputs.size() == 16);
    float refClk_MHz = lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx);
    if (rgrMODE_RX->GetSelection() == 0) //FCW mode        
    {
        for (int i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), lmsControl->GetNCOFrequency_MHz(LMS7002M::Rx, i, refClk_MHz, fromChip)));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%i"), lmsControl->SPI_read(0x0441, fromChip)));
        lblFCWPHOmodeName->SetLabel(_("PHO"));
    }
    else //PHO mode
    {
        for (int i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), lmsControl->GetNCOPhaseOffset_Deg(LMS7002M::Rx, i)));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.6f"), lmsControl->GetNCOFrequency_MHz(LMS7002M::Rx, 0, refClk_MHz, fromChip)));
        lblFCWPHOmodeName->SetLabel(_("FCW(MHz)"));
    }
}

void lms7002_pnlRxTSP_view::UpdateGUI()
{
    bool fromChip = false;
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    lblRefClk->SetLabel(wxString::Format(_("%3.3f"), lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Rx)));

    int16_t iqcorr_value = lmsControl->Get_SPI_Reg_bits(IQCORR_RXTSP, fromChip);
    int bitsToShift = (15 - IQCORR_RXTSP.msb - IQCORR_RXTSP.lsb);
    iqcorr_value = iqcorr_value << bitsToShift;
    iqcorr_value = iqcorr_value >> bitsToShift;
    cmbIQCORR_RXTSP->SetValue(iqcorr_value);

    long value = lmsControl->Get_SPI_Reg_bits(HBD_OVR_RXTSP, fromChip);
    cmbHBD_OVR_RXTSP->SetSelection(value2index(value, hbd_ovr_rxtsp_IndexValuePairs));

    value = lmsControl->Get_SPI_Reg_bits(TSGFCW_RXTSP, fromChip);
    rgrTSGFCW_RXTSP->SetSelection(value2index(value, tsgfcw_rxtsp_IndexValuePairs));

    value = lmsControl->Get_SPI_Reg_bits(SEL_RX, fromChip);
    assert(rgrNCOselections.size() == 16);
    rgrNCOselections[value & 0xF]->SetValue(true);
    UpdateNCOinputs();

    //check if B channel is enabled
    uint8_t channel = lmsControl->Get_SPI_Reg_bits(MAC, fromChip);
    if (channel > 1)
    {
        if (lmsControl->Get_SPI_Reg_bits(MIMO_SISO) != 0)
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
