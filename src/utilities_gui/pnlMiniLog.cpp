#include "pnlMiniLog.h"

pnlMiniLog::pnlMiniLog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: pnlMiniLog_view( parent, id, pos, size, style )
{
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
    strftime(buffer, 80, "[%H:%M:%S] ", timeinfo);    

    wxString line = wxString::From8BitData(buffer);
    line.append(event.GetString());
    mAllMessages.push_back(line);
    const int allMessageLimit = 2000;
    if (mAllMessages.size() > allMessageLimit)
        mAllMessages.pop_front();

    mMessageList.push_back(line);
    const int miniLogMessageLimit = 10;
    if (mMessageList.size() > miniLogMessageLimit)
        mMessageList.pop_front();
	++mNewMessages;
}

void pnlMiniLog::OnUpdateGUI(wxUpdateUIEvent& event)
{	
	if (mNewMessages == 0)
		return;
	wxString text;
	txtMessageField->Clear();
	for (auto msg : mMessageList)
	{
		text += (wxString(msg) + _("\n"));
	}
    txtMessageField->Clear();
	txtMessageField->AppendText(text);
	mNewMessages = 0;
}

void pnlMiniLog::OnBtnClearClicked(wxCommandEvent& event)
{
    mMessageList.clear();
    txtMessageField->Clear();
}

void pnlMiniLog::OnShowFullLog(wxCommandEvent& event)
{

}