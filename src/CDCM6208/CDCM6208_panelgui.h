#ifndef __CDCM6208_panelgui__
#define __CDCM6208_panelgui__

#include <iostream>
#include <vector>
#include "CDCM6208_Dev.h"
#include "CDCM6208_wxgui.h"

#include "lime/LimeSuite.h"

using namespace lime;

class CDCM6208_panelgui : public CDCM_controlpanel
{


public:
	/** Constructor */
	CDCM6208_panelgui( wxWindow* parent );
   ~CDCM6208_panelgui();
	//// end generated class members
   CDCM6208_panelgui( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
   void Initialize(CDCM_Dev* cdcm);
   void UpdateGUI();

   double primaryFreq = 30.72*1e6;
   double secondaryFreq = 30.72*1e6;

protected:
   CDCM_Dev* CDCM;
   void OnChange( wxCommandEvent& event )override;
   void OnRadio( wxCommandEvent& event )override;
   void OnChoice( wxCommandEvent& event )override;
   void OnFreqEntry( wxCommandEvent& event )override;
   void OnButton( wxCommandEvent& event )override;
   void onFP_chk( wxCommandEvent& event );
};

#endif // __CDCM6208_panelgui__
