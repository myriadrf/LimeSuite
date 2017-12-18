#include "pnlMiniLog.h"
#include "dlgFullMessageLog.h"

pnlMiniLog::pnlMiniLog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: pnlMiniLog_view( parent, id, pos, size, style ),
        log_level(2)
{
	mDefaultStyle = txtMessageField->GetDefaultStyle();
	wxUpdateUIEvent::SetUpdateInterval(100);
}

void pnlMiniLog::HandleMessage(wxCommandEvent &event)
{
    auto level = lime::LogLevel(event.GetInt());
    if (level > log_level)
        return;
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    //add time stamp
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%H:%M:%S", timeinfo);
    wxString line(wxString::Format("[%s] %s: %s", buffer, lime::logLevelToName(level), event.GetString()));

    mAllMessages.push_back(line);
    const int allMessageLimit = 3000;
    if (mAllMessages.size() > allMessageLimit)
        mAllMessages.pop_front();

    const int miniLogMessageLimit = 800;
    wxTextAttr style = mDefaultStyle;
    switch(level)
    {
        case lime::LOG_LEVEL_CRITICAL:
        case lime::LOG_LEVEL_ERROR:
                style.SetTextColour(*wxRED);
                break;
        case lime::LOG_LEVEL_WARNING:
                style.SetBackgroundColour(*wxYELLOW);
                style.SetTextColour(*wxBLACK);
                break;
        default: break;
    }
    txtMessageField->SetDefaultStyle(style);
    txtMessageField->AppendText(line);
    txtMessageField->SetDefaultStyle(mDefaultStyle);
    txtMessageField->AppendText(_("\n"));
    if (mAllMessages.size() > miniLogMessageLimit)
        txtMessageField->Remove(0,txtMessageField->GetLineLength(0)+1);
}

void pnlMiniLog::OnUpdateGUI(wxUpdateUIEvent& event)
{

}

void pnlMiniLog::OnBtnClearClicked(wxCommandEvent& event)
{
    mMessageList.clear();
    txtMessageField->Clear();
    mAllMessages.clear();
}

void pnlMiniLog::OnShowFullLog(wxCommandEvent& event)
{
    dlgFullMessageLog *dlg = new dlgFullMessageLog(this);
    dlg->AddMessages(mAllMessages);
    dlg->ShowModal();
}

void pnlMiniLog::onLogLvlChange(wxCommandEvent& event)
{
    log_level = 1+event.GetInt();
}
