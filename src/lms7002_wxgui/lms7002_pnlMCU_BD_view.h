#ifndef __lms7002_pnlMCU_BD_view__
#define __lms7002_pnlMCU_BD_view__

#include "lms7002_wxgui.h"

#include <wx/timer.h>
#include <thread>
#include "ILMS7002MTab.h"
#include "MCU_BD.h"

class lms7002_pnlMCU_BD_view : public ILMS7002MTab
{
  protected:
    // Handlers for pnlMCU_BD_view events.
    void OnButton_LOADHexClick(wxCommandEvent& event);
    void OnchkResetClick(wxCommandEvent& event);
    void OnbtnStartProgrammingClick(wxCommandEvent& event);
    void OnbtnLoadTestFileClick(wxCommandEvent& event);
    void OnbtnRunTestClick(wxCommandEvent& event);
    void OnDebugModeClick(wxCommandEvent& event);
    void OnRunInstruction(wxCommandEvent& event);
    void OnResetPCClick(wxCommandEvent& event);
    void OnViewSFRsClick(wxCommandEvent& event);
    void OnViewIRAMClick(wxCommandEvent& event);
    void OnEraseIRAMClick(wxCommandEvent& event);
    void OnSelDivSelect(wxCommandEvent& event);
    void Onm_cCtrlBasebandSelect(wxCommandEvent& event);
    void Onm_cCtrlMCU_BDSelect(wxCommandEvent& event);
    void OnRegWriteRead(wxCommandEvent& event);
    void OnbtnRunProductionTestClicked(wxCommandEvent& event);

    void OnProgramingStatusUpdate(wxCommandEvent& event);
    void OnReadIRAMfinished(wxThreadEvent& event);
    void OnEraseIRAMfinished(wxThreadEvent& event);
    void OnReadSFRfinished(wxThreadEvent& event);
    void OnProgrammingfinished(wxThreadEvent& event);

    static bool OnProgrammingCallback(int bsent, int btotal, const char* progressMsg);

  public:
    lms7002_pnlMCU_BD_view(wxWindow* parent, wxWindowID id);
    ~lms7002_pnlMCU_BD_view();
    void Initialize(ControllerType* pControl) override;

  protected:
    lime::MCU_BD* mcu;

    bool mThreadWorking;
    std::thread mWorkerThread;
    wxTimer* progressPooler;

    int m_iTestResultFileLine;
    int TestResultArray_code[256];
    int TestResultArray_address[256];
    int TestResultArray_value[256];

    int m_iDebug;
    int m_iMode0;
    int m_iMode1;
    int m_bLoadedProd;
    int m_bLoadedDebug;
    int m_iTestNo;
    unsigned int m_iInstrNo;
    int m_iPCvalue;

    static lms7002_pnlMCU_BD_view* obj_ptr;
    static const long ID_PROGRAMING_STATUS_EVENT;
    static const long ID_PROGRAMING_FINISH_EVENT;

    enum {
        ID_CHECKBOX_RESETMCU = 2048,
        ID_TESTNO,
        ID_DEBUGMODE,
        ID_RUNINSTR,
        ID_INSTRNO,
        ID_RESETPC,
        ID_SELDIV,
        ID_RADIOBUTTON4,
        ID_RADIOBUTTON5
    };

    wxStaticText* ID_STATICTEXT1;
    wxButton* Button_LOADHEX;
    wxCheckBox* chkReset;
    wxRadioBox* rgrMode;
    wxButton* btnStartProgramming;
    wxStaticText* ID_S_PROGFINISHED;
    wxStaticText* ID_STATICTEXT3;
    wxButton* btnRunTest;
    wxStaticText* ID_STATICTEXT4;
    wxTextCtrl* m_sTestNo;
    wxButton* btnRunProductionTest;
    wxCheckBox* DebugMode;
    wxButton* RunInstr;
    wxStaticText* ID_STATICTEXT6;
    wxTextCtrl* InstrNo;
    wxButton* ResetPC;
    wxStaticText* ID_STATICTEXT7;
    wxStaticText* PCValue;
    wxButton* ViewSFRs;
    wxButton* ViewIRAM;
    wxButton* EraseIRAM;
    wxStaticText* ID_STATICTEXT10;
    wxChoice* SelDiv;
    wxRadioButton* m_cCtrlBaseband;
    wxRadioButton* m_cCtrlMCU_BD;
    wxStaticText* ID_STATICTEXT5;
    wxChoice* cmbRegAddr;
    wxRadioButton* rbtnRegWrite;
    wxStaticText* ID_STATICTEXT11;
    wxTextCtrl* txtRegValueWr;
    wxRadioButton* rbtnRegRead;
    wxStaticText* ReadResult;
    wxButton* btnRdWr;
    wxStaticText* ID_STATICTEXT2;
    wxGauge* progressBar;
    wxStaticText* lblProgCodeFile;
    wxStaticText* lblTestResultsFile;
};

#endif // __lms7002_pnlMCU_BD_view__
