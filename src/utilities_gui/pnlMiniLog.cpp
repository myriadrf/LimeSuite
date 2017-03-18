#include "pnlMiniLog.h"
#include "dlgFullMessageLog.h"

pnlMiniLog::pnlMiniLog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: pnlMiniLog_view( parent, id, pos, size, style )
{
	mDefaultStyle = txtMessageField->GetDefaultStyle();
	mNewMessages = 0;
	wxUpdateUIEvent::SetUpdateInterval(100);
}

void pnlMiniLog::HandleMessage(wxCommandEvent &event)
{   
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    //add time stamp
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%H:%M:%S", timeinfo);

    auto level = lime::LogLevel(event.GetInt());
    if (level == 0) level = lime::LOG_LEVEL_INFO;
    wxString line(wxString::Format("[%s] %s: %s", buffer, lime::logLevelToName(level), event.GetString()));

    mAllMessages.push_back(line);
    const int allMessageLimit = 3000;
    if (mAllMessages.size() > allMessageLimit)
        mAllMessages.pop_front();

    mMessageList.emplace_back(level, line);
    const int miniLogMessageLimit = 50;
    if (mMessageList.size() > miniLogMessageLimit)
        mMessageList.pop_front();
	++mNewMessages;
}

void pnlMiniLog::OnUpdateGUI(wxUpdateUIEvent& event)
{	
	if (mNewMessages == 0)
		return;
	txtMessageField->Clear();
	for (auto msg : mMessageList)
	{
		wxTextAttr style = mDefaultStyle;
		switch(msg.first)
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
		txtMessageField->AppendText(msg.second);
		txtMessageField->SetDefaultStyle(mDefaultStyle);
		txtMessageField->AppendText(_("\n"));
	}
	mNewMessages = 0;
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
