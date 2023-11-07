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
#include <wx/textctrl.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <wx/dialog.h>
#include <LMSBoards.h>

#include "IModuleFrame.h"

namespace lime {
class SDRDevice;
}

#include "limesuite/SDRDevice.h"

class pnlBoardControls : public IModuleFrame
{
  public:
    struct ADC_DAC {
        std::string name;
        bool writable;
        double value;
        int32_t channel;
        std::string units;
        int8_t powerOf10;
        int minValue;
        int maxValue;
    };

    class MemoryParamGUI
    {
      public:
        MemoryParamGUI()
            : title(nullptr)
            , txtValue(nullptr)
            , btnRead(nullptr)
            , btnWrite(nullptr){};
        ~MemoryParamGUI()
        {
            if (title)
                title->Destroy();
            if (txtValue)
                txtValue->Destroy();
            if (btnRead)
                btnRead->Destroy();
            if (btnWrite)
                btnWrite->Destroy();
        }
        wxStaticText* title;
        wxTextCtrl* txtValue;
        wxButton* btnRead;
        wxButton* btnWrite;
        int32_t id;
        lime::SDRDevice::DataStorage::Region mem;
    };

    class Param_GUI
    {
      public:
        Param_GUI()
            : title(nullptr)
            , units(nullptr)
            , rValue(nullptr)
            , wValue(nullptr){};
        ~Param_GUI()
        {
            if (title)
                title->Destroy();
            if (units)
                units->Destroy();
            if (rValue)
                rValue->Destroy();
            if (wValue)
                wValue->Destroy();
        }
        wxStaticText* title;
        wxStaticText* units;
        wxStaticText* rValue;
        wxSpinCtrl* wValue;
    };

    pnlBoardControls(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = _(""),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE);
    ~pnlBoardControls();

    bool Initialize(lime::SDRDevice* device) override;
    void Update() override;

    void SetupControls(const std::string& boardID);
    void OnSetDACvalues(wxSpinEvent& event);
    void OnSetDACvaluesENTER(wxCommandEvent& event);
    void OnDACWrite(wxCommandEvent& event);
    static std::vector<ADC_DAC> mParameters;

  protected:
    wxPanel* pnlCustomControls;
    wxPanel* pnlReadControls;
    wxPanel* pnlEEPROMControls;
    wxFlexGridSizer* EEPROMsizer;
    wxSpinCtrl* spinCustomChannelRd;
    wxStaticText* txtCustomValueRd;
    wxStaticText* txtCustomUnitsRd;
    wxStaticText* txtCustomPowerOf10Rd;
    wxSpinCtrl* spinCustomChannelWr;
    wxSpinCtrl* spinCustomValueWr;
    wxChoice* cmbCustomUnitsWr;
    wxChoice* cmbCustomPowerOf10Wr;
    wxButton* btnCustomRd;
    wxButton* btnCustomWr;
    void OnMemoryWrite(wxCommandEvent& event);
    void OnMemoryRead(wxCommandEvent& event);
    void OnCustomRead(wxCommandEvent& event);
    void OnCustomWrite(wxCommandEvent& event);

    wxFlexGridSizer* sizerAnalogRd;
    wxPanel* additionalControls;
    wxStaticText* txtDACTitle;
    wxTextCtrl* txtDACValue;
    wxButton* btnDAC;
    wxFlexGridSizer* sizerDAC;

    wxFlexGridSizer* sizerAdditionalControls;

    std::vector<ADC_DAC> getBoardParams(const std::string& boardID);

    void OnUserChangedBoardType(wxCommandEvent& event);
    void OnReadAll(wxCommandEvent& event);
    void OnWriteAll(wxCommandEvent& event);

    wxButton* btnReadAll;
    wxButton* btnWriteAll;
    wxStaticText* m_staticText349;
    wxChoice* cmbBoardSelection;

    std::vector<Param_GUI*> mGUI_widgets;
    std::vector<MemoryParamGUI*> mMemoryGUI_widgets;
    static const std::vector<lime::eLMS_DEV> board_list;
};

#endif // __pnlAnalog_view__
