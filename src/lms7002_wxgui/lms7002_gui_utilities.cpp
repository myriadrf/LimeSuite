/**
@file 	gui_utilities.cpp
@author Lime Microsystems (www.limemicro.com)
@brief 	Implementation of common functions used by all panels
*/
#include "lms7002_gui_utilities.h"
#include <wx/panel.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include "numericSlider.h"
#include <wx/spinctrl.h>
#include <wx/object.h>
#include <LMS7002M.h>

void LMS7002_WXGUI::UpdateControlsByMap(wxPanel* panel, LMS7002M* lmsControl, const std::map<wxWindow*, LMS7Parameter> &wndId2param)
{
    if (panel == nullptr || lmsControl == nullptr)
        return;
    panel->Freeze();

    wxObject *wnd;
    unsigned long value = 0;
    wxClassInfo *wndClass;
    wxClassInfo *cmbInfo = wxClassInfo::FindClass(_("wxComboBox"));
    wxClassInfo *chkInfo = wxClassInfo::FindClass(_("wxCheckBox"));
    wxClassInfo *rgrInfo = wxClassInfo::FindClass(_("wxRadioBox"));
    wxClassInfo *numericSliderInfo = wxClassInfo::FindClass(_("NumericSlider"));
    wxClassInfo *spinCtrlInfo = wxClassInfo::FindClass(_("wxSpinCtrl"));
    wxClassInfo *labelInfo = wxClassInfo::FindClass(_("wxStaticText"));
    wxClassInfo *radioBtnInfo = wxClassInfo::FindClass(_("wxRadioButton"));
        
    for (auto idParam : wndId2param)
    {
        wnd = idParam.first;
        if (wnd == panel)
        {
            int a = 1;
        }
        if (wnd == nullptr)
            continue;
        wndClass = wnd->GetClassInfo();

        // read only from local copy to increase performance
        bool fromChip = false;
        value = lmsControl->Get_SPI_Reg_bits(idParam.second, fromChip);
        //cast window to specific control, to set value, or set selection
        if (wndClass->IsKindOf(cmbInfo))
        {
            wxComboBox *box = wxStaticCast(wnd, wxComboBox);
            if (box->GetCount() < value)
            {
                wxString str;
                str = wxString::Format(_("combobox value(%li) is out of range [0-%i]"), value, box->GetCount() - 1);
                //wxMessageBox(str, "WARNING!");
                value = 0;
            }
            box->SetSelection(value);
        }
        else if (wndClass->IsKindOf(chkInfo))
        {
            /*if (m_checkboxIDToInvert.find(wnd->GetId()) != m_checkboxIDToInvert.end())
            {
            if (value == true)
            value = 0;
            else
            value = 1;
            }*/
            wxStaticCast(wnd, wxCheckBox)->SetValue(value);
        }
        else if (wndClass->IsKindOf(rgrInfo))
        {
            wxRadioBox *box = wxStaticCast(wnd, wxRadioBox);
            if (box->GetCount() < value)
            {
                wxString str;
                str = wxString::Format(_("radiogroup value(%i) is out of range [0-%i]"), value, box->GetCount() - 1);
                wxMessageBox(str, "WARNING!");
                value = 0;
                continue;
            }
            box->SetSelection(value);
        }
        else if (wndClass->IsKindOf(labelInfo))
        {
            wxStaticCast(wnd, wxStaticText)->SetLabel(wxString::Format(_("%li"), value));
        }
        else if (wndClass->IsKindOf(numericSliderInfo))
        {
            wxStaticCast(wnd, NumericSlider)->SetValue(value);
        }
        else if (wndClass->IsKindOf(spinCtrlInfo))
        {
            wxStaticCast(wnd, wxSpinCtrl)->SetValue(value);
        }
        else if (wndClass->IsKindOf(radioBtnInfo))
        {
            wxStaticCast(wnd, wxRadioButton)->SetValue(value);
        }
        else
        {
            wxString str;
            str = wxString::Format(_("Unhandled control class type. className=%s, was assigned address %04X"), wndClass->GetClassName(), idParam.second.address);
            wxMessageBox(str, "ERROR!");
        }
    }
    panel->Thaw();
}

int LMS7002_WXGUI::index2value(int index, const indexValueMap &pairs)
{
    for (int i = 0; i < pairs.size(); ++i)
        if (index == pairs[i].first)
            return pairs[i].second;
    return 0;
}

int LMS7002_WXGUI::value2index(int value, const indexValueMap &pairs)
{
    for (int i = 0; i < pairs.size(); ++i)
        if (value == pairs[i].second)
            return pairs[i].first;
    return 0;
}
