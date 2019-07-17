///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __LIMERFE_GUI_H__
#define __LIMERFE_GUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/radiobut.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/tglbtn.h>
#include <wx/radiobox.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class limeRFE_view
///////////////////////////////////////////////////////////////////////////////
class limeRFE_view : public wxFrame 
{
	private:
	
	protected:
		wxPanel* pnlMain;
		wxPanel* pnlComm;
		wxRadioButton* rbI2C;
		wxRadioButton* rbUSB;
		wxStaticText* m_staticText22;
		wxTextCtrl* tcI2Caddress;
		wxComboBox* cmbbPorts;
		wxButton* btnRefreshPorts;
		wxButton* btnOpenPort;
		wxButton* btnClosePort;
		wxPanel* pnlI2Cmaster;
		wxCheckBox* cbI2CMaster;
		wxButton* btnOpen1;
		wxButton* btnSave;
		wxButton* btnReset;
		wxButton* btnBoard2GUI;
		wxPanel* pnlConfiguration;
		wxPanel* m_panel2;
		wxStaticText* m_staticText1;
		wxChoice* cType;
		wxPanel* m_panel3;
		wxStaticText* m_staticText2;
		wxChoice* cChannel;
		wxCheckBox* cbNotch;
		wxPanel* pnlTX2TXRX;
		wxStaticText* m_staticText11;
		wxChoice* cTX2TXRX;
		wxPanel* pnlTXRX;
		wxStaticText* m_staticText111;
		wxToggleButton* tbtnTXRX;
		wxPanel* pnlTXRXEN;
		wxStaticText* m_staticText1111;
		wxToggleButton* tbtnRX;
		wxToggleButton* tbtnTX;
		wxStaticText* m_staticText9;
		wxChoice* cAttenuation;
		wxPanel* m_panel4;
		wxButton* btnConfigure;
		wxPanel* pnlTXRXMode;
		wxStaticText* txtTXRX;
		wxPanel* pnlPowerMeter;
		wxPanel* m_panel16;
		wxButton* btnReadADC;
		wxButton* btnCalibrate;
		wxPanel* m_panel13;
		wxRadioBox* rbSWRsource;
		wxPanel* pnlADC1;
		wxStaticText* m_staticText13;
		wxStaticText* txtADC1;
		wxStaticText* m_staticText10;
		wxStaticText* txtPFWD_dBm;
		wxStaticText* m_staticText12;
		wxStaticText* txtPFWD_W;
		wxStaticText* m_staticText16;
		wxStaticText* m_staticText1022;
		wxStaticText* m_staticText102;
		wxTextCtrl* tcPowerCalibrate;
		wxStaticText* m_staticText122;
		wxPanel* pnlADC2;
		wxStaticText* m_staticText131;
		wxStaticText* txtADC2;
		wxStaticText* m_staticText101;
		wxStaticText* txtGamma_dB;
		wxStaticText* m_staticText121;
		wxStaticText* m_staticText31;
		wxStaticText* txtSWR;
		wxStaticText* m_staticText10221;
		wxStaticText* m_staticText1021;
		wxTextCtrl* tcGammaCalibrate;
		wxStaticText* m_staticText1221;
		wxTextCtrl* txtMessageField;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnrbI2CrbUSB( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnRefreshPorts( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnOpenPort( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnClosePort( wxCommandEvent& event ) { event.Skip(); }
		virtual void OncbI2CMaster( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnOpen( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnReset( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnBoard2GUI( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEraseBackground_pnlConfiguration( wxEraseEvent& event ) { event.Skip(); }
		virtual void OncType( wxCommandEvent& event ) { event.Skip(); }
		virtual void OncChannel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OncbNotch( wxCommandEvent& event ) { event.Skip(); }
		virtual void OncTX2TXRX( wxCommandEvent& event ) { event.Skip(); }
		virtual void OntbtnTXRX( wxCommandEvent& event ) { event.Skip(); }
		virtual void OntbtnTXRXEN( wxCommandEvent& event ) { event.Skip(); }
		virtual void OncAttenuation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnConfigure( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnReadADC( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnCalibrate( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnrbSWRsource( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		limeRFE_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("LimeRFE Control"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~limeRFE_view();
	
};

#endif //__LIMERFE_GUI_H__
