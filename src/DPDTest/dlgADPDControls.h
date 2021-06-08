/**
@file 	dlgADPDControls.h
@author Lime Microsystems
*/
#ifndef DLGADPDCONTROLS_H
#define DLGADPDCONTROLS_H

//(*Headers(dlgADPDControls)
#include <wx/sizer.h>
#include <wx/dialog.h>
//*)

#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>


class ConnectionManager;

class dlgADPDControls: public wxDialog
{
	public:

		int m_iValue;

		//int m_iCenterFreqRatio;
		//int m_iFreqSpanRatio;

		//int m_iCenterFreq;
		//int m_iFreqSpan;

		//double m_dCenterFreqRatio;
		//double m_dFreqSpanRatio;

		int m_iYaxisTop;
		int m_iYaxisBottom;
		int m_iXaxisLeft;
		int m_iXaxisRight;

		static const long ID_PLOTTYPE;
		static const long ID_TDXPI;
		static const long ID_TDXPQ;
		static const long ID_TDYPI;
		static const long ID_TDYPQ;
		static const long ID_TDXI;
		static const long ID_TDXQ;
		static const long ID_TDYI;
		static const long ID_TDYQ;
		static const long ID_UI;
		static const long ID_UQ;
		static const long ID_ERROR;
		static const long ID_PHASEERROR;
		static const long ID_STATICTEXT_ADPD0;
		static const long ID_IVSQ;
		static const long ID_FFTXP;
		static const long ID_FFTYP;
		static const long ID_FFTX;
		static const long ID_FFTY;
		//static const long ID_STATICTEXT10;
		//static const long ID_TEXTCTRL3;
		static const long ID_BUTTON3;
		//static const long ID_STATICTEXT11;
		//static const long ID_TEXTCTRL4;
		static const long ID_BUTTON4;
		static const long ID_STATICTEXT18;
		static const long ID_TEXTCTRL6;
		static const long ID_STATICTEXT19;
		static const long ID_TEXTCTRL5;
		


		void onCancel(wxCommandEvent& pEvent);
		void onOk(wxCommandEvent& pEvent);		

		dlgADPDControls(wxWindow* parent, int value,
			//int m_iCenterFreqRatioPlot, int m_iFreqSpanRatioPlot, 
			//double m_dCenterFreqRatioPlot, double m_dFreqSpanRatioPlot, 
			int m_iYaxisTopPlot, int m_iYaxisBottomPlot, int m_iXaxisLeftPlot, int m_iXaxisRightPlot, 
			wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~dlgADPDControls();

	protected:
        ConnectionManager *m_serPort;
	    void createControls();

	    void onParamChange(wxCommandEvent &evt);
		//void onCenterFreqRatio(wxCommandEvent &evt);
		//void onFreqSpanRatio(wxCommandEvent &evt);
		void onEnableDisable();

	private:
		

		wxStaticBoxSizer* StaticBox_main;
		wxFlexGridSizer* Sizer_main;
		wxStaticBoxSizer* StaticBox_td;
		wxFlexGridSizer* Sizer_td;
		wxStaticBoxSizer* StaticBox_IQ;
		wxFlexGridSizer* Sizer_IQ;
		//wxStaticBoxSizer* StaticBox_fft;
		//wxFlexGridSizer* Sizer_fft;
		wxFlexGridSizer* m_controlsSizer;
		wxStaticText* StaticText_ADPD0;

		wxRadioBox* plotType; // bits 1-0
		wxCheckBox* td_xpI; // bit 2
		wxCheckBox* td_xpQ; // bit 3
		wxCheckBox* td_ypI; // bit 4
		wxCheckBox* td_ypQ; // bit 5
		wxCheckBox* td_xI;  // bit 6
		wxCheckBox* td_xQ;  // bit 7
		wxCheckBox* td_yI;  // bit 8
		wxCheckBox* td_yQ;  // bit 9
		wxCheckBox* td_uI;  // bit 10
		wxCheckBox* td_uQ;  // bit 11
		wxCheckBox* td_error; // bit 12
		wxCheckBox* td_phase_error;  // bit 13
		wxRadioBox* IvsQ; // bits 15-14
		wxCheckBox* fft_xp;  // bit 16
		wxCheckBox* fft_yp;  // bit 17
		wxCheckBox* fft_x;   // bit 18
		wxCheckBox* fft_y;   // bit 19

		wxStaticBoxSizer * StaticBoxSizer5;
		wxFlexGridSizer*FlexGridSizer9;
		//wxFlexGridSizer*FlexGridSizer17;
		//wxFlexGridSizer*FlexGridSizer10;
		//wxStaticText* StaticText7;
		//wxTextCtrl* m_txtCenterFreq;
		//wxButton * btnCenterFreqRatio;
		//wxFlexGridSizer*FlexGridSizer11;
		//wxStaticText* StaticText8;
		//wxTextCtrl* m_txtFreqSpan;
		//wxButton * btnFreqSpanRatio;
		wxFlexGridSizer*FlexGridSizer16;
		wxStaticText*StaticText15;
		wxTextCtrl*txtYaxisTop;
		wxStaticText*StaticText16;
		wxTextCtrl*txtYaxisBottom;

		wxFlexGridSizer* Sizer_OC;
		wxButton* cancelButton;
		wxButton* okButton;	
		wxStaticText*StaticText17;		

		wxFlexGridSizer * FlexGridSizer_x;
		wxStaticText* StaticText_x1;
		static const long ID_STATICTEXT_X1;
		wxTextCtrl* txtXaxisLeft;
		static const long  ID_TEXTCTRL_X1;		
		wxStaticText* StaticText_x2;
		static const long ID_STATICTEXT_X2;
		wxTextCtrl* txtXaxisRight;
		static const long  ID_TEXTCTRL_X2;
		
		//wxStaticBoxSizer* StaticBoxSizer_x;

protected:

		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);
		DECLARE_EVENT_TABLE()
};

#endif
