/**
@file FPGAcontrols_wxgui.h
@author Lime Microsystems
*/
#ifndef GPS_WXGUI_H
#define GPS_WXGUI_H

#include <wx/frame.h>
#include <wx/wx.h>
class wxStaticText;
class wxButton;
class wxCheckBox;
class wxSpinCtrl;
#include "lime/LimeSuite.h"

class GPS_wxgui: public wxFrame
{
public:
        GPS_wxgui(wxWindow* parent,wxWindowID id=wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long styles = 0);
        virtual void Initialize(lms_device_t* dataPort);
        virtual ~GPS_wxgui();

private:
        wxStaticText* lblUTC;
        wxStaticText* lblStatus;
        wxStaticText* lblLatitude;
        wxStaticText* lblLongitude;
        wxStaticText* lblSpeed;
        wxStaticText* lblCourse;
        wxStaticText* lblDate;
        wxStaticText* lblFix;
        wxStaticText* lblAccuracy;
        wxStaticText* lblState;
        wxStaticText* lblDAC;
        wxStaticText* lblError1s;
        wxStaticText* lblError10s;
        wxStaticText* lblError100s;
        wxCheckBox* chkEnGPS;
        wxCheckBox* chkEnFreq;
        wxButton* btnRefresh;
        wxSpinCtrl* spinTol1s;
        wxSpinCtrl* spinTol10s;
        wxSpinCtrl* spinTol100s;
        void OnChkEnGPS(wxCommandEvent& event);
        void OnChkEnFreq(wxCommandEvent& event);
        void OnBtnRefresh(wxCommandEvent& event);
        void OnSpin1s(wxSpinEvent& event);
        void OnSpin10s(wxSpinEvent& event);
        void OnSpin100s(wxSpinEvent& event);

	protected:
        lms_device_t* lmsControl;
        DECLARE_EVENT_TABLE()
};

#endif
