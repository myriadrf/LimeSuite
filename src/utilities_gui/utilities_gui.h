///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
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
		wxStaticText* m_staticText472;
		wxStaticText* m_staticText473;
		wxListBox* mListLMS7ports;
		wxListBox* mListStreamports;
		wxButton* btnConnect;
		wxButton* btnCancel;
		wxButton* btnDisconnect;
		
		// Virtual event handlers, overide them in your derived class
		virtual void GetDeviceList( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnConnect( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDisconnect( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dlgConnectionSettings_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Connection Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~dlgConnectionSettings_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class pnlMiniLog_view
///////////////////////////////////////////////////////////////////////////////
class pnlMiniLog_view : public wxPanel 
{
	private:
	
	protected:
		wxTextCtrl* txtMessageField;
		wxButton* btnClear;
		wxButton* btnFullLog;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnUpdateGUI( wxUpdateUIEvent& event ) { event.Skip(); }
		virtual void OnBtnClearClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowFullLog( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLogDataClicked( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxCheckBox* chkLogData;
		
		pnlMiniLog_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~pnlMiniLog_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgFullMessageLog_view
///////////////////////////////////////////////////////////////////////////////
class dlgFullMessageLog_view : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* txtMessageField;
	
	public:
		
		dlgFullMessageLog_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Message log"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~dlgFullMessageLog_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgDeviceInfo_view
///////////////////////////////////////////////////////////////////////////////
class dlgDeviceInfo_view : public wxDialog 
{
	private:
	
	protected:
		wxButton* btnGetInfo;
		wxStaticText* m_staticText6;
		wxStaticText* lblDeviceCtr;
		wxStaticText* m_staticText8;
		wxStaticText* lblExpansionCtr;
		wxStaticText* m_staticText10;
		wxStaticText* lblFirmwareCtr;
		wxStaticText* m_staticText12;
		wxStaticText* lblHardwareCtr;
		wxStaticText* m_staticText14;
		wxStaticText* lblProtocolCtr;
		wxStaticText* m_staticText41;
		wxStaticText* lblGatewareCtr;
		wxStaticText* m_staticText43;
		wxStaticText* lblGatewareRevCtr;
		wxStaticText* m_staticText45;
		wxStaticText* lblGatewareTargetCtr;
		wxStaticText* m_staticText61;
		wxStaticText* lblDeviceData;
		wxStaticText* m_staticText81;
		wxStaticText* lblExpansionData;
		wxStaticText* m_staticText101;
		wxStaticText* lblFirmwareData;
		wxStaticText* m_staticText121;
		wxStaticText* lblHardwareData;
		wxStaticText* m_staticText141;
		wxStaticText* lblProtocolData;
		wxStaticText* m_staticText47;
		wxStaticText* lblGatewareData;
		wxStaticText* m_staticText49;
		wxStaticText* lblGatewareRevData;
		wxStaticText* m_staticText51;
		wxStaticText* lblGatewareTargetData;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnGetInfo( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dlgDeviceInfo_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Device Info"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~dlgDeviceInfo_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SPI_view
///////////////////////////////////////////////////////////////////////////////
class SPI_view : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* ID_STATICTEXT1;
		wxTextCtrl* txtLMSwriteAddr;
		wxStaticText* ID_STATICTEXT9;
		wxTextCtrl* txtLMSwriteValue;
		wxButton* btnLMSwrite;
		wxStaticText* ID_STATICTEXT5;
		wxStaticText* lblLMSwriteStatus;
		wxStaticText* ID_STATICTEXT2;
		wxTextCtrl* txtLMSreadAddr;
		wxButton* btnLMSread;
		wxStaticText* ID_STATICTEXT3;
		wxStaticText* lblLMSreadValue;
		wxStaticText* ID_STATICTEXT7;
		wxStaticText* lblLMSreadStatus;
		wxButton* ID_BUTTON23;
		wxCheckBox* ID_CHECKBOX5;
		wxStaticText* ID_STATICTEXT15;
		wxTextCtrl* txtBoardwriteAddr;
		wxStaticText* ID_STATICTEXT16;
		wxTextCtrl* txtBoardwriteValue;
		wxButton* ID_BUTTON24;
		wxStaticText* ID_STATICTEXT17;
		wxStaticText* lblBoardwriteStatus;
		wxStaticText* ID_STATICTEXT19;
		wxTextCtrl* txtBoardreadAddr;
		wxButton* ID_BUTTON25;
		wxStaticText* ID_STATICTEXT20;
		wxStaticText* lblBoardreadValue;
		wxStaticText* ID_STATICTEXT22;
		wxStaticText* lblBoardreadStatus;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onLMSwrite( wxCommandEvent& event ) { event.Skip(); }
		virtual void onLMSread( wxCommandEvent& event ) { event.Skip(); }
		virtual void onBoardWrite( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBoardRead( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		SPI_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SPI"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~SPI_view();
	
};

#endif //__UTILITIES_GUI_H__
