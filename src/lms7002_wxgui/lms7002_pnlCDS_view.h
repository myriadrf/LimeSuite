#ifndef __lms7002_pnlCDS_view__
#define __lms7002_pnlCDS_view__

#include <map>
#include "ILMS7002MTab.h"

namespace lime {

}

class lms7002_pnlCDS_view : public ILMS7002MTab
{
  public:
    lms7002_pnlCDS_view(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL);

  protected:
    enum {
        ID_CDSN_TXBTSP = 2048,
        ID_CDSN_TXATSP,
        ID_CDSN_RXBTSP,
        ID_CDSN_RXATSP,
        ID_CDSN_TXBLML,
        ID_CDSN_TXALML,
        ID_CDSN_RXBLML,
        ID_CDSN_RXALML,
        ID_CDSN_MCLK2,
        ID_CDSN_MCLK1,
        ID_CDS_MCLK2,
        ID_CDS_MCLK1,
        ID_CDS_TXBTSP,
        ID_CDS_TXATSP,
        ID_CDS_RXBTSP,
        ID_CDS_RXATSP,
        ID_CDS_TXBLML,
        ID_CDS_TXALML,
        ID_CDS_RXBLML,
        ID_CDS_RXALML
    };

    wxCheckBox* chkCDSN_TXBTSP;
    wxCheckBox* chkCDSN_TXATSP;
    wxCheckBox* chkCDSN_RXBTSP;
    wxCheckBox* chkCDSN_RXATSP;
    wxCheckBox* chkCDSN_TXBLML;
    wxCheckBox* chkCDSN_TXALML;
    wxCheckBox* chkCDSN_RXBLML;
    wxCheckBox* chkCDSN_RXALML;
    wxCheckBox* chkCDSN_MCLK2;
    wxCheckBox* chkCDSN_MCLK1;
    wxRadioBox* rgrCDS_MCLK2;
    wxRadioBox* rgrCDS_MCLK1;
    wxRadioBox* rgrCDS_TXBTSP;
    wxRadioBox* rgrCDS_TXATSP;
    wxRadioBox* rgrCDS_RXBTSP;
    wxRadioBox* rgrCDS_RXATSP;
    wxRadioBox* rgrCDS_TXBLML;
    wxRadioBox* rgrCDS_TXALML;
    wxRadioBox* rgrCDS_RXBLML;
    wxRadioBox* rgrCDS_RXALML;
};

#endif // __lms7002_pnlCDS_view__
