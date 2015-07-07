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
#ifndef NDEBUG
IMPLEMENT_APP_CONSOLE(lms7suiteApp);
#else
IMPLEMENT_APP(lms7suiteApp);
#endif

bool lms7suiteApp::OnInit()
{
    wxLongLong t1 = wxGetUTCTimeMillis();
    LMS7SuiteAppFrame* frame = new LMS7SuiteAppFrame(0L);
#ifndef NDEBUG
    printf("Create time %li ms\n", wxGetUTCTimeMillis() - t1);
#endif
    //frame->SetIcon(wxICON(aaaa)); // To Set App Icon
    frame->Show();
    return true;
}
