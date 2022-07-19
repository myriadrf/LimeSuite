#ifndef __lms7002_pnlBIAS_view__
#define __lms7002_pnlBIAS_view__

#include "lms7002_wxgui.h"

#include <map>
#include "ILMS7002MTab.h"

class lms7002_pnlBIAS_view : public ILMS7002MTab
{
  protected:
    void OnCalibrateRP_BIAS(wxCommandEvent &event);

  public:
    lms7002_pnlBIAS_view(wxWindow *parent, wxWindowID id = wxID_ANY,
                         const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                         long style = wxTAB_TRAVERSAL);

  protected:
    enum
    {
        ID_PD_FRP_BIAS = 2048,
        ID_PD_F_BIAS,
        ID_PD_PTRP_BIAS,
        ID_PD_PT_BIAS,
        ID_PD_BIAS_MASTER,
        ID_MUX_BIAS_OUT,
        ID_RP_CALIB_BIAS
    };

    wxCheckBox *chkPD_FRP_BIAS;
    wxCheckBox *chkPD_F_BIAS;
    wxCheckBox *chkPD_PTRP_BIAS;
    wxCheckBox *chkPD_PT_BIAS;
    wxCheckBox *chkPD_BIAS_MASTER;
    wxStaticText *ID_STATICTEXT1;
    wxComboBox *cmbMUX_BIAS_OUT;
    wxStaticText *ID_STATICTEXT2;
    wxComboBox *cmbRP_CALIB_BIAS;
    wxButton *btnCalibrateRP_BIAS;
};

#endif // __lms7002_pnlBIAS_view__
