/**
@file   ConnectionUSB.cpp
@author Lime Microsystems (www.limemicro.com)
@brief  Data writing and reading through USB port
*/

#include "ConnectionUSB.h"
#include <string.h>

#ifdef __unix__
    #include <thread>
    #include <chrono>
#endif

#define USB_TIMEOUT 1000

#define HW_LDIGIRED L"DigiRed"
#define HW_LDIGIGREEN L"DigiGreen"
#define HW_LSTREAMER L"Stream"

#define HW_DIGIRED "DigiRed"
#define HW_DIGIGREEN "DigiGreen"
#define HW_STREAMER "Stream"

#define CTR_W_REQCODE 0xC1
#define CTR_W_VALUE 0x0000
#define CTR_W_INDEX 0x0000

#define CTR_R_REQCODE 0xC0
#define CTR_R_VALUE 0x0000
#define CTR_R_INDEX 0x0000

/**	@brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionUSB::ConnectionUSB()
{
    m_hardwareName = "";
    isConnected = false;
    m_connectionType = USB_PORT;
#ifndef __unix__
    USBDevicePrimary = new CCyUSBDevice(NULL);
    OutCtrEndPt = NULL;
    InCtrEndPt = NULL;
	InCtrlEndPt3 = NULL;
	OutCtrlEndPt3 = NULL;
#else
    dev_handle = 0;
    devs = 0;
    int r = libusb_init(&ctx); //initialize the library for the session we just declared
    if(r < 0)
        printf("Init Error %i\n", r); //there was an error
    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
#endif
    currentDeviceIndex = -1;
}

/**	@brief Closes connection to chip and deallocates used memory.
*/
ConnectionUSB::~ConnectionUSB()
{
    Close();
#ifndef __unix__
    delete USBDevicePrimary;
#else
    libusb_exit(ctx);
#endif
}

/**	@brief Automatically open first available chip connected to usb port.
	@return 0-success, other-failure
*/
IConnection::DeviceStatus ConnectionUSB::Open()
{
    currentDeviceIndex = -1;
    if(m_deviceNames.size() == 0)
        RefreshDeviceList();

#ifndef __unix__
    for(int i=0; i<USBDevicePrimary->DeviceCount(); ++i)
        if( Open(i) == SUCCESS)
        {
            currentDeviceIndex = i;
            return SUCCESS;
        }
#else
    if(Open(0) == SUCCESS)
        return SUCCESS;
#endif
    return FAILURE;
}

/**	@brief Tries to open connected USB device and find communication endpoints.
	@return Returns 1-Success, 0-EndPoints not found or device didn't connect.
*/
IConnection::DeviceStatus ConnectionUSB::Open(unsigned index)
{
#ifndef __unix__
	wstring m_hardwareDesc = L"";
	if( index < USBDevicePrimary->DeviceCount())
	{
		if(USBDevicePrimary->Open(index))
		{
            m_hardwareDesc = USBDevicePrimary->Product;
            unsigned int pos;
            //determine connected board type
            pos = m_hardwareDesc.find(HW_LDIGIRED, 0);
            if( pos != wstring::npos )
                m_hardwareName = HW_DIGIRED;
            else if (m_hardwareDesc.find(HW_LSTREAMER, 0) != wstring::npos)
                m_hardwareName = HW_STREAMER;
            else
				m_hardwareName = HW_STREAMER;


			if (InCtrlEndPt3)
			{
				delete InCtrlEndPt3;
				InCtrlEndPt3 = NULL;
			}
			InCtrlEndPt3 = new CCyControlEndPoint(*USBDevicePrimary->ControlEndPt);

			if (OutCtrlEndPt3)
			{
				delete OutCtrlEndPt3;
				OutCtrlEndPt3 = NULL;
			}
			OutCtrlEndPt3 = new CCyControlEndPoint(*USBDevicePrimary->ControlEndPt);

			InCtrlEndPt3->ReqCode = CTR_R_REQCODE;
			InCtrlEndPt3->Value = CTR_R_VALUE;
			InCtrlEndPt3->Index = CTR_R_INDEX;

			OutCtrlEndPt3->ReqCode = CTR_W_REQCODE;
			OutCtrlEndPt3->Value = CTR_W_VALUE;
			OutCtrlEndPt3->Index = CTR_W_INDEX;

			for (int i=0; i<USBDevicePrimary->EndPointCount(); i++)
				if(USBDevicePrimary->EndPoints[i]->Address == 0x01)
				{
					OutEndPt = USBDevicePrimary->EndPoints[i];
					long len = OutEndPt->MaxPktSize * 64;
					OutEndPt->SetXferSize(len);
					break;
				}
			for (int i=0; i<USBDevicePrimary->EndPointCount(); i++)
				if(USBDevicePrimary->EndPoints[i]->Address == 0x81)
				{
					InEndPt = USBDevicePrimary->EndPoints[i];
					long len = InEndPt->MaxPktSize * 64;
					InEndPt->SetXferSize(len);
					break;
				}
			isConnected = true;
			return SUCCESS;
		} //successfully opened device
	} //if has devices
    return FAILURE;
#else
    if(index >= 0 && index < m_dev_pid_vid.size())
    {
		dev_handle = libusb_open_device_with_vid_pid(ctx, m_dev_pid_vid[index].second, m_dev_pid_vid[index].first);

		if(dev_handle == 0)
            return FAILURE;
        if(libusb_kernel_driver_active(dev_handle, 0) == 1)   //find out if kernel driver is attached
        {
            printf("Kernel Driver Active\n");
            if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
                printf("Kernel Driver Detached!\n");
        }
        int r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
        if(r < 0)
        {
            printf("Cannot Claim Interface\n");
            return CANNOT_CLAIM_INTERFACE;
        }
        printf("Claimed Interface\n");
        isConnected = true;
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
#endif
}

/**	@brief Closes communication to device.
*/
void ConnectionUSB::Close()
{
    #ifndef __unix__
	USBDevicePrimary->Close();
	InEndPt = NULL;
	OutEndPt = NULL;
	if (InCtrlEndPt3)
	{
		delete InCtrlEndPt3;
		InCtrlEndPt3 = NULL;
	}
	if (OutCtrlEndPt3)
	{
		delete OutCtrlEndPt3;
		OutCtrlEndPt3 = NULL;
	}
    #else
    if(dev_handle != 0)
    {
        libusb_release_interface(dev_handle, 0);
        libusb_close(dev_handle);
        dev_handle = 0;
    }
    #endif
    isConnected = false;
}

/**	@brief Returns connection status
	@return 1-connection open, 0-connection closed.
*/
bool ConnectionUSB::IsOpen()
{
    #ifndef __unix__
    return USBDevicePrimary->IsOpen() && isConnected;
    #else
    return isConnected;
    #endif
}

/**	@brief Sends given data buffer to chip through USB port.
	@param buffer data buffer, must not be longer than 64 bytes.
	@param length given buffer size.
    @param timeout_ms timeout limit for operation in milliseconds
	@return number of bytes sent.
*/
int ConnectionUSB::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
    long len = length;
    if(IsOpen())
    {
		unsigned char* wbuffer = new unsigned char[length];
		memcpy(wbuffer, buffer, length);
        if(m_hardwareName == HW_DIGIRED || m_hardwareName == HW_STREAMER)
        {
            #ifndef __unix__
            if(OutCtrlEndPt3)
                OutCtrlEndPt3->Write(wbuffer, len);
            else
                len = 0;
            #else
                len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR,CTR_W_REQCODE ,CTR_W_VALUE, CTR_W_INDEX, wbuffer, length, USB_TIMEOUT);
            #endif
        }
        else
        {
            #ifndef __unix__
            if(OutCtrEndPt)
                OutCtrEndPt->XferData(wbuffer, len);
            else
                len = 0;
            #else
                int actual = 0;
                libusb_bulk_transfer(dev_handle, 0x01, wbuffer, len, &actual, USB_TIMEOUT);
                len = actual;
            #endif
        }
		delete wbuffer;
    }
    else
        return 0;
    return len;
}

/**	@brief Reads data coming from the chip through USB port.
	@param buffer pointer to array where received data will be copied, array must be
	big enough to fit received data.
	@param length number of bytes to read from chip.
    @param timeout_ms timeout limit for operation in milliseconds
	@return number of bytes received.
*/
int ConnectionUSB::Read(unsigned char *buffer, const int length, int timeout_ms)
{
    long len = length;
    if(IsOpen())
    {
        if(m_hardwareName == HW_DIGIRED || m_hardwareName == HW_STREAMER)
        {
            #ifndef __unix__
            if(InCtrlEndPt3)
                InCtrlEndPt3->Read(buffer, len);
            else
                len = 0;
            #else
            len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN ,CTR_R_REQCODE ,CTR_R_VALUE, CTR_R_INDEX, buffer, len, USB_TIMEOUT);
            #endif
        }
        else
        {
            #ifndef __unix__
            if(InCtrEndPt)
                InCtrEndPt->XferData(buffer, len);
            else
                len = 0;
            #else
                int actual = 0;
                libusb_bulk_transfer(dev_handle, 0x81, buffer, len, &actual, USB_TIMEOUT);
                len = actual;
            #endif
        }
    }
    return len;
}

#ifdef __unix__
/**	@brief Function for handling libusb callbacks
*/
void callback_libusbtransfer(libusb_transfer *trans)
{
	USBTransferContext *context = reinterpret_cast<USBTransferContext*>(trans->user_data);
	switch(trans->status)
	{
    case LIBUSB_TRANSFER_CANCELLED:
        printf("Transfer canceled\n" );
        context->bytesXfered = trans->actual_length;
        context->done = true;
        context->used = false;
        context->reset();
        break;
    case LIBUSB_TRANSFER_COMPLETED:
        if(trans->actual_length == context->bytesExpected)
		{
			context->bytesXfered = trans->actual_length;
			context->done = true;
		}
		//printf("Transfer complete %i\n", trans->actual_length);
        break;
    case LIBUSB_TRANSFER_ERROR:
        printf("TRANSFER ERRRO\n");
        break;
    case LIBUSB_TRANSFER_TIMED_OUT:
        printf("transfer timed out\n");

        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        printf("transfer overflow\n");

        break;
    case LIBUSB_TRANSFER_STALL:
        printf("transfer stalled\n");
        break;
	}
    context->mPacketProcessed.notify_one();
}
#endif

/** @brief Finds all chips connected to usb ports
    @return number of devices found
*/
int ConnectionUSB::RefreshDeviceList()
{
    #ifndef __unix__
    USBDevicePrimary->Close();
    currentDeviceIndex = -1;
    m_deviceNames.clear();
    string name;
    if (USBDevicePrimary->DeviceCount())
    {
        for (int i=0; i<USBDevicePrimary->DeviceCount(); ++i)
        {
            Open(i);
            name = DeviceName();
            m_deviceNames.push_back(name);
        }
        currentDeviceIndex = -1;
    }
    #else
    m_dev_pid_vid.clear();
    m_deviceNames.clear();
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);
    if(usbDeviceCount > 0)
    {
        libusb_device_descriptor desc;
        for(int i=0; i<usbDeviceCount; ++i)
        {
            int r = libusb_get_device_descriptor(devs[i], &desc);
            if(r<0)
                printf("failed to get device description\n");
            int pid = desc.idProduct;
            int vid = desc.idVendor;

            if( vid == 1204)
            {
                if(pid == 34323)
                {
                    m_hardwareName = HW_DIGIGREEN;
                    m_deviceNames.push_back("DigiGreen");
                    m_dev_pid_vid.push_back( pair<int,int>(pid,vid));
                }
                else if(pid == 241)
                {
                    m_hardwareName = HW_DIGIRED;
                    libusb_device_handle *tempDev_handle;
                    tempDev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
                    if(libusb_kernel_driver_active(tempDev_handle, 0) == 1)   //find out if kernel driver is attached
                    {
                        if(libusb_detach_kernel_driver(tempDev_handle, 0) == 0) //detach it
                            printf("Kernel Driver Detached!\n");
                    }
                    if(libusb_claim_interface(tempDev_handle, 0) < 0) //claim interface 0 (the first) of device
                    {
                        printf("Cannot Claim Interface\n");
                    }

                    string fullName;
                    //check operating speed
                    int speed = libusb_get_device_speed(devs[i]);
                    if(speed == LIBUSB_SPEED_HIGH)
                        fullName = "USB 2.0";
                    else if(speed == LIBUSB_SPEED_SUPER)
                        fullName = "USB 3.0";
                    else
                        fullName = "USB";
                    fullName += " (";
                    //read device name
                    char data[255];
                    memset(data, 0, 255);
                    int st = libusb_get_string_descriptor_ascii(tempDev_handle, 2, (unsigned char*)data, 255);
                    if(strlen(data) > 0)
                        fullName += data;
                    fullName += ")";
                    libusb_close(tempDev_handle);

                    m_deviceNames.push_back(fullName);
                    m_dev_pid_vid.push_back( pair<int,int>(pid,vid));
                }
            }
        }
    }
    else
    {
        libusb_free_device_list(devs, 1);
        return 0;
    }
    #endif
    return m_deviceNames.size();
}

void ConnectionUSB::ClearComm()
{
}

/**	@return name of currently opened device as string.
*/
string ConnectionUSB::DeviceName()
{
#ifndef __unix__
	string name;
	char tempName[USB_STRING_MAXLEN];
	//memcpy(tempName, USBDevicePrimary->FriendlyName, USB_STRING_MAXLEN);
    //name = tempName;

	for (int i = 0; i < USB_STRING_MAXLEN; ++i)
		tempName[i] = USBDevicePrimary->DeviceName[i];
    if (USBDevicePrimary->bSuperSpeed == true)
        name = "USB 3.0";
    else if (USBDevicePrimary->bHighSpeed == true)
        name = "USB 2.0";
    else
        name = "USB";
    name += " (";
	name += tempName;
	name += ")";
    return name;
#else
    if(dev_handle != 0)
    {
        char data[255];
        int st = libusb_get_string_descriptor_ascii(dev_handle, 2, (unsigned char*)data, 255);
        return string(data);
    }
    return "no name";
#endif
}

/**
	@brief Starts asynchronous data reading from board
	@param *buffer buffer where to store received data
	@param length number of bytes to read
	@return handle of transfer context
*/
int ConnectionUSB::BeginDataReading(char *buffer, long length)
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
        return -1;
    contexts[i].used = true;
    #ifndef __unix__
    if(InEndPt)
        contexts[i].context = InEndPt->BeginDataXfer((unsigned char*)buffer, length, contexts[i].inOvLap);
	return i;
    #else
    unsigned int Timeout = 1000;
    libusb_transfer *tr = contexts[i].transfer;
	libusb_fill_bulk_transfer(tr, dev_handle, 0x81, (unsigned char*)buffer, length, callback_libusbtransfer, &contexts[i], Timeout);
	contexts[i].done = false;
	contexts[i].bytesXfered = 0;
	contexts[i].bytesExpected = length;
    int status = libusb_submit_transfer(tr);
    int actual = 0;
    //int status = libusb_bulk_transfer(dev_handle, 0x81, (unsigned char*)buffer, length, &actual, USB_TIMEOUT);
    if(status != 0)
        printf("ERROR BEGIN DATA TRANSFER %s\n", libusb_error_name(status));
    #endif
    return i;
}

/**
	@brief Waits for asynchronous data reception
	@param contextHandle handle of which context data to wait
	@param timeout_ms number of miliseconds to wait
	@return 1-data received, 0-data not received
*/
int ConnectionUSB::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    if( contexts[contextHandle].used == true && contextHandle >= 0)
    {
    int status = 0;
    #ifndef __unix__
	if(InEndPt)
        status = InEndPt->WaitForXfer(contexts[contextHandle].inOvLap, timeout_ms);
	return status;
    #else
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    while(contexts[contextHandle].done == false && std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms)
	{
	    struct timeval tv;
	    tv.tv_sec = 1;
	    tv.tv_usec = 0;
		//if(libusb_handle_events(ctx) != 0)
		if(libusb_handle_events_timeout_completed(ctx, &tv, NULL) != 0)
            printf("error libusb_handle_events %i\n", status);
		t2 = chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
    std::unique_lock<std::mutex> lck(contexts[contextHandle].m_lock);
    while(contexts[contextHandle].done == false) //is changed in libusb callback
    {
        if(contexts[contextHandle].mPacketProcessed.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
            return 0;
    }
	return contexts[contextHandle].done == true;
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
int ConnectionUSB::FinishDataReading(char *buffer, long &length, int contextHandle)
{
    if( contexts[contextHandle].used == true && contextHandle >= 0)
    {
    #ifndef __unix__
    int status = 0;
    if(InEndPt)
        status = InEndPt->FinishDataXfer((unsigned char*)buffer, length, contexts[contextHandle].inOvLap, contexts[contextHandle].context);
    contexts[contextHandle].used = false;
    contexts[contextHandle].reset();
    return length;
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

int ConnectionUSB::ReadDataBlocking(char *buffer, long &length, int timeout_ms)
{
#ifndef __unix__
    return InEndPt->XferData((unsigned char*)buffer, length);
#else
    return 0;
#endif
}


/**
	@brief Aborts reading operations
*/
void ConnectionUSB::AbortReading()
{
#ifndef __unix__
	InEndPt->Abort();
#else
    for(int i=0; i<USB_MAX_CONTEXTS; ++i)
    {
        libusb_cancel_transfer( contexts[i].transfer );
    }
#endif
}

/**
	@brief Starts asynchronous data Sending to board
	@param *buffer buffer to send
	@param length number of bytes to send
	@return handle of transfer context
*/
int ConnectionUSB::BeginDataSending(const char *buffer, long length)
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
    if(OutEndPt)
        contextsToSend[i].context = OutEndPt->BeginDataXfer((unsigned char*)buffer, length, contextsToSend[i].inOvLap);
	return i;
    #else
    unsigned int Timeout = 1000;
    libusb_transfer *tr = contextsToSend[i].transfer;
    //libusb_set_iso_packet_lengths(contexts[i].transfer, 512*64);
	libusb_fill_bulk_transfer(tr, dev_handle, 0x1, (unsigned char*)buffer, length, callback_libusbtransfer, &contextsToSend[i], Timeout);
	contextsToSend[i].done = false;
	contextsToSend[i].bytesXfered = 0;
	contextsToSend[i].bytesExpected = length;
    libusb_submit_transfer(tr);
    #endif
    return i;
}

/**
	@brief Waits for asynchronous data sending
	@param contextHandle handle of which context data to wait
	@param timeout_ms number of miliseconds to wait
	@return 1-data received, 0-data not received
*/
int ConnectionUSB::WaitForSending(int contextHandle, unsigned int timeout_ms)
{
    if( contextsToSend[contextHandle].used == true )
    {
    #ifndef __unix__
	int status = 0;
	if(OutEndPt)
        status = OutEndPt->WaitForXfer(contextsToSend[contextHandle].inOvLap, timeout_ms);
	return status;
    #else
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    while(contextsToSend[contextHandle].done == false && std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms)
	{
	    struct timeval tv;
	    tv.tv_sec = 1;
	    tv.tv_usec = 0;
        int status = libusb_handle_events_timeout_completed(ctx, &tv, NULL);
        if(status != 0)
            printf("error libusb_handle_events %i\n", status);
        t2 = chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	std::unique_lock<std::mutex> lck(contextsToSend[contextHandle].m_lock);
    while(contextsToSend[contextHandle].done == false) //is changed in libusb callback
    {
        if(contextsToSend[contextHandle].mPacketProcessed.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
            return 0;
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
int ConnectionUSB::FinishDataSending(const char *buffer, long &length, int contextHandle)
{
    if( contextsToSend[contextHandle].used == true)
    {
    #ifndef __unix__
	if(OutEndPt)
        OutEndPt->FinishDataXfer((unsigned char*)buffer, length, contextsToSend[contextHandle].inOvLap, contextsToSend[contextHandle].context);
    contextsToSend[contextHandle].used = false;
    contextsToSend[contextHandle].reset();
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
void ConnectionUSB::AbortSending()
{
#ifndef __unix__
	OutEndPt->Abort();
#else
    for (int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        libusb_cancel_transfer(contextsToSend[i].transfer);
    }
    for(int i=0; i<USB_MAX_CONTEXTS; ++i)
    {
        contextsToSend[i].used = false;
        contextsToSend[i].reset();
    }
#endif
}

int ConnectionUSB::GetOpenedIndex()
{
    return currentDeviceIndex;
}

/** @brief Returns found devices names
    @return vector of device names
*/
vector<string> ConnectionUSB::GetDeviceNames()
{
    return m_deviceNames;
}
