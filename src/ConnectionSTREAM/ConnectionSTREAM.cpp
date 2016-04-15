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

#include <thread>
#include <chrono>

using namespace std;

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

using namespace lime;

/**	@brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionSTREAM::ConnectionSTREAM(void *arg, const unsigned index, const int vid, const int pid)
{
    m_hardwareName = "";
    isConnected = false;
#ifndef __unix__
    USBDevicePrimary = (CCyUSBDevice *)arg;
    OutCtrEndPt = NULL;
    InCtrEndPt = NULL;
	InCtrlEndPt3 = NULL;
	OutCtrlEndPt3 = NULL;
#else
    dev_handle = 0;
    devs = 0;
    ctx = (libusb_context *)arg;
#endif
    if (this->Open(index, vid, pid) != 0)
        std::cerr << GetLastErrorMessage() << std::endl;

    //must configure synthesizer before using LimeSDR
    DeviceInfo info = this->GetDeviceInfo();
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
    mStreamService.reset();
    Close();
}

/**	@brief Tries to open connected USB device and find communication endpoints.
	@return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionSTREAM::Open(const unsigned index, const int vid, const int pid)
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
			return 0;
		} //successfully opened device
	} //if has devices
    return ReportError("No matching devices found");
#else

    if( vid == 1204)
    {
        if(pid == 34323)
        {
            m_hardwareName = HW_DIGIGREEN;
        }
        else if(pid == 241)
        {
            m_hardwareName = HW_DIGIRED;
        }
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);

    if(dev_handle == nullptr)
        return ReportError("libusb_open failed");
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
        return ReportError("Cannot claim interface - %s", libusb_strerror(libusb_error(r)));
    }
    printf("Claimed Interface\n");
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
int ConnectionSTREAM::Read(unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
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
int ConnectionSTREAM::BeginDataReading(char *buffer, long length)
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
    int status = 0;
    #ifndef __unix__
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
int ConnectionSTREAM::FinishDataReading(char *buffer, long &length, int contextHandle)
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

int ConnectionSTREAM::ReadDataBlocking(char *buffer, long &length, int timeout_ms)
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
int ConnectionSTREAM::BeginDataSending(const char *buffer, long length)
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
int ConnectionSTREAM::FinishDataSending(const char *buffer, long &length, int contextHandle)
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

/** @brief Configures Stream board FPGA clocks to Limelight interface
@param tx Rx/Tx selection
@param InterfaceClk_Hz Limelight interface frequency
@param phaseShift_deg IQ phase shift in degrees
@return 0-success, other-failure
*/
int ConnectionSTREAM::ConfigureFPGA_PLL(unsigned int pllIndex, const double interfaceClk_Hz, const double phaseShift_deg)
{
    const uint16_t baseAddr = 0x0020;
    if(IsOpen() == false)
        return ReportError(ENODEV, "ConnectionSTREAM: configure FPGA PLL, device not connected");

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
        uint16_t phase_reg_select = (phaseShift_deg / phaseStep_deg)+0.5;
        const float actualPhaseShift_deg = 360 * inputClock_Hz / (1 / (phase_reg_select * oversampleClock_ns*1e-9));
#ifndef NDEBUG
        printf("reg value : %i\n", phase_reg_select);
        printf("input clock: %f\n", inputClock_Hz);
        printf("phase : %.2f/%.2f\n", phaseShift_deg, actualPhaseShift_deg);
#endif
        if(WriteRegister(phase_reg_sel_addr, phase_reg_select) != 0)
            return ReportError(EIO, "ConnectionSTREAM: configure FPGA PLL, failed to write registers");
        const uint16_t LOAD_PH_REG = 1 << 10;
        WriteRegister(0x0006, drct_clk_ctrl_0006 | LOAD_PH_REG);
        WriteRegister(0x0006, drct_clk_ctrl_0006);
        return 0;
    }

    //if interface frequency >= 5MHz, configure PLLs
    WriteRegister(0x0005, drct_clk_ctrl_0005 & ~(1 << pllIndex));

    //select FPGA index
    pllIndex = pllIndex & 0x1F;
    uint16_t reg3val = 0;
    if(ReadRegister(0x0003, reg3val) != 0)
        return ReportError(ENODEV, "ConnectionSTREAM: configure FPGA PLL, failed to read register");
    reg3val &= 0x1F << 3; //clear PLL index
    reg3val &= ~1; //clear PLLCFG_START
    reg3val &= ~(1 << 2); //clear PLL reset
    reg3val &= ~(1 << 13); //clear PHCFG_UpDn
    reg3val |= pllIndex << 3;

    vector<uint32_t> addrs;
    vector<uint32_t> values;
    addrs.push_back(baseAddr + 0x0003);
    values.push_back(reg3val); //PLL_IND
    addrs.push_back(baseAddr + 0x0003);
    values.push_back(reg3val | 0x4); //PLLRST_START
    addrs.push_back(baseAddr + 0x0003);
    values.push_back(reg3val & ~0x4);
    WriteRegisters(addrs.data(), values.data(), values.size());

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
    if(interfaceClk_Hz*M/1e6 > vcoLimits_MHz[0] && interfaceClk_Hz*M/1e6 < vcoLimits_MHz[1])
    {
        //bypass N
        addrs.push_back(baseAddr + 0x0006);
        values.push_back(0x0001 | (M % 2 ? 0x8 : 0));

        addrs.push_back(baseAddr + 0x0007);
        values.push_back(0x5550 | (C % 2 ? 0xA : 0)); //bypass c7-c1
        addrs.push_back(baseAddr + 0x0008);
        values.push_back(0x5555); //bypass c15-c8

        addrs.push_back(baseAddr + 0x000A);
        values.push_back(0x0101); //N_high_cnt, N_low_cnt
        addrs.push_back(baseAddr + 0x000B);
        values.push_back(chigh << 8 | clow); //M_high_cnt, M_low_cnt

        for(int i = 0; i <= 1; ++i)
        {
            addrs.push_back(baseAddr + 0x000E + i);
            values.push_back(chigh << 8 | clow); // ci_high_cnt, ci_low_cnt
        }

        float Fstep_us = 1 / (8 * fOut_MHz*C);
        float Fstep_deg = (360 * Fstep_us) / (1 / fOut_MHz);
        short nSteps = phaseShift_deg / Fstep_deg;

        addrs.push_back(baseAddr + 0x0004);
        values.push_back(nSteps);

        addrs.push_back(baseAddr + 0x0003);
        int cnt_ind = 0x3 & 0x1F;
        reg3val = reg3val | (1 << 13) | (cnt_ind << 8);
        values.push_back(reg3val); //PHCFG_UpDn, CNT_IND

        addrs.push_back(baseAddr + 0x0003);
        values.push_back(reg3val | 0x1); //PLLCFG_START
        addrs.push_back(baseAddr + 0x0003);
        values.push_back(reg3val & ~0x1); //PLLCFG_START

        addrs.push_back(baseAddr + 0x0003);
        values.push_back(reg3val | 0x2); //PHCFG_START
        addrs.push_back(baseAddr + 0x0003);
        values.push_back(reg3val & ~0x2); //PHCFG_START

        if(WriteRegisters(addrs.data(), values.data(), values.size()) != 0  )
            ReportError(ERANGE, "ConnectionSTREAM: configure FPGA PLL, failed to write registers");
        return 0;
    }
    return ReportError(ERANGE, "ConnectionSTREAM: configure FPGA PLL, desired frequency out of range");
}
