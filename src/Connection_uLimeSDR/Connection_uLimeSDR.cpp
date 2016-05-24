/**
@file Connection_uLimeSDR.cpp
@author Lime Microsystems
@brief Implementation of STREAM board connection.
*/

#include "Connection_uLimeSDR.h"
#include "ErrorReporting.h"
#include <cstring>
#include <iostream>

#include <thread>
#include <chrono>

using namespace std;

using namespace lime;

#define USB_TIMEOUT 1000

Connection_uLimeSDR::Connection_uLimeSDR(void *arg)
{
    mStreamWrEndPtAddr = 0x03;
    mStreamRdEndPtAddr = 0x83;
    isConnected = false;
#ifndef __unix__
	mFTHandle = NULL;
	firstShot.store(true);
#else
    dev_handle = 0;
    devs = 0;
	mUsbCounter = 0;
    ctx = (libusb_context *)arg;
#endif
}

/**	@brief Initializes port type and object necessary to communicate to usb device.
*/
Connection_uLimeSDR::Connection_uLimeSDR(void *arg, const unsigned index, const int vid, const int pid)
{
    mStreamWrEndPtAddr = 0x03;
    mStreamRdEndPtAddr = 0x83;
    isConnected = false;
#ifndef __unix__
    mFTHandle = NULL;
	firstShot.store(true);
#else
    dev_handle = 0;
    devs = 0;
	mUsbCounter = 0;
    ctx = (libusb_context *)arg;
#endif
    if(this->Open(index, vid, pid) != 0)
        std::cerr << GetLastErrorMessage() << std::endl;
}

/**	@brief Closes connection to chip and deallocates used memory.
*/
Connection_uLimeSDR::~Connection_uLimeSDR()
{
    mStreamService.reset();
    Close();
}
#ifdef __unix__
int Connection_uLimeSDR::FT_FlushPipe(unsigned char ep)
{
    int actual = 0;
    unsigned char wbuffer[20]={0};
    
    mUsbCounter++;
    wbuffer[0] = (mUsbCounter)&0xFF;
    wbuffer[1] = (mUsbCounter>>8)&0xFF;
    wbuffer[2] = (mUsbCounter>>16)&0xFF;
    wbuffer[3] = (mUsbCounter>>24)&0xFF;
    wbuffer[4] = ep;
    wbuffer[5] = 0x03;
    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, USB_TIMEOUT);
    if (actual != 20)
        return -1;
    return 0;
}

int Connection_uLimeSDR::FT_SetStreamPipe(unsigned char ep, size_t size)
{
    
    int actual = 0;
    unsigned char wbuffer[20]={0};
    
    mUsbCounter++;
    wbuffer[0] = (mUsbCounter)&0xFF;
    wbuffer[1] = (mUsbCounter>>8)&0xFF;
    wbuffer[2] = (mUsbCounter>>16)&0xFF;
    wbuffer[3] = (mUsbCounter>>24)&0xFF;
    wbuffer[4] = ep;
    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, USB_TIMEOUT);
    if (actual != 20)
        return -1;
    
    mUsbCounter++;
    wbuffer[0] = (mUsbCounter)&0xFF;
    wbuffer[1] = (mUsbCounter>>8)&0xFF;
    wbuffer[2] = (mUsbCounter>>16)&0xFF;
    wbuffer[3] = (mUsbCounter>>24)&0xFF;
    wbuffer[5] = 0x02;
    wbuffer[8] = (size)&0xFF;
    wbuffer[9] = (size>>8)&0xFF;
    wbuffer[10] = (size>>16)&0xFF;
    wbuffer[11] = (size>>24)&0xFF;
    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, USB_TIMEOUT);
    if (actual != 20)
        return -1;
    return 0;
}
#endif

/**	@brief Tries to open connected USB device and find communication endpoints.
@return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int Connection_uLimeSDR::Open(const unsigned index, const int vid, const int pid)
{
#ifndef __unix__
	DWORD devCount;
	FT_STATUS ftStatus = FT_OK;
	DWORD dwNumDevices = 0;
	//
	// Open a device
	//
	ftStatus = FT_Create(0, FT_OPEN_BY_INDEX, &mFTHandle);
	if (FT_FAILED(ftStatus))
	{
		ReportError(ENODEV, "Failed to list USB Devices");
		return -1;
	}

	ftStatus = FT_FlushPipe(mFTHandle, mStreamRdEndPtAddr);
	ftStatus = FT_FlushPipe(mFTHandle, 0x82);
	ftStatus = FT_SetStreamPipe(mFTHandle, FALSE, FALSE, 0x02, 64);
	ftStatus = FT_SetStreamPipe(mFTHandle, FALSE, FALSE, 0x82, 64);
	//checkfor endpoints
	isConnected = true;
	return 0;
#else
    dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);

    if(dev_handle == nullptr)
        return ReportError("libusb_open failed");
    libusb_reset_device(dev_handle);
    if(libusb_kernel_driver_active(dev_handle, 1) == 1)   //find out if kernel driver is attached
    {
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 1) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    }
    int r = libusb_claim_interface(dev_handle, 1); //claim interface 0 (the first) of device
    if(r < 0)
    {
        printf("Cannot Claim Interface\n");
        return ReportError("Cannot claim interface - %s", libusb_strerror(libusb_error(r)));
    }
    r = libusb_claim_interface(dev_handle, 1); //claim interface 0 (the first) of device
    if(r < 0)
    {
        printf("Cannot Claim Interface\n");
        return ReportError("Cannot claim interface - %s", libusb_strerror(libusb_error(r)));
    }
    printf("Claimed Interface\n");
    FT_SetStreamPipe(mStreamRdEndPtAddr,65536);
    FT_SetStreamPipe(mStreamWrEndPtAddr,65536);    
    FT_SetStreamPipe(0x82,64);
    FT_SetStreamPipe(0x02,64);
    isConnected = true;
    return 0;
#endif
}

/**	@brief Closes communication to device.
*/
void Connection_uLimeSDR::Close()
{
#ifndef __unix__
	FT_Close(mFTHandle);
#else
    if(dev_handle != 0)
    {
        FT_FlushPipe(mStreamRdEndPtAddr);
        FT_FlushPipe(0x82);
        libusb_release_interface(dev_handle, 1);
        libusb_close(dev_handle);
        dev_handle = 0;
    }
#endif
    isConnected = false;
}

/**	@brief Returns connection status
@return 1-connection open, 0-connection closed.
*/
bool Connection_uLimeSDR::IsOpen()
{
    return isConnected;
}

/**	@brief Sends given data buffer to chip through USB port.
@param buffer data buffer, must not be longer than 64 bytes.
@param length given buffer size.
@param timeout_ms timeout limit for operation in milliseconds
@return number of bytes sent.
*/
int Connection_uLimeSDR::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = 0;
    if(IsOpen() == false)
        return 0;
    
    unsigned char* wbuffer = new unsigned char[length];
    memcpy(wbuffer, buffer, length);
#ifndef __unix__

	//
	// Write to channel 1 ep 0x02
	//
	ULONG ulBytesWrite = 0;
	FT_STATUS ftStatus = FT_OK;
	OVERLAPPED	vOverlapped = { 0 };
	memset(&vOverlapped, 0, sizeof(OVERLAPPED));
	ftStatus = FT_InitializeOverlapped(mFTHandle, &vOverlapped);
	if (FT_FAILED(ftStatus))
	{
		ReportError(ENODEV, "FT_InitializeOverlapped failed! status=0x%x", ftStatus);
		return -1;
	}
	ftStatus = FT_WritePipe(mFTHandle, 0x02, (unsigned char*)buffer, length, &ulBytesWrite, &vOverlapped);
	if (ftStatus != FT_IO_PENDING)
	{
		FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
		return -1;
	}

	DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, 2000);
	if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_TIMEOUT)
	{
		ftStatus = FT_GetOverlappedResult(mFTHandle, &vOverlapped, &ulBytesWrite, FALSE);
		if (FT_FAILED(ftStatus))
		{
			ulBytesWrite = -1;
		}
	}
	else
	{
		ulBytesWrite = -1;
	}
	FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
	return ulBytesWrite;
#else
    int actual = 0;
    libusb_bulk_transfer(dev_handle, 0x02, wbuffer, length, &actual, USB_TIMEOUT);
    len = actual;
#endif
    delete wbuffer;
    return len;
}

/**	@brief Reads data coming from the chip through USB port.
@param buffer pointer to array where received data will be copied, array must be
big enough to fit received data.
@param length number of bytes to read from chip.
@param timeout_ms timeout limit for operation in milliseconds
@return number of bytes received.
*/
int Connection_uLimeSDR::Read(unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = length;
    if(IsOpen() == false)
        return 0;
#ifndef __unix__
	//
	// Read from channel 1 ep 0x82
	// FT_ReadPipe is a blocking/synchronous function.
	// It will not return until it has received all data requested
	//
	ULONG ulBytesRead = 0;
	FT_STATUS ftStatus = FT_OK;
	OVERLAPPED	vOverlapped = { 0 };
	memset(&vOverlapped, 0, sizeof(OVERLAPPED));
	ftStatus = FT_InitializeOverlapped(mFTHandle, &vOverlapped);
	if (FT_FAILED(ftStatus))
	{
		ReportError(ENODEV, "FT_InitializeOverlapped failed! status=0x%x", ftStatus);
		return -1;
	}
	ftStatus = FT_ReadPipe(mFTHandle, 0x82, buffer, length, &ulBytesRead, &vOverlapped);
	if (ftStatus != FT_IO_PENDING)
	{
		FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
		return -1;;
	}

	DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, 2000);
	if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_TIMEOUT)
	{
		ftStatus = FT_GetOverlappedResult(mFTHandle, &vOverlapped, &ulBytesRead, FALSE);
		if (FT_FAILED(ftStatus))
		{
			ulBytesRead = -1;
		}
	}
	else
	{
		ulBytesRead = -1;
	}
	FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
	return ulBytesRead;
#else
    int actual = 0;
    libusb_bulk_transfer(dev_handle, 0x82, buffer, len, &actual, USB_TIMEOUT);
    len = actual;
#endif
    return len;
}

#ifdef __unix__
/**	@brief Function for handling libusb callbacks
*/
static void callback_libusbtransfer(libusb_transfer *trans)
{
    Connection_uLimeSDR::USBTransferContext *context = reinterpret_cast<Connection_uLimeSDR::USBTransferContext*>(trans->user_data);
    switch(trans->status)
    {
    case LIBUSB_TRANSFER_CANCELLED:
        //printf("Transfer %i canceled\n", context->id);
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        //context->used = false;
        //context->reset();
        break;
    case LIBUSB_TRANSFER_COMPLETED:
        //if(trans->actual_length == context->bytesExpected)
    {
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
    }
    break;
    case LIBUSB_TRANSFER_ERROR:
        printf("TRANSFER ERRRO\n");
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        //context->used = false;
        break;
    case LIBUSB_TRANSFER_TIMED_OUT:
        //printf("transfer timed out %i\n", context->id);
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        //context->used = false;

        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        printf("transfer overflow\n");

        break;
    case LIBUSB_TRANSFER_STALL:
        printf("transfer stalled\n");
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        printf("transfer no device\n");

        break;
    }
    context->transferLock.unlock();
    context->cv.notify_one();
}
#endif

/**
@brief Starts asynchronous data reading from board
@param *buffer buffer where to store received data
@param length number of bytes to read
@return handle of transfer context
*/
int Connection_uLimeSDR::BeginDataReading(char *buffer, long length)
{
    int i = 0;
    bool contextFound = false;
    //find not used context
    for(i = 0; i<USB_MAX_CONTEXTS; i++)
    {
        if(!contexts[i].used)
        {
            contextFound = true;
            break;
        }
    }
    if(!contextFound)
    {
        printf("No contexts left for reading data\n");
        return -1;
    }
    contexts[i].used = true;
#ifndef __unix__
 
#else
    unsigned int Timeout = 500;
    libusb_transfer *tr = contexts[i].transfer;
    libusb_fill_bulk_transfer(tr, dev_handle, mStreamRdEndPtAddr, (unsigned char*)buffer, length, callback_libusbtransfer, &contexts[i], Timeout);
    contexts[i].done = false;
    contexts[i].bytesXfered = 0;
    contexts[i].bytesExpected = length;
    int status = libusb_submit_transfer(tr);
    if(status != 0)
    {
        printf("ERROR BEGIN DATA READING %s\n", libusb_error_name(status));
        contexts[i].used = false;
        return -1;
    }
    else
        contexts[i].transferLock.lock();
#endif
    return i;
}

/**
@brief Waits for asynchronous data reception
@param contextHandle handle of which context data to wait
@param timeout_ms number of miliseconds to wait
@return 1-data received, 0-data not received
*/
int Connection_uLimeSDR::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
        int status = 0;
#ifndef __unix__
		return 1;
#else
        auto t1 = chrono::high_resolution_clock::now();
        auto t2 = chrono::high_resolution_clock::now();

        std::unique_lock<std::mutex> lck(contexts[contextHandle].transferLock);
        while(contexts[contextHandle].done.load() == false && std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms)
        {
            //blocking not to waste CPU
            contexts[contextHandle].cv.wait(lck);
            t2 = chrono::high_resolution_clock::now();
        }
        return contexts[contextHandle].done.load() == true;
#endif
    }
    else
        return 0;
}

/**
@brief Finishes asynchronous data reading from board
@param buffer array where to store received data
@param length number of bytes to read, function changes this value to number of bytes actually received
@param contextHandle handle of which context to finish
@return false failure, true number of bytes received
*/
int Connection_uLimeSDR::FinishDataReading(char *buffer, long &length, int contextHandle)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
#ifndef __unix__
		static UCHAR ppBuffers[USB_MAX_CONTEXTS][65536] = { 0 };
		contexts[contextHandle].used = false;
		FT_STATUS ftStatus;
		ULONG i = 0;

		FT_HANDLE ftHandle = mFTHandle;

		ULONG ulActualBytesTransferred = 0;
		const ULONG ulActualBytesToTransfer = length;
		ULONG len = 0;
		const ULONG m_ulQueueSize = USB_MAX_CONTEXTS;

		if (firstShot.load() == true)
		{
			firstShot.store(false);
			ftStatus = FT_SetStreamPipe(ftHandle, FALSE, FALSE, mStreamWrEndPtAddr, length);
			ftStatus = FT_SetStreamPipe(ftHandle, FALSE, FALSE, mStreamRdEndPtAddr, length);
			if (FT_FAILED(ftStatus))
			{
				return -1;
			}

			for (i = 0; i < m_ulQueueSize; i++)
			{
				memset(&contexts[i].inOvLap, 0, sizeof(OVERLAPPED));
				ftStatus = FT_InitializeOverlapped(mFTHandle, &contexts[i].inOvLap);
				if (FT_FAILED(ftStatus))
				{
					ReportError(ENODEV, "FT_InitializeOverlapped failed! status=0x%x", ftStatus);
					return -1;
				}
				ftStatus = FT_ReadPipe(ftHandle, mStreamRdEndPtAddr, &ppBuffers[i][0], ulActualBytesToTransfer, &ulActualBytesTransferred, &contexts[i].inOvLap);
				if (ftStatus != FT_IO_PENDING)
				{
					return -1;
				}
			}
	}

		ulActualBytesTransferred = 0;
		contexts[contextHandle].inOvLap.InternalHigh = 0;
		while (1)
		{
			DWORD dwRet = WaitForSingleObject(contexts[contextHandle].inOvLap.hEvent, 1000);
			if (dwRet == WAIT_OBJECT_0)
			{
				ftStatus = FT_GetOverlappedResult(ftHandle, &contexts[contextHandle].inOvLap, &ulActualBytesTransferred, FALSE);
				len = ulActualBytesTransferred;
				memcpy(buffer, &ppBuffers[contextHandle][0], len);
				if (ftStatus == FT_DEVICE_NOT_CONNECTED)
				{
					return -1;
				}
				else if (FT_FAILED(ftStatus))
				{
					return -1;
				}

				break;
			}
			else if (dwRet == WAIT_TIMEOUT)
			{
				ftStatus = FT_GetOverlappedResult(ftHandle, &contexts[contextHandle].inOvLap, &ulActualBytesTransferred, FALSE);
				len = ulActualBytesTransferred;
				memcpy(buffer, &ppBuffers[contextHandle][0], len);
				if (FT_FAILED(ftStatus))
				{
					return -1;
				}

				return -1;
			}
			else
			{
				return -1;
			}
		}

		/*ftStatus = FT_ReleaseOverlapped(ftHandle, &contexts[contextHandle].inOvLap);
		if (FT_FAILED(ftStatus))
		{
		return -1;
		}

		memset(&contexts[contextHandle].inOvLap, 0, sizeof(OVERLAPPED));
		ftStatus = FT_InitializeOverlapped(mFTHandle, &contexts[contextHandle].inOvLap);
		if (FT_FAILED(ftStatus))
		{
		return -1;
		}*/

		ftStatus = FT_ReadPipe(mFTHandle, mStreamRdEndPtAddr, &ppBuffers[contextHandle][0], length, &ulActualBytesTransferred, &contexts[contextHandle].inOvLap);
		if (ftStatus != FT_IO_PENDING)
		{
			return -1;
		}
		return len;
#else
        length = contexts[contextHandle].bytesXfered;
        contexts[contextHandle].used = false;
        contexts[contextHandle].reset();
        return length;
#endif
    }
    else    
        return 0;
}

int Connection_uLimeSDR::ReadDataBlocking(char *buffer, long &length, int timeout_ms)
{
#ifndef __unix__
	return 0;
#else
    return 0;
#endif
}


/**
@brief Aborts reading operations
*/
void Connection_uLimeSDR::AbortReading()
{
#ifndef __unix__
	FT_AbortPipe(mFTHandle, mStreamRdEndPtAddr);
	FT_FlushPipe(mFTHandle, mStreamRdEndPtAddr);
	FT_ClearStreamPipe(mFTHandle, FALSE, FALSE, mStreamRdEndPtAddr);
	firstShot.store(true);
	for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
	{
		contexts[i].used = false;
		FT_ReleaseOverlapped(mFTHandle, &contexts[i].inOvLap);
	}
#else
    
    for(int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contexts[i].used)
            libusb_cancel_transfer(contexts[i].transfer);
    }
    FT_FlushPipe(mStreamRdEndPtAddr);
#endif
}

/**
@brief Starts asynchronous data Sending to board
@param *buffer buffer to send
@param length number of bytes to send
@return handle of transfer context
*/
int Connection_uLimeSDR::BeginDataSending(const char *buffer, long length)
{
    int i = 0;
    //find not used context
    bool contextFound = false;
    for(i = 0; i<USB_MAX_CONTEXTS; i++)
    {
        if(!contextsToSend[i].used)
        {
            contextFound = true;
            break;
        }
    }
    if(!contextFound)
        return -1;
    contextsToSend[i].used = true;
#ifndef __unix__
	FT_STATUS ftStatus = FT_OK;
	ULONG ulActualBytesSend = 0;
	memset(&contextsToSend[i].inOvLap, 0, sizeof(OVERLAPPED));
	ftStatus = FT_InitializeOverlapped(mFTHandle, &contextsToSend[i].inOvLap);
	if (FT_FAILED(ftStatus))
	{
		ReportError(ENODEV, "FT_InitializeOverlapped failed! status=0x%x", ftStatus);
		return -1;
	}
	ftStatus = FT_WritePipe(mFTHandle, mStreamWrEndPtAddr, (unsigned char*)buffer, length, &ulActualBytesSend, &contextsToSend[i].inOvLap);
	if (ftStatus != FT_IO_PENDING)
	{
		return -1;
	}
#else
    int actual;
   
    unsigned int Timeout = 500;
    libusb_transfer *tr = contextsToSend[i].transfer;
    libusb_fill_bulk_transfer(tr, dev_handle, mStreamWrEndPtAddr, (unsigned char*)buffer, length, callback_libusbtransfer, &contextsToSend[i], Timeout);
    contextsToSend[i].done = false;
    contextsToSend[i].bytesXfered = 0;
    contextsToSend[i].bytesExpected = length;
    int status = libusb_submit_transfer(tr);
    if(status != 0)
    {
        printf("ERROR BEGIN DATA SENDING %s\n", libusb_error_name(status));
        contextsToSend[i].used = false;
        return -1;
    }
    else
        contextsToSend[i].transferLock.lock();
#endif
    return i;
}

/**
@brief Waits for asynchronous data sending
@param contextHandle handle of which context data to wait
@param timeout_ms number of miliseconds to wait
@return 1-data received, 0-data not received
*/
int Connection_uLimeSDR::WaitForSending(int contextHandle, unsigned int timeout_ms)
{
    if(contextsToSend[contextHandle].used == true)
    {
#ifndef __unix__
		return 1;
#else
        auto t1 = chrono::high_resolution_clock::now();
        auto t2 = chrono::high_resolution_clock::now();
        std::unique_lock<std::mutex> lck(contextsToSend[contextHandle].transferLock);
        while(contextsToSend[contextHandle].done.load() == false && std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms)
        {
            //blocking not to waste CPU
            contextsToSend[contextHandle].cv.wait(lck);
            t2 = chrono::high_resolution_clock::now();
        }
        return contextsToSend[contextHandle].done == true;
#endif
    }
    else
        return 0;
}

/**
@brief Finishes asynchronous data sending to board
@param buffer array where to store received data
@param length number of bytes to read, function changes this value to number of bytes acctually received
@param contextHandle handle of which context to finish
@return false failure, true number of bytes sent
*/
int Connection_uLimeSDR::FinishDataSending(const char *buffer, long &length, int contextHandle)
{
    if(contextsToSend[contextHandle].used == true)
    {
#ifndef __unix__
		FT_STATUS ftStatus = FT_OK;
		ULONG ulActualBytesTransferred = 0;
		contextsToSend[contextHandle].inOvLap.InternalHigh = 0;
		while (1)
		{
			DWORD dwRet = WaitForSingleObject(contextsToSend[contextHandle].inOvLap.hEvent, 1000);
			if (dwRet == WAIT_OBJECT_0)
			{
				ftStatus = FT_GetOverlappedResult(mFTHandle, &contextsToSend[contextHandle].inOvLap, &ulActualBytesTransferred, FALSE);
				if (FT_FAILED(ftStatus))
				{
					return -1;
				}

				break;
			}
			else if (dwRet == WAIT_TIMEOUT)
			{

				ftStatus = FT_GetOverlappedResult(mFTHandle, &contextsToSend[contextHandle].inOvLap, &ulActualBytesTransferred, FALSE);
				if (FT_FAILED(ftStatus))
				{
					return -1;
				}
				break;
			}
			else
			{
				return -1;
			}
		}
		ftStatus = FT_ReleaseOverlapped(mFTHandle, &contextsToSend[contextHandle].inOvLap);
		if (FT_FAILED(ftStatus))
		{
			return -1;
		}
		contextsToSend[contextHandle].used = false;
		return length;
#else
        length = contextsToSend[contextHandle].bytesXfered;
        contextsToSend[contextHandle].used = false;
        contextsToSend[contextHandle].reset();
        return length;
#endif
    }
    else
        return 0;
}

/**
@brief Aborts sending operations
*/
void Connection_uLimeSDR::AbortSending()
{
#ifndef __unix__
	FT_AbortPipe(mFTHandle, mStreamWrEndPtAddr);
	FT_FlushPipe(mFTHandle, mStreamWrEndPtAddr);
	for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
	{
		if (contextsToSend[i].used == true)
		{
			FT_ReleaseOverlapped(mFTHandle, &contextsToSend[i].inOvLap);
			contextsToSend[i].used = false;
		}
	}
#else
    for(int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contextsToSend[i].used)
            libusb_cancel_transfer(contextsToSend[i].transfer);
    }
#endif
}

/** @brief Configures Stream board FPGA clocks to Limelight interface
@param tx Rx/Tx selection
@param InterfaceClk_Hz Limelight interface frequency
@param phaseShift_deg IQ phase shift in degrees
@return 0-success, other-failure
*/
int Connection_uLimeSDR::ConfigureFPGA_PLL(unsigned int pllIndex, const double interfaceClk_Hz, const double phaseShift_deg)
{
    eLMS_DEV boardType = GetDeviceInfo().deviceName == GetDeviceName(LMS_DEV_QSPARK) ? LMS_DEV_QSPARK : LMS_DEV_UNKNOWN;
    const uint16_t busyAddr = 0x0021;
    if(IsOpen() == false)
        return ReportError(ENODEV, "Connection_uLimeSDR: configure FPGA PLL, device not connected");

    uint16_t drct_clk_ctrl_0005 = 0;
    ReadRegister(0x0005, drct_clk_ctrl_0005);

    if(interfaceClk_Hz < 5e6)
    {
        //enable direct clocking
        WriteRegister(0x0005, drct_clk_ctrl_0005 | (1 << pllIndex));
        uint16_t drct_clk_ctrl_0006;
        ReadRegister(0x0006, drct_clk_ctrl_0006);
        drct_clk_ctrl_0006 = drct_clk_ctrl_0006 & ~0x3FF;
        const int cnt_ind = 1 << 5;
        const int clk_ind = pllIndex;
        drct_clk_ctrl_0006 = drct_clk_ctrl_0006 | cnt_ind | clk_ind;
        WriteRegister(0x0006, drct_clk_ctrl_0006);
        const uint16_t phase_reg_sel_addr = 0x0004;
        float inputClock_Hz = interfaceClk_Hz;
        const float oversampleClock_Hz = 100e6;
        const int registerChainSize = 128;
        const float phaseShift_deg = 90;
        const float oversampleClock_ns = 1e9 / oversampleClock_Hz;
        const float phaseStep_deg = 360 * oversampleClock_ns*(1e-9) / (1 / inputClock_Hz);
        uint16_t phase_reg_select = (phaseShift_deg / phaseStep_deg) + 0.5;
        const float actualPhaseShift_deg = 360 * inputClock_Hz / (1 / (phase_reg_select * oversampleClock_ns*1e-9));
#ifndef NDEBUG
        printf("reg value : %i\n", phase_reg_select);
        printf("input clock: %f\n", inputClock_Hz);
        printf("phase : %.2f/%.2f\n", phaseShift_deg, actualPhaseShift_deg);
#endif
        if(WriteRegister(phase_reg_sel_addr, phase_reg_select) != 0)
            return ReportError(EIO, "Connection_uLimeSDR: configure FPGA PLL, failed to write registers");
        const uint16_t LOAD_PH_REG = 1 << 10;
        WriteRegister(0x0006, drct_clk_ctrl_0006 | LOAD_PH_REG);
        WriteRegister(0x0006, drct_clk_ctrl_0006);
        return 0;
    }

    //if interface frequency >= 5MHz, configure PLLs
    WriteRegister(0x0005, drct_clk_ctrl_0005 & ~(1 << pllIndex));

    //select FPGA index
    pllIndex = pllIndex & 0x1F;
    uint16_t reg23val = 0;
    if(ReadRegister(0x0003, reg23val) != 0)
        return ReportError(ENODEV, "Connection_uLimeSDR: configure FPGA PLL, failed to read register");

    const uint16_t PLLCFG_START = 0x1;
    const uint16_t PHCFG_START = 0x2;
    const uint16_t PLLRST_START = 0x4;
    const uint16_t PHCFG_UPDN = 1 << 13;
    reg23val &= 0x1F << 3; //clear PLL index
    reg23val &= ~PLLCFG_START; //clear PLLCFG_START
    reg23val &= ~PHCFG_START; //clear PHCFG
    reg23val &= ~PLLRST_START; //clear PLL reset
    reg23val &= ~PHCFG_UPDN; //clear PHCFG_UpDn
    reg23val |= pllIndex << 3;

    uint16_t statusReg;
    bool done = false;
    uint8_t errorCode = 0;
    vector<uint32_t> addrs;
    vector<uint32_t> values;
    addrs.push_back(0x0023);
    values.push_back(reg23val); //PLL_IND
    addrs.push_back(0x0023);
    values.push_back(reg23val | PLLRST_START); //PLLRST_START
    WriteRegisters(addrs.data(), values.data(), values.size());

    if(boardType == LMS_DEV_QSPARK) do //wait for reset to activate
    {
        ReadRegister(busyAddr, statusReg);
        done = statusReg & 0x1;
        errorCode = (statusReg >> 7) & 0xFF;
    } while(!done && errorCode == 0);
    if(errorCode != 0)
        return ReportError(EBUSY, "Connection_uLimeSDR: error resetting PLL");

    addrs.clear();
    values.clear();
    addrs.push_back(0x0023);
    values.push_back(reg23val & ~PLLRST_START);

    //configure FPGA PLLs
    const float vcoLimits_MHz[2] = { 600, 1300 };
    int M, C;
    const short bufSize = 64;

    float fOut_MHz = interfaceClk_Hz / 1e6;
    float coef = 0.8*vcoLimits_MHz[1] / fOut_MHz;
    M = C = (int)coef;
    int chigh = (((int)coef) / 2) + ((int)(coef) % 2);
    int clow = ((int)coef) / 2;

    addrs.clear();
    values.clear();
    if(interfaceClk_Hz*M / 1e6 > vcoLimits_MHz[0] && interfaceClk_Hz*M / 1e6 < vcoLimits_MHz[1])
    {
        //bypass N
        addrs.push_back(0x0026);
        values.push_back(0x0001 | (M % 2 ? 0x8 : 0));

        addrs.push_back(0x0027);
        values.push_back(0x5550 | (C % 2 ? 0xA : 0)); //bypass c7-c1
        addrs.push_back(0x0028);
        values.push_back(0x5555); //bypass c15-c8

        addrs.push_back(0x002A);
        values.push_back(0x0101); //N_high_cnt, N_low_cnt
        addrs.push_back(0x002B);
        values.push_back(chigh << 8 | clow); //M_high_cnt, M_low_cnt

        for(int i = 0; i <= 1; ++i)
        {
            addrs.push_back(0x002E + i);
            values.push_back(chigh << 8 | clow); // ci_high_cnt, ci_low_cnt
        }

        float Fstep_us = 1 / (8 * fOut_MHz*C);
        float Fstep_deg = (360 * Fstep_us) / (1 / fOut_MHz);
        short nSteps = phaseShift_deg / Fstep_deg;

        addrs.push_back(0x0024);
        values.push_back(nSteps);

        addrs.push_back(0x0023);
        int cnt_ind = 0x3 & 0x1F;
        reg23val = reg23val | PHCFG_UPDN | (cnt_ind << 8);
        values.push_back(reg23val); //PHCFG_UpDn, CNT_IND

        addrs.push_back(0x0023);
        values.push_back(reg23val | PLLCFG_START); //PLLCFG_START
        if(WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
            ReportError(EIO, "Connection_uLimeSDR: configure FPGA PLL, failed to write registers");
        if(boardType == LMS_DEV_QSPARK) do //wait for config to activate
        {
            ReadRegister(busyAddr, statusReg);
            done = statusReg & 0x1;
            errorCode = (statusReg >> 7) & 0xFF;
        } while(!done && errorCode == 0);
        if(errorCode != 0)
            return ReportError(EBUSY, "Connection_uLimeSDR: error configuring PLLCFG");

        addrs.clear();
        values.clear();
        addrs.push_back(0x0023);
        values.push_back(reg23val & ~PLLCFG_START); //PLLCFG_START
        addrs.push_back(0x0023);
        values.push_back(reg23val | PHCFG_START); //PHCFG_START
        if(WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
            ReportError(EIO, "Connection_uLimeSDR: configure FPGA PLL, failed to write registers");
        if(boardType == LMS_DEV_QSPARK) do
        {
            ReadRegister(busyAddr, statusReg);
            done = statusReg & 0x1;
            errorCode = (statusReg >> 7) & 0xFF;
        } while(!done && errorCode == 0);
        if(errorCode != 0)
            return ReportError(EBUSY, "Connection_uLimeSDR: error configuring PHCFG");
        addrs.clear();
        values.clear();
        addrs.push_back(0x0023);
        values.push_back(reg23val & ~PHCFG_START); //PHCFG_START
        if(WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
            ReportError(EIO, "Connection_uLimeSDR: configure FPGA PLL, failed to write registers");
        return 0;
    }
    return ReportError(ERANGE, "Connection_uLimeSDR: configure FPGA PLL, desired frequency out of range");
}