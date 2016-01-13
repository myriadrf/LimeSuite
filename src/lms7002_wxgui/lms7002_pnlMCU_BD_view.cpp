#include "lms7002_pnlMCU_BD_view.h"
#include "MCU_BD.h"
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include "dlgViewIRAM.h"
#include "dlgViewSFR.h"
using namespace lime;

lms7002_pnlMCU_BD_view::lms7002_pnlMCU_BD_view(wxWindow* parent)
    :
    pnlMCU_BD_view(parent), mcuControl(nullptr)
{
    mThreadWorking = false;
    progressPooler = new wxTimer(this, wxNewId());
    Connect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(lms7002_pnlMCU_BD_view::OnProgressPoll), NULL, this);

    m_iRegAddress = 1;
    m_iRegRead = 1;
    m_iTestNo = 0;
    m_bLoadedDebug = 0;
    m_bLoadedProd = 0;

    m_iDebug = 0;
    m_iPCvalue = 0;
    m_iTestNo = 1;
    m_iInstrNo = 1;
    m_iMode0 = 0;
    m_iMode1 = 0;

    m_iRegAddress = 1;
    m_iRegRead = 1;

    RunInstr->Enable(false);
    ResetPC->Enable(false);
    InstrNo->Enable(false);
    ViewSFRs->Enable(false);
    ViewIRAM->Enable(false);
    EraseIRAM->Enable(false);
    SelDiv->Enable(false);
    SelDiv->SetSelection(0);
    cmbRegAddr->SetSelection(1);

    PCValue->SetLabel(wxString::Format(_("0x%04X"), m_iPCvalue));

    InstrNo->SetLabel(_("1"));
    m_sTestNo->SetLabel(_("1"));
    chkReset->SetValue(true);
}

lms7002_pnlMCU_BD_view::~lms7002_pnlMCU_BD_view()
{    
    if(mThreadWorking)
        mWorkerThread.join();
    Disconnect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(lms7002_pnlMCU_BD_view::OnProgressPoll), NULL, this);
}

void lms7002_pnlMCU_BD_view::OnButton_LOADHexClick( wxCommandEvent& event )
{   
    wxFileDialog dlg(this, _("Open hex file"), _("hex"), _("*.hex"), _("HEX Files (*.hex)|*.hex|BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL) 
        return;

    wxString m_sHexFileName = dlg.GetPath();
    if (dlg.GetFilterIndex() == 0)
        mcuControl->GetProgramCode(m_sHexFileName.mb_str(), false);
    else
        mcuControl->GetProgramCode(m_sHexFileName.mb_str(), true);

    wxString temps;
    temps = _(".hex file: ");
    temps = temps << m_sHexFileName;
    lblProgCodeFile->SetLabel(temps);
}

void lms7002_pnlMCU_BD_view::OnchkResetClick( wxCommandEvent& event )
{
    assert(mcuControl != nullptr);
    if (chkReset->IsChecked())
    {
        // MODE=0
        // RESET
        m_iMode0 = 0;
        m_iMode1 = 0;
        mcuControl->Reset_MCU();
        rgrMode->Enable(false);
        btnStartProgramming->Enable(false);
        DebugMode->SetValue(false);

        m_iDebug = 0;
        RunInstr->Enable(false);
        ResetPC->Enable(false);
        InstrNo->Enable(false);
        ViewSFRs->Enable(false);
        ViewIRAM->Enable(false);
        EraseIRAM->Enable(false);
        SelDiv->Enable(false);
        // global variables
        m_bLoadedDebug = 0;
        m_bLoadedProd = 0;
    }
    else
    {
        rgrMode->Enable(true);
        btnStartProgramming->Enable(true);
    }
}

void lms7002_pnlMCU_BD_view::OnbtnStartProgrammingClick( wxCommandEvent& event )
{
    progressBar->SetValue(0);
    DebugMode->SetValue(false);
    int mode = rgrMode->GetSelection() + 1;
    m_iMode0 = mode & 0x01;
    m_iMode1 = (mode >> 1) & 0x01;

    if (chkReset->IsChecked())
    {
        wxMessageBox(_("Turn off reset."));
        return;
    }


    if (mThreadWorking)
        return;
    Disable();
    progressBar->SetValue(0);
    progressPooler->Start(200);
    Connect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnProgrammingfinished), NULL, this);

    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis, char mode1, char mode0)
    {
        int retval = pthis->mcuControl->Program_MCU(mode1, mode0);
        wxThreadEvent *evt = new wxThreadEvent();
        evt->SetInt(retval);
        wxQueueEvent(pthis, evt);
    }, this, m_iMode1, m_iMode0);
}

void lms7002_pnlMCU_BD_view::OnbtnLoadTestFileClick( wxCommandEvent& event )
{
    int  test_code = 0;
    int  address = 0;
    int  value = 0;
    int i = 0;
    //int temp;
    wxFileDialog dlg(this, _("Open txt file"), _("txt"), _("*.txt"), _("TXT Files (*.txt)|*.txt|TXT Files (*.txt)|*.txt |All Files (*.*)|*.*||"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL) 
        return;

    wxString m_sTxtFileName = dlg.GetPath();
    wxString temps;
    temps = _("Test results file: ");
    temps = temps + m_sTxtFileName;
    lblTestResultsFile->SetLabel(temps);
    
    FILE * inFile = NULL;
    inFile = fopen(m_sTxtFileName.mb_str(), "r");

    // debugging
    //FILE * outFile=NULL;
    //outFile = fopen("Out.txt", "w");
    // end debugging
    if (inFile != NULL)
    {
        m_iTestResultFileLine = 0;
        for (i = 0; i<256; i++)
        {
            TestResultArray_code[i] = 0;
            TestResultArray_address[i] = 0;
            TestResultArray_value[i] = 0;
        }

        m_iTestResultFileLine = 0;
        fscanf(inFile, "%d", &test_code);
        while (!feof(inFile))
        {
            //fscanf(inFile, "%d %d %d", &test_code, &address, &value);
            fscanf(inFile, "%d ", &address);
            fscanf(inFile, "%d\n", &value);
            TestResultArray_code[m_iTestResultFileLine] = (unsigned char)(test_code);
            TestResultArray_address[m_iTestResultFileLine] = (unsigned char)(address);
            TestResultArray_value[m_iTestResultFileLine] = (unsigned char)(value);

            m_iTestResultFileLine++;
            fscanf(inFile, "%d", &test_code);
        }
    }
    fclose(inFile);

    // debugging
    //for (i=0; i<m_iTestResultFileLine; i++) {
    //  fprintf(outFile, "%d %d %d \n",
    //          TestResultArray_code[i],
    //          TestResultArray_address[i],
    //          TestResultArray_value[i]);
    //}
    // fclose(outFile);
    // end debugging

}

void lms7002_pnlMCU_BD_view::OnbtnRunTestClick( wxCommandEvent& event )
{
    wxString m_sTxtFileName = _("lms7suite_mcu/TestResults.txt");
    lblTestResultsFile->SetLabel("Test results file: " + m_sTxtFileName);

    FILE * inFile = NULL;
    inFile = fopen(m_sTxtFileName.mb_str(), "r");

    // debugging
    //FILE * outFile=NULL;
    //outFile = fopen("Out.txt", "w");
    // end debugging
    if (inFile != NULL)
    {
        m_iTestResultFileLine = 0;
        for (int i = 0; i < 256; i++)
        {
            TestResultArray_code[i] = 0;
            TestResultArray_address[i] = 0;
            TestResultArray_value[i] = 0;
        }

        m_iTestResultFileLine = 0;
        int  test_code = 0;
        int  address = 0;
        int  value = 0;
        fscanf(inFile, "%d", &test_code);
        while (!feof(inFile))
        {
            //fscanf(inFile, "%d %d %d", &test_code, &address, &value);
            fscanf(inFile, "%d ", &address);
            fscanf(inFile, "%d\n", &value);
            TestResultArray_code[m_iTestResultFileLine] = (unsigned char)(test_code);
            TestResultArray_address[m_iTestResultFileLine] = (unsigned char)(address);
            TestResultArray_value[m_iTestResultFileLine] = (unsigned char)(value);

            m_iTestResultFileLine++;
            fscanf(inFile, "%d", &test_code);
        }
    }
    else
    {
        wxMessageBox(_("lms7suite_mcu/TestResults.txt file not found"));
        return;
    }
    fclose(inFile);

    unsigned char tempc1, tempc2, tempc3 = 0x00;
    int retval = 0;
    int m_iError = 0;
    int i = 0;

    if ((m_iTestNo <= 0) || (m_iTestNo>15))
    {
        m_iTestNo = 0;
        m_sTestNo->SetValue(_("0"));
    }
    else
    {
        mcuControl->RunTest_MCU(m_iMode1, m_iMode0, m_iTestNo, m_iDebug);
    }

    // check the results
    if (m_iDebug == 0)
        mcuControl->DebugModeSet_MCU(m_iMode1, m_iMode0);
    // Go to Debug mode
    m_iError = 0;
    i = 0;
    while ((i<m_iTestResultFileLine) && (m_iError == 0))
    {
        if (TestResultArray_code[i] == m_iTestNo) {
            retval = mcuControl->Three_byte_command(0x78, (unsigned char)(TestResultArray_address[i]), 0x00, &tempc1, &tempc2, &tempc3);
            if ((retval == -1) || (tempc3 != TestResultArray_value[i])) m_iError = 1;
            else i++;
        }
        else i++;
    };
    //exit the debug mode
    mcuControl->DebugModeExit_MCU(m_iMode1, m_iMode0);

    wxString temps, temps1, temps2, temps3;
    if (m_iError == 1)
    {
        temps1 = wxString::Format(_("0x%02X"), TestResultArray_address[i]);
        temps2 = wxString::Format(_("0x%02X"), TestResultArray_value[i]);
        temps3 = wxString::Format(_("0x%02X"), tempc3);

        temps = _("Test failed. No.:");
        temps = temps << TestResultArray_code[i];
        temps = temps << _(", at address ") << temps1;
        temps = temps << _(", should be ") << temps2;
        temps = temps << _(", but is read ") << temps3;
    }
    else 
        temps = _("OK");
    wxMessageBox(temps);
    return;

}

void lms7002_pnlMCU_BD_view::OnDebugModeClick( wxCommandEvent& event )
{
    if (DebugMode->IsChecked())
    {
        RunInstr->Enable(true);
        ResetPC->Enable(true);
        InstrNo->Enable(true);
        ViewSFRs->Enable(true);
        ViewIRAM->Enable(true);
        EraseIRAM->Enable(true);
        SelDiv->Enable(true);

        m_iDebug = 1;
        mcuControl->DebugModeSet_MCU(m_iMode1, m_iMode0);
    }
    else
    {
        RunInstr->Enable(false);
        ResetPC->Enable(false);
        InstrNo->Enable(false);
        ViewSFRs->Enable(false);
        ViewIRAM->Enable(false);
        EraseIRAM->Enable(false);
        SelDiv->Enable(false);

        m_iDebug = 0;
        mcuControl->DebugModeExit_MCU(m_iMode1, m_iMode0);
    }
}

void lms7002_pnlMCU_BD_view::OnRunInstruction( wxCommandEvent& event )
{
    wxString m_sPCVal;
    int i = 0;
    int retval = 0;
    unsigned short m_iPC = 0;

    if ((m_iInstrNo <1) || (m_iInstrNo>100))
    {
        m_iInstrNo = 0; // default TestNo.
        InstrNo->SetValue(_("0"));
        wxMessageBox(_("Number of instructions must be in the range [1-100]!"));
    }
    else
    {
        for (i = 0; i<m_iInstrNo; i++)
        {
            retval = mcuControl->RunInstr_MCU(&m_iPC);
            if (retval == -1)
            {
                i = m_iInstrNo; // end loop
                m_iPCvalue = 0;
            }
            else
            {
                m_iPCvalue = m_iPC;
            }
        }
        m_sPCVal.Printf(_("0x%04X"), m_iPCvalue);
        PCValue->SetLabel(m_sPCVal);
    }
}

void lms7002_pnlMCU_BD_view::OnResetPCClick( wxCommandEvent& event )
{
    int retval = mcuControl->ResetPC_MCU();
    if (retval == -1)
        wxMessageBox(_("Unable to reset MCU's Program Counter"));
}

void lms7002_pnlMCU_BD_view::OnViewSFRsClick( wxCommandEvent& event )
{
    if (mThreadWorking)
        return;
    Disable();
    progressBar->SetValue(0);
    progressPooler->Start(200);
    Connect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadSFRfinished), NULL, this);

    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis)
    {
        int retval = pthis->mcuControl->Read_SFR();
        wxThreadEvent *evt = new wxThreadEvent();
        evt->SetInt(retval);
        wxQueueEvent(pthis, evt);
    }, this);
}

void lms7002_pnlMCU_BD_view::OnViewIRAMClick( wxCommandEvent& event )
{
    if (mThreadWorking)
        return;
    Disable();
    progressBar->SetValue(0);
    progressPooler->Start(200);
    Connect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadIRAMfinished), NULL, this);

    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis)
        {   
            int retval = pthis->mcuControl->Read_IRAM();
            wxThreadEvent *evt = new wxThreadEvent();
            evt->SetInt(retval);
            wxQueueEvent(pthis, evt);            
        }, this);
}

void lms7002_pnlMCU_BD_view::OnEraseIRAMClick( wxCommandEvent& event )
{
    if (mThreadWorking)
        return;
    Disable();
    progressBar->SetValue(0);
    progressPooler->Start(200);
    Connect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnEraseIRAMfinished), NULL, this);

    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis)
    {
        int retval = pthis->mcuControl->Erase_IRAM();
        wxThreadEvent *evt = new wxThreadEvent();
        evt->SetInt(retval);
        wxQueueEvent(pthis, evt);
    }, this);
}

void lms7002_pnlMCU_BD_view::OnSelDivSelect( wxCommandEvent& event )
{
    int retval = 0;
    int tempi = 0;

    tempi = SelDiv->GetSelection();
    retval = mcuControl->Change_MCUFrequency(tempi);
    if (retval == -1)
        wxMessageBox(_("Cannot set the MCU's frequency"));
    else
        wxMessageBox(wxString::Format(_("Current selection: %s"), SelDiv->GetString(tempi)));
}

void lms7002_pnlMCU_BD_view::Onm_cCtrlBasebandSelect( wxCommandEvent& event )
{
    mcuControl->mSPI_write(0x0006, 0x0000); //REG6 write
}

void lms7002_pnlMCU_BD_view::Onm_cCtrlMCU_BDSelect( wxCommandEvent& event )
{
    mcuControl->mSPI_write(0x0006, 0x0001); //REG6 write
}

void lms7002_pnlMCU_BD_view::OnRegWriteRead( wxCommandEvent& event )
{   
    unsigned short addr = cmbRegAddr->GetSelection();
    long data;
    txtRegValueWr->GetValue().ToLong(&data);
    if (data < 0 || data > 255)
        data = 0;

    if (rbtnRegWrite->GetValue() == 1)
        mcuControl->mSPI_write(0x8000 + addr, data); //REG write
    else
    {
        unsigned short retval = 0;
        retval = mcuControl->mSPI_read(addr); //REG read        
        ReadResult->SetLabel(wxString::Format("Result is: 0x%02X", retval));
    }
}

void lms7002_pnlMCU_BD_view::Initialize(MCU_BD* pMCUcontrol)
{
    assert(pMCUcontrol != nullptr);
    mcuControl = pMCUcontrol;
}

void lms7002_pnlMCU_BD_view::OnProgressPoll(wxTimerEvent& evt)
{
    assert(mcuControl != nullptr);
    MCU_BD::ProgressInfo info = mcuControl->GetProgressInfo();
    progressBar->SetRange(100);
    float percent = 100.0*info.stepsDone / info.stepsTotal;
    progressBar->SetValue(percent);
}

void lms7002_pnlMCU_BD_view::OnReadIRAMfinished(wxThreadEvent &event)
{
    if (mThreadWorking)
        mWorkerThread.join();
    mThreadWorking = false;
    progressPooler->Stop();
    Disconnect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadIRAMfinished), NULL, this);
    progressBar->SetValue(100);
    Enable();
    if (event.GetInt() == -1)
    {
        wxMessageBox(_("Unable to read IRAM"));
        return;
    }
    dlgViewIRAM dlg(this);
    dlg.InitGridData(mcuControl->m_IRAM);
    dlg.ShowModal();        
}

void lms7002_pnlMCU_BD_view::OnEraseIRAMfinished(wxThreadEvent &event)
{
    if(mThreadWorking)
        mWorkerThread.join();
    mThreadWorking = false;
    progressPooler->Stop();
    Disconnect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnEraseIRAMfinished), NULL, this);
    progressBar->SetValue(100);
    Enable();
    if (event.GetInt() == -1)
        wxMessageBox(_("Unable to erase IRAM"));
}

void lms7002_pnlMCU_BD_view::OnReadSFRfinished(wxThreadEvent &event)
{
    if (mThreadWorking)
        mWorkerThread.join();
    mThreadWorking = false;
    progressPooler->Stop();
    Disconnect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadSFRfinished), NULL, this);
    progressBar->SetValue(100);
    Enable();
    if (event.GetInt() == -1)
    {
        wxMessageBox(_("Unable to read SFR"));
        return;
    }
    dlgViewSFR dlg(this);
    dlg.InitGridData(mcuControl->m_SFR);
    dlg.ShowModal();
}

void lms7002_pnlMCU_BD_view::OnProgrammingfinished(wxThreadEvent &event)
{
    if (mThreadWorking)
        mWorkerThread.join();
    mThreadWorking = false;
    progressPooler->Stop();
    Disconnect(wxID_ANY, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnProgrammingfinished), NULL, this);
    progressBar->SetValue(100);
    Enable();
    if (event.GetInt() == -1)
    {
        wxMessageBox(_("Unable to program the MCU"));
        return;
    }
    else
    {
        rgrMode->Enable();
        btnStartProgramming->Enable();
    }    
}

void lms7002_pnlMCU_BD_view::OnbtnRunProductionTestClicked(wxCommandEvent& event)
{
    int status = mcuControl->RunProductionTest_MCU();
    lblProgCodeFile->SetLabel("Program code file: " + mcuControl->GetProgramFilename());
    if (status == 0)
        wxMessageBox(_("Test passed"));
    else
        wxMessageBox(_("Test FAILED"));
}