#ifndef __pnlAnalog_view__
#define __pnlAnalog_view__

/**	@brief GUI for writing and reading analog controls
	@author Lime Microsystems (www.limemicro.com)
*/

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/frame.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>
#include <stdint.h>
#include "lms7002_defines.h"

class LMS64CProtocol;
class IConnection;

class pnlBoardControls : public wxFrame
{
	public:
        struct ADC_DAC
        {
            std::string name;
            int16_t value;
            uint8_t channel;
            uint8_t units;
            int8_t powerOf10;
            int16_t minValue;
            int16_t maxValue;
        };
                
        class ADC_GUI
        {
        public:
            ADC_GUI() : channel(nullptr), title(nullptr), units(nullptr),
                powerOf10(nullptr), value(nullptr){};
            ~ADC_GUI()
            {
                if (channel)
                    channel->Destroy();
                if (title)
                    title->Destroy();
                if (units)
                    units->Destroy();
                if (powerOf10)
                    powerOf10->Destroy();
                if (value)
                    value->Destroy();
            }
            wxStaticText* channel;
            wxStaticText* title;
            wxStaticText* units;
            wxStaticText* powerOf10;
            wxStaticText* value;
        };

        class DAC_GUI
        {
        public:
            DAC_GUI() : channel(nullptr), title(nullptr), units(nullptr), value(nullptr){};
            ~DAC_GUI()
            {
                if (channel)
                    channel->Destroy();
                if (title)
                    title->Destroy();
                if (units)
                    units->Destroy();
                if (value)
                    value->Destroy();
            }
            wxStaticText* channel;
            wxStaticText* title;
            wxStaticText* units;
            wxSpinCtrl* value;
        };

        pnlBoardControls(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &title = _(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
        ~pnlBoardControls();
	
        void UpdatePanel();
        void Initialize(IConnection* controlPort);
        LMS64CProtocol* serPort;

        void SetupControls(eLMS_DEV boardID);
        void OnSetDACvalues(wxSpinEvent &event);
        void OnSetDACvaluesENTER(wxCommandEvent &event);
	protected:
        wxPanel* pnlCustomControls;
        wxSpinCtrl *spinCustomChannelRd;
        wxStaticText *txtCustomValueRd;
        wxStaticText *txtCustomUnitsRd;
        wxStaticText *txtCustomPowerOf10Rd;
        wxSpinCtrl *spinCustomChannelWr;
        wxSpinCtrl *spinCustomValueWr;
        wxChoice *cmbCustomUnitsWr;
        wxChoice *cmbCustomPowerOf10Wr;
        wxButton *btnCustomRd;
        wxButton *btnCustomWr;
        void OnCustomRead(wxCommandEvent& event);
        void OnCustomWrite(wxCommandEvent& event);

        wxFlexGridSizer* sizerAnalogRd;
        wxFlexGridSizer* sizerAnalogWr;

        std::vector<ADC_DAC> getBoardADCs(eLMS_DEV boardID);
        std::vector<ADC_DAC> getBoardDACs(eLMS_DEV boardID);

        void OnUserChangedBoardType(wxCommandEvent& event);
		void OnReadAll( wxCommandEvent& event );
		void OnWriteAll( wxCommandEvent& event );
		
		wxButton* btnReadAll;
		wxButton* btnWriteAll;
		wxStaticText* m_staticText349;
		wxChoice* cmbBoardSelection;

        std::vector<ADC_DAC> mADCparameters;
        std::vector<ADC_DAC> mDACparameters;

        std::vector<ADC_GUI*> mADC_GUI_widgets;
        std::vector<DAC_GUI*> mDAC_GUI_widgets;
};

#endif // __pnlAnalog_view__
