#include "lms7002_pnlSX_view.h"
#include "LMS7002M.h"

#include <wx/textdlg.h>
#include <wx/valnum.h>
#include <wx/msgdlg.h>
#include <map>
#include <assert.h>
#include "numericSlider.h"
#include "lms7002_gui_utilities.h"
#include "lms7suiteEvents.h"

lms7002_pnlSX_view::lms7002_pnlSX_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : pnlSX_view(parent, id, pos, size, style), lmsControl(nullptr)
{
    //ids for updating from chip
    wndId2Enum[chkBYPLDO_VCO] = BYPLDO_VCO;
    wndId2Enum[cmbCP2_PLL] = CP2_PLL;
    wndId2Enum[cmbCP3_PLL] = CP3_PLL;
    wndId2Enum[ctrCSW_VCO] = CSW_VCO;
    wndId2Enum[chkCURLIM_VCO] = CURLIM_VCO;
    wndId2Enum[cmbCZ] = CZ;
    wndId2Enum[ctrDIV_LOCH] = DIV_LOCH;
    wndId2Enum[chkEN_COARSEPLL] = EN_COARSEPLL;
    wndId2Enum[chkEN_DIV2_DIVPROGenabled] = EN_DIV2_DIVPROG;
    wndId2Enum[chkEN_G] = EN_G;
    wndId2Enum[chkEN_INTONLY_SDM] = EN_INTONLY_SDM;
    wndId2Enum[chkEN_SDM_CLK] = EN_SDM_CLK;
    wndId2Enum[cmbICT_VCO] = ICT_VCO;
    wndId2Enum[cmbIOFFSET_CP] = IOFFSET_CP;
    wndId2Enum[cmbIPULSE_CP] = IPULSE_CP;
    wndId2Enum[chkPD_CP] = PD_CP;
    wndId2Enum[chkPD_FDIV] = PD_FDIV;
    wndId2Enum[chkPD_LOCH_T2RBUF] = PD_LOCH_T2RBUF;
    wndId2Enum[chkPD_SDM] = PD_SDM;
    wndId2Enum[chkPD_VCO] = PD_VCO;
    wndId2Enum[chkPD_VCO_COMP] = PD_VCO_COMP;
    wndId2Enum[ctrPW_DIV2_LOCH] = PW_DIV2_LOCH;
    wndId2Enum[ctrPW_DIV4_LOCH] = PW_DIV4_LOCH;
    wndId2Enum[chkRESET_N] = RESET_N;
    wndId2Enum[chkREVPH_PFD] = REVPH_PFD;
    wndId2Enum[chkREV_SDMCLK] = REV_SDMCLK;
    wndId2Enum[cmbRSEL_LDO_VCO] = RSEL_LDO_VCO;
    wndId2Enum[cmbSEL_SDMCLK] = SEL_SDMCLK;
    wndId2Enum[rgrSEL_VCO] = SEL_VCO;
    wndId2Enum[chkSPDUP_VCO] = SPDUP_VCO;
    wndId2Enum[chkSX_DITHER_EN] = SX_DITHER_EN;
    wndId2Enum[cmbTST_SX] = TST_SX;
    wndId2Enum[cmbVDIV_VCO] = VDIV_VCO;
    wndId2Enum[chkPD_FBDIV] = PD_FBDIV;
    wndId2Enum[chkEN_DIR_SXRSXT] = EN_DIR_SXRSXT;
    wndId2Enum[lblINT_SDM] = INT_SDM;    
    wndId2Enum[lblEN_DIV2_DIVPROG] = EN_DIV2_DIVPROG;

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
}

void lms7002_pnlSX_view::Initialize(LMS7002M* pControl)
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
    lmsControl->Modify_SPI_Reg_bits(parameter, event.GetInt());

    if (parameter == CSW_VCO) //for convenience refresh comparator values
    {
        wxCommandEvent evt;
        OnbtnReadComparators(evt);
    }
}

void lms7002_pnlSX_view::OnbtnReadComparators(wxCommandEvent& event)
{   
    assert(lmsControl != nullptr);
    int value;
    value = lmsControl->Get_SPI_Reg_bits(VCO_CMPHO);
    lblVCO_CMPHO->SetLabel(wxString::Format(_("%i"), value));
    if (value == 1)
        lblVCO_CMPHO->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPHO->SetBackgroundColour(*wxRED);

    value = lmsControl->Get_SPI_Reg_bits(VCO_CMPLO);
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
    const int ch = lmsControl->Get_SPI_Reg_bits(MAC);
    dlg->SetValue(wxString::Format(_("%f"), lmsControl->GetReferenceClk_SX(ch == 2 ? LMS7002M::Tx : LMS7002M::Rx)));
    if (dlg->ShowModal() == wxID_OK)
    {
        dlg->GetValue().ToDouble(&refClkMHz);
        if (refClkMHz != 0)
        {
            double currentFreq;
            txtFrequency->GetValue().ToDouble(&currentFreq);
            liblms7_status status = lmsControl->SetFrequencySX(ch == 2 ? LMS7002M::Tx : LMS7002M::Rx, currentFreq, refClkMHz);
            if (status != LIBLMS7_SUCCESS)
                wxMessageBox(wxString::Format(_("Set frequency SX: %s"), wxString::From8BitData(liblms7_status2string(status))));
            UpdateGUI();
        }
    }
}

void lms7002_pnlSX_view::OnDIV2PrescalerChange( wxCommandEvent& event )
{
// TODO: Implement OnDIV2PrescalerChange
}

void lms7002_pnlSX_view::OnbtnCalculateClick( wxCommandEvent& event )
{
    assert(lmsControl != nullptr);
    double freqMHz;
    txtFrequency->GetValue().ToDouble(&freqMHz);
    const int ch = lmsControl->Get_SPI_Reg_bits(MAC);
    double RefClkMHz;
    lblRefClk_MHz->GetLabel().ToDouble(&RefClkMHz);
    liblms7_status status;    
    status = lmsControl->SetFrequencySX(ch == 2 ? LMS7002M::Tx : LMS7002M::Rx, freqMHz, RefClkMHz);
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Set frequency SX: %s"), wxString::From8BitData(liblms7_status2string(status))));
    else
    {
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        unsigned char channel = lmsControl->Get_SPI_Reg_bits(MAC, false);
        wxString msg;
        if (channel == 1)
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
    const int ch = lmsControl->Get_SPI_Reg_bits(MAC);
    liblms7_status status = lmsControl->TuneVCO(ch == 2 ? LMS7002M::VCO_SXT : LMS7002M::VCO_SXR);
    if (status != LIBLMS7_SUCCESS)
        wxMessageBox(wxString::Format(_("Tune: %s"), wxString::From8BitData(liblms7_status2string(status))));
    UpdateGUI();
}

void lms7002_pnlSX_view::UpdateGUI()
{
    assert(lmsControl != nullptr);

    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    const int ch = lmsControl->Get_SPI_Reg_bits(MAC);
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"), lmsControl->GetReferenceClk_SX(ch == 2 ? LMS7002M::Tx : LMS7002M::Rx)));    
    double refClk = lmsControl->GetReferenceClk_SX(ch == 2 ? LMS7002M::Tx : LMS7002M::Rx);
    double freq = lmsControl->GetFrequencySX_MHz(ch == 2 ? LMS7002M::Tx : LMS7002M::Rx, refClk);
    lblRealOutFrequency->SetLabel(wxString::Format(_("%.3f"), freq));
    txtFrequency->SetValue(wxString::Format(_("%.3f"), freq));

    int fracValue = (lmsControl->Get_SPI_Reg_bits(FRAC_SDM_MSB, false) << 16) | lmsControl->Get_SPI_Reg_bits(FRAC_SDM_LSB, false);
    lblFRAC_SDM->SetLabel(wxString::Format("%i", fracValue));

    wxCommandEvent evt;
    OnbtnReadComparators(evt);
}
