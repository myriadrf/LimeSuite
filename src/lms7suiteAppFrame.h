#ifndef __LMS7SuiteAppFrame__
#define __LMS7SuiteAppFrame__

#include "limesuite/Logger.h"

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/treectrl.h>
#include <wx/treebase.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statusbr.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/frame.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/choice.h>

#include <map>
#include "IModuleFrame.h"
#include "pnlBoardControls.h"
#include "GUI/ISOCPanel.h"
#include "GUI/DeviceConnectionPanel.h"
#include "dlgAbout.h"

class pnlMiniLog;
class lms7002_mainPanel;
class fftviewer_frFFTviewer;
class LMS_Programing_wxgui;

class LMS7SuiteAppFrame : public wxFrame
{
  protected:
    void AddModule(IModuleFrame* module, const std::string& title);
    void RemoveModule(IModuleFrame* module);
    // Handlers for AppFrame events.
    void OnClose(wxCloseEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnDeviceHandleChange(wxCommandEvent& event);
    void OnModuleClose(wxCloseEvent& event);
    void OnShowModule(wxCommandEvent& event);
    void OnDeviceDisconnect();

    void DeviceTreeSelectionChanged(wxTreeEvent& event);

  public:
    LMS7SuiteAppFrame(wxWindow* parent);

    virtual ~LMS7SuiteAppFrame();
    static int m_lmsSelection;

  protected:
    static void OnGlobalLogEvent(const lime::LogLevel level, const char* message);
    static void OnLogDataTransfer(bool Tx, const unsigned char* data, const unsigned int length);
    void OnLogMessage(wxCommandEvent& event);
    void UpdateConnections(lime::SDRDevice* port);

    lime::SDRDevice* lmsControl;
    pnlMiniLog* mMiniLog;
    fftviewer_frFFTviewer* fftviewer;
    static LMS7SuiteAppFrame* obj_ptr;

    std::map<wxWindowID, IModuleFrame*> mModules;

    enum { idMenuQuit = 1000, ID_MENUITEM_LIMERFE, idMenuAbout };

    wxTreeCtrl* deviceTree;
    wxMenuBar* mbar;
    wxMenu* fileMenu;
    wxMenu* mnuOptions;
    wxMenuItem* mnuCacheValues;
    wxMenu* mnuModules;
    wxMenu* helpMenu;
    wxStatusBar* statusBar;
    wxFlexGridSizer* mainSizer;
    wxScrolledWindow* m_scrolledWindow1;
    wxFlexGridSizer* contentSizer;
    ISOCPanel* mContent;
    LMS_Programing_wxgui* programmer;
    pnlBoardControls* boardControlsGui;
    lime::DeviceConnectionPanel* pnlDeviceConnection;
};

#endif // __LMS7SuiteAppFrame__
