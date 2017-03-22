#include "lms7002_pnlLDO_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlLDO_view::lms7002_pnlLDO_view( wxWindow* parent )
:
pnlLDO_view( parent )
{

}

lms7002_pnlLDO_view::lms7002_pnlLDO_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlLDO_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    /*wndId2Enum[chkBYP_LDO_AFE] = BYP_LDO_AFE;
    wndId2Enum[chkBYP_LDO_CPGN] = BYP_LDO_CPGN;
    wndId2Enum[chkBYP_LDO_CPSXR] = BYP_LDO_CPSXR;
    wndId2Enum[chkBYP_LDO_CPSXT] = BYP_LDO_CPSXT;
    wndId2Enum[chkBYP_LDO_DIG] = BYP_LDO_DIG;
    wndId2Enum[chkBYP_LDO_DIGGN] = BYP_LDO_DIGGN;
    wndId2Enum[chkBYP_LDO_DIGIp1] = BYP_LDO_DIGIp1;
    wndId2Enum[chkBYP_LDO_DIGIp2] = BYP_LDO_DIGIp2;
    wndId2Enum[chkBYP_LDO_DIGSXR] = BYP_LDO_DIGSXR;
    wndId2Enum[chkBYP_LDO_DIGSXT] = BYP_LDO_DIGSXT;
    wndId2Enum[chkBYP_LDO_DIVGN] = BYP_LDO_DIVGN;
    wndId2Enum[chkBYP_LDO_DIVSXR] = BYP_LDO_DIVSXR;
    wndId2Enum[chkBYP_LDO_DIVSXT] = BYP_LDO_DIVSXT;
    wndId2Enum[chkBYP_LDO_LNA12] = BYP_LDO_LNA12;
    wndId2Enum[chkBYP_LDO_LNA14] = BYP_LDO_LNA14;
    wndId2Enum[chkBYP_LDO_MXRFE] = BYP_LDO_MXRFE;
    wndId2Enum[chkBYP_LDO_RBB] = BYP_LDO_RBB;
    wndId2Enum[chkBYP_LDO_RXBUF] = BYP_LDO_RXBUF;
    wndId2Enum[chkBYP_LDO_SPIBUF] = BYP_LDO_SPIBUF;
    wndId2Enum[chkBYP_LDO_TBB] = BYP_LDO_TBB;
    wndId2Enum[chkBYP_LDO_TIA12] = BYP_LDO_TIA12;
    wndId2Enum[chkBYP_LDO_TIA14] = BYP_LDO_TIA14;
    wndId2Enum[chkBYP_LDO_TLOB] = BYP_LDO_TLOB;
    wndId2Enum[chkBYP_LDO_TPAD] = BYP_LDO_TPAD;
    wndId2Enum[chkBYP_LDO_TXBUF] = BYP_LDO_TXBUF;
    wndId2Enum[chkBYP_LDO_VCOGN] = BYP_LDO_VCOGN;
    wndId2Enum[chkBYP_LDO_VCOSXR] = BYP_LDO_VCOSXR;
    wndId2Enum[chkBYP_LDO_VCOSXT] = BYP_LDO_VCOSXT;*/
    wndId2Enum[chkEN_G_LDO] = LMS7param(EN_G_LDO);
    wndId2Enum[chkEN_G_LDOP] = LMS7param(EN_G_LDOP);
    wndId2Enum[chkEN_LDO_AFE] = LMS7param(EN_LDO_AFE);
    wndId2Enum[chkEN_LDO_CPGN] = LMS7param(EN_LDO_CPGN);
    wndId2Enum[chkEN_LDO_CPSXR] = LMS7param(EN_LDO_CPSXR);
    wndId2Enum[chkEN_LDO_CPSXT] = LMS7param(EN_LDO_CPSXT);
    wndId2Enum[chkEN_LDO_DIG] = LMS7param(EN_LDO_DIG);
    wndId2Enum[chkEN_LDO_DIGGN] = LMS7param(EN_LDO_DIGGN);
    wndId2Enum[chkPD_LDO_DIGIp1] = LMS7param(PD_LDO_DIGIp1);
    wndId2Enum[chkPD_LDO_DIGIp2] = LMS7param(PD_LDO_DIGIp2);
    wndId2Enum[chkEN_LDO_DIGSXR] = LMS7param(EN_LDO_DIGSXR);
    wndId2Enum[chkEN_LDO_DIGSXT] = LMS7param(EN_LDO_DIGSXT);
    wndId2Enum[chkEN_LDO_DIVGN] = LMS7param(EN_LDO_DIVGN);
    wndId2Enum[chkEN_LDO_DIVSXR] = LMS7param(EN_LDO_DIVSXR);
    wndId2Enum[chkEN_LDO_DIVSXT] = LMS7param(EN_LDO_DIVSXT);
    wndId2Enum[chkEN_LDO_LNA12] = LMS7param(EN_LDO_LNA12);
    wndId2Enum[chkEN_LDO_LNA14] = LMS7param(EN_LDO_LNA14);
    wndId2Enum[chkEN_LDO_MXRFE] = LMS7param(EN_LDO_MXRFE);
    wndId2Enum[chkEN_LDO_RBB] = LMS7param(EN_LDO_RBB);
    wndId2Enum[chkEN_LDO_RXBUF] = LMS7param(EN_LDO_RXBUF);
    wndId2Enum[chkPD_LDO_SPIBUF] = LMS7param(PD_LDO_SPIBUF);
    wndId2Enum[chkEN_LDO_TBB] = LMS7param(EN_LDO_TBB);
    wndId2Enum[chkEN_LDO_TIA12] = LMS7param(EN_LDO_TIA12);
    wndId2Enum[chkEN_LDO_TIA14] = LMS7param(EN_LDO_TIA14);
    wndId2Enum[chkEN_LDO_TLOB] = LMS7param(EN_LDO_TLOB);
    wndId2Enum[chkEN_LDO_TPAD] = LMS7param(EN_LDO_TPAD);
    wndId2Enum[chkEN_LDO_TXBUF] = LMS7param(EN_LDO_TXBUF);
    wndId2Enum[chkEN_LDO_VCOGN] = LMS7param(EN_LDO_VCOGN);
    wndId2Enum[chkEN_LDO_VCOSXR] = LMS7param(EN_LDO_VCOSXR);
    wndId2Enum[chkEN_LDO_VCOSXT] = LMS7param(EN_LDO_VCOSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_AFE] = LMS7param(EN_LOADIMP_LDO_AFE);
    wndId2Enum[chkEN_LOADIMP_LDO_CPGN] = LMS7param(EN_LOADIMP_LDO_CPGN);
    wndId2Enum[chkEN_LOADIMP_LDO_CPSXR] = LMS7param(EN_LOADIMP_LDO_CPSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_CPSXT] = LMS7param(EN_LOADIMP_LDO_CPSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_DIG] = LMS7param(EN_LOADIMP_LDO_DIG);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGGN] = LMS7param(EN_LOADIMP_LDO_DIGGN);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGIp1] = LMS7param(EN_LOADIMP_LDO_DIGIp1);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGIp2] = LMS7param(EN_LOADIMP_LDO_DIGIp2);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGSXR] = LMS7param(EN_LOADIMP_LDO_DIGSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGSXT] = LMS7param(EN_LOADIMP_LDO_DIGSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_DIVGN] = LMS7param(EN_LOADIMP_LDO_DIVGN);
    wndId2Enum[chkEN_LOADIMP_LDO_DIVSXR] = LMS7param(EN_LOADIMP_LDO_DIVSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_DIVSXT] = LMS7param(EN_LOADIMP_LDO_DIVSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_LNA12] = LMS7param(EN_LOADIMP_LDO_LNA12);
    wndId2Enum[chkEN_LOADIMP_LDO_LNA14] = LMS7param(EN_LOADIMP_LDO_LNA14);
    wndId2Enum[chkEN_LOADIMP_LDO_MXRFE] = LMS7param(EN_LOADIMP_LDO_MXRFE);
    wndId2Enum[chkEN_LOADIMP_LDO_RBB] = LMS7param(EN_LOADIMP_LDO_RBB);
    wndId2Enum[chkEN_LOADIMP_LDO_RXBUF] = LMS7param(EN_LOADIMP_LDO_RXBUF);
    wndId2Enum[chkEN_LOADIMP_LDO_SPIBUF] = LMS7param(EN_LOADIMP_LDO_SPIBUF);
    wndId2Enum[chkEN_LOADIMP_LDO_TBB] = LMS7param(EN_LOADIMP_LDO_TBB);
    wndId2Enum[chkEN_LOADIMP_LDO_TIA12] = LMS7param(EN_LOADIMP_LDO_TIA12);
    wndId2Enum[chkEN_LOADIMP_LDO_TIA14] = LMS7param(EN_LOADIMP_LDO_TIA14);
    wndId2Enum[chkEN_LOADIMP_LDO_TLOB] = LMS7param(EN_LOADIMP_LDO_TLOB);
    wndId2Enum[chkEN_LOADIMP_LDO_TPAD] = LMS7param(EN_LOADIMP_LDO_TPAD);
    wndId2Enum[chkEN_LOADIMP_LDO_TXBUF] = LMS7param(EN_LOADIMP_LDO_TXBUF);
    wndId2Enum[chkEN_LOADIMP_LDO_VCOGN] = LMS7param(EN_LOADIMP_LDO_VCOGN);
    wndId2Enum[chkEN_LOADIMP_LDO_VCOSXR] = LMS7param(EN_LOADIMP_LDO_VCOSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_VCOSXT] = LMS7param(EN_LOADIMP_LDO_VCOSXT);
    wndId2Enum[cmbRDIV_AFE] = LMS7param(RDIV_AFE);
    wndId2Enum[cmbRDIV_CPGN] = LMS7param(RDIV_CPGN);
    wndId2Enum[cmbRDIV_CPSXR] = LMS7param(RDIV_CPSXR);
    wndId2Enum[cmbRDIV_CPSXT] = LMS7param(RDIV_CPSXT);
    wndId2Enum[cmbRDIV_DIG] = LMS7param(RDIV_DIG);
    wndId2Enum[cmbRDIV_DIGGN] = LMS7param(RDIV_DIGGN);
    wndId2Enum[cmbRDIV_DIGIp1] = LMS7param(RDIV_DIGIp1);
    wndId2Enum[cmbRDIV_DIGIp2] = LMS7param(RDIV_DIGIp2);
    wndId2Enum[cmbRDIV_DIGSXR] = LMS7param(RDIV_DIGSXR);
    wndId2Enum[cmbRDIV_DIGSXT] = LMS7param(RDIV_DIGSXT);
    wndId2Enum[cmbRDIV_DIVGN] = LMS7param(RDIV_DIVGN);
    wndId2Enum[cmbRDIV_DIVSXR] = LMS7param(RDIV_DIVSXR);
    wndId2Enum[cmbRDIV_DIVSXT] = LMS7param(RDIV_DIVSXT);
    wndId2Enum[cmbRDIV_LNA12] = LMS7param(RDIV_LNA12);
    wndId2Enum[cmbRDIV_LNA14] = LMS7param(RDIV_LNA14);
    wndId2Enum[cmbRDIV_MXRFE] = LMS7param(RDIV_MXRFE);
    wndId2Enum[cmbRDIV_RBB] = LMS7param(RDIV_RBB);
    wndId2Enum[cmbRDIV_RXBUF] = LMS7param(RDIV_RXBUF);
    wndId2Enum[cmbRDIV_SPIBUF] = LMS7param(RDIV_SPIBUF);
    wndId2Enum[cmbRDIV_TBB] = LMS7param(RDIV_TBB);
    wndId2Enum[cmbRDIV_TIA12] = LMS7param(RDIV_TIA12);
    wndId2Enum[cmbRDIV_TIA14] = LMS7param(RDIV_TIA14);
    wndId2Enum[cmbRDIV_TLOB] = LMS7param(RDIV_TLOB);
    wndId2Enum[cmbRDIV_TPAD] = LMS7param(RDIV_TPAD);
    wndId2Enum[cmbRDIV_TXBUF] = LMS7param(RDIV_TXBUF);
    wndId2Enum[cmbRDIV_VCOGN] = LMS7param(RDIV_VCOGN);
    wndId2Enum[cmbRDIV_VCOSXR] = LMS7param(RDIV_VCOSXR);
    wndId2Enum[cmbRDIV_VCOSXT] = LMS7param(RDIV_VCOSXT);
    wndId2Enum[chkSPDUP_LDO_AFE] = LMS7param(SPDUP_LDO_AFE);
    wndId2Enum[chkSPDUP_LDO_CPGN] = LMS7param(SPDUP_LDO_CPGN);
    wndId2Enum[chkSPDUP_LDO_CPSXR] = LMS7param(SPDUP_LDO_CPSXR);
    wndId2Enum[chkSPDUP_LDO_CPSXT] = LMS7param(SPDUP_LDO_CPSXT);
    wndId2Enum[chkSPDUP_LDO_DIG] = LMS7param(SPDUP_LDO_DIG);
    wndId2Enum[chkSPDUP_LDO_DIGGN] = LMS7param(SPDUP_LDO_DIGGN);
    wndId2Enum[chkSPDUP_LDO_DIGIp1] = LMS7param(SPDUP_LDO_DIGIp1);
    wndId2Enum[chkSPDUP_LDO_DIGIp2] = LMS7param(SPDUP_LDO_DIGIp2);
    wndId2Enum[chkSPDUP_LDO_DIGSXR] = LMS7param(SPDUP_LDO_DIGSXR);
    wndId2Enum[chkSPDUP_LDO_DIGSXT] = LMS7param(SPDUP_LDO_DIGSXT);
    wndId2Enum[chkSPDUP_LDO_DIVGN] = LMS7param(SPDUP_LDO_DIVGN);
    wndId2Enum[chkSPDUP_LDO_DIVSXR] = LMS7param(SPDUP_LDO_DIVSXR);
    wndId2Enum[chkSPDUP_LDO_DIVSXT] = LMS7param(SPDUP_LDO_DIVSXT);
    wndId2Enum[chkSPDUP_LDO_LNA12] = LMS7param(SPDUP_LDO_LNA12);
    wndId2Enum[chkSPDUP_LDO_LNA14] = LMS7param(SPDUP_LDO_LNA14);
    wndId2Enum[chkSPDUP_LDO_MXRFE] = LMS7param(SPDUP_LDO_MXRFE);
    wndId2Enum[chkSPDUP_LDO_RBB] = LMS7param(SPDUP_LDO_RBB);
    wndId2Enum[chkSPDUP_LDO_RXBUF] = LMS7param(SPDUP_LDO_RXBUF);
    wndId2Enum[chkSPDUP_LDO_SPIBUF] = LMS7param(SPDUP_LDO_SPIBUF);
    wndId2Enum[chkSPDUP_LDO_TBB] = LMS7param(SPDUP_LDO_TBB);
    wndId2Enum[chkSPDUP_LDO_TIA12] = LMS7param(SPDUP_LDO_TIA12);
    wndId2Enum[chkSPDUP_LDO_TIA14] = LMS7param(SPDUP_LDO_TIA14);
    wndId2Enum[chkSPDUP_LDO_TLOB] = LMS7param(SPDUP_LDO_TLOB);
    wndId2Enum[chkSPDUP_LDO_TPAD] = LMS7param(SPDUP_LDO_TPAD);
    wndId2Enum[chkSPDUP_LDO_TXBUF] = LMS7param(SPDUP_LDO_TXBUF);
    wndId2Enum[chkSPDUP_LDO_VCOGN] = LMS7param(SPDUP_LDO_VCOGN);
    wndId2Enum[chkSPDUP_LDO_VCOSXR] = LMS7param(SPDUP_LDO_VCOSXR);
    wndId2Enum[chkSPDUP_LDO_VCOSXT] = LMS7param(SPDUP_LDO_VCOSXT);
    wndId2Enum[cmbISINK_SPIBUFF] = LMS7param(ISINK_SPIBUFF);
    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<256; ++i)
    {
        temp.push_back(wxString::Format(_("%.3f V"), (860.0 + 3.92*i) / 1000.0));
    }
    cmbRDIV_AFE->Append(temp);
    cmbRDIV_CPGN->Append(temp);
    cmbRDIV_CPSXR->Append(temp);
    cmbRDIV_CPSXT->Append(temp);
    cmbRDIV_DIG->Append(temp);
    cmbRDIV_DIGGN->Append(temp);
    cmbRDIV_DIGIp1->Append(temp);
    cmbRDIV_DIGIp2->Append(temp);
    cmbRDIV_DIGSXR->Append(temp);
    cmbRDIV_DIGSXT->Append(temp);
    cmbRDIV_DIVGN->Append(temp);
    cmbRDIV_DIVSXR->Append(temp);
    cmbRDIV_DIVSXT->Append(temp);
    cmbRDIV_LNA12->Append(temp);
    cmbRDIV_LNA14->Append(temp);
    cmbRDIV_MXRFE->Append(temp);
    cmbRDIV_RBB->Append(temp);
    cmbRDIV_RXBUF->Append(temp);
    cmbRDIV_SPIBUF->Append(temp);
    cmbRDIV_TBB->Append(temp);
    cmbRDIV_TIA12->Append(temp);
    cmbRDIV_TIA14->Append(temp);
    cmbRDIV_TLOB->Append(temp);
    cmbRDIV_TPAD->Append(temp);
    cmbRDIV_TXBUF->Append(temp);
    cmbRDIV_VCOGN->Append(temp);
    cmbRDIV_VCOSXR->Append(temp);
    cmbRDIV_VCOSXT->Append(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlLDO_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlLDO_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlLDO_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
