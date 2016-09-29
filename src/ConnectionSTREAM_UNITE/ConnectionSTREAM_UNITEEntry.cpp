/**
    @file ConnectionSTREAMEntry.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#include "ConnectionSTREAM_UNITE.h"
#include <fstream>
#include <stdio.h>
using namespace lime;

//! make a static-initialized entry in the registry
void __loadConnectionSTREAM_UNITEEntry(void) //TODO fixme replace with LoadLibrary/dlopen
{
static ConnectionSTREAM_UNITEEntry STREAM_UNITEEntry;
}

ConnectionSTREAM_UNITEEntry::ConnectionSTREAM_UNITEEntry(void):
    ConnectionSTREAMEntry("STREAM+UNITE")
{

}

ConnectionSTREAM_UNITEEntry::~ConnectionSTREAM_UNITEEntry(void)
{

}

static std::vector<std::string> FindAllComPorts()
{
    std::vector<std::string> comPortList;
#ifndef __unix__
	HKEY hSERIALCOMM;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),	0, KEY_QUERY_VALUE, &hSERIALCOMM) == ERROR_SUCCESS)
	{
		// Get the max value name and max value lengths
		DWORD dwMaxValueNameLen;
		DWORD dwMaxValueLen;
		DWORD dwQueryInfo = RegQueryInfoKey(hSERIALCOMM, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL);
		if (dwQueryInfo == ERROR_SUCCESS)
		{
			DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1;
			// Include space for the NULL terminator
			DWORD dwMaxValueNameSizeInBytes = dwMaxValueNameSizeInChars*sizeof(TCHAR);
			DWORD dwMaxValueDataSizeInChars = dwMaxValueLen / sizeof(TCHAR) + 1;
			// Include space for the NULL terminator
			DWORD dwMaxValueDataSizeInBytes = dwMaxValueDataSizeInChars*sizeof(TCHAR);

			// Allocate some space for the value name and value data
			TCHAR* szValueName = new TCHAR[dwMaxValueNameSizeInChars];
			TCHAR* byValue = new TCHAR[dwMaxValueDataSizeInBytes];
			if (szValueName && byValue)
			{
				// Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
				DWORD dwIndex = 0;
				DWORD dwType;
				DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
				DWORD dwDataSize = dwMaxValueDataSizeInBytes;
				memset(szValueName, 0, dwMaxValueNameSizeInBytes);
				memset(byValue, 0, dwMaxValueDataSizeInBytes);
				LONG nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, (LPBYTE)byValue, &dwDataSize);
				while (nEnum == ERROR_SUCCESS)
				{
					// If the value is of the correct type, then add it to the array
					if (dwType == REG_SZ)
					{
					    char portname[512];
						TCHAR* szPort = byValue;
						int nUserNameLenUnicode = lstrlen( szPort ); // Convert all UNICODE characters
						int nUserNameLen = WideCharToMultiByte( CP_ACP, // ANSI Code Page
                        0, // No special handling of unmapped chars
                        (LPCWSTR)szPort, // wide-character string to be converted
                        nUserNameLenUnicode,
                        NULL, 0, // No output buffer since we are calculating length
                        NULL, NULL ); // Unrepresented char replacement - Use Default
                        TCHAR* pszUserName = new TCHAR[ nUserNameLen ]; // nUserNameLen includes the NULL character
                        WideCharToMultiByte( CP_ACP, // ANSI Code Page
                        0, // No special handling of unmapped chars
						(LPCWSTR)szPort, // wide-character string to be converted
                        nUserNameLenUnicode,
                        portname,
                        nUserNameLen,
                        NULL, NULL ); // Unrepresented char replacement - Use Default
						portname[nUserNameLen] = 0;
#ifdef UNICODE
                        comPortList.push_back(portname);
#else
                        comPortList.push_back(szPort);
#endif
					}
					// Prepare for the next time around
					dwValueNameSize = dwMaxValueNameSizeInChars;
					dwDataSize = dwMaxValueDataSizeInBytes;
					memset(szValueName, 0, dwMaxValueNameSizeInBytes);
					memset(byValue, 0, dwMaxValueDataSizeInBytes);
					++dwIndex;
					nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName,	&dwValueNameSize, NULL, &dwType, (LPBYTE)byValue, &dwDataSize);
				}
			}
			delete szValueName;
			delete byValue;
		}
		// Close the registry key now that we are finished with it
		RegCloseKey(hSERIALCOMM);

		if (dwQueryInfo != ERROR_SUCCESS)
			SetLastError(dwQueryInfo);
	}
#else
    FILE *lsPipe;
    lsPipe = popen("ls /dev/* -d -1 | grep ttyACM", "r");
    char tempBuffer[512];
    while(fscanf(lsPipe, "%s", tempBuffer) == 1)
        comPortList.push_back(tempBuffer);
    pclose(lsPipe);
#endif // __unix__
    return comPortList;
}

std::vector<ConnectionHandle> ConnectionSTREAM_UNITEEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> usbHandles;
    std::vector<ConnectionHandle> comHandles;
    std::vector<ConnectionHandle> handles;

    usbHandles = ConnectionSTREAMEntry::enumerate(hint);

    std::vector<std::string> comPorts = FindAllComPorts();
    for(auto comName : comPorts)
    {
        ConnectionEVB7COM port(comName.c_str(), 9600);
        if(port.IsOpen() == false)
            continue;

        DeviceInfo info = port.GetDeviceInfo();

        ConnectionHandle hnd;
        hnd.media = "COM";
        hnd.name = info.deviceName+"("+comName+")";
        hnd.addr = comName;
        comHandles.push_back(hnd);
    }

    int index = 0;
    for(auto usb : usbHandles)
    {
        for(auto com : comHandles)
        {
            ConnectionHandle hnd;
            hnd.module = "STREAM+UNITE";
            hnd.media = "USB+COM";
            hnd.name = usb.name+"+"+com.name;
            hnd.addr = usb.addr+"+"+com.addr;
            hnd.serial = "";
            hnd.index = index++;
            handles.push_back(hnd);
        }
    }
    return handles;
}

IConnection *ConnectionSTREAM_UNITEEntry::make(const ConnectionHandle &handle)
{
#ifndef __unix__
    const auto pidvid = handle.addr;
    const auto comAddrPos = pidvid.find("+");
    std::string comName;
    if(comAddrPos == std::string::npos)
    {
        return new ConnectionSTREAM_UNITE(nullptr, handle.index);
    }
    else
    {
        comName = pidvid.substr(comAddrPos + 1);
        return new ConnectionSTREAM_UNITE(nullptr, handle.index, -1, -1, comName.c_str());
    }
#else
    const auto pidvid = handle.addr;
    const auto splitPos = pidvid.find(":");
    const auto pid = std::stoi(pidvid.substr(0, splitPos));
    const auto comAddrPos = pidvid.find("+");
    if(comAddrPos == std::string::npos)
    {
        const auto vid = std::stoi(pidvid.substr(splitPos+1));
        return new ConnectionSTREAM_UNITE(ctx, handle.index, vid, pid, nullptr);
    }
    else
    {
        const auto vid = std::stoi(pidvid.substr(splitPos+1, comAddrPos));
        const auto comName = pidvid.substr(comAddrPos+1);
        return new ConnectionSTREAM_UNITE(ctx, handle.index, vid, pid, comName.c_str());
    }

#endif
}
