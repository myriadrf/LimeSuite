#ifndef __lms7002_pnlXBUF_view__
#define __lms7002_pnlXBUF_view__

#include "lms7002_wxgui.h"

#include <map>
#include "ILMS7002MTab.h"

class lms7002_pnlXBUF_view : public ILMS7002MTab
{
public:
  lms7002_pnlXBUF_view(wxWindow *parent, wxWindowID id = wxID_ANY,
                       const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                       long style = wxTAB_TRAVERSAL);

protected:
  enum
  {
      ID_PD_XBUF_RX = 2048,
      ID_PD_XBUF_TX,
      ID_EN_G_XBUF,
      ID_SLFB_XBUF_RX,
      ID_SLFB_XBUF_TX,
      ID_BYP_XBUF_RX,
      ID_BYP_XBUF_TX,
      ID_EN_OUT2_XBUF_TX,
      ID_EN_TBUFIN_XBUF_RX
  };

  wxCheckBox *chkPD_XBUF_RX;
  wxCheckBox *chkPD_XBUF_TX;
  wxCheckBox *chkEN_G_XBUF;
  wxCheckBox *chkSLFB_XBUF_RX;
  wxCheckBox *chkSLFB_XBUF_TX;
  wxCheckBox *chkBYP_XBUF_RX;
  wxCheckBox *chkBYP_XBUF_TX;
  wxCheckBox *chkEN_OUT2_XBUF_TX;
  wxCheckBox *chkEN_TBUFIN_XBUF_RX;
};

#endif // __lms7002_pnlXBUF_view__
