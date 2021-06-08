/**
@file 	myriad7_wxgui.h
@author Lime Microsystems
*/
#ifndef MYRIAD7_WXGUI_H
#define MYRIAD7_WXGUI_H

#include <wx/frame.h>
class wxComboBox;
class wxStaticText;
class wxFlexGridSizer;

namespace lime{
class IConnection;
}

class Myriad7_wxgui: public wxFrame
{
	public:

		Myriad7_wxgui(wxWindow* parent,wxWindowID id=wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long styles = 0);
		void Initialize(lime::IConnection* serPort);
		virtual ~Myriad7_wxgui();
		virtual void UpdatePanel();

		wxStaticText* StaticText2;
		wxComboBox* cmbGPIO2;
		wxStaticText* StaticText1;
		wxComboBox* cmbGPIO_1_0;

        static const long ID_STATICTEXT1;
		static const long ID_COMBOBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_COMBOBOX2;

        void ParameterChangeHandler(wxCommandEvent& event);
	protected:
        lime::IConnection* serPort;
		DECLARE_EVENT_TABLE()
};

#endif
