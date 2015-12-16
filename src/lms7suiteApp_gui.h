///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __LMS7SUITEAPP_GUI_H__
#define __LMS7SUITEAPP_GUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
class lms7002_mainPanel;

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
#include <wx/frame.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class AppFrame_view
///////////////////////////////////////////////////////////////////////////////
class AppFrame_view : public wxFrame 
{
	private:
	
	protected:
		enum
		{
			idMenuQuit = 1000,
			idMenuAbout
		};
		
		wxMenuBar* mbar;
		wxMenu* fileMenu;
		wxMenu* mnuOptions;
		wxMenu* helpMenu;
		wxMenu* mnuModules;
		wxStatusBar* statusBar;
		wxFlexGridSizer* contentSizer;
		lms7002_mainPanel* mContent;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowConnectionSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowFFTviewer( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowADF4002( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowSi5351C( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowPrograming( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowRFSpark( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowHPM7( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowFPGAcontrols( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowMyriad7( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowDeviceInfo( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowSPI( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowNovena( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowBoardControls( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		AppFrame_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("LMS7002 GUI"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~AppFrame_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgAbout_view
///////////////////////////////////////////////////////////////////////////////
class dlgAbout_view : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* imgLogo;
		wxStaticText* ID_STATICTEXT1;
		wxStaticText* txtVersion;
		wxStaticText* txtDescription;
		wxStaticText* ID_STATICTEXT4;
		wxStaticText* ID_STATICTEXT5;
		wxHyperlinkCtrl* ID_HYPERLINKCTRL1;
		wxButton* btnClose;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnInit( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnbtnClose( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dlgAbout_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("About"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~dlgAbout_view();
	
};

#endif //__LMS7SUITEAPP_GUI_H__
