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

#include <thread>
#include <chrono>

using namespace std;

#define USB_TIMEOUT 1000

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
    mExpectedSampleRate = 0;
    generateData.store(false);
    rxRunning.store(false);
    txRunning.store(false);
    isConnected = false;
    terminateRx.store(false);
    terminateTx.store(false);
    rxDataRate_Bps.store(0);
    txDataRate_Bps.store(0);
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
        delete i;
    for(auto i : mRxStreams)
        delete i;
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

    m_hardwareDesc = USBDevicePrimary->Product;
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
        return ReportError("ConnectionSTREAM: libusb_open failed");
    if(libusb_kernel_driver_active(dev_handle, 0) == 1)   //find out if kernel driver is attached
    {
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    }
    int r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
    if(r < 0)
        return ReportError("ConnectionSTREAM: Cannot claim interface - %s", libusb_strerror(libusb_error(r)));
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
        len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR,CTR_W_REQCODE ,CTR_W_VALUE, CTR_W_INDEX, wbuffer, length, USB_TIMEOUT);
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
    len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN ,CTR_R_REQCODE ,CTR_R_VALUE, CTR_R_INDEX, buffer, len, USB_TIMEOUT);
#endif
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
        printf("Wait Rx\n");
        contexts[contextHandle].cv.wait_for(lck, chrono::milliseconds(900));
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
        printf("Wait Tx\n");
        contextsToSend[contextHandle].cv.wait_for(lck, chrono::milliseconds(900));
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

int ConnectionSTREAM::UpdateThreads()
{
    bool needTx = false;
    bool needRx = false;

    //check which threads are needed
    for(auto i : mRxStreams)
        if(i->IsActive())
        {
            needRx = true;
            break;
        }
    for(auto i : mTxStreams)
        if(i->IsActive())
        {
            needTx = true;
            break;
        }

    //stop threads if not needed
    if(not needTx and txRunning.load())
    {
        terminateTx.store(true);
        txThread.join();
        txRunning.store(false);
    }
    if(not needRx and rxRunning.load())
    {
        terminateRx.store(true);
        rxThread.join();
        rxRunning.store(false);
    }

    //configure FPGA on first start, or disable FPGA when not streaming
    if((needTx or needRx) && (not rxRunning.load() and not txRunning.load()))
    {
        //enable FPGA streaming
        fpga::StopStreaming(this);
        fpga::ResetTimestamp(this);
        //USB FIFO reset
        // TODO : USB FIFO reset command for IConnection
        LMS64CProtocol::GenericPacket ctrPkt;
        ctrPkt.cmd = CMD_USB_FIFO_RST;
        ctrPkt.outBuffer.push_back(0x01);
        TransferPacket(ctrPkt);
        ctrPkt.outBuffer[0] = 0x00;
        TransferPacket(ctrPkt);

        //enable MIMO mode, 12 bit compressed values
        StreamConfig config;
        config.linkFormat = StreamConfig::STREAM_12_BIT_COMPRESSED;
        uint16_t smpl_width; // 0-16 bit, 1-14 bit, 2-12 bit
        if(config.linkFormat == StreamConfig::STREAM_12_BIT_IN_16)
            smpl_width = 0x0;
        else if(config.linkFormat == StreamConfig::STREAM_12_BIT_COMPRESSED)
            smpl_width = 0x2;
        else
            smpl_width = 0x2;
        WriteRegister(0x0008, 0x0100 | smpl_width);

        uint16_t channelEnables = 0;
        for(uint8_t i=0; i<mRxStreams.size(); ++i)
            channelEnables |= (1 << mRxStreams[i]->config.channelID);
        for(uint8_t i=0; i<mTxStreams.size(); ++i)
            channelEnables |= (1 << mTxStreams[i]->config.channelID);
        WriteRegister(0x0007, channelEnables);
        fpga::StartStreaming(this);
    }
    else if(not needTx and not needRx)
    {
        //disable FPGA streaming
        fpga::StopStreaming(this);
    }

    //FPGA should be configured and activated, start needed threads
    if(needRx and not rxRunning.load())
    {
        ThreadData args;
        args.terminate = &terminateRx;
        args.dataPort = this;
        args.dataRate_Bps = &rxDataRate_Bps;
        args.channels = mRxStreams;
        args.generateData = &generateData;
        args.safeToConfigInterface = &safeToConfigInterface;

        rxRunning.store(true);
        terminateRx.store(false);
        rxThread = std::thread(ReceivePacketsLoop, args);
    }
    if(needTx and not txRunning.load())
    {
        ThreadData args;
        args.terminate = &terminateTx;
        args.dataPort = this;
        args.dataRate_Bps = &txDataRate_Bps;
        args.channels = mTxStreams;
        args.generateData = &generateData;
        args.safeToConfigInterface = &safeToConfigInterface;

        txRunning.store(true);
        terminateTx.store(false);
        txThread = std::thread(TransmitPacketsLoop, args);
    }
    return 0;
}
