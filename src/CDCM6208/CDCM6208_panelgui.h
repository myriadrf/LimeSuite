#ifndef __CDCM6208_panelgui__
#define __CDCM6208_panelgui__

#include <iostream>
#include <vector>

#include "ISOCPanel.h"

#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include "CDCM6208_Dev.h"

class CDCM6208_panelgui : public ISOCPanel
{
  public:
    CDCM6208_panelgui(wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0);
    ~CDCM6208_panelgui();
    void Initialize(CDCM_Dev* cdcm);
    virtual void UpdateGUI() override;

    double primaryFreq = 30.72 * 1e6;
    double secondaryFreq = 30.72 * 1e6;

    bool Initialize(lime::SDRDevice* device);
    virtual void Update() override;

  protected:
    CDCM_Dev* CDCM;
    void OnChange(wxCommandEvent& event);
    void OnRadio(wxCommandEvent& event);
    void OnChoice(wxCommandEvent& event);
    void OnFreqEntry(wxCommandEvent& event);
    void OnButton(wxCommandEvent& event);
    void onFP_chk(wxCommandEvent& event);

  protected:
    wxStaticText* m_staticText1;
    wxTextCtrl* m_PrimaryFreq;
    wxStaticText* m_staticText2;
    wxTextCtrl* m_SecondaryFreq;
    wxStaticText* m_staticText3;
    wxTextCtrl* m_RDivider;
    wxStaticText* m_staticText4;
    wxRadioButton* m_PrimarySel;
    wxRadioButton* m_SecondarySel;
    wxStaticText* m_staticText5;
    wxTextCtrl* m_MDivider;
    wxStaticText* m_staticText6;
    wxTextCtrl* m_NMultiplier;
    wxStaticText* m_staticText7;
    wxStaticText* m_VCOFREQ;
    wxStaticText* m_staticText9;
    wxChoice* m_PSB;
    wxStaticText* m_staticText10;
    wxChoice* m_PSA;
    wxStaticText* m_staticText50;
    wxChoice* m_CDCM_VER;
    wxStaticText* m_staticText51;
    wxStaticText* m_VCORANGE;
    wxStaticText* m_staticText26;
    wxTextCtrl* m_Baseaddr;
    wxStaticText* m_staticText27;
    wxStaticText* m_LockStatus;
    wxButton* m_WriteAll;
    wxButton* m_ReadAll;
    wxButton* m_Reset;
    wxStaticText* m_staticText42;
    wxCheckBox* m_Y0Y1_chk;
    wxStaticText* m_staticText43;
    wxCheckBox* m_Y2Y3_chk;
    wxStaticText* m_staticText442;
    wxCheckBox* m_Y4_chk;
    wxStaticText* m_staticText452;
    wxCheckBox* m_Y5_chk;
    wxStaticText* m_staticText46;
    wxCheckBox* m_Y6_chk;
    wxStaticText* m_staticText47;
    wxCheckBox* m_Y7_chk;
    wxButton* m_button4;
    wxStaticText* m_FrequencyPlanRes;
    wxStaticText* m_staticText20;
    wxStaticText* m_staticText21;
    wxStaticText* m_staticText49;
    wxTextCtrl* m_Y2Y3_DIV;
    wxTextCtrl* m_Y2Y3_FREQ;
    wxTextCtrl* m_Y2Y3_FREQ_req;
    wxStaticText* m_staticText38;
    wxStaticText* m_staticText39;
    wxStaticText* m_staticText501;
    wxTextCtrl* m_Y6_DIV;
    wxTextCtrl* m_Y6_FREQ;
    wxTextCtrl* m_Y6_FREQ_req;
    wxStaticText* m_staticText24;
    wxStaticText* m_staticText25;
    wxStaticText* m_staticText511;
    wxTextCtrl* m_Y7_DIV;
    wxTextCtrl* m_Y7_FREQ;
    wxTextCtrl* m_Y7_FREQ_req;
    wxStaticText* m_staticText40;
    wxStaticText* m_staticText41;
    wxStaticText* m_staticText52;
    wxTextCtrl* m_Y0Y1_DIV;
    wxTextCtrl* m_Y0Y1_FREQ;
    wxTextCtrl* m_Y0Y1_FREQ_req;
    wxStaticText* m_staticText44;
    wxStaticText* m_staticText45;
    wxStaticText* m_staticText53;
    wxTextCtrl* m_Y4_DIV;
    wxTextCtrl* m_Y4_FREQ;
    wxTextCtrl* m_Y4_FREQ_req;
    wxStaticText* m_staticText441;
    wxStaticText* m_staticText451;
    wxStaticText* m_staticText54;
    wxTextCtrl* m_Y5_DIV;
    wxTextCtrl* m_Y5_FREQ;
    wxTextCtrl* m_Y5_FREQ_req;
};

#endif // __CDCM6208_panelgui__
