#include "lms7002_pnlSX_view.h"

#include <wx/textdlg.h>
#include <wx/valnum.h>
#include <wx/msgdlg.h>
#include <map>
#include <assert.h>
#include "numericSlider.h"
#include "lms7002_gui_utilities.h"
#include "lms7suiteEvents.h"
#include "lms7002_dlgVCOfrequencies.h"
#include <string>
using namespace std;
using namespace lime;

static bool showRefClkSpurCancelation = true;

lms7002_pnlSX_view::lms7002_pnlSX_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlSX_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    sizerR3->Add(new wxStaticText(this, wxID_ANY, _("PLL LPF zero resistor:")), 1, wxEXPAND, 0);
    cmbRZ_CTRL = new wxComboBox(this, wxID_ANY);
    cmbRZ_CTRL->Append(_("Rzero = 20 kOhm"));
    cmbRZ_CTRL->Append(_("Rzero = 8 kOhm"));
    cmbRZ_CTRL->Append(_("Rzero = 4 kOhm"));
    cmbRZ_CTRL->Append(_("LPF resistors are in bypass mode (<100 Ohm)"));
    cmbRZ_CTRL->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlSX_view::ParameterChangeHandler), NULL, this);
    wndId2Enum[cmbRZ_CTRL] = LMS7_RZ_CTRL;
    sizerR3->Add(cmbRZ_CTRL);

    sizerR3->Add(new wxStaticText(this, wxID_ANY, _("CMPLO_CTRL:")), 1, wxEXPAND, 0);
    cmbCMPLO_CTRL = new wxComboBox(this, wxID_ANY);
    cmbCMPLO_CTRL->Append(_("Low treshold is set to 0.18V"));
    cmbCMPLO_CTRL->Append(_("Low treshold is set to 0.1V"));
    cmbCMPLO_CTRL->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlSX_view::ParameterChangeHandler), NULL, this);
    sizerR3->Add(cmbCMPLO_CTRL, 0, 0, 5);
    wndId2Enum[cmbCMPLO_CTRL] = LMS7_CMPLO_CTRL_SX;

    //ids for updating from chip
    wndId2Enum[chkBYPLDO_VCO] = LMS7param(BYPLDO_VCO);
    wndId2Enum[cmbCP2_PLL] = LMS7param(CP2_PLL);
    wndId2Enum[cmbCP3_PLL] = LMS7param(CP3_PLL);
    wndId2Enum[ctrCSW_VCO] = LMS7param(CSW_VCO);
    wndId2Enum[chkCURLIM_VCO] = LMS7param(CURLIM_VCO);
    wndId2Enum[cmbCZ] = LMS7param(CZ);
    wndId2Enum[ctrDIV_LOCH] = LMS7param(DIV_LOCH);
    wndId2Enum[chkEN_COARSEPLL] = LMS7param(EN_COARSEPLL);
    wndId2Enum[chkEN_DIV2_DIVPROGenabled] = LMS7param(EN_DIV2_DIVPROG);
    wndId2Enum[chkEN_G] = LMS7param(EN_G);
    wndId2Enum[chkEN_INTONLY_SDM] = LMS7param(EN_INTONLY_SDM);
    wndId2Enum[chkEN_SDM_CLK] = LMS7param(EN_SDM_CLK);
    wndId2Enum[cmbICT_VCO] = LMS7param(ICT_VCO);
    wndId2Enum[cmbIOFFSET_CP] = LMS7param(IOFFSET_CP);
    wndId2Enum[cmbIPULSE_CP] = LMS7param(IPULSE_CP);
    wndId2Enum[chkPD_CP] = LMS7param(PD_CP);
    wndId2Enum[chkPD_FDIV] = LMS7param(PD_FDIV);
    wndId2Enum[chkPD_LOCH_T2RBUF] = LMS7param(PD_LOCH_T2RBUF);
    wndId2Enum[chkPD_SDM] = LMS7param(PD_SDM);
    wndId2Enum[chkPD_VCO] = LMS7param(PD_VCO);
    wndId2Enum[chkPD_VCO_COMP] = LMS7param(PD_VCO_COMP);
    wndId2Enum[ctrPW_DIV2_LOCH] = LMS7param(PW_DIV2_LOCH);
    wndId2Enum[ctrPW_DIV4_LOCH] = LMS7param(PW_DIV4_LOCH);
    wndId2Enum[chkRESET_N] = LMS7param(RESET_N);
    wndId2Enum[chkREVPH_PFD] = LMS7param(REVPH_PFD);
    wndId2Enum[chkREV_SDMCLK] = LMS7param(REV_SDMCLK);
    wndId2Enum[cmbRSEL_LDO_VCO] = LMS7param(RSEL_LDO_VCO);
    wndId2Enum[cmbSEL_SDMCLK] = LMS7param(SEL_SDMCLK);
    wndId2Enum[rgrSEL_VCO] = LMS7param(SEL_VCO);
    wndId2Enum[chkSPDUP_VCO] = LMS7param(SPDUP_VCO);
    wndId2Enum[chkSX_DITHER_EN] = LMS7param(SX_DITHER_EN);
    wndId2Enum[cmbTST_SX] = LMS7param(TST_SX);
    wndId2Enum[cmbVDIV_VCO] = LMS7param(VDIV_VCO);
    wndId2Enum[chkPD_FBDIV] = LMS7param(PD_FBDIV);
    wndId2Enum[chkEN_DIR_SXRSXT] = LMS7param(EN_DIR_SXRSXT);
    wndId2Enum[lblINT_SDM] = LMS7param(INT_SDM);
    wndId2Enum[lblEN_DIV2_DIVPROG] = LMS7param(EN_DIV2_DIVPROG);

    char ctemp[80];
    wxArrayString temp;

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        sprintf(ctemp, "%.3f pF", (i * 6 * 387.0) / 1000.0); temp.push_back(ctemp);
    }
    cmbCP2_PLL->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        sprintf(ctemp, "%.3f pF", (i * 6 * 980.0) / 1000.0); temp.push_back(ctemp);
    }
    cmbCP3_PLL->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        sprintf(ctemp, "%.3f pF", i * 8 * 5.88); temp.push_back(ctemp);
    }
    cmbCZ->Set(temp);

    temp.clear();
    for (int i = 0; i<64; ++i)
    {
        sprintf(ctemp, "%.3f uA", 0.243*i); temp.push_back(ctemp);
    }
    cmbIOFFSET_CP->Set(temp);

    temp.clear();
    for (int i = 0; i<64; ++i)
    {
        sprintf(ctemp, "%.3f uA", 2.312*i); temp.push_back(ctemp);
    }
    cmbIPULSE_CP->Set(temp);

    temp.clear();
    for (int i = 0; i <= 6; ++i)
    {
        sprintf(ctemp, "1.6 V"); temp.push_back(ctemp);
    }
    for (int i = 7; i<32; ++i)
    {
        sprintf(ctemp, "%.3f V", (0.000060*180000.0 / i)); temp.push_back(ctemp);
    }
    cmbRSEL_LDO_VCO->Set(temp);

    temp.clear();
    temp.push_back("CLK_DIV");
    temp.push_back("CLK_REF");
    cmbSEL_SDMCLK->Set(temp);

    temp.clear();
    temp.push_back("TST disabled");
    temp.push_back("TST[0]=VCO/20 clk*; TST[1]=VCO/40 clk*; TSTA=Hi Z");
    temp.push_back("TST[0]=SDM clk; TST[1]=FBD output; TSTA=Vtune@60kOhm");
    temp.push_back("TST[0]=Ref clk; TST[1]=FBD output; TSTA=Vtune@10kOhm");
    temp.push_back("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Hi Z");
    temp.push_back("TST[0]=CP Down offset; TST[1]=CP Up offset; TSTA=Hi Z");
    temp.push_back("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Vtune@60kOhm");
    temp.push_back("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Vtune@10kOhm");
    cmbTST_SX->Set(temp);

    temp.clear();
    sprintf(ctemp, "%.4f V", 1.6628); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6626); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6623); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6621); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6618); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6616); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6614); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6611); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6608); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6606); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6603); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6601); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6598); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6596); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6593); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.659); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6608); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6606); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6603); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6601); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6598); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6596); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6593); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.659); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6587); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6585); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6582); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6579); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6576); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6574); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6571); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6568); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6587); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6585); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6582); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6579); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6576); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6574); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6571); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6568); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6565); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6562); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6559); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6556); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6553); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.655); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6547); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6544); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6565); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6562); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6559); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6556); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6553); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.655); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6547); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6544); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6541); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6537); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6534); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6531); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6528); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6524); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6521); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6518); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6421); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6417); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6412); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6408); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6403); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6399); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6394); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6389); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6384); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.638); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6375); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.637); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6365); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.636); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6354); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6349); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6384); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.638); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6375); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.637); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6365); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.636); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6354); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6349); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6344); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6339); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6333); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6328); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6322); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6316); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6311); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6305); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6344); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6339); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6333); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6328); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6322); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6316); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6311); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6305); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6299); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6293); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6287); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6281); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6275); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6269); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6262); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6256); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6299); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6293); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6287); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6281); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6275); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6269); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6262); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6256); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6249); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6243); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6236); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6229); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6222); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6215); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6208); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.6201); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5981); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.597); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5959); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5947); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5936); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5924); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5912); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.59); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5888); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5875); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5862); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5849); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5836); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5822); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5808); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5794); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5888); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5875); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5862); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5849); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5836); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5822); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5808); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5794); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5779); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5765); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.575); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5734); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5718); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5702); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5686); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5669); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5779); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5765); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.575); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5734); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5718); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5702); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5686); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5669); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5652); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5634); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5616); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5598); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5579); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.556); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.554); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.552); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5652); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5634); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5616); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5598); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5579); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.556); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.554); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.552); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5499); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5478); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5456); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5433); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5411); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5387); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5363); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.5338); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.4388); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.433); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.4268); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.4205); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.4138); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.4069); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3996); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.392); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.384); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3756); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3667); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3574); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3476); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3373); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3264); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3148); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.384); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3756); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3667); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3574); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3476); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3373); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3264); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3148); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3025); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2895); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2756); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2608); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.245); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.228); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2098); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.1902); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.3025); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2895); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2756); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2608); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.245); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.228); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.2098); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.1902); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.169); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.1461); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.1211); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.0939); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.0641); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.0313); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.9951); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.9549); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.169); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.1461); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.1211); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.0939); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.0641); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 1.0313); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.9951); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.9549); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.9099); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.8593); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.802); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.7365); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.6609); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.5727); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.4685); temp.push_back(ctemp);
    sprintf(ctemp, "%.4f V", 0.3436); temp.push_back(ctemp);
    cmbVDIV_VCO->Set(temp);

    txtRefSpurBW->SetValue(_("5"));

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
    if(showRefClkSpurCancelation)
    {
        pnlRefClkSpur->Show();
        showRefClkSpurCancelation = false;
    }
}

void lms7002_pnlSX_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlSX_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlSX_view::ParameterChangeHandler(wxCommandEvent& event)
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

    if(event.GetEventObject() == ctrCSW_VCO) //for convenience refresh comparator values
    {
        wxCommandEvent evt;
        OnbtnReadComparators(evt);
    }
}

void lms7002_pnlSX_view::OnbtnReadComparators(wxCommandEvent& event)
{
    assert(lmsControl != nullptr);
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(VCO_CMPHO),&value);

    lblVCO_CMPHO->SetLabel(wxString::Format(_("%i"), value));
    if (value == 1)
        lblVCO_CMPHO->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPHO->SetBackgroundColour(*wxRED);

    LMS_ReadParam(lmsControl,LMS7param(VCO_CMPLO),&value);
    lblVCO_CMPLO->SetLabel(wxString::Format(_("%i"), value));
    if (value == 0)
        lblVCO_CMPLO->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPLO->SetBackgroundColour(*wxRED);
}

void lms7002_pnlSX_view::OnbtnChangeRefClkClick( wxCommandEvent& event )
{
    assert(lmsControl != nullptr);
    wxTextEntryDialog *dlg = new wxTextEntryDialog(this, _("Enter reference clock, MHz"), _("Reference clock"));
    double refClkMHz;
    dlg->SetTextValidator(wxFILTER_NUMERIC);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    const auto isTx = (ch == 2)? true : false;
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    dlg->SetValue(wxString::Format(_("%f"), freq/1e6));
    if (dlg->ShowModal() == wxID_OK)
    {
        dlg->GetValue().ToDouble(&refClkMHz);
        if (refClkMHz != 0)
        {
            double currentFreq_MHz;
            txtFrequency->GetValue().ToDouble(&currentFreq_MHz);
            LMS_SetClockFreq(lmsControl,LMS_CLOCK_REF,refClkMHz * 1e6);
            int status = LMS_SetClockFreq(lmsControl, isTx ? LMS_CLOCK_SXT : LMS_CLOCK_SXR,currentFreq_MHz * 1e6);
            if (status != 0)
                wxMessageBox(wxString::Format(_("%s"), wxString::From8BitData(LMS_GetLastErrorMessage())));
            UpdateGUI();
        }
    }
}

void lms7002_pnlSX_view::OnbtnCalculateClick( wxCommandEvent& event )
{
    assert(lmsControl != nullptr);
    double freqMHz;
    txtFrequency->GetValue().ToDouble(&freqMHz);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    const auto isTx = (ch == 2)? true : false;
    double RefClkMHz;
    lblRefClk_MHz->GetLabel().ToDouble(&RefClkMHz);
    LMS_SetClockFreq(lmsControl,LMS_CLOCK_REF,RefClkMHz * 1e6);

    double BWMHz;
    txtRefSpurBW->GetValue().ToDouble(&BWMHz);
    int status;
    if(chkEnableRefSpurCancelation->IsChecked())
        status = LMS_SetClockFreqWithSpurCancelation(lmsControl, isTx ? LMS_CLOCK_SXT : LMS_CLOCK_SXR,freqMHz * 1e6, BWMHz*1e6);
    else
        status = LMS_SetClockFreq(lmsControl, isTx ? LMS_CLOCK_SXT : LMS_CLOCK_SXR,freqMHz * 1e6);

    if (status != 0)
        wxMessageBox(wxString::Format(_("%s"), wxString::From8BitData(LMS_GetLastErrorMessage())));
    else
    {
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        wxString msg;
        if (ch == 1)
            msg = _("SXR");
        else
            msg = _("SXT");
        msg += wxString::Format(_(" frequency set to %f MHz"), freqMHz);
        evt.SetString(msg);
        wxPostEvent(this, evt);
    }
    UpdateGUI();
}

void lms7002_pnlSX_view::OnbtnTuneClick( wxCommandEvent& event )
{
    assert(lmsControl != nullptr);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    int status = LMS_SetClockFreq(lmsControl,ch == 2 ? LMS_CLOCK_SXT : LMS_CLOCK_SXR,-1); //Tune
    if (status != 0)
        wxMessageBox(wxString::Format(_("%s"), wxString::From8BitData(LMS_GetLastErrorMessage())));
    UpdateGUI();
}

void lms7002_pnlSX_view::UpdateGUI()
{
    assert(lmsControl != nullptr);

    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    const auto isTx = (ch == 2)? true : false;
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"), freq / 1e6));
    LMS_GetClockFreq(lmsControl,isTx ? LMS_CLOCK_SXT: LMS_CLOCK_SXR,&freq);
    lblRealOutFrequency->SetLabel(wxString::Format(_("%.3f"), freq / 1e6));
    txtFrequency->SetValue(wxString::Format(_("%.3f"), freq / 1e6));
    uint16_t div;
    LMS_ReadParam(lmsControl,LMS7param(DIV_LOCH),&div);
    lblDivider->SetLabel(wxString::Format("2^%i", div));

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(FRAC_SDM_MSB),&value);
    int fracValue = value << 16;
    LMS_ReadParam(lmsControl,LMS7param(FRAC_SDM_LSB),&value);
    fracValue |= value;
    lblFRAC_SDM->SetLabel(wxString::Format("%i", fracValue));

    //check if B channel is enabled
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    if (ch >= 2)
    {
        LMS_ReadParam(lmsControl,LMS7param(MIMO_SISO),&ch);
        if (ch != 0)
        {
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
            return;
        }
    }

    wxCommandEvent evt;
    OnbtnReadComparators(evt);
}

void lms7002_pnlSX_view::OnShowVCOclicked(wxCommandEvent& event)
{
    lms7002_dlgVCOfrequencies* dlg = new lms7002_dlgVCOfrequencies(this, lmsControl);
    dlg->ShowModal();
    dlg->Destroy();
}

void lms7002_pnlSX_view::OnEnableRefSpurCancelation(wxCommandEvent& event)
{
    txtRefSpurBW->Enable(chkEnableRefSpurCancelation->IsChecked());
}
