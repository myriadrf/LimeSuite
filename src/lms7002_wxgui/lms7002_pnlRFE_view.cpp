#include "lms7002_pnlRFE_view.h"
#include <wx/msgdlg.h>
#include <map>
#include <vector>
#include <assert.h>
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include "LMS7002M_parameters.h"

using namespace lime;
using namespace LMS7002_WXGUI;

static indexValueMap g_lna_rfe_IndexValuePairs;
static indexValueMap g_tia_rfe_IndexValuePairs;

lms7002_pnlRFE_view::lms7002_pnlRFE_view( wxWindow* parent )
    : pnlRFE_view(parent), lmsControl(nullptr)
{

}

lms7002_pnlRFE_view::lms7002_pnlRFE_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : pnlRFE_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    //ids for updating from chip
	wndId2Enum[cmbCAP_RXMXO_RFE] = LMS7param(CAP_RXMXO_RFE);
    wndId2Enum[cmbCCOMP_TIA_RFE] = LMS7param(CCOMP_TIA_RFE);
    wndId2Enum[cmbCFB_TIA_RFE] = LMS7param(CFB_TIA_RFE);
    wndId2Enum[cmbCGSIN_LNA_RFE] = LMS7param(CGSIN_LNA_RFE);
    wndId2Enum[cmbDCOFFI_RFE] = LMS7param(DCOFFI_RFE);
    wndId2Enum[cmbDCOFFQ_RFE] = LMS7param(DCOFFQ_RFE);
    wndId2Enum[chkEN_DCOFF_RXFE_RFE] = LMS7param(EN_DCOFF_RXFE_RFE);
    wndId2Enum[chkEN_G_RFE] = LMS7param(EN_G_RFE);
    wndId2Enum[chkEN_INSHSW_LB1_RFE] = LMS7param(EN_INSHSW_LB1_RFE);
    wndId2Enum[chkEN_INSHSW_LB2_RFE] = LMS7param(EN_INSHSW_LB2_RFE);
    wndId2Enum[chkEN_INSHSW_L_RFE] = LMS7param(EN_INSHSW_L_RFE);
    wndId2Enum[chkEN_INSHSW_W_RFE] = LMS7param(EN_INSHSW_W_RFE);
    wndId2Enum[chkEN_NEXTRX_RFE] = LMS7param(EN_NEXTRX_RFE);
    wndId2Enum[cmbG_LNA_RFE] = LMS7param(G_LNA_RFE);
    wndId2Enum[cmbG_RXLOOPB_RFE] = LMS7param(G_RXLOOPB_RFE);
    wndId2Enum[cmbG_TIA_RFE] = LMS7param(G_TIA_RFE);
    wndId2Enum[cmbICT_LNACMO_RFE] = LMS7param(ICT_LNACMO_RFE);
    wndId2Enum[cmbICT_LNA_RFE] = LMS7param(ICT_LNA_RFE);
    wndId2Enum[cmbICT_LODC_RFE] = LMS7param(ICT_LODC_RFE);
    wndId2Enum[cmbICT_LOOPB_RFE] = LMS7param(ICT_LOOPB_RFE);
    wndId2Enum[cmbICT_TIAMAIN_RFE] = LMS7param(ICT_TIAMAIN_RFE);
    wndId2Enum[cmbICT_TIAOUT_RFE] = LMS7param(ICT_TIAOUT_RFE);
    wndId2Enum[chkPD_LNA_RFE] = LMS7param(PD_LNA_RFE);
    wndId2Enum[chkPD_MXLOBUF_RFE] = LMS7param(PD_MXLOBUF_RFE);
    wndId2Enum[chkPD_QGEN_RFE] = LMS7param(PD_QGEN_RFE);
    wndId2Enum[chkPD_RLOOPB_1_RFE] = LMS7param(PD_RLOOPB_1_RFE);
    wndId2Enum[chkPD_RLOOPB_2_RFE] = LMS7param(PD_RLOOPB_2_RFE);
    wndId2Enum[chkPD_RSSI_RFE] = LMS7param(PD_RSSI_RFE);
    wndId2Enum[chkPD_TIA_RFE] = LMS7param(PD_TIA_RFE);
    wndId2Enum[cmbRCOMP_TIA_RFE] = LMS7param(RCOMP_TIA_RFE);
    wndId2Enum[cmbRFB_TIA_RFE] = LMS7param(RFB_TIA_RFE);
    wndId2Enum[cmbSEL_PATH_RFE] = LMS7param(SEL_PATH_RFE);
    wndId2Enum[cmbCDC_I_RFE] = LMS7param(CDC_I_RFE);
    wndId2Enum[cmbCDC_Q_RFE] = LMS7param(CDC_Q_RFE);
    wndId2Enum[chkEN_DIR_RFE] = LMS7param(EN_DIR_RFE);

    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i fF"), (i + 1) * 80));
    cmbCAP_RXMXO_RFE->Set(temp);

    for (int i = 0; i < 15; ++i)
        g_lna_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-30"));
    temp.push_back(_("Gmax-27"));
    temp.push_back(_("Gmax-24"));
    temp.push_back(_("Gmax-21"));
    temp.push_back(_("Gmax-18"));
    temp.push_back(_("Gmax-15"));
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-9"));
    temp.push_back(_("Gmax-6"));
    temp.push_back(_("Gmax-5"));
    temp.push_back(_("Gmax-4"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax-2"));
    temp.push_back(_("Gmax-1"));
    temp.push_back(_("Gmax"));
    cmbG_LNA_RFE->Set(temp);

    temp.clear();
    temp.push_back(_("Gmax-40"));
    temp.push_back(_("Gmax-24"));
    temp.push_back(_("Gmax-17"));
    temp.push_back(_("Gmax-14"));
    temp.push_back(_("Gmax-11"));
    temp.push_back(_("Gmax-9"));
    temp.push_back(_("Gmax-7.5"));
    temp.push_back(_("Gmax-6.2"));
    temp.push_back(_("Gmax-5"));
    temp.push_back(_("Gmax-4"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax-2.4"));
    temp.push_back(_("Gmax-1.6"));
    temp.push_back(_("Gmax-1"));
    temp.push_back(_("Gmax-0.5"));
    temp.push_back(_("Gmax"));
    cmbG_RXLOOPB_RFE->Set(temp);


    for (int i = 0; i < 3; ++i)
        g_tia_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax"));
    cmbG_TIA_RFE->Set(temp);

    temp.clear();
    float nominalCurrent = 500; //uA
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.1f uA"), nominalCurrent*(i / 12.0)));
    cmbICT_LNA_RFE->Set(temp);

    temp.clear();
    float Vth = 0.440; //V
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.3f V"), Vth + 3500 * 0.000020*(i / 12.0)));
    cmbICT_LODC_RFE->Set(temp);

    temp.clear();
    float IsupplyNominal = 1.80; //uA
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.3f uA"), IsupplyNominal*(i / 12.0)));
    cmbICT_LOOPB_RFE->Set(temp);

    temp.clear();
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbRFB_TIA_RFE->Set(temp);
    cmbICT_LNACMO_RFE->Set(temp);
    cmbCGSIN_LNA_RFE->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbRCOMP_TIA_RFE->Set(temp);
    cmbCDC_I_RFE->Set(temp);
    cmbCDC_Q_RFE->Set(temp);
    cmbCCOMP_TIA_RFE->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlRFE_view::Initialize(lms_device_t* pControl)
{
	lmsControl = pControl;
	assert(lmsControl != nullptr);
}

void lms7002_pnlRFE_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(G_LNA_RFE),&value);
    cmbG_LNA_RFE->SetSelection( value2index(value, g_lna_rfe_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(G_TIA_RFE),&value);
    cmbG_TIA_RFE->SetSelection( value2index(value, g_tia_rfe_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(DCOFFI_RFE),&value);
    int16_t dcvalue = value & 0x3F;
    if((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFI_RFE->SetValue(dcvalue);
    LMS_ReadParam(lmsControl,LMS7param(DCOFFQ_RFE),&value);
    dcvalue = value & 0x3F;
    if((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFQ_RFE->SetValue(dcvalue);

    //check if B channel is enabled
    uint16_t macBck;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&macBck);
    if (macBck >= 2)
        chkEN_NEXTRX_RFE->Hide();
    else
        chkEN_NEXTRX_RFE->Show();

    LMS_ReadParam(lmsControl,LMS7param(TRX_GAIN_SRC),&value);
    cmbG_LNA_RFE->Enable(!value);
    cmbG_TIA_RFE->Enable(!value);
}

void lms7002_pnlRFE_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlRFE_view::ParameterChangeHandler( wxCommandEvent& event )
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
    if (event.GetEventObject() == cmbG_LNA_RFE)
    {
        value = index2value(value, g_lna_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbG_TIA_RFE)
    {
        value = index2value(value, g_tia_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbDCOFFI_RFE || event.GetEventObject() == cmbDCOFFQ_RFE)
    {
        uint16_t valToSend = 0;
        if (value < 0)
            valToSend |= 0x40;
        valToSend |= labs(value);
        LMS_WriteParam(lmsControl,parameter,valToSend);
        return;
    }
    LMS_WriteParam(lmsControl,parameter,value);
}
