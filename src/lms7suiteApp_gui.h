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
