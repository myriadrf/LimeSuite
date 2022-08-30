#ifndef LIME_SDR_CONFIGURATION_VIEW
#define LIME_SDR_CONFIGURATION_VIEW

#include "wx/panel.h"
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/button.h>

#include <map>

namespace lime{
class SDRDevice;
}

struct ChannelConfigGUI
{
  wxCheckBox* enable;
  wxChoice* path;
  wxChoice* gain;
  wxTextCtrl* lpf;
  wxTextCtrl* nco;
};

struct SDRConfigGUI
{
  wxTextCtrl* rxLO;
  wxTextCtrl* txLO;
  wxCheckBox* tdd;
  wxTextCtrl* nyquist;
  wxChoice* oversample;
  ChannelConfigGUI rx[2];
  ChannelConfigGUI tx[2];
};

class SDRConfiguration_view : public wxPanel
{
  protected:
        
  public:
    SDRConfiguration_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    void UpdateVisiblePanel();
    void UpdateGUI();
    void Initialize(lime::SDRDevice *device);

  protected:
    void SubmitConfiguration(wxCommandEvent &event);

    wxPanel* CreateChannelPanel(SDRConfigGUI &config ,wxWindow *parent, wxWindowID id);
    lime::SDRDevice *sdrDevice;

    std::map<wxWindowID, SDRConfigGUI> mConfigControls;
};

#endif // LIME_SDR_CONFIGURATION_VIEW
