#include "lms7002_pnlTRF_view.h"
#include "LMS7002M.h"
#include <map>
#include <vector>
#include "lms7002_gui_utilities.h"
#include "wx/msgdlg.h"
#include "lms7suiteEvents.h"

using namespace LMS7002_WXGUI;
static indexValueMap en_amphf_pdet_trfIndexValuePairs;

lms7002_pnlTRF_view::lms7002_pnlTRF_view( wxWindow* parent )
:
pnlTRF_view( parent )
{

}

lms7002_pnlTRF_view::lms7002_pnlTRF_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlTRF_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    wndId2Enum[cmbEN_AMPHF_PDET_TRF] = EN_AMPHF_PDET_TRF;
    wndId2Enum[chkEN_G_TRF] = EN_G_TRF;
    wndId2Enum[chkEN_LOOPB_TXPAD_TRF] = EN_LOOPB_TXPAD_TRF;
    wndId2Enum[cmbEN_LOWBWLOMX_TMX_TRF] = EN_LOWBWLOMX_TMX_TRF;
    wndId2Enum[chkEN_NEXTTX_TRF] = EN_NEXTTX_TRF;
    wndId2Enum[cmbF_TXPAD_TRF] = F_TXPAD_TRF;
    wndId2Enum[rgrGCAS_GNDREF_TXPAD_TRF] = GCAS_GNDREF_TXPAD_TRF;
    wndId2Enum[cmbICT_LIN_TXPAD_TRF] = ICT_LIN_TXPAD_TRF;
    wndId2Enum[cmbICT_MAIN_TXPAD_TRF] = ICT_MAIN_TXPAD_TRF;
    wndId2Enum[cmbLOADR_PDET_TRF] = LOADR_PDET_TRF;
    wndId2Enum[cmbLOBIASN_TXM_TRF] = LOBIASN_TXM_TRF;
    wndId2Enum[cmbLOBIASP_TXX_TRF] = LOBIASP_TXX_TRF;
    wndId2Enum[cmbLOSS_LIN_TXPAD_TRF] = LOSS_LIN_TXPAD_TRF;
    wndId2Enum[cmbLOSS_MAIN_TXPAD_TRF] = LOSS_MAIN_TXPAD_TRF;
    wndId2Enum[cmbL_LOOPB_TXPAD_TRF] = L_LOOPB_TXPAD_TRF;
    wndId2Enum[chkPD_PDET_TRF] = PD_PDET_TRF;
    wndId2Enum[chkPD_TLOBUF_TRF] = PD_TLOBUF_TRF;
    wndId2Enum[chkPD_TXPAD_TRF] = PD_TXPAD_TRF;
    wndId2Enum[cmbVGCAS_TXPAD_TRF] = VGCAS_TXPAD_TRF;
    wndId2Enum[cmbCDC_I_TRF] = CDC_I_TRF;
    wndId2Enum[cmbCDC_Q_TRF] = CDC_Q_TRF;
    wndId2Enum[chkEN_DIR_TRF] = EN_DIR_TRF;

    wxArrayString temp;
    temp.clear();
    temp.push_back(_("-10 db"));
    en_amphf_pdet_trfIndexValuePairs.push_back(indexValuePair(0,0));
    temp.push_back(_("7 db"));
    en_amphf_pdet_trfIndexValuePairs.push_back(indexValuePair(1,1));
    temp.push_back(_("25 db"));
    en_amphf_pdet_trfIndexValuePairs.push_back(indexValuePair(2,3));
    cmbEN_AMPHF_PDET_TRF->Set(temp);

    temp.clear();
    for(int i=0; i<8; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbF_TXPAD_TRF->Append(temp);

    temp.clear();
    float IbiasNominal = 20; //uA
    for(int i=0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.1f uA"), IbiasNominal*(i/12.0)));
    cmbICT_LIN_TXPAD_TRF->Append(temp);

    temp.clear();
    for(int i=0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbICT_MAIN_TXPAD_TRF->Append(temp);
    cmbLOBIASN_TXM_TRF->Append(temp);
    cmbLOBIASP_TXX_TRF->Append(temp);
    cmbLOSS_LIN_TXPAD_TRF->Append(temp);
    cmbLOSS_MAIN_TXPAD_TRF->Append(temp);
    cmbVGCAS_TXPAD_TRF->Append(temp);

    temp.clear();
    for(int i=0; i<16; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbCDC_I_TRF->Append(temp);
    cmbCDC_Q_TRF->Append(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlTRF_view::Initialize(LMS7002M* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlTRF_view::ParameterChangeHandler(wxCommandEvent& event)
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
    if(event.GetEventObject() == cmbEN_AMPHF_PDET_TRF)
        value = index2value(value, en_amphf_pdet_trfIndexValuePairs);
    lmsControl->Modify_SPI_Reg_bits(parameter, value);
}

void lms7002_pnlTRF_view::OnBandChange( wxCommandEvent& event )
{
    switch (cmbTXFEoutput->GetSelection())
    {
    case 0:
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND1_TRF, true);
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND2_TRF, false);
        break;
    case 1:
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND1_TRF, false);
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND2_TRF, true);
        break;
    case 2:
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND1_TRF, false);
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND2_TRF, false);
        break;
    default:
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND1_TRF, false);
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND2_TRF, false);
    }
    wxCommandEvent evt;
    evt.SetEventType(LMS7_TXBAND_CHANGED);
    evt.SetEventObject(this);
    evt.SetInt(event.GetInt());
    wxPostEvent(this, evt);
}

void lms7002_pnlTRF_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);

    long value = lmsControl->Get_SPI_Reg_bits(EN_AMPHF_PDET_TRF);
    cmbEN_AMPHF_PDET_TRF->SetSelection(value2index(value, en_amphf_pdet_trfIndexValuePairs));

    long SEL_BAND1_TRFvalue = lmsControl->Get_SPI_Reg_bits(SEL_BAND1_TRF);
    long SEL_BAND2_TRFvalue = lmsControl->Get_SPI_Reg_bits(SEL_BAND2_TRF);
    long TXFEoutputValue = 0;
    if (!SEL_BAND1_TRFvalue && !SEL_BAND2_TRFvalue)
        TXFEoutputValue = 2;
    else if (SEL_BAND1_TRFvalue && !SEL_BAND2_TRFvalue)
        TXFEoutputValue = 0;
    else if (!SEL_BAND1_TRFvalue && SEL_BAND2_TRFvalue)
        TXFEoutputValue = 1;
    else
    {
        TXFEoutputValue = 2;
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND1_TRF, false);
        lmsControl->Modify_SPI_Reg_bits(SEL_BAND2_TRF, false);
    }
    cmbTXFEoutput->SetSelection(TXFEoutputValue);

    //check if B channel is enabled
    uint8_t channel = lmsControl->Get_SPI_Reg_bits(MAC);
    if (channel > 1)
    {
        if (lmsControl->Get_SPI_Reg_bits(MIMO_SISO) != 0)
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
    }
}
