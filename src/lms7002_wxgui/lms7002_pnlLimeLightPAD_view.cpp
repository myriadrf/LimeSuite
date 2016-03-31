#include "lms7002_pnlLimeLightPAD_view.h"
#include "LMS7002M.h"
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
	wndId2Enum[rgrDIQ1_DS] = DIQ1_DS;
    wndId2Enum[chkDIQ1_PE] = DIQ1_PE;
    wndId2Enum[rgrDIQ2_DS] = DIQ2_DS;
    wndId2Enum[chkDIQ2_PE] = DIQ2_PE;
    wndId2Enum[rgrRX_MUX] = RX_MUX;
    wndId2Enum[chkFCLK1_PE] = FCLK1_PE;
    wndId2Enum[chkFCLK2_PE] = FCLK2_PE;
    wndId2Enum[chkIQ_SEL_EN_1_PE] = IQ_SEL_EN_1_PE;
    wndId2Enum[chkIQ_SEL_EN_2_PE] = IQ_SEL_EN_2_PE;
    wndId2Enum[cmbLML1_AIP] = LML1_AIP;
    wndId2Enum[cmbLML1_AQP] = LML1_AQP;
    wndId2Enum[cmbLML1_BIP] = LML1_BIP;
    wndId2Enum[cmbLML1_BQP] = LML1_BQP;
    wndId2Enum[cmbLML1_RX_PRE] = LML1_RF2BB_PRE;
    wndId2Enum[cmbLML1_RX_PST] = LML1_RF2BB_PST;
    wndId2Enum[cmbLML1_S0S] = LML1_S0S;
    wndId2Enum[cmbLML1_S1S] = LML1_S1S;
    wndId2Enum[cmbLML1_S2S] = LML1_S2S;
    wndId2Enum[cmbLML1_S3S] = LML1_S3S;
    wndId2Enum[cmbLML1_TX_PRE] = LML1_BB2RF_PRE;
    wndId2Enum[cmbLML1_TX_PST] = LML1_BB2RF_PST;
    wndId2Enum[cmbLML2_AIP] = LML2_AIP;
    wndId2Enum[cmbLML2_AQP] = LML2_AQP;
    wndId2Enum[cmbLML2_BIP] = LML2_BIP;
    wndId2Enum[cmbLML2_BQP] = LML2_BQP;
    wndId2Enum[cmbLML2_RX_PRE] = LML2_RF2BB_PRE;
    wndId2Enum[cmbLML2_RX_PST] = LML2_RF2BB_PST;
    wndId2Enum[cmbLML2_S0S] = LML2_S0S;
    wndId2Enum[cmbLML2_S1S] = LML2_S1S;
    wndId2Enum[cmbLML2_S2S] = LML2_S2S;
    wndId2Enum[cmbLML2_S3S] = LML2_S3S;
    wndId2Enum[cmbLML2_TX_PRE] = LML2_BB2RF_PRE;
    wndId2Enum[cmbLML2_TX_PST] = LML2_BB2RF_PST;
    wndId2Enum[rgrLML_MODE1] = LML1_MODE;
    wndId2Enum[rgrLML_MODE2] = LML2_MODE;
    wndId2Enum[rgrLML_TXNRXIQ1] = LML1_TXNRXIQ;
    wndId2Enum[rgrLML_TXNRXIQ2] = LML2_TXNRXIQ;
    wndId2Enum[chkLRST_RX_A] = LRST_RX_A;
    wndId2Enum[chkLRST_RX_B] = LRST_RX_B;
    wndId2Enum[chkLRST_TX_A] = LRST_TX_A;
    wndId2Enum[chkLRST_TX_B] = LRST_TX_B;
    wndId2Enum[cmbMCLK1SRC] = MCLK1SRC;
    wndId2Enum[cmbRXTSPCLKA_DIV] = RXTSPCLKA_DIV;
    wndId2Enum[chkMCLK1_PE] = MCLK1_PE;
    wndId2Enum[cmbMCLK2SRC] = MCLK2SRC;
    wndId2Enum[cmbTXTSPCLKA_DIV] = TXTSPCLKA_DIV;
    wndId2Enum[chkMCLK2_PE] = MCLK2_PE;
    wndId2Enum[chkMIMO_SISO] = MIMO_SISO;
    wndId2Enum[chkMOD_EN] = MOD_EN;
    wndId2Enum[chkMRST_RX_A] = MRST_RX_A;
    wndId2Enum[chkMRST_RX_B] = MRST_RX_B;
    wndId2Enum[chkMRST_TX_A] = MRST_TX_A;
    wndId2Enum[chkMRST_TX_B] = MRST_TX_B;
    wndId2Enum[chkRXDIVEN] = RXDIVEN;
    wndId2Enum[chkRXEN_A] = RXEN_A;
    wndId2Enum[chkRXEN_B] = RXEN_B;
    wndId2Enum[cmbRXRDCLK_MUX] = RXRDCLK_MUX;
    wndId2Enum[cmbRXWRCLK_MUX] = RXWRCLK_MUX;
    wndId2Enum[chkRX_CLK_PE] = RX_CLK_PE;
    wndId2Enum[chkSCLK_PE] = SCLK_PE;
    wndId2Enum[rgrSCL_DS] = SCL_DS;
    wndId2Enum[chkSCL_PE] = SCL_PE;
    wndId2Enum[rgrSDA_DS] = SDA_DS;
    wndId2Enum[chkSDA_PE] = SDA_PE;
    wndId2Enum[rgrSDIO_DS] = SDIO_DS;
    wndId2Enum[chkSDIO_PE] = SDIO_PE;
    wndId2Enum[chkSDO_PE] = SDO_PE;
    wndId2Enum[chkSEN_PE] = SEN_PE;
    wndId2Enum[rgrSPIMODE] = SPIMODE;
    wndId2Enum[chkSRST_RXFIFO] = SRST_RXFIFO;
    wndId2Enum[chkSRST_TXFIFO] = SRST_TXFIFO;
    wndId2Enum[chkTXDIVEN] = TXDIVEN;
    wndId2Enum[chkTXEN_A] = TXEN_A;
    wndId2Enum[chkTXEN_B] = TXEN_B;
    wndId2Enum[rgrTX_MUX] = TX_MUX;
    wndId2Enum[chkTXNRX1_PE] = TXNRX1_PE;
    wndId2Enum[chkTXNRX2_PE] = TXNRX2_PE;
    wndId2Enum[cmbTXRDCLK_MUX] = TXRDCLK_MUX;
    wndId2Enum[cmbTXWRCLK_MUX] = TXWRCLK_MUX;
    wndId2Enum[chkTX_CLK_PE] = TX_CLK_PE;
    wndId2Enum[rgrLML_FIDM1] = LML1_FIDM;
    wndId2Enum[rgrLML_FIDM2] = LML2_FIDM;
    wndId2Enum[chkFCLK1_INV] = FCLK1_INV;
    wndId2Enum[chkFCLK2_INV] = FCLK2_INV;
    wndId2Enum[cmbMCLK1DLY] = MCLK1DLY;
    wndId2Enum[cmbMCLK2DLY] = MCLK2DLY;
    wndId2Enum[rgrDIQDIRCTR2] = DIQDIRCTR2;
    wndId2Enum[rgrDIQDIR2] = DIQDIR2;
    wndId2Enum[rgrDIQDIRCTR1] = DIQDIRCTR1;
    wndId2Enum[rgrDIQDIR1] = DIQDIR1;
    wndId2Enum[rgrENABLEDIRCTR2] = ENABLEDIRCTR2;
    wndId2Enum[rgrENABLEDIR2] = ENABLEDIR2;
    wndId2Enum[rgrENABLEDIRCTR1] = ENABLEDIRCTR1;
    wndId2Enum[rgrENABLEDIR1] = ENABLEDIR1;

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlLimeLightPAD_view::Initialize(LMS7002M* pControl)
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
    lmsControl->Modify_SPI_Reg_bits(parameter, event.GetInt());
}

void lms7002_pnlLimeLightPAD_view::onbtnReadVerRevMask( wxCommandEvent& event )
{
    int value = 0;
    value = lmsControl->Get_SPI_Reg_bits(VER);
    lblVER->SetLabel(wxString::Format(_("%i"), value));

    value = lmsControl->Get_SPI_Reg_bits(REV);
    lblREV->SetLabel(wxString::Format(_("%i"), value));

    value = lmsControl->Get_SPI_Reg_bits(MASK);
    lblMASK->SetLabel(wxString::Format(_("%i"), value));
}

void lms7002_pnlLimeLightPAD_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
