#include "pnlMiniLog.h"

pnlMiniLog::pnlMiniLog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: pnlMiniLog_view( parent, id, pos, size, style )
{
	mNewMessages = 0;
	wxUpdateUIEvent::SetUpdateInterval(100);
}
/*
void pnlMiniLog::HandleMessage(const LMS_Message &msg)
{
	mMessageList.push_back(wxString(msg.text));
	if (mMessageList.size() > 6)
		mMessageList.erase(mMessageList.begin());
	++mNewMessages;
}*/

void pnlMiniLog::OnUpdateGUI(wxUpdateUIEvent& event)
{	
	if (mNewMessages == 0)
		return;
	wxString text;
	txtMessageField->Clear();
	for (auto msg : mMessageList)
	{
		text += (wxString(msg) + "\n");
	}
	txtMessageField->SetLabel(text);
	mNewMessages = 0;
}