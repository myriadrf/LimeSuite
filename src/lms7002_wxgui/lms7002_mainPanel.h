#ifndef __lms7002_mainPanel__
#define __lms7002_mainPanel__

#include "lime/LimeSuite.h"
#include "lms7002_wxgui.h"
#include <unordered_map>
#include "ILMS7002MTab.h"

#include "GUI/ISOCPanel.h"

namespace lime{
class MCU_BD;
class SDRDevice;
}
class lms7002_pnlMCU_BD_view;
class lms7002_pnlR3_view;
class SDRConfiguration_view;

class lms7002_mainPanel : public ISOCPanel
{
protected:
  // Handlers for mainPanel events.
  void OnNewProject( wxCommandEvent& event );
  void OnOpenProject( wxCommandEvent& event );
  void OnSaveProject( wxCommandEvent& event );
  void OnUploadAll( wxCommandEvent& event );
  void OnDownloadAll( wxCommandEvent& event );
  void OnReset( wxCommandEvent& event );
  void OnRegistersTest( wxCommandEvent& event );
  void Onnotebook_modulesPageChanged( wxNotebookEvent& event );
  void OnResetChip(wxCommandEvent& event);
  void OnLoadDefault(wxCommandEvent& event);
  void OnReadTemperature(wxCommandEvent& event);
  void OnEnableMIMOchecked(wxCommandEvent& event);

  void OnChannelOrSOCChange(wxCommandEvent &event);

  lime::LMS7002M* GetSelectedChip() const;

public:

  lms7002_mainPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
  virtual ~lms7002_mainPanel();
  void UpdateVisiblePanel();
  void UpdateGUI();
  void Initialize(lime::LMS7002M* socPtr);

  std::unordered_map<wxWindowID, ILMS7002MTab *> mTabs;

protected:
  //lime::SDRDevice *sdrDevice;
  lime::LMS7002M* soc;

  enum
  {
      ID_G_LNA_RFE = 2048,
      ID_BTN_CH_A,
      ID_BTN_CH_B,
      ID_BTN_CHIP_TO_GUI,
      ID_BTN_RESET_CHIP,
      ID_TABS_NOTEBOOK,
      ID_TAB_CALIBRATIONS,
      ID_TAB_RFE,
      ID_TAB_RBB,
      ID_TAB_TRF,
      ID_TAB_TBB,
      ID_TAB_AFE,
      ID_TAB_BIAS,
      ID_TAB_LDO,
      ID_TAB_XBUF,
      ID_TAB_CLKGEN,
      ID_TAB_SXR,
      ID_TAB_SXT,
      ID_TAB_LIMELIGHT,
      ID_TAB_TXTSP,
      ID_TAB_RXTSP,
      ID_TAB_CDS,
      ID_TAB_BIST,
      ID_TAB_GAINS,
      ID_TAB_MCU,
      ID_TAB_R3
  };

  wxButton *btnOpen;
  wxButton *btnSave;
  wxChoice* cmbLmsDevice;
  wxRadioButton* rbChannelA;
  wxRadioButton* rbChannelB;
  wxCheckBox* chkEnableMIMO;
  wxButton* btnDownloadAll;
  wxButton* btnUploadAll;
  wxButton* btnResetChip;
  wxButton* btnLoadDefault;
  wxStaticText* txtTemperature;
  wxButton* btnReadTemperature;
  wxNotebook* tabsNotebook;
};

#endif // __lms7002_mainPanel__
