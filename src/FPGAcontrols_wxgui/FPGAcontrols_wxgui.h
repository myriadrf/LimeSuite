/**
@file FPGAcontrols_wxgui.h
@author Lime Microsystems
*/
#ifndef FPGA_CONTROLS_WXGUI_H
#define FPGA_CONTROLS_WXGUI_H

#include <wx/frame.h>
#include <wx/timer.h>
class wxGauge;
class wxStaticText;
class wxBitmapButton;
class wxFlexGridSizer;
class wxButton;
class wxStaticBoxSizer;
class wxToggleButton;
class wxCheckBox;

#include <lime/LimeSuite.h>
#include <vector>
#include <map>


class FPGAcontrols_wxgui: public wxFrame
{
	public:
        FPGAcontrols_wxgui(wxWindow* parent,wxWindowID id=wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long styles = 0);
        virtual void Initialize(lms_device_t* dataPort);
        virtual ~FPGAcontrols_wxgui();

        int UploadFile(const wxString &filename);
        wxButton* btnPlayWFM;
        wxButton* btnStopWFM;
        wxStaticText* lblProgressPercent;
        wxGauge* progressBar;
        wxBitmapButton* btnOpenWFM;
        wxToggleButton* btnLoadOnetone;
        wxStaticText* txtFilename;
        wxToggleButton* btnLoadCustom;
        wxToggleButton* btnLoadWCDMA;
        wxStaticText* txtDataRate;
        wxButton* btnLoadSamples;
        wxButton* btnStartStreaming;
        wxButton* btnStopStreaming;
        wxCheckBox* chkDigitalLoopbackEnable;

	protected:
        static const long ID_BUTTON6;
        static const long ID_BUTTON7;
        static const long ID_BUTTON8;
        static const long ID_BITMAPBUTTON1;
        static const long ID_STATICTEXT2;
        static const long ID_STATICTEXT5;
        static const long ID_GAUGE1;
        static const long ID_BUTTON3;
        static const long ID_BUTTON4;
        static const long ID_STREAMING_TIMER;

	private:
        void OnbtnUploadClick(wxCommandEvent& event);
        void OnbtnOpenFileClick(wxCommandEvent& event);
        void OnbtnMifClick(wxCommandEvent& event);
        void OnbtnHexClick(wxCommandEvent& event);
        void OnbtnPlayWFMClick(wxCommandEvent& event);
        void OnbtnStopWFMClick(wxCommandEvent& event);
        void OnbtnLoadOnetoneClick(wxCommandEvent& event);
        void OnbtnLoadWCDMAClick(wxCommandEvent& event);
        void OnbtnLoadCustomClick(wxCommandEvent& event);

        void OnbtnLoadFileClick(wxCommandEvent& event);
        void OnbtnStartStreamingClick(wxCommandEvent& event);
        void OnbtnStopStreamingClick(wxCommandEvent& event);

        void OnUpdateStats(wxTimerEvent& event);
        void OnChkDigitalLoopbackEnableClick(wxCommandEvent& event);

	protected:
        wxString fileForCyclicTransmitting;
        lms_device_t* lmsControl;
        wxTimer* mStreamingTimer;
        DECLARE_EVENT_TABLE()
};

#endif
