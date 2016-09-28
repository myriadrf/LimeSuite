#include "dlgAbout.h"
#include "lime/LimeSuite.h"
#include "resources/splash.h"
dlgAbout::dlgAbout( wxWindow* parent )
:
dlgAbout_view( parent )
{

}

void dlgAbout::OnInit( wxInitDialogEvent& event )
{
    wxBitmap splashBitmap = wxBITMAP_PNG_FROM_DATA(splash);
    imgLogo->SetBitmap(splashBitmap);
	wxString msg;
    msg << "Version: " << LMS_GetLibraryVersion() << "\n";
    msg << "Build date: " << LMS_GetBuildTimestamp();
	txtVersion->SetLabel(msg);
	txtDescription->SetLabel("");
}

void dlgAbout::OnbtnClose( wxCommandEvent& event )
{
	EndModal(wxID_OK);
	Destroy();
}
