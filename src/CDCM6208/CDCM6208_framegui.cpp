#include "CDCM6208_framegui.h"
#include "API/lms7_device.h"

CDCM6208_framegui::CDCM6208_framegui(wxWindow* parent):CDCM6208_CONTROL( parent ), device(nullptr)
{}

CDCM6208_framegui::CDCM6208_framegui(wxWindow* parent, wxWindowID id , const wxString& title , const wxPoint& pos , const wxSize& size , long style )
:
CDCM6208_CONTROL(parent,id,title,pos,size,style), device(nullptr)
{}

void CDCM6208_framegui::Initialize(lms_device_t* dev)
{
    device = dev;
    CDCM_Dev* cdcm = ((LMS7_Device*)dev)->GetCDCMDev(0);
    if(cdcm)
        CDCM6208_framegui::m_CDCM1->Initialize(cdcm);
    else
        CDCM6208_framegui::m_CDCM1->Disable();
    
    // cdcm = ((LMS7_Device*)dev)->GetCDCMDev(1);
    // if(cdcm)
        // CDCM6208_framegui::m_CDCM2->Initialize(cdcm);
    // else
        // CDCM6208_framegui::m_CDCM2->Disable();
}
