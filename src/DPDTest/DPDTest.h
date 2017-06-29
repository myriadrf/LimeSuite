#ifndef __DPDTest_H__
#define __DPDTest_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <stdint.h>
class OpenGLGraph;

#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/gauge.h>
#include <wx/frame.h>
#include "../kissFFT/kiss_fft.h"
#include "lms7_device.h"

class LMScomms;

class DPDTest : public wxFrame
{
private:

protected:
    OpenGLGraph* mFFT_xp;
    OpenGLGraph* mFFT_yp;
    OpenGLGraph* mFFT_xp1;
    OpenGLGraph* mFFT_yp1;
    OpenGLGraph* mFFT_x;

    OpenGLGraph* mTime_xp;
    OpenGLGraph* mTime_yp;
    OpenGLGraph* mTime_xp1;
    OpenGLGraph* mTime_yp1;
    OpenGLGraph* mTime_x;
    wxButton* btnCapture;
    wxChoice* cmbLmsDevice;
    
    void OnbtnCaptureClicked(wxCommandEvent& event);
    lms_device_t *lmsControl;
    float mLmsNyquist_MHz;

    void PlotFFT(OpenGLGraph* plot, const kiss_fft_cpx* fftOutput, const int samplesCount, const float nyquist_MHz);
    void PlotTimeDomain(OpenGLGraph* plot, const kiss_fft_cpx* samples, const int samplesCount);
public:
    void Initialize(lms_device_t* dataPort);
    void SetNyquist(float LmsNyquistMHz);
    DPDTest(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("FFT viewer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~DPDTest();
};

#endif //__FFTVIEWER_WXGUI_H__
