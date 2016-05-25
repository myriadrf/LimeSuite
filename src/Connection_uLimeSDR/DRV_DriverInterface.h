/*
 * FT600 Data Loopback Demo App
 *
 * Copyright (C) 2016 FTDI Chip
 *
 */

#pragma once
#include "FTD3XXLibrary/FTD3XX.h"



typedef enum _EOPEN_BY
{
    EOPEN_BY_GUID,
    EOPEN_BY_DESC,
	EOPEN_BY_SERIAL,
	EOPEN_BY_INDEX,

} EOPEN_BY;




class CDriverInterface
{

public:
    CDriverInterface();
    ~CDriverInterface();


    BOOL Initialize(
        EOPEN_BY a_eTypeOpenBy, 
        PVOID a_pvOpenBy
        );

    VOID Cleanup();

    BOOL GetEP(
        PUCHAR a_pucNumReadEP, 
        PUCHAR a_pucNumWriteEP, 
        PUCHAR m_pucReadEP, 
        PUCHAR m_pucWriteEP
        );

    BOOL GetVIDPID(
        PUSHORT a_puwVID, 
        PUSHORT a_puwPID
    );

    BOOL GetFirmwareVersion(
        PULONG pulFirmwareVersion
    );

    BOOL FlushPipe(
        _In_  UCHAR a_ucPipeID
        );

    BOOL AbortPipe(
        _In_  UCHAR a_ucPipeID
        );

    FT_STATUS WritePipe(
        UCHAR a_ucPipeID,
        PUCHAR a_pucBuffer,
        ULONG a_ulBufferLength,
        PULONG a_pulLengthTransferred,
        LPOVERLAPPED a_pOverlapped
        );

    FT_STATUS ReadPipe(
        UCHAR a_ucPipeID,
        PUCHAR a_pucBuffer,
        ULONG a_ulBufferLength,
        PULONG a_pulLengthTransferred,
        LPOVERLAPPED a_pOverlapped
        );

    FT_STATUS GetOverlappedResult(
        LPOVERLAPPED a_pOverlapped,
        PULONG a_pulBytesTransferred,
        BOOL a_bWait
        );

    FT_STATUS InitializeOverlapped(
        LPOVERLAPPED a_pOverlapped
        );

    VOID ReleaseOverlapped(
        LPOVERLAPPED a_pOverlapped
        );

    BOOL StartStreamPipe(
        _In_  UCHAR a_ucPipeID,
        _In_  ULONG a_ulStreamSize
        );

    BOOL StopStreamPipe(
        _In_  UCHAR a_ucPipeID
        );

	DWORD GetNumberOfDevicesConnected();
	DWORD GetDevicesInfoList(FT_DEVICE_LIST_INFO_NODE **pptDevicesInfo);
	VOID ReleaseDevicesInfoList(FT_DEVICE_LIST_INFO_NODE *ptDevicesInfo);

	DWORD GetDeviceIndex(
		FT_DEVICE_LIST_INFO_NODE* ptDevicesInfo,
		DWORD dwNumDevices,
		EOPEN_BY a_eTypeOpenBy,
		PVOID a_pvOpenBy
		);

	BOOL IsDevicePath(CONST CHAR* pucDevicePath);

	BOOL GetDriverVersion(LPDWORD pDriverVersion);

	BOOL GetLibraryVersion(LPDWORD pLibraryVersion);



private:
    FT_HANDLE   m_FTHandle;

};
