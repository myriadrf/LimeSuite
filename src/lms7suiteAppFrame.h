#ifndef __LMS7SuiteAppFrame__
#define __LMS7SuiteAppFrame__

#include "lms7suiteApp_gui.h"
#include "Logger.h"

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>

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

class pnlMiniLog;
class lms7002_mainPanel;
class fftviewer_frFFTviewer;

class LMS7SuiteAppFrame : public wxFrame
{
    protected:
      void AddModule(IModuleFrame *module, const char *title);
      void RemoveModule(IModuleFrame *module);
      // Handlers for AppFrame events.
      void OnClose(wxCloseEvent &event);
      void OnQuit(wxCommandEvent &event);
      // void OnShowConnectionSettings( wxCommandEvent& event );
      void OnAbout(wxCommandEvent &event);
      void OnDeviceConnect(wxCommandEvent &event);
      void HandleLMSevent(wxCommandEvent &event);
      void OnModuleClose(wxCloseEvent &event);
      void OnShowModule(wxCommandEvent &event);
      // void OnADF4002Close(wxCloseEvent& event);
      // void OnShowADF4002(wxCommandEvent& event);
      // void OnSi5351Close(wxCloseEvent& event);
      // void OnShowSi5351C(wxCommandEvent& event);
      // void OnProgramingClose(wxCloseEvent& event);
      // void OnShowPrograming(wxCommandEvent& event);
      // void OnFPGAcontrolsClose(wxCloseEvent& event);
      // void OnShowFPGAcontrols(wxCommandEvent& event);
      // void OnDeviceInfoClose(wxCloseEvent& event);
      // void OnShowDeviceInfo(wxCommandEvent& event);
      // void OnSPIClose(wxCloseEvent& event);
      // void OnShowSPI(wxCommandEvent& event);
      // void OnBoardControlsClose(wxCloseEvent& event);
      // void OnAPIClose(wxCloseEvent& event);
      // void OnShowBoardControls(wxCommandEvent& event);
      // void OnChangeCacheSettings(wxCommandEvent& event);
      // void OnLmsChanged(wxCommandEvent& event);
      // void OnShowAPICalls( wxCommandEvent& event );
      // void OnShowLimeRFE(wxCommandEvent& event);
    public:
		/** Constructor */
        LMS7SuiteAppFrame( wxWindow* parent );

        virtual ~LMS7SuiteAppFrame();
        // void UpdateVisiblePanel() const;
        static int m_lmsSelection;
        // void OnLimeRFEClose(wxCloseEvent& event);
      protected:
        static void OnGlobalLogEvent(const lime::LogLevel level, const char *message);
        static void OnLogDataTransfer(bool Tx, const unsigned char* data, const unsigned int length);
        void OnLogMessage(wxCommandEvent &event);
        static const int cDeviceInfoCollumn = 1;
        static const int cDeviceVerRevMaskCollumn = 2;
        void UpdateConnections(lime::SDRDevice *port);

        lime::SDRDevice *lmsControl;
        pnlMiniLog* mMiniLog;
        fftviewer_frFFTviewer* fftviewer;
        static LMS7SuiteAppFrame* obj_ptr;

        std::map<wxWindowID, IModuleFrame *> mModules;

        enum
        {
            idMenuQuit = 1000,
            ID_MENUITEM_LIMERFE,
            idMenuAbout
        };

        wxMenuBar *mbar;
        wxMenu *fileMenu;
        wxMenu *mnuOptions;
        wxMenuItem *mnuCacheValues;
        wxMenu *mnuModules;
        wxMenu *helpMenu;
        wxStatusBar *statusBar;
        wxFlexGridSizer *mainSizer;
        wxScrolledWindow *m_scrolledWindow1;
        wxFlexGridSizer *contentSizer;
        lms7002_mainPanel *mContent;

        wxChoice *mDeviceChoice;
        //       ADF4002_wxgui* adfGUI;

        //       Si5351C_wxgui* si5351gui;

        //       LMS_Programing_wxgui* programmer;
        //       FPGAcontrols_wxgui* fpgaControls;
        //       dlgDeviceInfo* deviceInfo;
        //       SPI_wxgui* spi;
        pnlBoardControls* boardControlsGui;
        //       pnlAPI* api;
        // limeRFE_wxgui* limeRFEwin;
};


#endif // __LMS7SuiteAppFrame__

