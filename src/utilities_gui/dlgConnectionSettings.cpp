#include "dlgConnectionSettings.h"
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <wx/msgdlg.h>
#include <vector>
#include "lms7suiteEvents.h"
using namespace std;

dlgConnectionSettings::dlgConnectionSettings( wxWindow* parent )
	: dlgConnectionSettings_view( parent )
{
	lmsOpenedIndex = -1;
	streamOpenedIndex = -1;
	lms7Conn = nullptr;
	streamBrdConn = nullptr;
}

void dlgConnectionSettings::GetDeviceList( wxInitDialogEvent& event )
{
    mListLMS7ports->Clear();
    mListStreamports->Clear();
    cachedHandles = ConnectionRegistry::findConnections();

    for (unsigned i = 0; i<cachedHandles.size(); ++i)
        mListLMS7ports->Append(cachedHandles[i].serialize());
    if (lmsOpenedIndex >= 0 && lmsOpenedIndex < mListLMS7ports->GetCount())
        mListLMS7ports->SetSelection(lmsOpenedIndex);

    for (unsigned i = 0; i<cachedHandles.size(); ++i)
        mListStreamports->Append(cachedHandles[i].serialize());
    if (streamOpenedIndex >= 0 && streamOpenedIndex < mListStreamports->GetCount())
        mListStreamports->SetSelection(streamOpenedIndex);
}

void dlgConnectionSettings::OnConnect( wxCommandEvent& event )
{
    if(mListLMS7ports->GetSelection() != wxNOT_FOUND)
    {
        lms7Conn = ConnectionRegistry::makeConnection(cachedHandles.at(mListLMS7ports->GetSelection()));
        if (lms7Conn != nullptr and not lms7Conn->IsOpen())
        {
            ConnectionRegistry::freeConnection(lms7Conn);
            lms7Conn = nullptr;
            wxMessageBox("Failed to open LMS7 control device", "Error", wxICON_STOP);
        }
        else
        {
            wxCommandEvent evt;
            evt.SetInt(mListLMS7ports->GetSelection());
            evt.SetEventType(CONTROL_PORT_CONNECTED);
            evt.SetString(mListLMS7ports->GetString(event.GetInt()));
            if(GetParent())
                wxPostEvent(GetParent(), evt);
        }
    }

    if(mListStreamports->GetSelection() != wxNOT_FOUND)
    {
        streamBrdConn = ConnectionRegistry::makeConnection(cachedHandles.at(mListStreamports->GetSelection()));
        if (streamBrdConn != nullptr and not streamBrdConn->IsOpen())
        {
            ConnectionRegistry::freeConnection(streamBrdConn);
            streamBrdConn = nullptr;
            wxMessageBox("Failed to open Stream board connection", "Error", wxICON_STOP);
        }
        else
        {
            wxCommandEvent evt;
            evt.SetInt(mListStreamports->GetSelection());
            evt.SetEventType(DATA_PORT_CONNECTED);
            evt.SetString(mListStreamports->GetString(event.GetInt()));
            if(GetParent())
                wxPostEvent(GetParent(), evt);
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
    if (lms7Conn != nullptr)
    {
        ConnectionRegistry::freeConnection(lms7Conn);
        lms7Conn = nullptr;
        wxCommandEvent evt;
        evt.SetEventType(CONTROL_PORT_DISCONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }
    mListLMS7ports->SetSelection(-1);

    if (streamBrdConn != nullptr)
    {
        ConnectionRegistry::freeConnection(streamBrdConn);
        streamBrdConn = nullptr;
        wxCommandEvent evt;
        evt.SetEventType(DATA_PORT_DISCONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }
    mListStreamports->SetSelection(-1);
}
