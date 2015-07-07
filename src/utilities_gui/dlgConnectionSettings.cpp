#include "dlgConnectionSettings.h"
#include "connectionManager/ConnectionManager.h"
#include <wx/msgdlg.h>
#include <vector>
#include "lms7suiteEvents.h"
using namespace std;

dlgConnectionSettings::dlgConnectionSettings( wxWindow* parent )
	: dlgConnectionSettings_view( parent )
{
	lms7port = nullptr;
	streamBrdPort = nullptr;
}

void dlgConnectionSettings::SetConnectionManagers(ConnectionManager* lms7ctr, ConnectionManager* streamBrdctr)
{
	lms7port = lms7ctr;
	streamBrdPort = streamBrdctr;
}

void dlgConnectionSettings::GetDeviceList( wxInitDialogEvent& event )
{
	mListLMS7ports->Clear();
	mListStreamports->Clear();
	if (lms7port)
	{
		lms7port->RefreshDeviceList();
		vector<string> deviceNames = lms7port->GetDeviceList();
		for (unsigned i = 0; i<deviceNames.size(); ++i)
			mListLMS7ports->Append(deviceNames[i]);
		int openedIndex = lms7port->GetOpenedIndex();
        if (openedIndex >= 0 && openedIndex < mListLMS7ports->GetCount())
			mListLMS7ports->SetSelection(openedIndex);
	}
	else
		mListLMS7ports->Append("Connection Manager not initialized");

	if (streamBrdPort)
	{
		streamBrdPort->RefreshDeviceList();
		vector<string> deviceNames = streamBrdPort->GetDeviceList();
		for (unsigned i = 0; i<deviceNames.size(); ++i)
			mListStreamports->Append(deviceNames[i]);
		int openedIndex = streamBrdPort->GetOpenedIndex();
		if (openedIndex >= 0 && openedIndex < mListStreamports->GetCount())
			mListStreamports->SetSelection(openedIndex);
	}
	else
		mListStreamports->Append("Connection Manager not initialized");
}

void dlgConnectionSettings::OnConnect( wxCommandEvent& event )
{
	if(lms7port)
	{
        if(mListLMS7ports->GetSelection() != wxNOT_FOUND)
        {
            if (lms7port->Open(mListLMS7ports->GetSelection()) != IConnection::SUCCESS)
                wxMessageBox("Failed to open LMS7 control device", "Error", wxICON_STOP);
            else
            {
                wxCommandEvent evt;
                evt.SetInt(mListLMS7ports->GetSelection());
                evt.SetEventType(CONTROL_PORT_CONNECTED);
                evt.SetString(mListLMS7ports->GetString(event.GetInt()));
                wxPostEvent(this, evt);
            }
        }
	}
	if (streamBrdPort)
	{
        if(mListStreamports->GetSelection() != wxNOT_FOUND)
        {
            if (streamBrdPort->Open(mListStreamports->GetSelection()) != IConnection::SUCCESS)
                wxMessageBox("Failed to open Stream board connection", "Error", wxICON_STOP);
            else
            {
                wxCommandEvent evt;
                evt.SetInt(mListStreamports->GetSelection());
                evt.SetEventType(DATA_PORT_CONNECTED);
                evt.SetString(mListStreamports->GetString(event.GetInt()));
                wxPostEvent(this, evt);
            }
        }
	}
    Destroy();
}

void dlgConnectionSettings::OnCancel( wxCommandEvent& event )
{
	Destroy();
}

void dlgConnectionSettings::OnDisconnect( wxCommandEvent& event )
{
	if (lms7port)
	{
		lms7port->Close();
        wxCommandEvent evt;
        evt.SetEventType(CONTROL_PORT_DISCONNECTED);
        wxPostEvent(this, evt);
	}
	mListLMS7ports->SetSelection(-1);
	if (streamBrdPort)
	{
		streamBrdPort->Close();
        wxCommandEvent evt;
        evt.SetEventType(DATA_PORT_DISCONNECTED);
        wxPostEvent(this, evt);
	}
	mListStreamports->SetSelection(-1);
}
