#include "lms7002_dlgGFIR_Coefficients.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "CoefficientFileParser.h"

lms7002_dlgGFIR_Coefficients::lms7002_dlgGFIR_Coefficients( wxWindow* parent )
:
dlgGFIR_Coefficients( parent )
{

}

void lms7002_dlgGFIR_Coefficients::OnLoadFromFile( wxCommandEvent& event )
{
    wxFileDialog dlg(this, _("Open coefficients file"), "", "", "FIR Coeffs (*.fir)|*.fir", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    float cbuf[200];
    int iVal = Parser::getcoeffs((const char*)dlg.GetPath().ToStdString().c_str(), cbuf, 200);

    switch (iVal)
    {
    case -2:
        wxMessageDialog(this, "syntax error within the file", "Warning");
        break;
    case -3:
        wxMessageDialog(this, "filename is empty string", "Warning");
        break;
    case -4:
        wxMessageDialog(this, "can not open the file", "Warning");
        break;
    case -5:
        wxMessageDialog(this, "too many coefficients in the file", "Warning");
        break;
    }
    if (iVal < 0)
        return;

    spinCoefCount->SetValue(iVal);
    if (gridCoef->GetTable()->GetRowsCount() > 0)
        gridCoef->GetTable()->DeleteRows(0, gridCoef->GetTable()->GetRowsCount());
    gridCoef->GetTable()->AppendRows(spinCoefCount->GetValue());
    for (int i = 0; i<iVal; ++i)
    {
        gridCoef->SetCellValue(i, 0, wxString::Format("%f", cbuf[i]));
    }
}

void lms7002_dlgGFIR_Coefficients::OnSaveToFile( wxCommandEvent& event )
{
    wxFileDialog dlg(this, _("Save coefficients file"), "", "", "FIR Coeffs (*.fir)|*.fir", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    float coefficients[200];
    memset(coefficients, 0, sizeof(unsigned short) * 200);
    double ltemp;
    for (int i = 0; i<spinCoefCount->GetValue(); ++i)
    {
        ltemp = 0;
        gridCoef->GetCellValue(i, 0).ToDouble(&ltemp);
        coefficients[i] = ltemp;
    }
    Parser::saveToFile((const char*)dlg.GetPath().ToStdString().c_str(), coefficients, spinCoefCount->GetValue());
}

void lms7002_dlgGFIR_Coefficients::OnClearTable( wxCommandEvent& event )
{
    if (gridCoef->GetTable()->GetRowsCount() > 0)
        gridCoef->GetTable()->DeleteRows(0, gridCoef->GetTable()->GetRowsCount());
    gridCoef->GetTable()->AppendRows(spinCoefCount->GetValue());
    for (int i = 0; i<spinCoefCount->GetValue(); ++i)
    {
        gridCoef->SetCellValue(i, 0, wxString::Format("%i", 0));
    }
}

void lms7002_dlgGFIR_Coefficients::OnspinCoefCountChange(wxSpinEvent& event)
{
    if (spinCoefCount->GetValue() < gridCoef->GetTable()->GetRowsCount())
        gridCoef->GetTable()->DeleteRows(spinCoefCount->GetValue(), gridCoef->GetTable()->GetRowsCount() - spinCoefCount->GetValue());
    else
        gridCoef->GetTable()->AppendRows(spinCoefCount->GetValue() - gridCoef->GetTable()->GetRowsCount());
}

void lms7002_dlgGFIR_Coefficients::SetCoefficients(const std::vector<double> &coefficients)
{
    spinCoefCount->SetValue(coefficients.size());
    if (gridCoef->GetTable()->GetRowsCount() > 0)
        gridCoef->GetTable()->DeleteRows(0, gridCoef->GetTable()->GetRowsCount());
    gridCoef->GetTable()->AppendRows(coefficients.size());
    for (unsigned i = 0; i<coefficients.size(); ++i)
        gridCoef->SetCellValue(i, 0, wxString::Format("%.6f", coefficients[i]));
}

std::vector<double> lms7002_dlgGFIR_Coefficients::GetCoefficients()
{
    std::vector<double> coefficients;
    coefficients.resize(spinCoefCount->GetValue(), 0);
    for (int i = 0; i<spinCoefCount->GetValue(); ++i)
    {
        double dtemp = 0;
        gridCoef->GetCellValue(i, 0).ToDouble(&dtemp);
        coefficients[i] = dtemp;
    }
    return coefficients;
}

void lms7002_dlgGFIR_Coefficients::OnBtnOkClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void lms7002_dlgGFIR_Coefficients::OnBtnCancelClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}