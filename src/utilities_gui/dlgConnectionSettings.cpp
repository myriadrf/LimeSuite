#include "dlgConnectionSettings.h"
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <wx/msgdlg.h>
#include <vector>
#include "lms7suiteEvents.h"
#include <iso646.h> // alternative operators for visual c++: not, and, or...
using namespace std;

dlgConnectionSettings::dlgConnectionSettings( wxWindow* parent )
	: dlgConnectionSettings_view( parent )
{
	lmsOpenedIndex = -1;
	streamOpenedIndex = -1;
	lms7Manager = nullptr;
	streamBrdManager = nullptr;
}

void dlgConnectionSettings::SetConnectionManagers(IConnection **lms7ctr, IConnection **streamBrdctr)
{
	lms7Manager = lms7ctr;
	streamBrdManager = streamBrdctr;
}

void dlgConnectionSettings::GetDeviceList( wxInitDialogEvent& event )
{
    mListLMS7ports->Clear();
    mListStreamports->Clear();
    cachedHandles = ConnectionRegistry::findConnections();

    //select the currently opened index
    lmsOpenedIndex = -1;
    streamOpenedIndex = -1;
    for (size_t i = 0; i < cachedHandles.size(); i++)
    {
        if (*lms7Manager != nullptr and (*lms7Manager)->GetHandle() == cachedHandles[i])
        {
            lmsOpenedIndex = i;
        }
        if (*streamBrdManager != nullptr and (*streamBrdManager)->GetHandle() == cachedHandles[i])
        {
            streamOpenedIndex = i;
        }
    }

    for (unsigned i = 0; i<cachedHandles.size(); ++i)
        mListLMS7ports->Append(cachedHandles[i].ToString());
    if (lmsOpenedIndex >= 0 && lmsOpenedIndex < mListLMS7ports->GetCount())
        mListLMS7ports->SetSelection(lmsOpenedIndex);

    for (unsigned i = 0; i<cachedHandles.size(); ++i)
        mListStreamports->Append(cachedHandles[i].ToString());
    if (streamOpenedIndex >= 0 && streamOpenedIndex < mListStreamports->GetCount())
        mListStreamports->SetSelection(streamOpenedIndex);
}

void dlgConnectionSettings::OnConnect( wxCommandEvent& event )
{
    if(mListLMS7ports->GetSelection() != wxNOT_FOUND)
    {
        //free previously used connection
        ConnectionRegistry::freeConnection(*lms7Manager);
        *lms7Manager = nullptr;
        auto lms7Conn = ConnectionRegistry::makeConnection(cachedHandles.at(mListLMS7ports->GetSelection()));
        if (lms7Conn != nullptr and not lms7Conn->IsOpen())
        {
            ConnectionRegistry::freeConnection(lms7Conn);
            lms7Conn = nullptr;
            wxMessageBox("Failed to open LMS7 control device", "Error", wxICON_STOP);
        }
        else
        {
            *lms7Manager = lms7Conn;
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
        //free previously used connection
        ConnectionRegistry::freeConnection(*streamBrdManager);
        *streamBrdManager = nullptr;
        auto streamBrdConn = ConnectionRegistry::makeConnection(cachedHandles.at(mListStreamports->GetSelection()));
        if (streamBrdConn != nullptr and not streamBrdConn->IsOpen())
        {
            ConnectionRegistry::freeConnection(streamBrdConn);
            streamBrdConn = nullptr;
            wxMessageBox("Failed to open Stream board connection", "Error", wxICON_STOP);
        }
        else
        {
            *streamBrdManager = streamBrdConn;
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
    auto lms7Conn = *lms7Manager;
    if (lms7Conn != nullptr)
    {
        *lms7Manager = nullptr;
        ConnectionRegistry::freeConnection(lms7Conn);
        wxCommandEvent evt;
        evt.SetEventType(CONTROL_PORT_DISCONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }
    mListLMS7ports->SetSelection(-1);

    auto streamBrdConn = *streamBrdManager;
    if (streamBrdConn != nullptr)
    {
        *streamBrdManager = nullptr;
        ConnectionRegistry::freeConnection(streamBrdConn);
        wxCommandEvent evt;
        evt.SetEventType(DATA_PORT_DISCONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }
    mListStreamports->SetSelection(-1);
}
