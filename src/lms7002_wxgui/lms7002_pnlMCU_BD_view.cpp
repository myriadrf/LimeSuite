#include <fstream>
#include "lms7002_pnlMCU_BD_view.h"
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include "dlgViewIRAM.h"
#include "dlgViewSFR.h"
#include "MCU_File.h"


const long lms7002_pnlMCU_BD_view::ID_PROGRAMING_STATUS_EVENT = wxNewId();
const long lms7002_pnlMCU_BD_view::ID_PROGRAMING_FINISH_EVENT = wxNewId();

lms7002_pnlMCU_BD_view::lms7002_pnlMCU_BD_view(wxWindow* parent)
    :
    pnlMCU_BD_view(parent), lmsControl(nullptr)
{
    mThreadWorking = false;
    progressPooler = new wxTimer(this, wxNewId());

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
    mLoadedProgramFilename = "";
    m_iLoopTries=20;
    obj_ptr=nullptr;
}

lms7002_pnlMCU_BD_view::~lms7002_pnlMCU_BD_view()
{    
    if(mThreadWorking)
        mWorkerThread.join();
    Disconnect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(lms7002_pnlMCU_BD_view::OnProgressPoll), NULL, this);
}

/** @brief Read program code from file into memory
    @param inFileName source file path
    @param bin binary or hex file
    @return 0:success, -1:file not found
*/
int lms7002_pnlMCU_BD_view::GetProgramCode(const char* inFileName, bool bin)
{
    unsigned char ch=0x00;
    bool find_byte=false;
    int i=0;

    if(!bin)
    {
        MCU_File	inFile(inFileName, "rb");
        if (inFile.FileOpened() == false)
            return -1;

        mLoadedProgramFilename = inFileName;
        try
        {
            inFile.ReadHex(8191);
        }
        catch (...)
        {
            return -1;
        }

        for (i=0; i<8192; i++)
        {
            find_byte=inFile.GetByte(i, ch);
            if (find_byte==true)
                byte_array[i]=ch;
            else
                byte_array[i]=0x00;
        };
    }
    else
    {
        char inByte = 0;
        std::fstream fin;
        fin.open(inFileName, std::ios::in | std::ios::binary);
        if (fin.good() == false)
            return -1;
        mLoadedProgramFilename = inFileName;
        memset(byte_array, 0, 8192);
        for(int i=0; i<8192 && !fin.eof(); ++i)
        {
            inByte = 0;
            fin.read(&inByte, 1);
            byte_array[i]=inByte;
        }
    }
    return 0;
}

void lms7002_pnlMCU_BD_view::OnButton_LOADHexClick( wxCommandEvent& event )
{   
    wxFileDialog dlg(this, _("Open hex file"), _("hex"), _("*.hex"), _("HEX Files (*.hex)|*.hex|BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL) 
        return;

    wxString m_sHexFileName = dlg.GetPath();
    int status = 0;
    if (dlg.GetFilterIndex() == 0)
        status = GetProgramCode(m_sHexFileName.mb_str(), false);
    else
        status = GetProgramCode(m_sHexFileName.mb_str(), true);

    if (status != 0)
    {
        wxMessageBox(_("Failed to load Hex file"));
        return;
    }

    wxString temps;
    temps = _(".hex file: ");
    temps = temps << m_sHexFileName;
    lblProgCodeFile->SetLabel(temps);
}

void lms7002_pnlMCU_BD_view::OnchkResetClick( wxCommandEvent& event )
{
    assert(lmsControl != nullptr);
    if (chkReset->IsChecked())
    {
        // MODE=0
        // RESET
        m_iMode0 = 0;
        m_iMode1 = 0;
        LMS_ResetLMSMCU(lmsControl);
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
    Connect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnProgrammingfinished), NULL, this);
    Connect(ID_PROGRAMING_STATUS_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);
    obj_ptr = this;
    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis, char mode1, char mode0)
    {
        lms_target_t target;
        int retval=0;
        if (mode1 == 0 && mode0 == 0)
            retval = LMS_ResetLMSMCU(pthis->lmsControl);
        else
        {
            if (mode0 == 1 && mode1 == 0)
                target = LMS_TARGET_FLASH;
            else if (mode0 == 0 && mode1 == 1)
                target = LMS_TARGET_RAM;
            else if (mode0 == 1 && mode1 == 1)
                target = LMS_TARGET_BOOT;
            
            retval = LMS_ProgramLMSMCU(pthis->lmsControl,(const char*)pthis->byte_array,8192,target,OnProgrammingCallback);
        }
        wxThreadEvent *evt = new wxThreadEvent();
        evt->SetInt(retval);
        evt->SetId(ID_PROGRAMING_FINISH_EVENT);
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

void lms7002_pnlMCU_BD_view::Wait_CLK_Cycles(int delay)
{
	//// some delay
	int i=0;
        uint16_t val;
	for (i=0;i<(delay/64);i++)
           LMS_ReadLMSReg(lmsControl,0x0003,&val);
}

void lms7002_pnlMCU_BD_view::RunTest_MCU(int m_iMode1, int m_iMode0, unsigned short test_code, int m_iDebug) {

	int i=0;
	int limit=0;
	unsigned short tempi=0x0000;
	unsigned short basei=0x0000;

	if  (test_code<=15) basei=(test_code<<4);
	else basei=0x0000;

	basei=basei&0xFFF0; // not necessery
	// 4 LSBs are zeros

	// variable basei contains test no. value at bit positions 7-4
	// used for driving the P0 input
	// P0 defines the test no.

	if ((test_code>7)||(test_code==0))
        limit=1;
	else
        limit=50;

	// tests 8 to 14 have short duration

	if (m_iDebug==1) return; // normal MCU operating mode required

	// EXT_INT2=1, external interrupt 2 is raised
	tempi=0x0000;  // changed
	int m_iExt2=1;

	if (m_iExt2==1)  tempi=tempi|0x0004;
	if (m_iMode1==1) tempi=tempi|0x0002;
	if (m_iMode0==1) tempi=tempi|0x0001;

	// tempi variable is driving the mspi_REG2

        LMS_WriteLMSReg(lmsControl,0x8002,tempi);// REG2 write

	// generating waveform
	for (i=0; i<=limit; i++)
    {
		tempi=basei|0x000C;
                LMS_WriteLMSReg(lmsControl,0x8000,tempi);
		// REG0 write
		Wait_CLK_Cycles(256);
                tempi=basei|0x000D;
                LMS_WriteLMSReg(lmsControl,0x8000,tempi);
		// REG0 write  - P0(0) set
		Wait_CLK_Cycles(256);
		tempi=basei|0x000C;
		LMS_WriteLMSReg(lmsControl,0x8000,tempi);
		// REG0 write
		Wait_CLK_Cycles(256);
		tempi=basei|0x000E;
		LMS_WriteLMSReg(lmsControl,0x8000,tempi);
		// REG0 write - PO(1) set
		Wait_CLK_Cycles(256);

		if (i==0) {
            // EXT_INT2=0
            // external interrupt 2 is pulled down
			tempi=0x0000; // changed
			m_iExt2=0;
			if (m_iExt2==1)  tempi=tempi|0x0004;
			if (m_iMode1==1) tempi=tempi|0x0002;
			if (m_iMode0==1) tempi=tempi|0x0001;
			LMS_WriteLMSReg(lmsControl,0x8002,tempi);
            // REG2 write
		}
	}
}

void lms7002_pnlMCU_BD_view::DebugModeSet_MCU(int m_iMode1, int m_iMode0)
{
        unsigned short tempi=0x00C0;
        // bit DEBUG is set
		int m_iExt2=0;
		if (m_iExt2==1)  tempi=tempi|0x0004;
		if (m_iMode1==1) tempi=tempi|0x0002;
		if (m_iMode0==1) tempi=tempi|0x0001;

		// Select debug mode
                LMS_WriteLMSReg(lmsControl,0x8002,tempi);
		// REG2 write
}

 void lms7002_pnlMCU_BD_view::DebugModeExit_MCU(int m_iMode1, int m_iMode0)
{

        unsigned short tempi=0x0000; // bit DEBUG is zero
		int m_iExt2=0;

		if (m_iExt2==1)  tempi=tempi|0x0004;
		if (m_iMode1==1) tempi=tempi|0x0002;
		if (m_iMode0==1) tempi=tempi|0x0001;
		// To run mode
		LMS_WriteLMSReg(lmsControl,0x8002,tempi);  // REG2 write
}
 
 int lms7002_pnlMCU_BD_view::WaitUntilWritten(){

	 // waits if WRITE_REQ (REG3[2]) flag is equal to '1'
	 // this means that  write operation is in progress
	unsigned short tempi=0x0000;
	int countDown=m_iLoopTries;  // Time out value
        LMS_ReadLMSReg(lmsControl,0x0003,&tempi); // REG3 read

	while (( (tempi&0x0004) == 0x0004) && (countDown>0))
        {
		LMS_ReadLMSReg(lmsControl,0x0003,&tempi); // REG3 read
		countDown--;
	}
	if (countDown==0)
        return -1; // an error occured, timer elapsed
	else
        return 0; // Finished regularly
	// pass if WRITE_REQ is '0'
}
 
 int lms7002_pnlMCU_BD_view::ReadOneByte(unsigned char * data)
{
	unsigned short tempi=0x0000;
	int countDown=m_iLoopTries;

     // waits when READ_REQ (REG3[3]) flag is equal to '0'
	 // this means that there is nothing to read
	LMS_ReadLMSReg(lmsControl,0x0003,&tempi); // REG3 read

    while (((tempi&0x0008)==0x0000) && (countDown>0))
    {
        // wait if READ_REQ is '0'
        LMS_ReadLMSReg(lmsControl,0x0003,&tempi); // REG3 read
        countDown--;
	}

	if (countDown>0)
    { // Time out has not occured
		 LMS_ReadLMSReg(lmsControl,0x0005,&tempi);; // REG5 read
		  // return the read byte
		 (* data) = (unsigned char) (tempi);
	}
	else
        (* data) =0;
	 // return the zero, default value

	if (countDown==0)
        return -1; // an error occured
	else
        return 0; // finished regularly
}
 
int lms7002_pnlMCU_BD_view::One_byte_command(unsigned short data1, unsigned char * rdata1)
{
	unsigned char tempc=0x00;
	int retval=0;
	*rdata1=0x00; //default return value

	// sends the one byte command
        LMS_WriteLMSReg(lmsControl,0x8004,data1); //REG4 write
	retval=WaitUntilWritten();
	if (retval==-1) return -1;
	// error if operation executes too long

    // gets the one byte answer
	retval=ReadOneByte(&tempc);
    if (retval==-1) return -1;
	// error if operation takes too long

	*rdata1=tempc;
	return 0;
}

int lms7002_pnlMCU_BD_view::ResetPC_MCU()
{
     unsigned char tempc1=0x00;
     int retval=0;
     retval=One_byte_command(0x70, &tempc1);
     return retval;
}

int lms7002_pnlMCU_BD_view::Three_byte_command(
	    unsigned char data1,unsigned char data2,unsigned char data3,
		unsigned char * rdata1,unsigned char * rdata2,unsigned char * rdata3){


		int retval=0;
        *rdata1=0x00;
		*rdata2=0x00;
		*rdata3=0x00;
                
                LMS_WriteLMSReg(lmsControl,0x8004,(unsigned short)data1);
		retval=WaitUntilWritten();
		if (retval==-1) return -1;
                LMS_WriteLMSReg(lmsControl,0x8004,(unsigned short)data2); //REG4 write
		retval=WaitUntilWritten();
		if (retval==-1) return -1;

		LMS_WriteLMSReg(lmsControl,0x8004,(unsigned short)data3); //REG4 write
		retval=WaitUntilWritten();
		if (retval==-1) return -1;
                
		retval= ReadOneByte(rdata1);
		if (retval==-1) return -1;

		retval= ReadOneByte(rdata2);
		if (retval==-1) return -1;

		retval= ReadOneByte(rdata3);
		if (retval==-1) return -1;

		return 0;
}

int lms7002_pnlMCU_BD_view::RunInstr_MCU(unsigned short * pPCVAL)
{
    unsigned char tempc1, tempc2, tempc3=0x00;
    int retval=0;
    retval=Three_byte_command(0x74, 0x00, 0x00, &tempc1, &tempc2, &tempc3);
	if (retval==-1) (*pPCVAL)=0;
	else (*pPCVAL)=tempc2*256+tempc3;
	return retval;
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
        RunTest_MCU(m_iMode1, m_iMode0, m_iTestNo, m_iDebug);
    }

    // check the results
    if (m_iDebug == 0)
        DebugModeSet_MCU(m_iMode1, m_iMode0);
    // Go to Debug mode
    m_iError = 0;
    i = 0;
    while ((i<m_iTestResultFileLine) && (m_iError == 0))
    {
        if (TestResultArray_code[i] == m_iTestNo) {
            retval = Three_byte_command(0x78, (unsigned char)(TestResultArray_address[i]), 0x00, &tempc1, &tempc2, &tempc3);
            if ((retval == -1) || (tempc3 != TestResultArray_value[i])) m_iError = 1;
            else i++;
        }
        else i++;
    };
    //exit the debug mode
    DebugModeExit_MCU(m_iMode1, m_iMode0);

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
        DebugModeSet_MCU(m_iMode1, m_iMode0);
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
        DebugModeExit_MCU(m_iMode1, m_iMode0);
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
            retval = RunInstr_MCU(&m_iPC);
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
    int retval = ResetPC_MCU();
    if (retval == -1)
        wxMessageBox(_("Unable to reset MCU's Program Counter"));
}

int lms7002_pnlMCU_BD_view::Read_SFR()
{
    int i=0;
	unsigned char tempc1, tempc2, tempc3=0x00;
	int retval=0;

	//default m_SFR array initialization
	for (i=0; i<=255; i++)
        m_SFR[i]=0x00;
        OnProgrammingCallback(0, 48, "");
	// code 0x7A is for reading the SFR registers
	retval=Three_byte_command(0x7A, 0x80, 0x00, &tempc1, &tempc2, &tempc3); // P0
	if (retval==-1) return -1;
	m_SFR[0x80]=tempc3;

	retval=Three_byte_command(0x7A, 0x81, 0x00, &tempc1, &tempc2, &tempc3); // SP
	if (retval==-1) return -1;
	m_SFR[0x81]=tempc3;

	retval=Three_byte_command(0x7A, 0x82, 0x00, &tempc1, &tempc2, &tempc3); // DPL0
	if (retval==-1) return -1;
	m_SFR[0x82]=tempc3;

	retval=Three_byte_command(0x7A, 0x83, 0x00, &tempc1, &tempc2, &tempc3); // DPH0
	if (retval==-1) return -1;
	m_SFR[0x83]=tempc3;

	retval=Three_byte_command(0x7A, 0x84, 0x00, &tempc1, &tempc2, &tempc3); // DPL1
	if (retval==-1) return -1;
	m_SFR[0x84]=tempc3;

	retval=Three_byte_command(0x7A, 0x85, 0x00, &tempc1, &tempc2, &tempc3); // DPH1
	if (retval==-1) return -1;
	m_SFR[0x85]=tempc3;
        OnProgrammingCallback(6, 48, "");
        
	retval=Three_byte_command(0x7A, 0x86, 0x00, &tempc1, &tempc2, &tempc3); // DPS
	if (retval==-1) return -1;
	m_SFR[0x86]=tempc3;

	retval=Three_byte_command(0x7A, 0x87, 0x00, &tempc1, &tempc2, &tempc3); // PCON
	if (retval==-1) return -1;
	m_SFR[0x87]=tempc3;

	retval=Three_byte_command(0x7A, 0x88, 0x00, &tempc1, &tempc2, &tempc3); // TCON
	if (retval==-1) return -1;
	m_SFR[0x88]=tempc3;

	retval=Three_byte_command(0x7A, 0x89, 0x00, &tempc1, &tempc2, &tempc3); // TMOD
	if (retval==-1) return -1;
	m_SFR[0x89]=tempc3;

	retval=Three_byte_command(0x7A, 0x8A, 0x00, &tempc1, &tempc2, &tempc3); // TL0
	if (retval==-1) return -1;
	m_SFR[0x8A]=tempc3;

	retval=Three_byte_command(0x7A, 0x8B, 0x00, &tempc1, &tempc2, &tempc3); // TL1
	if (retval==-1) return -1;
	m_SFR[0x8B]=tempc3;
        OnProgrammingCallback(12, 48, "");
        
	retval=Three_byte_command(0x7A, 0x8C, 0x00, &tempc1, &tempc2, &tempc3); // TH0
	if (retval==-1) return -1;
	m_SFR[0x8C]=tempc3;

	retval=Three_byte_command(0x7A, 0x8D, 0x00, &tempc1, &tempc2, &tempc3); // TH1
	if (retval==-1) return -1;
        m_SFR[0x8D]=tempc3;

	retval=Three_byte_command(0x7A, 0x8E, 0x00, &tempc1, &tempc2, &tempc3); // PMSR
	if (retval==-1) return -1;
	m_SFR[0x8E]=tempc3;

	retval=Three_byte_command(0x7A, 0x90, 0x00, &tempc1, &tempc2, &tempc3); // P1
	if (retval==-1) return -1;
	m_SFR[0x90]=tempc3;

	retval=Three_byte_command(0x7A, 0x91, 0x00, &tempc1, &tempc2, &tempc3); // DIR1
	if (retval==-1) return -1;
	m_SFR[0x91]=tempc3;

	retval=Three_byte_command(0x7A, 0x98, 0x00, &tempc1, &tempc2, &tempc3); // SCON
	if (retval==-1) return -1;
	m_SFR[0x98]=tempc3;
        OnProgrammingCallback(18, 48, "");
        
	retval=Three_byte_command(0x7A, 0x99, 0x00, &tempc1, &tempc2, &tempc3); // SBUF
	if (retval==-1) return -1;
	m_SFR[0x99]=tempc3;

	retval=Three_byte_command(0x7A, 0xA0, 0x00, &tempc1, &tempc2, &tempc3); // P2
	if (retval==-1) return -1;
	m_SFR[0xA0]=tempc3;

	retval=Three_byte_command(0x7A, 0xA1, 0x00, &tempc1, &tempc2, &tempc3); // DIR2
	if (retval==-1) return -1;
	m_SFR[0xA1]=tempc3;

	retval=Three_byte_command(0x7A, 0xA2, 0x00, &tempc1, &tempc2, &tempc3); // DIR0
	if (retval==-1) return -1;
	m_SFR[0xA2]=tempc3;

	retval=Three_byte_command(0x7A, 0xA8, 0x00, &tempc1, &tempc2, &tempc3); // IEN0
	if (retval==-1) return -1;
	m_SFR[0xA8]=tempc3;

	retval=Three_byte_command(0x7A, 0xA9, 0x00, &tempc1, &tempc2, &tempc3); // IEN1
	if (retval==-1) return -1;
	m_SFR[0xA9]=tempc3;
        OnProgrammingCallback(24, 48, "");
        
	retval=Three_byte_command(0x7A, 0xB0, 0x00, &tempc1, &tempc2, &tempc3); // EECTRL
	if (retval==-1) return -1;
        m_SFR[0xB0]=tempc3;

	retval=Three_byte_command(0x7A, 0xB1, 0x00, &tempc1, &tempc2, &tempc3); // EEDATA
	if (retval==-1) return -1;
	m_SFR[0xB1]=tempc3;

	retval=Three_byte_command(0x7A, 0xB8, 0x00, &tempc1, &tempc2, &tempc3); // IP0
	if (retval==-1) return -1;
	m_SFR[0xB8]=tempc3;

	retval=Three_byte_command(0x7A, 0xB9, 0x00, &tempc1, &tempc2, &tempc3); // IP1
	if (retval==-1) return -1;
	m_SFR[0xB9]=tempc3;

	retval=Three_byte_command(0x7A, 0xBF, 0x00, &tempc1, &tempc2, &tempc3); // USR2
	if (retval==-1) return -1;
	m_SFR[0xBF]=tempc3;

	retval=Three_byte_command(0x7A, 0xC0, 0x00, &tempc1, &tempc2, &tempc3); // IRCON
	if (retval==-1) return -1;
	m_SFR[0xC0]=tempc3;
        OnProgrammingCallback(30, 48, "");
        
	retval=Three_byte_command(0x7A, 0xC8, 0x00, &tempc1, &tempc2, &tempc3); // T2CON
	if (retval==-1) return -1;
	m_SFR[0xC8]=tempc3;

	retval=Three_byte_command(0x7A, 0xCA, 0x00, &tempc1, &tempc2, &tempc3); // RCAP2L
	if (retval==-1) return -1;
	m_SFR[0xCA]=tempc3;

	retval=Three_byte_command(0x7A, 0xCB, 0x00, &tempc1, &tempc2, &tempc3); // RCAP2H
	if (retval==-1) return -1;
	m_SFR[0xCB]=tempc3;

	retval=Three_byte_command(0x7A, 0xCC, 0x00, &tempc1, &tempc2, &tempc3); // TL2
	if (retval==-1) return -1;
	m_SFR[0xCC]=tempc3;

	retval=Three_byte_command(0x7A, 0xCD, 0x00, &tempc1, &tempc2, &tempc3); // TH2
	if (retval==-1) return -1;
	m_SFR[0xCD]=tempc3;

	retval=Three_byte_command(0x7A, 0xD0, 0x00, &tempc1, &tempc2, &tempc3); // PSW
	if (retval==-1) return -1;
        m_SFR[0xD0]=tempc3;
        OnProgrammingCallback(36, 48, "");
        
	retval=Three_byte_command(0x7A, 0xE0, 0x00, &tempc1, &tempc2, &tempc3); // ACC
	if (retval==-1) return -1;
        m_SFR[0xE0]=tempc3;

	retval=Three_byte_command(0x7A, 0xF0, 0x00, &tempc1, &tempc2, &tempc3); // B
	if (retval==-1) return -1;
	m_SFR[0xF0]=tempc3;

	retval=Three_byte_command(0x7A, 0xEC, 0x00, &tempc1, &tempc2, &tempc3); // REG0
	if (retval==-1) return -1;
        m_SFR[0xEC]=tempc3;

	retval=Three_byte_command(0x7A, 0xED, 0x00, &tempc1, &tempc2, &tempc3); // REG1
	if (retval==-1) return -1;
	m_SFR[0xED]=tempc3;

	retval=Three_byte_command(0x7A, 0xEE, 0x00, &tempc1, &tempc2, &tempc3); // REG2
	if (retval==-1) return -1;
	m_SFR[0xEE]=tempc3;

	retval=Three_byte_command(0x7A, 0xEF, 0x00, &tempc1, &tempc2, &tempc3); // REG3
	if (retval==-1) return -1;
	m_SFR[0xEF]=tempc3;
        OnProgrammingCallback(42, 48, "");
	retval=Three_byte_command(0x7A, 0xF4, 0x00, &tempc1, &tempc2, &tempc3); // REG4
	if (retval==-1) return -1;
	m_SFR[0xF4]=tempc3;

	retval=Three_byte_command(0x7A, 0xF5, 0x00, &tempc1, &tempc2, &tempc3); // REG5
	if (retval==-1) return -1;
	m_SFR[0xF5]=tempc3;

	retval=Three_byte_command(0x7A, 0xF6, 0x00, &tempc1, &tempc2, &tempc3); // REG6
	if (retval==-1) return -1;
	m_SFR[0xF6]=tempc3;

	retval=Three_byte_command(0x7A, 0xF7, 0x00, &tempc1, &tempc2, &tempc3); // REG7
	if (retval==-1) return -1;
	m_SFR[0xF7]=tempc3;

	retval=Three_byte_command(0x7A, 0xFC, 0x00, &tempc1, &tempc2, &tempc3); // REG8
	if (retval==-1) return -1;
	m_SFR[0xFC]=tempc3;

	retval=Three_byte_command(0x7A, 0xFD, 0x00, &tempc1, &tempc2, &tempc3); // REG9
	if (retval==-1) return -1;
	m_SFR[0xFD]=tempc3;
        OnProgrammingCallback(48, 48, "");
	return 0;
}

void lms7002_pnlMCU_BD_view::OnViewSFRsClick( wxCommandEvent& event )
{
    if (mThreadWorking)
        return;
    Disable();
    progressBar->SetValue(0);
    progressPooler->Start(200);

    Connect(ID_PROGRAMING_STATUS_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);
    Connect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadSFRfinished), NULL, this);

    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis)
    {
        int retval = pthis->Read_SFR();
        wxThreadEvent *evt = new wxThreadEvent();
        evt->SetInt(retval);
        evt->SetId(ID_PROGRAMING_FINISH_EVENT);
        wxQueueEvent(pthis, evt);
    }, this);
}

int lms7002_pnlMCU_BD_view::Read_IRAM()
{
    unsigned char tempc1, tempc2, tempc3=0x00;
    int i=0;
    int retval=0;

    //default
    //IRAM array initialization
    for (i=0; i<=255; i++)
            m_IRAM[i]=0x00;

    /*stepsTotal.store(256);
    stepsDone.store(0);
    aborted.store(false);*/
    unsigned stepsDone=0;
    OnProgrammingCallback(stepsDone, 256, "");
    for (i=0; i<=255; i++)
    {
        // code 0x78 is for reading the IRAM locations
		retval=Three_byte_command(0x78, ((unsigned char)(i)), 0x00,&tempc1, &tempc2, &tempc3);
		if (retval==0)
            m_IRAM[i]=tempc3;
		else
        {
            i=256; // error, stop
            //aborted.store(true);
        }
       OnProgrammingCallback(++stepsDone, 256, "");
#ifndef NDEBUG
        //printf("MCU reading IRAM: %2i/256\r", stepsDone.load());
#endif
        Wait_CLK_Cycles(64);
	}
#ifndef NDEBUG
    printf("\nMCU reading IRAM finished\n");
#endif
	return retval;
}

void lms7002_pnlMCU_BD_view::OnViewIRAMClick( wxCommandEvent& event )
{
    if (mThreadWorking)
        return;
    Disable();
    progressBar->SetValue(0);
    progressPooler->Start(200);
    Connect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadIRAMfinished), NULL, this);
    Connect(ID_PROGRAMING_STATUS_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);

    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis)
        {   
            int retval = pthis->Read_IRAM();
            wxThreadEvent *evt = new wxThreadEvent();
            evt->SetInt(retval);
            evt->SetId(ID_PROGRAMING_FINISH_EVENT);
            wxQueueEvent(pthis, evt);            
        }, this);
}

int lms7002_pnlMCU_BD_view::Erase_IRAM()
{
	unsigned char tempc1, tempc2, tempc3=0x00;
	int retval=0;
	int i=0;

	//default ini.
	for (i=0; i<=255; i++)
			m_IRAM[i]=0x00;

    /*stepsTotal.store(256);
    stepsDone.store(0);
    aborted.store(false);*/
    unsigned stepsDone=0;
    OnProgrammingCallback(stepsDone, 256, "");
	for (i=0; i<=255; i++)
    {
			m_IRAM[i]=0x00;
            // code 0x7C is for writing the IRAM locations
			retval=Three_byte_command(0x7C, ((unsigned char)(i)), 0x00,&tempc1, &tempc2, &tempc3);
            if (retval == -1)
            {
                i = 256;
                //aborted.store(true);
            }
            OnProgrammingCallback(++stepsDone, 256, "");
#ifndef NDEBUG
            //printf("MCU erasing IRAM: %2i/256\r", stepsDone.load());
#endif
	}
#ifndef NDEBUG
    printf("\nMCU erasing IRAM finished\n");
#endif
	return retval;
}

void lms7002_pnlMCU_BD_view::OnEraseIRAMClick( wxCommandEvent& event )
{
    if (mThreadWorking)
        return;
    Disable();
    progressBar->SetValue(0);
    progressPooler->Start(200);
    Connect(ID_PROGRAMING_STATUS_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);
    Connect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnEraseIRAMfinished), NULL, this);

    mThreadWorking = true;
    mWorkerThread = std::thread([](lms7002_pnlMCU_BD_view* pthis)
    {
        int retval = pthis->Erase_IRAM();
        wxThreadEvent *evt = new wxThreadEvent();
        evt->SetInt(retval);
        evt->SetId(ID_PROGRAMING_FINISH_EVENT);
        wxQueueEvent(pthis, evt);
    }, this);
}

int lms7002_pnlMCU_BD_view::Change_MCUFrequency(unsigned char data) {

	unsigned char tempc1, tempc2, tempc3=0x00;
	int retval=0;
    // code 0x7E is for writing the SFR registers
	retval=Three_byte_command(0x7E, 0x8E, data, &tempc1, &tempc2, &tempc3);
	// PMSR register, address 0x8E
	return retval;
}

void lms7002_pnlMCU_BD_view::OnSelDivSelect( wxCommandEvent& event )
{
    int retval = 0;
    int tempi = 0;

    tempi = SelDiv->GetSelection();
    retval = Change_MCUFrequency(tempi);
    if (retval == -1)
        wxMessageBox(_("Cannot set the MCU's frequency"));
    else
        wxMessageBox(wxString::Format(_("Current selection: %s"), SelDiv->GetString(tempi)));
}

void lms7002_pnlMCU_BD_view::Onm_cCtrlBasebandSelect( wxCommandEvent& event )
{
    LMS_WriteLMSReg(lmsControl,0x0006, 0x0000);
}

void lms7002_pnlMCU_BD_view::Onm_cCtrlMCU_BDSelect( wxCommandEvent& event )
{
    LMS_WriteLMSReg(lmsControl,0x0006, 0x0001);; //REG6 write
}

void lms7002_pnlMCU_BD_view::OnRegWriteRead( wxCommandEvent& event )
{   
    unsigned short addr = cmbRegAddr->GetSelection();
    long data;
    txtRegValueWr->GetValue().ToLong(&data);
    if (data < 0 || data > 255)
        data = 0;

    if (rbtnRegWrite->GetValue() == 1)
        LMS_WriteLMSReg(lmsControl,0x8000 + addr, data); //REG write
    else
    {
        unsigned short retval = 0;
        LMS_ReadLMSReg(lmsControl,addr, &retval); //REG read       
        ReadResult->SetLabel(wxString::Format("Result is: 0x%02X", retval));
    }
}

void lms7002_pnlMCU_BD_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlMCU_BD_view::OnProgressPoll(wxTimerEvent& evt)
{
    /*assert(mcuControl != nullptr);
    MCU_BD::ProgressInfo info = mcuControl->GetProgressInfo();
    progressBar->SetRange(100);
    float percent = 100.0*info.stepsDone / info.stepsTotal;
    progressBar->SetValue(percent);*/
}

lms7002_pnlMCU_BD_view* lms7002_pnlMCU_BD_view::obj_ptr=nullptr;
bool lms7002_pnlMCU_BD_view::OnProgrammingCallback(int bsent, int btotal, const char* progressMsg)
{
    wxCommandEvent evt;
    evt.SetEventObject(obj_ptr);
    evt.SetInt(100.0 * bsent / btotal); //round to int
    evt.SetString(wxString::From8BitData(progressMsg));
    evt.SetEventType(wxEVT_COMMAND_THREAD);
    evt.SetId(ID_PROGRAMING_STATUS_EVENT);
    wxPostEvent(obj_ptr, evt);
    return false;
}
void lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate(wxCommandEvent& event)
{
    progressBar->SetValue(event.GetInt());
}

void lms7002_pnlMCU_BD_view::OnReadIRAMfinished(wxThreadEvent &event)
{
    if (mThreadWorking)
        mWorkerThread.join();
    mThreadWorking = false;
    progressPooler->Stop();
    Disconnect(ID_PROGRAMING_STATUS_EVENT, wxEVT_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);
    Disconnect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadIRAMfinished), NULL, this);
    progressBar->SetValue(100);
    Enable();
    if (event.GetInt() == -1)
    {
        wxMessageBox(_("Unable to read IRAM"));
        return;
    }
    dlgViewIRAM dlg(this);
    dlg.InitGridData(m_IRAM);
    dlg.ShowModal();        
}

void lms7002_pnlMCU_BD_view::OnEraseIRAMfinished(wxThreadEvent &event)
{
    if(mThreadWorking)
        mWorkerThread.join();
    mThreadWorking = false;
    progressPooler->Stop();
    Disconnect(ID_PROGRAMING_STATUS_EVENT, wxEVT_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);
    Disconnect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnEraseIRAMfinished), NULL, this);
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
    Disconnect(ID_PROGRAMING_STATUS_EVENT, wxEVT_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);
    Disconnect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnReadSFRfinished), NULL, this);
    progressBar->SetValue(100);
    Enable();
    if (event.GetInt() == -1)
    {
        wxMessageBox(_("Unable to read SFR"));
        return;
    }
    dlgViewSFR dlg(this);
    dlg.InitGridData(m_SFR);
    dlg.ShowModal();
}

void lms7002_pnlMCU_BD_view::OnProgrammingfinished(wxThreadEvent &event)
{
    if (mThreadWorking)
        mWorkerThread.join();
    mThreadWorking = false;
    progressPooler->Stop();
    Disconnect(ID_PROGRAMING_STATUS_EVENT, wxEVT_THREAD, (wxObjectEventFunction)&lms7002_pnlMCU_BD_view::OnProgramingStatusUpdate);
    Disconnect(ID_PROGRAMING_FINISH_EVENT, wxEVT_THREAD, wxThreadEventHandler(lms7002_pnlMCU_BD_view::OnProgrammingfinished), NULL, this);
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
    /*int status = mcuControl->RunProductionTest_MCU();
    lblProgCodeFile->SetLabel("Program code file: " + mLoadedProgramFilename);
    if (status == 0)
        wxMessageBox(_("Test passed"));
    else
        wxMessageBox(_("Test FAILED"));*/
}