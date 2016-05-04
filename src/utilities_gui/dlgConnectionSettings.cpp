#include "dlgConnectionSettings.h"
#include <wx/msgdlg.h>
#include <vector>
#include "lms7suiteEvents.h"
#include <iso646.h> // alternative operators for visual c++: not, and, or...
using namespace std;

dlgConnectionSettings::dlgConnectionSettings( wxWindow* parent )
	: dlgConnectionSettings_view( parent )
{
	lmsOpenedIndex = -1;
	lmsControl = nullptr;
}

void dlgConnectionSettings::SetConnectionManagers(lms_device_t** lms)
{
    lmsControl = lms;
}

void dlgConnectionSettings::GetDeviceList( wxInitDialogEvent& event )
{
    mListLMS7ports->Clear();
    mListStreamports->Clear();
    lms_info_str_t list[32];
  
    //select the currently opened index
    lmsOpenedIndex = -1;
    int ret = LMS_GetDeviceList(list);
    if (ret <= 0)
        return;
    for (unsigned i = 0; i<ret; ++i)
    {
        std::string str = list[i];
        mListLMS7ports->Append(str.substr(0,str.find(',')));
        mListStreamports->Append(str.substr(0,str.find(',')));
    }
    if (lmsOpenedIndex >= 0 && lmsOpenedIndex < mListLMS7ports->GetCount())
        mListLMS7ports->SetSelection(lmsOpenedIndex);
}

void dlgConnectionSettings::OnConnect( wxCommandEvent& event )
{
    LMS_Disconnect(*lmsControl);
    lms_info_str_t list[32];
    int ret = LMS_GetDeviceList(list);
    if(mListLMS7ports->GetSelection() != wxNOT_FOUND)
    {

        LMS_Open(lmsControl,list[mListLMS7ports->GetSelection()]); 
        
        wxCommandEvent evt;
        evt.SetEventType(CONTROL_PORT_CONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }
    
   /* if(mListStreamports->GetSelection() != wxNOT_FOUND)
    {
        LMS_Open(lmsControl,list[mListStreamports->GetSelection()]); 
        
        wxCommandEvent evt;
        evt.SetEventType(DATA_PORT_CONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }*/
    Destroy();
}

void dlgConnectionSettings::OnCancel( wxCommandEvent& event )
{
	Destroy();
}

void dlgConnectionSettings::OnDisconnect( wxCommandEvent& event )
{
    if (*lmsControl != nullptr)
    {
        LMS_Disconnect(*lmsControl);
        wxCommandEvent evt;
        evt.SetEventType(CONTROL_PORT_DISCONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }
    mListLMS7ports->SetSelection(-1);
}
