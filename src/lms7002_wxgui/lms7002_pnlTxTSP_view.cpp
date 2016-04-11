#include "lms7002_pnlTxTSP_view.h"
#include "lms7002_gui_utilities.h"
#include "LMS7002M.h"
#include "ErrorReporting.h"
#include "numericSlider.h"
#include "lms7002_dlgGFIR_Coefficients.h"

using namespace lime;
using namespace LMS7002_WXGUI;

indexValueMap hbi_ovr_txtsp_IndexValuePairs;
indexValueMap tsgfcw_txtsp_IndexValuePairs;

lms7002_pnlTxTSP_view::lms7002_pnlTxTSP_view( wxWindow* parent )
:
pnlTxTSP_view( parent )
{

}

lms7002_pnlTxTSP_view::lms7002_pnlTxTSP_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlTxTSP_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[rgrMODE_TX] = MODE_TX;
    wndId2Enum[chkBSTART_TXTSP] = BSTART_TXTSP;
    wndId2Enum[chkCMIX_BYP_TXTSP] = CMIX_BYP_TXTSP;
    wndId2Enum[cmbCMIX_GAIN_TXTSP] = CMIX_GAIN_TXTSP;
    wndId2Enum[cmbDCCORRI_TXTSP] = DCCORRI_TXTSP;
    wndId2Enum[cmbDCCORRQ_TXTSP] = DCCORRQ_TXTSP;
    wndId2Enum[chkDC_BYP_TXTSP] = DC_BYP_TXTSP;
    wndId2Enum[chkEN_TXTSP] = EN_TXTSP;
    wndId2Enum[cmbGCORRI_TXTSP] = GCORRI_TXTSP;
    wndId2Enum[cmbGCORRQ_TXTSP] = GCORRQ_TXTSP;
    wndId2Enum[chkGC_BYP_TXTSP] = GC_BYP_TXTSP;
    wndId2Enum[chkGFIR1_BYP_TXTSP] = GFIR1_BYP_TXTSP;
    wndId2Enum[cmbGFIR1_L_TXTSP] = GFIR1_L_TXTSP;
    wndId2Enum[cmbGFIR1_N_TXTSP] = GFIR1_N_TXTSP;
    wndId2Enum[chkGFIR2_BYP_TXTSP] = GFIR2_BYP_TXTSP;
    wndId2Enum[cmbGFIR2_L_TXTSP] = GFIR2_L_TXTSP;
    wndId2Enum[cmbGFIR2_N_TXTSP] = GFIR2_N_TXTSP;
    wndId2Enum[chkGFIR3_BYP_TXTSP] = GFIR3_BYP_TXTSP;
    wndId2Enum[cmbGFIR3_L_TXTSP] = GFIR3_L_TXTSP;
    wndId2Enum[cmbGFIR3_N_TXTSP] = GFIR3_N_TXTSP;
    wndId2Enum[cmbHBI_OVR_TXTSP] = HBI_OVR_TXTSP;
    wndId2Enum[cmbIQCORR_TXTSP] = IQCORR_TXTSP;
    wndId2Enum[chkISINC_BYP_TXTSP] = ISINC_BYP_TXTSP;
    wndId2Enum[chkPH_BYP_TXTSP] = PH_BYP_TXTSP;
    wndId2Enum[cmbCMIX_SC_TXTSP] = CMIX_SC_TXTSP;

    wndId2Enum[rgrTSGFCW_TXTSP] = TSGFCW_TXTSP;
    wndId2Enum[chkTSGSWAPIQ_TXTSP] = TSGSWAPIQ_TXTSP;
    wndId2Enum[rgrTSGMODE_TXTSP] = TSGMODE_TXTSP;
    wndId2Enum[rgrINSEL_TXTSP] = INSEL_TXTSP;
    wndId2Enum[rgrTSGFC_TXTSP] = TSGFC_TXTSP;
    wndId2Enum[cmbDTHBIT_TX] = DTHBIT_TX;

    wndId2Enum[rgrSEL0] = SEL_TX;
    wndId2Enum[rgrSEL01] = SEL_TX;
    wndId2Enum[rgrSEL02] = SEL_TX;
    wndId2Enum[rgrSEL03] = SEL_TX;
    wndId2Enum[rgrSEL04] = SEL_TX;
    wndId2Enum[rgrSEL05] = SEL_TX;
    wndId2Enum[rgrSEL06] = SEL_TX;
    wndId2Enum[rgrSEL07] = SEL_TX;
    wndId2Enum[rgrSEL08] = SEL_TX;
    wndId2Enum[rgrSEL09] = SEL_TX;
    wndId2Enum[rgrSEL10] = SEL_TX;
    wndId2Enum[rgrSEL11] = SEL_TX;
    wndId2Enum[rgrSEL12] = SEL_TX;
    wndId2Enum[rgrSEL13] = SEL_TX;
    wndId2Enum[rgrSEL14] = SEL_TX;
    wndId2Enum[rgrSEL15] = SEL_TX;

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

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlTxTSP_view::Initialize(LMS7002M* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
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
    if (parameter == IQCORR_TXTSP)
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
    lmsControl->Modify_SPI_Reg_bits(parameter, value);

    if (parameter == MODE_TX)
        UpdateNCOinputs();
}

void lms7002_pnlTxTSP_view::OnNCOSelectionChange(wxCommandEvent& event)
{
    wxRadioButton* btn = reinterpret_cast<wxRadioButton*>(event.GetEventObject());
    int value = 0;
    for (int i = 0; i < rgrNCOselections.size(); ++i)
        if (btn == rgrNCOselections[i])
        {
            value = i;
            break;
        }
    lmsControl->Modify_SPI_Reg_bits(SEL_TX, value);
}

void lms7002_pnlTxTSP_view::onbtnReadBISTSignature( wxCommandEvent& event )
{    
    int value;
    value = lmsControl->Get_SPI_Reg_bits(BSTATE_TXTSP);
    lblBSTATE_TXTSP->SetLabel(wxString::Format("%i", value));
    value = lmsControl->Get_SPI_Reg_bits(BSIGI_TXTSP);
    lblBSIGI_TXTSP->SetLabel(wxString::Format("0x%0.6X", value));
    value = lmsControl->Get_SPI_Reg_bits(BSIGQ_TXTSP);
    lblBSIGQ_TXTSP->SetLabel(wxString::Format("0x%0.6X", value));    
}

void lms7002_pnlTxTSP_view::OnbtnLoadDCIClick( wxCommandEvent& event )
{   
    long value = 0;
    txtDC_REG_TXTSP->GetValue().ToLong(&value, 16);
    lmsControl->Modify_SPI_Reg_bits(DC_REG_TXTSP, value);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDI_TXTSP, 0);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDI_TXTSP, 1);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDI_TXTSP, 0);    
}

void lms7002_pnlTxTSP_view::OnbtnLoadDCQClick( wxCommandEvent& event )
{
    long value = 0;
    txtDC_REG_TXTSP->GetValue().ToLong(&value, 16);
    lmsControl->Modify_SPI_Reg_bits(DC_REG_TXTSP, value);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDQ_TXTSP, 0);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDQ_TXTSP, 1);
    lmsControl->Modify_SPI_Reg_bits(TSGDCLDQ_TXTSP, 0);
}

void lms7002_pnlTxTSP_view::onbtnGFIR1Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<int16_t> coefficients;
    const int gfirIndex = 0;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    int status = lmsControl->GetGFIRCoefficients(true, gfirIndex, &coefficients[0], maxCoefCount);
    if (status != 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(GetLastErrorMessage()), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        lmsControl->SetGFIRCoefficients(true, gfirIndex, &coefficients[0], coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlTxTSP_view::onbtnGFIR2Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<int16_t> coefficients;
    const int gfirIndex = 1;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    int status = lmsControl->GetGFIRCoefficients(true, gfirIndex, &coefficients[0], maxCoefCount);
    if (status != 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(GetLastErrorMessage()), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }
    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        lmsControl->SetGFIRCoefficients(true, gfirIndex, &coefficients[0], coefficients.size());
    }
    dlg->Destroy();
}

void lms7002_pnlTxTSP_view::onbtnGFIR3Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this);
    std::vector<int16_t> coefficients;
    const int gfirIndex = 2;
    const int maxCoefCount = 120;
    coefficients.resize(maxCoefCount, 0);
    int status = lmsControl->GetGFIRCoefficients(true, gfirIndex, &coefficients[0], maxCoefCount);
    if (status != 0)
    {
        wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(GetLastErrorMessage()), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }
    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        status = lmsControl->SetGFIRCoefficients(true, gfirIndex, &coefficients[0], coefficients.size());
        if (status != 0)
            wxMessageBox(_("Error reading GFIR coefficients: ") + wxString::From8BitData(GetLastErrorMessage()), _("ERROR"), wxICON_ERROR | wxOK);
    }
    dlg->Destroy();
}

void lms7002_pnlTxTSP_view::OnbtnUploadNCOClick( wxCommandEvent& event )
{
    lmsControl->Modify_SPI_Reg_bits(MODE_TX, rgrMODE_TX->GetSelection());
    assert(txtNCOinputs.size() == 16);
    if (rgrMODE_TX->GetSelection() == 0)
    {
        for (int i = 0; i < 16; ++i)
        {
            double freq;
            txtNCOinputs[i]->GetValue().ToDouble(&freq);
            lmsControl->SetNCOFrequency(LMS7002M::Tx, i, freq);
        }
        long value;
        txtFCWPHOmodeAdditional->GetValue().ToLong(&value);
        lmsControl->SPI_write(0x0241, value);
    }
    else //PHO mode
    {
        long value;
        for (int i = 0; i < 16; ++i)
        {
            
            txtNCOinputs[i]->GetValue().ToLong(&value);
            lmsControl->SPI_write(0x0244+i, value);
        }        
        double freq;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq);
        lmsControl->SetNCOFrequency(LMS7002M::Tx, 0, freq);
    }
}

void lms7002_pnlTxTSP_view::UpdateNCOinputs()
{
    bool fromChip = false;
    assert(txtNCOinputs.size() == 16);
    if (rgrMODE_TX->GetSelection() == 0) //FCW mode        
    {   
        for (int i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), lmsControl->GetNCOFrequency_MHz(LMS7002M::Tx, i, fromChip)));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%i"), lmsControl->SPI_read(0x0241, fromChip)));
        lblFCWPHOmodeName->SetLabel(_("PHO"));
    }
    else //PHO mode
    {
        for (int i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), (65536.0/360.0)*lmsControl->GetNCOPhaseOffset_Deg(LMS7002M::Tx, i)));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.6f"), lmsControl->GetNCOFrequency_MHz(LMS7002M::Tx, 0, fromChip)));
        lblFCWPHOmodeName->SetLabel(_("FCW(MHz)"));
    }
}

void lms7002_pnlTxTSP_view::UpdateGUI()
{
    bool fromChip = false;
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    lblRefClk->SetLabel(wxString::Format(_("%3.3f"), lmsControl->GetReferenceClk_TSP_MHz(LMS7002M::Tx)));

    long hbi = lmsControl->Get_SPI_Reg_bits(HBI_OVR_TXTSP, fromChip);
    cmbHBI_OVR_TXTSP->SetSelection(value2index(hbi, hbi_ovr_txtsp_IndexValuePairs));

    int16_t value = lmsControl->Get_SPI_Reg_bits(TSGFCW_RXTSP, fromChip);
    rgrTSGFCW_TXTSP->SetSelection(value2index(value, tsgfcw_txtsp_IndexValuePairs));

    value = lmsControl->Get_SPI_Reg_bits(IQCORR_TXTSP, fromChip);
    int bitsToShift = (15 - IQCORR_TXTSP.msb - IQCORR_TXTSP.lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_TXTSP->SetValue(value);

    value = lmsControl->Get_SPI_Reg_bits(SEL_TX, fromChip);
    assert(rgrNCOselections.size() == 16);
    rgrNCOselections[value & 0xF]->SetValue(true);
    UpdateNCOinputs();

    int8_t dccorr = lmsControl->Get_SPI_Reg_bits(DCCORRI_TXTSP, fromChip);
    cmbDCCORRI_TXTSP->SetValue(dccorr);
    dccorr = lmsControl->Get_SPI_Reg_bits(DCCORRQ_TXTSP, fromChip);
    cmbDCCORRQ_TXTSP->SetValue(dccorr);

    //check if B channel is enabled
    if (lmsControl->GetActiveChannel(fromChip) >= LMS7002M::ChB)
    {
        if (lmsControl->Get_SPI_Reg_bits(MIMO_SISO) != 0)
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
    }
}

void lms7002_pnlTxTSP_view::PHOinputChanged(wxCommandEvent& event)
{
    assert(lblNCOangles.size() == 16);
    if (rgrMODE_TX->GetSelection() == 1)
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
