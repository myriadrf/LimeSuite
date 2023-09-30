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

#include "limesuite/SDRDevice.h"

#include "GUI/ISOCPanel.h"

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
  wxStaticBox* titledBox;
  wxTextCtrl* rxLO;
  wxTextCtrl* txLO;
  wxCheckBox* tdd;
  wxTextCtrl* sampleRate;
  wxChoice* decimation;
  wxChoice* interpolation;
  ChannelConfigGUI rx[2];
  ChannelConfigGUI tx[2];
};

class SOCConfig_view : public wxPanel
{
public:
  SOCConfig_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
  void Setup(lime::SDRDevice *sdrDevice, int index);
  void UpdateGUI(const lime::SDRDevice::SDRConfig &config);

  void SubmitConfig(wxCommandEvent &event);
protected:
  SDRConfigGUI gui;
  lime::SDRDevice *sdrDevice;
  int socIndex;
};

class SDRConfiguration_view : public ISOCPanel
{
  public:
    SDRConfiguration_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    void Setup(lime::SDRDevice *device);

  protected:
    SDRConfiguration_view() = delete;
    std::vector<SOCConfig_view*> socGUI;
    lime::SDRDevice *sdrDevice;
};

#endif // LIME_SDR_CONFIGURATION_VIEW
