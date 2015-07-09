/**
@file 	LMS_Programing_wxgui.h
@author Lime Microsystems
*/
#ifndef LMS_Programing_wxgui_H
#define LMS_Programing_wxgui_H

//(*Headers(LMS_Programing_wxgui)
#include <wx/frame.h>
#include <wx/wx.h>
class wxGauge;
class wxStaticText;
class wxFlexGridSizer;
class wxButton;
class wxChoice;
//*)
class LMScomms;
class LMS_Programing;

class LMS_Programing_wxgui : public wxFrame
{
	public:        
		LMS_Programing_wxgui(LMScomms* pSerPort, wxWindow* parent,wxWindowID id=wxID_ANY,const wxString &title =_(""), const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, int style=0, wxString name = wxEmptyString);
		virtual ~LMS_Programing_wxgui();

		wxChoice* cmbDevice;
		wxStaticText* lblProgressPercent;
		wxStaticText* StaticText2;
		wxGauge* progressBar;
		wxStaticText* lblFilename;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxChoice* cmbProgMode;
		wxButton* btnStartStop;        
		wxButton* btnOpen;

	protected:
        wxTimer* progressPooler;                
        void OnProgressPoll(wxTimerEvent& evt);
        void OnProgramingFinished();
	    LMS_Programing* m_programmer;
		
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_BUTTON2;
		static const long ID_STATICTEXT5;
		static const long ID_GAUGE1;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE2;
		static const long ID_CHOICE1;

	private:
		void OnbtnOpenClick(wxCommandEvent& event);
		void OnbtnProgMyriadClick(wxCommandEvent& event);
        void OnAbortProgramming(wxCommandEvent& event);
		void OnbtnProgFPGAClick(wxCommandEvent& event);
		void OncmbDeviceSelect(wxCommandEvent& event);

	protected:
        DECLARE_EVENT_TABLE()
};

#endif
