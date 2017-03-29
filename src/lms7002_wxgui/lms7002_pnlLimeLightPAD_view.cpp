#include "lms7002_pnlLimeLightPAD_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlLimeLightPAD_view::lms7002_pnlLimeLightPAD_view( wxWindow* parent )
:
pnlLimeLightPAD_view( parent )
{

}

lms7002_pnlLimeLightPAD_view::lms7002_pnlLimeLightPAD_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlLimeLightPAD_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    //ids for updating from chip
    wndId2Enum[rgrDIQ1_DS] = LMS7param(DIQ1_DS);
    wndId2Enum[chkDIQ1_PE] = LMS7param(DIQ1_PE);
    wndId2Enum[rgrDIQ2_DS] = LMS7param(DIQ2_DS);
    wndId2Enum[chkDIQ2_PE] = LMS7param(DIQ2_PE);
    wndId2Enum[rgrRX_MUX] = LMS7param(RX_MUX);
    wndId2Enum[chkFCLK1_PE] = LMS7param(FCLK1_PE);
    wndId2Enum[chkFCLK2_PE] = LMS7param(FCLK2_PE);
    wndId2Enum[chkIQ_SEL_EN_1_PE] = LMS7param(IQ_SEL_EN_1_PE);
    wndId2Enum[chkIQ_SEL_EN_2_PE] = LMS7param(IQ_SEL_EN_2_PE);
    wndId2Enum[cmbLML1_AIP] = LMS7param(LML1_AIP);
    wndId2Enum[cmbLML1_AQP] = LMS7param(LML1_AQP);
    wndId2Enum[cmbLML1_BIP] = LMS7param(LML1_BIP);
    wndId2Enum[cmbLML1_BQP] = LMS7param(LML1_BQP);
    wndId2Enum[cmbLML1_RX_PRE] = LMS7param(LML1_RF2BB_PRE);
    wndId2Enum[cmbLML1_RX_PST] = LMS7param(LML1_RF2BB_PST);
    wndId2Enum[cmbLML1_S0S] = LMS7param(LML1_S0S);
    wndId2Enum[cmbLML1_S1S] = LMS7param(LML1_S1S);
    wndId2Enum[cmbLML1_S2S] = LMS7param(LML1_S2S);
    wndId2Enum[cmbLML1_S3S] = LMS7param(LML1_S3S);
    wndId2Enum[cmbLML1_TX_PRE] = LMS7param(LML1_BB2RF_PRE);
    wndId2Enum[cmbLML1_TX_PST] = LMS7param(LML1_BB2RF_PST);
    wndId2Enum[cmbLML2_AIP] = LMS7param(LML2_AIP);
    wndId2Enum[cmbLML2_AQP] = LMS7param(LML2_AQP);
    wndId2Enum[cmbLML2_BIP] = LMS7param(LML2_BIP);
    wndId2Enum[cmbLML2_BQP] = LMS7param(LML2_BQP);
    wndId2Enum[cmbLML2_RX_PRE] = LMS7param(LML2_RF2BB_PRE);
    wndId2Enum[cmbLML2_RX_PST] = LMS7param(LML2_RF2BB_PST);
    wndId2Enum[cmbLML2_S0S] = LMS7param(LML2_S0S);
    wndId2Enum[cmbLML2_S1S] = LMS7param(LML2_S1S);
    wndId2Enum[cmbLML2_S2S] = LMS7param(LML2_S2S);
    wndId2Enum[cmbLML2_S3S] = LMS7param(LML2_S3S);
    wndId2Enum[cmbLML2_TX_PRE] = LMS7param(LML2_BB2RF_PRE);
    wndId2Enum[cmbLML2_TX_PST] = LMS7param(LML2_BB2RF_PST);
    wndId2Enum[rgrLML_MODE1] = LMS7param(LML1_MODE);
    wndId2Enum[rgrLML_MODE2] = LMS7param(LML2_MODE);
    wndId2Enum[rgrLML_TXNRXIQ1] = LMS7param(LML1_TXNRXIQ);
    wndId2Enum[rgrLML_TXNRXIQ2] = LMS7param(LML2_TXNRXIQ);
    wndId2Enum[chkLRST_RX_A] = LMS7param(LRST_RX_A);
    wndId2Enum[chkLRST_RX_B] = LMS7param(LRST_RX_B);
    wndId2Enum[chkLRST_TX_A] = LMS7param(LRST_TX_A);
    wndId2Enum[chkLRST_TX_B] = LMS7param(LRST_TX_B);
    wndId2Enum[cmbMCLK1SRC] = LMS7param(MCLK1SRC);
    wndId2Enum[cmbRXTSPCLKA_DIV] = LMS7param(RXTSPCLKA_DIV);
    wndId2Enum[chkMCLK1_PE] = LMS7param(MCLK1_PE);
    wndId2Enum[cmbMCLK2SRC] = LMS7param(MCLK2SRC);
    wndId2Enum[cmbTXTSPCLKA_DIV] = LMS7param(TXTSPCLKA_DIV);
    wndId2Enum[chkMCLK2_PE] = LMS7param(MCLK2_PE);
    wndId2Enum[chkMIMO_SISO] = LMS7param(MIMO_SISO);
    wndId2Enum[chkMOD_EN] = LMS7param(MOD_EN);
    wndId2Enum[chkMRST_RX_A] = LMS7param(MRST_RX_A);
    wndId2Enum[chkMRST_RX_B] = LMS7param(MRST_RX_B);
    wndId2Enum[chkMRST_TX_A] = LMS7param(MRST_TX_A);
    wndId2Enum[chkMRST_TX_B] = LMS7param(MRST_TX_B);
    wndId2Enum[chkRXDIVEN] = LMS7param(RXDIVEN);
    wndId2Enum[chkRXEN_A] = LMS7param(RXEN_A);
    wndId2Enum[chkRXEN_B] = LMS7param(RXEN_B);
    wndId2Enum[cmbRXRDCLK_MUX] = LMS7param(RXRDCLK_MUX);
    wndId2Enum[cmbRXWRCLK_MUX] = LMS7param(RXWRCLK_MUX);
    wndId2Enum[chkRX_CLK_PE] = LMS7param(RX_CLK_PE);
    wndId2Enum[chkSCLK_PE] = LMS7param(SCLK_PE);
    wndId2Enum[rgrSCL_DS] = LMS7param(SCL_DS);
    wndId2Enum[chkSCL_PE] = LMS7param(SCL_PE);
    wndId2Enum[rgrSDA_DS] = LMS7param(SDA_DS);
    wndId2Enum[chkSDA_PE] = LMS7param(SDA_PE);
    wndId2Enum[rgrSDIO_DS] = LMS7param(SDIO_DS);
    wndId2Enum[chkSDIO_PE] = LMS7param(SDIO_PE);
    wndId2Enum[chkSDO_PE] = LMS7param(SDO_PE);
    wndId2Enum[chkSEN_PE] = LMS7param(SEN_PE);
    wndId2Enum[rgrSPIMODE] = LMS7param(SPIMODE);
    wndId2Enum[chkSRST_RXFIFO] = LMS7param(SRST_RXFIFO);
    wndId2Enum[chkSRST_TXFIFO] = LMS7param(SRST_TXFIFO);
    wndId2Enum[chkTXDIVEN] = LMS7param(TXDIVEN);
    wndId2Enum[chkTXEN_A] = LMS7param(TXEN_A);
    wndId2Enum[chkTXEN_B] = LMS7param(TXEN_B);
    wndId2Enum[rgrTX_MUX] = LMS7param(TX_MUX);
    wndId2Enum[chkTXNRX1_PE] = LMS7param(TXNRX1_PE);
    wndId2Enum[chkTXNRX2_PE] = LMS7param(TXNRX2_PE);
    wndId2Enum[cmbTXRDCLK_MUX] = LMS7param(TXRDCLK_MUX);
    wndId2Enum[cmbTXWRCLK_MUX] = LMS7param(TXWRCLK_MUX);
    wndId2Enum[chkTX_CLK_PE] = LMS7param(TX_CLK_PE);
    wndId2Enum[rgrLML_FIDM1] = LMS7param(LML1_FIDM);
    wndId2Enum[rgrLML_FIDM2] = LMS7param(LML2_FIDM);
    wndId2Enum[chkFCLK1_INV] = LMS7param(FCLK1_INV);
    wndId2Enum[chkFCLK2_INV] = LMS7param(FCLK2_INV);
    wndId2Enum[cmbMCLK1DLY] = LMS7param(MCLK1DLY);
    wndId2Enum[cmbMCLK2DLY] = LMS7param(MCLK2DLY);
    wndId2Enum[rgrDIQDIRCTR2] = LMS7param(DIQDIRCTR2);
    wndId2Enum[rgrDIQDIR2] = LMS7param(DIQDIR2);
    wndId2Enum[rgrDIQDIRCTR1] = LMS7param(DIQDIRCTR1);
    wndId2Enum[rgrDIQDIR1] = LMS7param(DIQDIR1);
    wndId2Enum[rgrENABLEDIRCTR2] = LMS7param(ENABLEDIRCTR2);
    wndId2Enum[rgrENABLEDIR2] = LMS7param(ENABLEDIR2);
    wndId2Enum[rgrENABLEDIRCTR1] = LMS7param(ENABLEDIRCTR1);
    wndId2Enum[rgrENABLEDIR1] = LMS7param(ENABLEDIR1);
    wndId2Enum[chkMCLK1_INV] = LMS7param(MCLK1_INV);
    wndId2Enum[chkMCLK2_INV] = LMS7param(MCLK2_INV);
    wndId2Enum[cmbFCLK1_DLY] = LMS7param(FCLK1_DLY);
    wndId2Enum[cmbFCLK2_DLY] = LMS7param(FCLK2_DLY);
    wndId2Enum[cmbMCLK1_DLY] = LMS7param(MCLK1_DLY);
    wndId2Enum[cmbMCLK2_DLY] = LMS7param(MCLK2_DLY);
    wndId2Enum[chkLML1_TRXIQPULSE] = LMS7param(LML1_TRXIQPULSE);
    wndId2Enum[chkLML2_TRXIQPULSE] = LMS7param(LML2_TRXIQPULSE);
    wndId2Enum[chkLML1_SISODDR] = LMS7param(LML1_SISODDR);
    wndId2Enum[chkLML2_SISODDR] = LMS7param(LML2_SISODDR);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlLimeLightPAD_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlLimeLightPAD_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlLimeLightPAD_view::ParameterChangeHandler(wxCommandEvent& event)
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
}

void lms7002_pnlLimeLightPAD_view::onbtnReadVerRevMask( wxCommandEvent& event )
{
    uint16_t value = 0;

    LMS_ReadParam(lmsControl,LMS7param(VER),&value);
    lblVER->SetLabel(wxString::Format(_("%i"), value));

    LMS_ReadParam(lmsControl,LMS7param(REV),&value);
    lblREV->SetLabel(wxString::Format(_("%i"), value));

    LMS_ReadParam(lmsControl,LMS7param(MASK),&value);
    lblMASK->SetLabel(wxString::Format(_("%i"), value));
}

void lms7002_pnlLimeLightPAD_view::UpdateGUI()
{
    wxArrayString padStrenghts;
    uint16_t value = 0;
    LMS_ReadParam(lmsControl, LMS7_MASK, &value);
    if(value == 0)
    {
        padStrenghts.push_back("4 mA");
        padStrenghts.push_back("8 mA");
    }
    else
    {
        padStrenghts.push_back("8 mA");
        padStrenghts.push_back("12 mA");
    }
    wxRadioBox* padStrenghtCtrl[] = {rgrSDA_DS, rgrSCL_DS, rgrSDIO_DS, rgrDIQ2_DS, rgrDIQ1_DS};
    for(auto i : padStrenghtCtrl)
    {
        i->SetString(0, padStrenghts[0]);
        i->SetString(1, padStrenghts[1]);
    }
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
