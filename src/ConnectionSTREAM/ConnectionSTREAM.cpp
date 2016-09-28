/**
    @file ConnectionSTREAM.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#include "ConnectionSTREAM.h"
#include "ErrorReporting.h"
#include <cstring>
#include <iostream>
#include "Si5351C.h"
#include "FPGA_common.h"
#include "LMS7002M.h"
#include <ciso646>

#include <thread>
#include <chrono>

using namespace std;

#define CTR_W_REQCODE 0xC1
#define CTR_W_VALUE 0x0000
#define CTR_W_INDEX 0x0000

#define CTR_R_REQCODE 0xC0
#define CTR_R_VALUE 0x0000
#define CTR_R_INDEX 0x0000

using namespace lime;

/**	@brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionSTREAM::ConnectionSTREAM(void *arg, const unsigned index, const int vid, const int pid)
{
    RxLoopFunction = bind(&ConnectionSTREAM::ReceivePacketsLoop, this, std::placeholders::_1);
    TxLoopFunction = bind(&ConnectionSTREAM::TransmitPacketsLoop, this, std::placeholders::_1);

    isConnected = false;
#ifndef __unix__
    USBDevicePrimary = (CCyUSBDevice *)arg;
	InCtrlEndPt3 = NULL;
	OutCtrlEndPt3 = NULL;
#else
    dev_handle = 0;
    devs = 0;
    ctx = (libusb_context *)arg;
#endif
    if (this->Open(index, vid, pid) != 0)
        std::cerr << GetLastErrorMessage() << std::endl;

    DeviceInfo info = this->GetDeviceInfo();


    //expected version numbers based on HW number
    std::string expectedFirmware, expectedGateware = "1";
    if (info.hardwareVersion == "1") expectedFirmware = "5";
    else if (info.hardwareVersion == "2") expectedFirmware = "0";
    else std::cerr << "Unknown hardware version " << info.hardwareVersion << std::endl;

    //check and warn about firmware mismatch problems
    if (info.firmwareVersion != expectedFirmware) std::cerr << std::endl
        << "########################################################" << std::endl
        << "##   !!!  Warning: firmware version mismatch  !!!" << std::endl
        << "## Expected firmware version " << expectedFirmware << ", but found version " << info.firmwareVersion << std::endl
        << "## Follow the FW and FPGA upgrade instructions:" << std::endl
        << "## http://wiki.myriadrf.org/Lime_Suite#Flashing_images" << std::endl
        << "########################################################" << std::endl
        << std::endl;

    //check and warn about gateware mismatch problems
    if (info.gatewareVersion != expectedGateware) std::cerr << std::endl
        << "########################################################" << std::endl
        << "##   !!!  Warning: gateware version mismatch  !!!" << std::endl
        << "## Expected gateware version " << expectedGateware << ", but found version " << info.gatewareVersion << std::endl
        << "## Follow the FW and FPGA upgrade instructions:" << std::endl
        << "## http://wiki.myriadrf.org/Lime_Suite#Flashing_images" << std::endl
        << "########################################################" << std::endl
        << std::endl;

    //must configure synthesizer before using LimeSDR
    if (info.deviceName == GetDeviceName(LMS_DEV_LIMESDR))
    {
        std::shared_ptr<Si5351C> si5351module(new Si5351C());
        si5351module->Initialize(this);
        si5351module->SetPLL(0, 25000000, 0);
        si5351module->SetPLL(1, 25000000, 0);
        si5351module->SetClock(0, 27000000, true, false);
        si5351module->SetClock(1, 27000000, true, false);
        for (int i = 2; i < 8; ++i)
            si5351module->SetClock(i, 27000000, false, false);
        Si5351C::Status status = si5351module->ConfigureClocks();
        if (status != Si5351C::SUCCESS)
        {
            std::cerr << "Warning: Failed to configure Si5351C" << std::endl;
            return;
        }
        status = si5351module->UploadConfiguration();
        if (status != Si5351C::SUCCESS)
            std::cerr << "Warning: Failed to upload Si5351C configuration" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //some settle time
    }
}

/**	@brief Closes connection to chip and deallocates used memory.
*/
ConnectionSTREAM::~ConnectionSTREAM()
{
    for(auto i : mTxStreams)
        ControlStream((size_t)i, false);
    for(auto i : mRxStreams)
        ControlStream((size_t)i, false);
    for(auto i : mTxStreams)
        CloseStream((size_t)i);
    for(auto i : mRxStreams)
        CloseStream((size_t)i);
    UpdateThreads();
    Close();
}

/**	@brief Tries to open connected USB device and find communication endpoints.
	@return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionSTREAM::Open(const unsigned index, const int vid, const int pid)
{
#ifndef __unix__
    if(index > USBDevicePrimary->DeviceCount())
        return ReportError(ERANGE, "ConnectionSTREAM: Device index out of range");

    if(USBDevicePrimary->Open(index) == false)
        return ReportError(-1, "ConnectionSTREAM: Failed to open device");

    unsigned int pos;

    if (InCtrlEndPt3)
    {
        delete InCtrlEndPt3;
        InCtrlEndPt3 = nullptr;
    }
    InCtrlEndPt3 = new CCyControlEndPoint(*USBDevicePrimary->ControlEndPt);

    if (OutCtrlEndPt3)
    {
        delete OutCtrlEndPt3;
        OutCtrlEndPt3 = nullptr;
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
    return 0;
#else
    dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
    if(dev_handle == nullptr)
        return ReportError(-1, "ConnectionSTREAM: libusb_open failed");
    if(libusb_kernel_driver_active(dev_handle, 0) == 1)   //find out if kernel driver is attached
    {
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    }
    int r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
    if(r < 0)
        return ReportError(-1, "ConnectionSTREAM: Cannot claim interface - %s", libusb_strerror(libusb_error(r)));
    isConnected = true;
    return 0;
#endif
}

/**	@brief Closes communication to device.
*/
void ConnectionSTREAM::Close()
{
    #ifndef __unix__
	USBDevicePrimary->Close();
	InEndPt = nullptr;
	OutEndPt = nullptr;
	if (InCtrlEndPt3)
	{
		delete InCtrlEndPt3;
		InCtrlEndPt3 = nullptr;
	}
	if (OutCtrlEndPt3)
	{
		delete OutCtrlEndPt3;
		OutCtrlEndPt3 = nullptr;
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
bool ConnectionSTREAM::IsOpen()
{
    #ifndef __unix__
    return USBDevicePrimary.IsOpen() && isConnected;
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
int ConnectionSTREAM::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = length;
    if(IsOpen() == false)
        return 0;

    unsigned char* wbuffer = new unsigned char[length];
    memcpy(wbuffer, buffer, length);

    #ifndef __unix__
    if(OutCtrlEndPt3)
        OutCtrlEndPt3->Write(wbuffer, len);
    else
        len = 0;
    #else
        len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR,CTR_W_REQCODE ,CTR_W_VALUE, CTR_W_INDEX, wbuffer, length, timeout_ms);
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
int ConnectionSTREAM::Read(unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = length;
    if(IsOpen() == false)
        return 0;

#ifndef __unix__
    if(InCtrlEndPt3)
        InCtrlEndPt3->Read(buffer, len);
    else
        len = 0;
#else
    len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN ,CTR_R_REQCODE ,CTR_R_VALUE, CTR_R_INDEX, buffer, len, timeout_ms);
#endif
    return len;
}

#ifdef __unix__
/**	@brief Function for handling libusb callbacks
*/
void callback_libusbtransfer(libusb_transfer *trans)
{
	USBTransferContext *context = reinterpret_cast<USBTransferContext*>(trans->user_data);
	std::unique_lock<std::mutex> lck(context->transferLock);
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
	lck.unlock();
	context->cv.notify_one();
}
#endif

/**
	@brief Starts asynchronous data reading from board
	@param *buffer buffer where to store received data
	@param length number of bytes to read
	@return handle of transfer context
*/
int ConnectionSTREAM::BeginDataReading(char *buffer, uint32_t length)
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
    if(InEndPt)
        contexts[i].context = InEndPt->BeginDataXfer((unsigned char*)buffer, length, contexts[i].inOvLap);
	return i;
    #else
    unsigned int Timeout = 500;
    libusb_transfer *tr = contexts[i].transfer;
	libusb_fill_bulk_transfer(tr, dev_handle, 0x81, (unsigned char*)buffer, length, callback_libusbtransfer, &contexts[i], Timeout);
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
    #endif
    return i;
}

/**
	@brief Waits for asynchronous data reception
	@param contextHandle handle of which context data to wait
	@param timeout_ms number of miliseconds to wait
	@return 1-data received, 0-data not received
*/
int ConnectionSTREAM::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
    #ifndef __unix__
    int status = 0;
	if(InEndPt)
        status = InEndPt->WaitForXfer(contexts[contextHandle].inOvLap, timeout_ms);
	return status;
    #else
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    std::unique_lock<std::mutex> lck(contexts[contextHandle].transferLock);
    while(contexts[contextHandle].done.load() == false && std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms)
    {
        //blocking not to waste CPU
        contexts[contextHandle].cv.wait_for(lck, chrono::milliseconds(timeout_ms));
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
	@param length number of bytes to read
	@param contextHandle handle of which context to finish
	@return negative values failure, positive number of bytes received
*/
int ConnectionSTREAM::FinishDataReading(char *buffer, uint32_t length, int contextHandle)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
    #ifndef __unix__
    int status = 0;
    long len = length;
    if(InEndPt)
        status = InEndPt->FinishDataXfer((unsigned char*)buffer, len, contexts[contextHandle].inOvLap, contexts[contextHandle].context);
    contexts[contextHandle].used = false;
    contexts[contextHandle].reset();
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

/**
	@brief Aborts reading operations
*/
void ConnectionSTREAM::AbortReading()
{
#ifndef __unix__
	InEndPt->Abort();
#else
    for(int i=0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contexts[i].used)
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
int ConnectionSTREAM::BeginDataSending(const char *buffer, uint32_t length)
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
    unsigned int Timeout = 500;
    libusb_transfer *tr = contextsToSend[i].transfer;
	libusb_fill_bulk_transfer(tr, dev_handle, 0x1, (unsigned char*)buffer, length, callback_libusbtransfer, &contextsToSend[i], Timeout);
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
    #endif
    return i;
}

/**
	@brief Waits for asynchronous data sending
	@param contextHandle handle of which context data to wait
	@param timeout_ms number of miliseconds to wait
	@return 1-data received, 0-data not received
*/
int ConnectionSTREAM::WaitForSending(int contextHandle, unsigned int timeout_ms)
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

    std::unique_lock<std::mutex> lck(contextsToSend[contextHandle].transferLock);
    while(contextsToSend[contextHandle].done.load() == false && std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeout_ms)
    {
        //blocking not to waste CPU
        contextsToSend[contextHandle].cv.wait_for(lck, chrono::milliseconds(timeout_ms));
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
int ConnectionSTREAM::FinishDataSending(const char *buffer, uint32_t length, int contextHandle)
{
    if( contextsToSend[contextHandle].used == true)
    {
#ifndef __unix__
    long len = length;
	if(OutEndPt)
        OutEndPt->FinishDataXfer((unsigned char*)buffer, len, contextsToSend[contextHandle].inOvLap, contextsToSend[contextHandle].context);
    contextsToSend[contextHandle].used = false;
    contextsToSend[contextHandle].reset();
    return len;
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
void ConnectionSTREAM::AbortSending()
{
#ifndef __unix__
	OutEndPt->Abort();
#else
    for (int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contextsToSend[i].used)
            libusb_cancel_transfer(contextsToSend[i].transfer);
    }
#endif
}

int ConnectionSTREAM::ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int device, ProgrammingCallback callback)
{
    if (device == LMS64CProtocol::FX3 && programmingMode == 1)
    {
#ifdef __unix__
        libusb_device_descriptor desc;
        int ret = libusb_get_device_descriptor(libusb_get_device(dev_handle), &desc);
        if(ret<0)
            printf("failed to get device description\n");
        else if (desc.idProduct == 243)
#else
		if (USBDevicePrimary.ProductID == 243)
#endif
        {
#ifdef __unix__
            return fx3_usbboot_download((unsigned char*)buffer,length);
#else
            char* filename = "fx3fw_image_tmp.img";
            int ret = 0;
            std::ofstream myfile(filename, ios::out | ios::binary | ios::trunc);
            if (!myfile.is_open())
            {
                ReportError("FX3 FW:Unable to create temporary file");
                return -1;
            }
            myfile.write(buffer,length);
            if (myfile.fail())
            {
                ReportError("FX3 FW:Unable to write to temporary file");
                ret = -1;
            }
            myfile.close();

            if (ret != -1)
            {
                if ((ret=USBDevicePrimary.DownloadFw(filename, FX3_FWDWNLOAD_MEDIA_TYPE::RAM))!=0)
                    ReportError("FX3: Failed to upload FW to RAM");
            }

            std::remove(filename);
            return ret;
#endif
        }
		else
		{
			ReportError("FX3 bootloader NOT detected");
			return -1;
		}
    }
    return LMS64CProtocol::ProgramWrite(buffer,length,programmingMode,device,callback);
}

#ifdef __unix__

#define MAX_FWIMG_SIZE  (512 * 1024)		// Maximum size of the firmware binary.
#define GET_LSW(v)	((unsigned short)((v) & 0xFFFF))
#define GET_MSW(v)	((unsigned short)((v) >> 16))

#define VENDORCMD_TIMEOUT	(5000)		// Timeout for each vendor command is set to 5 seconds.


int ConnectionSTREAM::ram_write(unsigned char *buf, unsigned int ramAddress, int len)
{
    const unsigned MAX_WRITE_SIZE = (2 * 1024);		// Max. size of data that can be written through one vendor command.
	int r;
	int index = 0;
	int size;

	while ( len > 0 )
    {
		size = (len > MAX_WRITE_SIZE) ? MAX_WRITE_SIZE : len;
		r = libusb_control_transfer(dev_handle, 0x40, 0xA0, GET_LSW(ramAddress), GET_MSW(ramAddress),&buf[index], size, VENDORCMD_TIMEOUT);
		if ( r != size )
		{
			printf("Vendor write to FX3 RAM failed\n");
			return -1;
		}
		ramAddress += size;
		index      += size;
		len        -= size;
	}
	return 0;
}

int ConnectionSTREAM::fx3_usbboot_download(unsigned char *fwBuf, int filesize)
{
	unsigned int  *data_p;
	unsigned int i, checksum;
	unsigned int address, length;
	int r, index;

	if ( filesize > MAX_FWIMG_SIZE ) {
		ReportError("File size exceeds maximum firmware image size\n");
		return -2;
	}

	if ( strncmp((char *)fwBuf,"CY",2) ) {
		ReportError("Image does not have 'CY' at start. aborting\n");
		return -4;
	}

	if ( fwBuf[2] & 0x01 ) {
		ReportError("Image does not contain executable code\n");
		return -5;
	}

	if ( !(fwBuf[3] == 0xB0) ) {
		ReportError("Not a normal FW binary with checksum\n");
		return -6;
	}

	// Run through each section of code, and use vendor commands to download them to RAM.
	index    = 4;
	checksum = 0;
	while ( index < filesize )
	{
		data_p  = (unsigned int *)(fwBuf + index);
		length  = data_p[0];
		address = data_p[1];
		if (length != 0)
		{
			for (i = 0; i < length; i++)
				checksum += data_p[2 + i];
			r = ram_write(fwBuf + index + 8, address, length * 4);
			if (r != 0)
			{
				ReportError("Failed to download data to FX3 RAM\n");
				return -3;
			}
		}
		else
		{
			if (checksum != data_p[2]) {
				ReportError ("Checksum error in firmware binary\n");
				return -4;
			}

			r = libusb_control_transfer(dev_handle, 0x40, 0xA0, GET_LSW(address), GET_MSW(address), NULL,0, VENDORCMD_TIMEOUT);
			if ( r != 0 )
				printf("Ignored error in control transfer: %d\n", r);
			break;
		}
		index += (8 + length * 4);
	}

	return 0;
}
#endif
