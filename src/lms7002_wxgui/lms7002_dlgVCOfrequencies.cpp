#include "lms7002_dlgVCOfrequencies.h"
#include "LMS7002M.h"
#include <wx/filedlg.h>
#include <vector>
#include <fstream>

using namespace std;

lms7002_dlgVCOfrequencies::lms7002_dlgVCOfrequencies( wxWindow* parent , LMS7002M* plmsControl)
:
dlgVCOfrequencies( parent )
{
    lmsControl = plmsControl;
    double multiplier = 0.001;
    txtVCOL_low->SetLabel(wxString::Format(_("%.3f"), lmsControl->gVCO_frequency_table[0][0] * multiplier));
    txtVCOL_high->SetLabel(wxString::Format(_("%.3f"), lmsControl->gVCO_frequency_table[0][1] * multiplier));
    txtVCOM_low->SetLabel(wxString::Format(_("%.3f"), lmsControl->gVCO_frequency_table[1][0] * multiplier));
    txtVCOM_high->SetLabel(wxString::Format(_("%.3f"), lmsControl->gVCO_frequency_table[1][1] * multiplier));
    txtVCOH_low->SetLabel(wxString::Format(_("%.3f"), lmsControl->gVCO_frequency_table[2][0] * multiplier));
    txtVCOH_high->SetLabel(wxString::Format(_("%.3f"), lmsControl->gVCO_frequency_table[2][1] * multiplier));
    txtVCOCGEN_low->SetLabel(wxString::Format(_("%.3f"), lmsControl->gCGEN_VCO_frequencies[0] * multiplier));
    txtVCOCGEN_high->SetLabel(wxString::Format(_("%.3f"), lmsControl->gCGEN_VCO_frequencies[1] * multiplier));
}

void lms7002_dlgVCOfrequencies::OnBtnOkClick( wxCommandEvent& event )
{
    double freq;
    double multiplier = 1000;
    txtVCOL_low->GetValue().ToDouble(&freq);
    lmsControl->gVCO_frequency_table[0][0] = freq * multiplier;
    txtVCOL_high->GetValue().ToDouble(&freq);
    lmsControl->gVCO_frequency_table[0][1] = freq * multiplier;

    txtVCOM_low->GetValue().ToDouble(&freq);
    lmsControl->gVCO_frequency_table[1][0] = freq * multiplier;
    txtVCOM_high->GetValue().ToDouble(&freq);
    lmsControl->gVCO_frequency_table[1][1] = freq * multiplier;

    txtVCOH_low->GetValue().ToDouble(&freq);
    lmsControl->gVCO_frequency_table[2][0] = freq * multiplier;
    txtVCOH_high->GetValue().ToDouble(&freq);
    lmsControl->gVCO_frequency_table[2][1] = freq * multiplier;

    txtVCOCGEN_low->GetValue().ToDouble(&freq);
    lmsControl->gCGEN_VCO_frequencies[0] = freq * multiplier;
    txtVCOCGEN_high->GetValue().ToDouble(&freq);
    lmsControl->gCGEN_VCO_frequencies[1] = freq * multiplier;

    EndModal(wxID_OK);
}

void lms7002_dlgVCOfrequencies::OnBtnCancelClick( wxCommandEvent& event )
{
    EndModal(wxID_CANCEL);
}

void lms7002_dlgVCOfrequencies::OnSaveFile(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Save vco file"), "", "", "text (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    fstream fout;
    fout.open(dlg.GetPath().mb_str(), ios::out);

    fout << "[VCOH] count: " << 2 << "," << endl;
    fout << txtVCOH_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOH_high->GetValue() << ' ' << "GHz," << endl;

    fout << "[VCOM] count: " << 2 << "," << endl;
    fout << txtVCOM_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOM_high->GetValue() << ' ' << "GHz," << endl;

    fout << "[VCOL] count: " << 2 << "," << endl;
    fout << txtVCOL_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOL_high->GetValue() << ' ' << "GHz," << endl;

    fout << "[VCO_CGEN] count: " << 2 << "," << endl;
    fout << txtVCOCGEN_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOCGEN_high->GetValue() << ' ' << "GHz," << endl;
    fout.close();
}

void lms7002_dlgVCOfrequencies::OnLoadFile(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Open vco file"), "", "", "text (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    fstream fin;
    fin.open(dlg.GetPath().ToStdString().c_str(), ios::in);

    char cline[128];
    int lineLen = 128;

    char vconame[80];
    char ctemp[80];
    double freq = 0;
    int freqCount = 0;

    std::vector<double> frequencies;

    while (!fin.eof())
    {
        freqCount = 0;
        fin.getline(cline, lineLen);
        //find name and frequency count
        sscanf(cline, "%s %s %i", vconame, ctemp, &freqCount);
        if (strcmp(vconame, "[VCOH]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOH_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOH_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
        if (strcmp(vconame, "[VCOM]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOM_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOM_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
        if (strcmp(vconame, "[VCOL]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOL_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOL_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
        if (strcmp(vconame, "[VCO_CGEN]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOCGEN_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOCGEN_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
    }
    fin.close();
}