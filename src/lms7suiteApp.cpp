/***************************************************************
 * Name:      lms7projectApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Lime Microsystems ()
 * Created:   2015-03-05
 * Copyright: Lime Microsystems (limemicro.com)
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "lms7suiteApp.h"
#include "lms7suiteAppFrame.h"
#include <wx/time.h>
#include <wx/splash.h>
#ifndef NDEBUG
IMPLEMENT_APP_CONSOLE(lms7suiteApp);
#else
IMPLEMENT_APP(lms7suiteApp);
#endif

#include "splash_image.h"

bool lms7suiteApp::OnInit()
{
    wxInitAllImageHandlers();
    wxBitmap splashBitmap = wxBITMAP_PNG_FROM_DATA(splash);
    wxSplashScreen* splash = new wxSplashScreen(splashBitmap,
        wxSPLASH_CENTRE_ON_SCREEN,
        6000, NULL, -1, wxDefaultPosition, wxDefaultSize,
        wxSIMPLE_BORDER | wxSTAY_ON_TOP);
    wxLongLong t1 = wxGetUTCTimeMillis();
    LMS7SuiteAppFrame* frame = new LMS7SuiteAppFrame(0L);
#ifndef NDEBUG
    printf("Create time %li ms\n", wxGetUTCTimeMillis() - t1);
#endif
    splash->Destroy();
    frame->Show();
    return true;
}
