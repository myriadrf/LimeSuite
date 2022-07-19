///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __UTILITIES_GUI_H__
#define __UTILITIES_GUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/statbox.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class dlgConnectionSettings_view
///////////////////////////////////////////////////////////////////////////////
class dlgConnectionSettings_view : public wxDialog
{
	private:
    protected:
      wxStaticText *m_staticText472;
      wxListBox *mListLMS7ports;
      wxButton *btnConnect;
      wxButton *btnCancel;
      wxButton *btnDisconnect;

      // Virtual event handlers, overide them in your derived class
      virtual void GetDeviceList(wxInitDialogEvent &event)
      {
          event.Skip(); }
		virtual void OnConnect( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDisconnect( wxCommandEvent& event ) { event.Skip(); }

      public:
        dlgConnectionSettings_view(wxWindow *parent, wxWindowID id = wxID_ANY,
                                   const wxString &title = wxT("Connection Settings"),
                                   const wxPoint &pos = wxDefaultPosition,
                                   const wxSize &size = wxDefaultSize,
                                   long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        ~dlgConnectionSettings_view();
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgFullMessageLog_view
///////////////////////////////////////////////////////////////////////////////
class dlgFullMessageLog_view : public wxDialog
{
	private:
    protected:
      wxTextCtrl *txtMessageField;

    public:
      dlgFullMessageLog_view(wxWindow *parent, wxWindowID id = wxID_ANY,
                             const wxString &title = wxT("Message log"),
                             const wxPoint &pos = wxDefaultPosition,
                             const wxSize &size = wxDefaultSize,
                             long style = wxCAPTION | wxCLOSE_BOX | wxDEFAULT_DIALOG_STYLE |
                                          wxMAXIMIZE_BOX | wxRESIZE_BORDER);
      ~dlgFullMessageLog_view();
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgDeviceInfo_view
///////////////////////////////////////////////////////////////////////////////
class dlgDeviceInfo_view : public wxDialog
{
	private:
    protected:
      wxButton *btnGetInfo;
      wxStaticText *m_staticText6;
      wxStaticText *lblDeviceCtr;
      wxStaticText *m_staticText8;
      wxStaticText *lblExpansionCtr;
      wxStaticText *m_staticText10;
      wxStaticText *lblFirmwareCtr;
      wxStaticText *m_staticText12;
      wxStaticText *lblHardwareCtr;
      wxStaticText *m_staticText14;
      wxStaticText *lblProtocolCtr;
      wxStaticText *m_staticText41;
      wxStaticText *lblGatewareCtr;
      wxStaticText *m_staticText45;
      wxStaticText *lblGatewareTargetCtr;

      // Virtual event handlers, overide them in your derived class
      virtual void OnGetInfo(wxCommandEvent &event)
      {
          event.Skip(); }

    public:
      dlgDeviceInfo_view(wxWindow *parent, wxWindowID id = wxID_ANY,
                         const wxString &title = wxT("Device Info"),
                         const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                         long style = wxDEFAULT_DIALOG_STYLE);
      ~dlgDeviceInfo_view();
};

#endif //__UTILITIES_GUI_H__
