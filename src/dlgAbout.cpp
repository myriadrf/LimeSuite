#include "dlgAbout.h"
#include "version.h"
dlgAbout::dlgAbout( wxWindow* parent )
:
dlgAbout_view( parent )
{

}

void dlgAbout::OnInit( wxInitDialogEvent& event )
{
#ifdef __WIN32__
	//wxBitmap *imgLogo = CreateBitmapFromPngResource("splashImage");
#endif
	wxString msg;
    msg << "Version: " << wxString::Format("%i.%i.%i.%i %s\n", AutoVersion::year % 100, AutoVersion::month, AutoVersion::day, AutoVersion::buildsCounter, AutoVersion::branchName);
    msg << "Build date: " << wxString::Format("%04i-%02i-%02i", AutoVersion::year, AutoVersion::month, AutoVersion::day);
	txtVersion->SetLabel(msg);
	txtDescription->SetLabel("");
}

void dlgAbout::OnbtnClose( wxCommandEvent& event )
{
	Destroy();
}
